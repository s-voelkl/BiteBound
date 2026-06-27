#include "src/network/time/TimeManager.h"
#include "src/network/wifi-connection/WifiManager.h"
#include "src/network/mqtt/MqttManager.h"
#include "src/network/json-builder/JsonBuilder.h"
#include "src/network/json-parser/CommandParser.h"
#include "src/network/shared/SharedStateData.h"
#include "src/network/shared/MutexLock.h"
#include "src/sensors/SensorManager.h"
#include "src/graphics/GraphicsManager.h"
#include "src/maze/MazeManager.h"
#include "config.h"

#include <AUnit.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include <freertos/FreeRTOS.h>

// Set to 1 to use mock sensor data instead of real hardware
#ifndef MOCK_SENSORS
#define MOCK_SENSORS 0
#endif

// Set to 1 to run AUnit tests; set 0 for main functionality.
// Can be overridden at compile time via -DRUN_TESTS=1 (used by CI).
#ifndef RUN_TESTS
#define RUN_TESTS 0
#endif

// Threading Synchronizations
SemaphoreHandle_t stateMutex = nullptr;
QueueHandle_t commandQueue = nullptr;
SharedStateData sharedState;

// Global Graphics Setup
Arduino_DataBus *bus = new Arduino_ESP32SPI(pin_lcd_dc, pin_lcd_cs, pin_lcd_sck, pin_lcd_mosi);
Arduino_GFX *gfx = new Arduino_ST7789(bus, pin_lcd_rst /* RST */,0 /* rotation */, 
  true /* IPS */, display_width, display_height, 0, 20, 0, 0);
GraphicsManager graphicsManager(display_width, display_height);

// Play space calculations
const int play_width = display_width;
const int play_height = display_height - ui_header_height;

// Global Maze Manager Setup
MazeManager mazeManager(play_width, play_height, default_wall_thickness_px);

// Memory allocation for the game board bitmap
uint8_t *gameBoard = nullptr;

// FreeRTOS Task on Core 0 to isolate network operations
void vNetworkTask(void *pvParameters);

// Helper function declarations
void processIncomingCommands();
void updateGameStep();

/**
 * @brief Main setup function for the MCU controller.
 * This function initializes the serial communication, connects to WiFi and MQTT.
 */
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 BiteBound Hardware starting...");

  // Initialize threading primitives
  // xSemaphoreCreateMutex(): Creates a mutex type semaphore and returns a handle to it. 
  // xQueueCreate(): Creates a new queue instance and returns a handle to it.
  stateMutex = xSemaphoreCreateMutex();
  commandQueue = xQueueCreate(cmd_queue_length, cmd_queue_item_size);
  initSharedState(sharedState); // from src/network/shared/SharedStateData.h for default values

#if RUN_TESTS
  // serial initialization delay for test output
  delay(3000);
  Serial.println("Starting AUnit tests...");

#else
  // Initialize Display and Graphics Manager
  if (!gfx->begin()) {
    Serial.println("Failed to initialize GFX display!");
  }
  pinMode(pin_lcd_bl, OUTPUT);
  digitalWrite(pin_lcd_bl, HIGH);
  graphicsManager.begin(gfx);

  // Connect to WiFi
  graphicsManager.drawLoadingScreen("Connecting WiFi...");
  wifiManager.connect();

  // Synchronize time (required for TLS certificate validation and timestamps).
  graphicsManager.drawLoadingScreen("Syncing Time...");
  timeManager.sync();

  // Seed standard library random engine with timing noise
  std::srand(micros()); 

  // Initialize sensors
  graphicsManager.drawLoadingScreen("Initializing Sensors...");
  sensorManager.begin();

  // Setup MQTT client
  graphicsManager.drawLoadingScreen("Starting MQTT...");
  mqttManager.begin();

  // TODO: Move game board init and maze gen!
  graphicsManager.drawLoadingScreen("Creating Maze...");
  // Allocate memory for the global gameBoard
  gameBoard = new uint8_t[play_width * play_height];

  // Generate maze
  if (gameBoard) {
    bool generated = mazeManager.generate(gameBoard);
    if (generated) {
      Serial.println("Procedural maze successfully generated.");
    } else {
      Serial.println("Maze generation failed.");
    }
  }

  // Create Network Task on Core 0
  // xTaskCreatePinnedToCore(): Creates a new task and pins it to a specific core.
  // Parameter: pvTaskCode, *constpcName, usStackDepth, *constpvParameters, uxPriority, *constpvCreatedTask, xCoreID
  // see: https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/system/freertos.html#functions
  xTaskCreatePinnedToCore(
      vNetworkTask,
      "NetworkTask",
      8192,
      NULL,
      1,
      NULL,
      core_network
  );

  // End of setup, ready to enter main loop
  graphicsManager.drawLoadingScreen("BiteBound is Ready!");
#endif
}

/**
 * @brief Main loop function for the MCU controller.
 * This function runs continuously after setup.
 */
void loop() {
#if RUN_TESTS
  // aunit::TestRunner::list(); // List all registered tests.
  aunit::TestRunner::setTimeout(30) ; // Set a timeout of 30 seconds for tests.
  aunit::TestRunner::run();
#else
  static TickType_t lastTickTime = xTaskGetTickCount();

  // 1. Core 1 Command Processing
  processIncomingCommands();

  // 2. Local Game Step
  updateGameStep();

  // Preserve 50Hz precision step constraints
  vTaskDelayUntil(&lastTickTime, pdMS_TO_TICKS(game_tick_rate_ms));
  #endif
}

/**
 * @brief Processes incoming command structures transmitted from Core 0.
 *
 * Checks the queue for raw decoded packets, acquires the execution lock,
 * and sets global states according to start, stop, or parameter modification messages.
 */
void processIncomingCommands() {
  CommandMsg commandMsg;

  if (xQueueReceive(commandQueue, &commandMsg, 0) == pdPASS) {
    MutexLock lock(stateMutex);

    if (lock.isLocked()) {
      if (commandMsg.type == CommandType::START) {
        strncpy(sharedState.playerName, commandMsg.playerName, sizeof(sharedState.playerName) - 1);

        sharedState.cookiesCount = commandMsg.cookiesCount;
        sharedState.wallThicknessPx = commandMsg.wallThicknessPx;

        sharedState.imuSensitivity = commandMsg.imuSensitivity;
        sharedState.bounceRestitution = commandMsg.bounceRestitution;
        sharedState.emaAlpha = commandMsg.emaAlpha;
        sharedState.deadzoneThreshold = commandMsg.deadzoneThreshold;

        sharedState.ballPosX = 0.0f;
        sharedState.ballPosY = 0.0f;
        sharedState.velocityX = 0.0f;
        sharedState.velocityY = 0.0f;
        sharedState.accX = 0.0f;
        sharedState.accY = 0.0f;

        sharedState.gameId = commandMsg.gameId;
        sharedState.cookiesCollected = 0;
        sharedState.cookiesRemaining = commandMsg.cookiesCount;
        sharedState.currentRound = 1;
        sharedState.elapsedTimeSec = 0.0f;
        
      } else if (commandMsg.type == CommandType::STOP) {
        sharedState.isRunning = false;

      } else if (commandMsg.type == CommandType::PARAM_CHANGE) {
        sharedState.imuSensitivity = commandMsg.imuSensitivity;
        sharedState.bounceRestitution = commandMsg.bounceRestitution;
        sharedState.emaAlpha = commandMsg.emaAlpha;
        sharedState.deadzoneThreshold = commandMsg.deadzoneThreshold;
      }
    }
  }

  // Debugging output for command processing, moved out for faster execution of the critical section,
  // as printing to Serial can be slow and blocking.
  if (commandMsg.type == CommandType::START) {
    Serial.println("Game started and initialized dynamically on command.");
  } else if (commandMsg.type == CommandType::STOP) {
    Serial.println("Game stopped on command.");
  } else if (commandMsg.type == CommandType::PARAM_CHANGE) {
    Serial.println("Dynamic parameter configurations applied.");
  } else{
    Serial.println("Unknown command type received.");
  }
  
}

/**
 * @brief Computes a single integration step of physical element properties.
 *
 * Evaluates acceleration vectors derived from physical IMU or simulated inputs,
 * updates velocities, steps object positions, and wraps limits to bounds.
 */
void updateGameStep() {
  bool active = false;
  {
      MutexLock lock(stateMutex);
      if (lock.isLocked()) {
          active = sharedState.isRunning;
      }
  }

  if (!active) {
    // Skip processing if the game is not active/running. This prevents unnecessary sensor reads and calculations.
    return; 
  }

  // 1. Read sensor data
#if MOCK_SENSORS
  SensorData sensorData = sensorManager.readMock();
#else
  SensorData sensorData = sensorManager.read();
#endif

  // 2. Update game state
  MutexLock lock(stateMutex);
  if (lock.isLocked()) {
    // TODO: Implement physics and game engine.
      sharedState.accX = 0.0f; // Placeholder for actual acceleration computation
      sharedState.accY = 0.0f; // Placeholder for actual acceleration computation
  }
}


/**
 * @brief Concurrent FreeRTOS network management task pinned to Core 0.
 * 
 * This task handles MQTT connection maintenance, telemetry publishing, and command reception.
 * It runs in a loop, ensuring the MQTT client remains connected and processes incoming messages.
 * Telemetry data is published at a fixed rate, and the task yields periodically to avoid starving other tasks.
 */
void vNetworkTask(void *pvParameters) {
  // // Subscribe to command topic once connection completes
  // mqttManager.subscribe(mqtt_command_topic, 1);
  TickType_t lastTelemetryTime = xTaskGetTickCount();

  while (true) {
    // Ensure active connection is maintained
    mqttManager.connect();

    // Process background MQTT client actions, non-blocking
    mqttManager.loop();

    // Periodic telemetry publishing loop at 2Hz
    TickType_t currentTick = xTaskGetTickCount();
    if ((currentTick - lastTelemetryTime) >= pdMS_TO_TICKS(telemetry_rate_ms)) {
      lastTelemetryTime = currentTick;

      // Fetch the latest sensor data for telemetry, is often updated by the main loop
      // Using the cached value is enough here, as telemetry does not require the absolute latest reading.
      SensorData sensorData = sensorManager.getLastData();

      TelemetryData telemetry;

      // Populate Telemetry metadata
      telemetry.client_id = device_id;
      telemetry.hardware = device_hardware;
      telemetry.firmware_version = device_firmware_version;
      telemetry.uptime_ms = millis();
      telemetry.wifi_ssid = wifiManager.getSSID();

      // Populate Telemetry sensor metrics
      telemetry.accel_x = sensorData.accelerometerX;
      telemetry.accel_y = sensorData.accelerometerY;
      telemetry.accel_z = sensorData.accelerometerZ;
      telemetry.gyro_x = sensorData.gyroscopeX;
      telemetry.gyro_y = sensorData.gyroscopeY;
      telemetry.gyro_z = sensorData.gyroscopeZ;
      telemetry.battery_voltage = sensorData.batteryVoltage;
      telemetry.button = sensorData.button;

      // Thread-safe copy of shared properties
      {
          MutexLock lock(stateMutex);
          if (lock.isLocked()) {
              telemetry.game_id = sharedState.gameId;
              telemetry.player_name = sharedState.playerName;
              telemetry.target_cookies = sharedState.cookiesCount;
              telemetry.screen_width = display_width;
              telemetry.screen_height = display_height;
              telemetry.wall_thickness_px = sharedState.wallThicknessPx;

              telemetry.status = sharedState.isRunning ? "running" : "idle";
              telemetry.cookies_collected = sharedState.cookiesCollected;
              telemetry.cookies_remaining = sharedState.cookiesRemaining;
              telemetry.current_round = sharedState.currentRound;
              telemetry.elapsed_time_sec = sharedState.elapsedTimeSec;

              telemetry.ball_pos_x = sharedState.ballPosX;
              telemetry.ball_pos_y = sharedState.ballPosY;
              telemetry.velocity_x = sharedState.velocityX;
              telemetry.velocity_y = sharedState.velocityY;
              telemetry.acc_x = sharedState.accX;
              telemetry.acc_y = sharedState.accY;
          }
      }

      // Send telemetry data as JSON payload to the MQTT broker
      String payload = buildTelemetryJson(telemetry);
      mqttManager.publish(mqtt_telemetry_topic, payload.c_str(), mqtt_retain);
    }

    // Short yield to protect watchdogs from core starvation
    // vTaskDelay(): Delays the task for a specified number of ticks. 
    // pdMS_TO_TICKS(): Converts milliseconds to ticks.
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}