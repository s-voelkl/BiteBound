#include "src/network/time/TimeManager.h"
#include "src/network/wifi-connection/WifiManager.h"
#include "src/network/mqtt/MqttManager.h"
#include "src/network/json-builder/JsonBuilder.h"
#include "src/network/json-parser/CommandParser.h"
#include "src/network/shared/SharedStateData.h"
#include "src/network/shared/MutexLock.h"
#include "src/sensors/SensorManager.h"
#include "src/graphics/GraphicsManager.h"
#include "src/engine/GameEngine.h"
#include "config.h"

#include <AUnit.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

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
Arduino_GFX *gfx = new Arduino_ST7789(bus, pin_lcd_rst /* RST */, 0 /* rotation */,
  true /* IPS */, display_width, display_height, 0, 20, 0, 0);
GraphicsManager graphicsManager(display_width, display_height);

// Play space calculations (HUD occupies the top ui_header_height pixels)
const int play_width = display_width;
const int play_height = display_height - ui_header_height;

// Game orchestration: owns physics and both games, and drives the active one.
GameEngine gameEngine;

// FreeRTOS Task on Core 0 to isolate network operations.
void vNetworkTask(void *pvParameters);

// Helper function declarations
void processIncomingCommands();
void updateGameStep();

/**
 * @brief Main setup function for the MCU controller.
 * This function initializes processing primitives, connectivity, and the game loop.
 */
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 BiteBound Hardware starting...");

  // Initialize threading primitives
  stateMutex = xSemaphoreCreateMutex();
  commandQueue = xQueueCreate(cmd_queue_length, cmd_queue_item_size);
  initSharedState(sharedState);

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

  // Initialize the game engine and start in idle mode.
  graphicsManager.drawLoadingScreen("Creating Maze...");
  gameEngine.begin(play_width, play_height);
  gameEngine.chooseGameMode(default_game_id);

  // Set running state to true to match main branch behavior (playable on boot).
  {
      MutexLock lock(stateMutex);
      if (lock.isLocked()) {
          sharedState.gameId = default_game_id;
          sharedState.runningStatus = RunningStatus::RUNNING;
      }
  }

  // Create Network Task on Core 0 (isolated from logic/rendering)
  xTaskCreatePinnedToCore(
      vNetworkTask,
      "NetworkTask",
      8192,
      NULL,
      1,
      NULL,
      core_network
  );

  graphicsManager.drawLoadingScreen("BiteBound is Ready!");
#endif
}

/**
 * @brief Main loop function for the MCU controller (running on Core 1).
 */
void loop() {
#if RUN_TESTS
  // aunit::TestRunner::list(); // List all registered tests.
  aunit::TestRunner::setTimeout(30); // Set a timeout of 30 seconds for tests.
  aunit::TestRunner::run();
#else
  static TickType_t lastTickTime = xTaskGetTickCount();

  // 1. Core 1 Command Processing
  processIncomingCommands();

  // 2. Local Game Step & Rendering
  updateGameStep();

  // Preserve precision step constraints (e.g. 50Hz for smooth physics)
  vTaskDelayUntil(&lastTickTime, pdMS_TO_TICKS(game_tick_rate_ms));
#endif
}

/**
 * @brief Processes incoming command structures transmitted from the network task.
 */
void processIncomingCommands() {
  CommandMsg commandMsg;
  while (xQueueReceive(commandQueue, &commandMsg, 0) == pdPASS) {
    {
      MutexLock lock(stateMutex);
      if (lock.isLocked()) {
        if (commandMsg.type == CommandType::START) {
          GameConfig config;
          config.gameId = commandMsg.gameId;
          config.targetCookies = commandMsg.cookiesCount;
          config.wallThicknessPx = commandMsg.wallThicknessPx;
          config.physics.sensitivity = commandMsg.imuSensitivity;
          config.physics.restitution = commandMsg.bounceRestitution;
          config.physics.emaAlpha = commandMsg.emaAlpha;
          config.physics.deadzone = commandMsg.deadzoneThreshold;
          
          gameEngine.applyConfig(config);
          gameEngine.chooseGameMode(commandMsg.gameId);
          
          sharedState.runningStatus = RunningStatus::RUNNING;
          sharedState.gameId = commandMsg.gameId;
          strncpy(sharedState.playerName, commandMsg.playerName, sizeof(sharedState.playerName) - 1);
        } 
        else if (commandMsg.type == CommandType::STOP) {
          sharedState.runningStatus = RunningStatus::IDLE;
        } 
        else if (commandMsg.type == CommandType::PARAM_CHANGE) {
          GameConfig config;
          config.physics.sensitivity = commandMsg.imuSensitivity;
          config.physics.restitution = commandMsg.bounceRestitution;
          config.physics.emaAlpha = commandMsg.emaAlpha;
          config.physics.deadzone = commandMsg.deadzoneThreshold;
          gameEngine.applyConfig(config);
        }
      }
    }

    // Debugging output for command processing is outside the critical lock
    // section, as printing to Serial can be slow and blocking.
    if (commandMsg.type == CommandType::START) {
      Serial.println("Command: Start game received.");
    } else if (commandMsg.type == CommandType::STOP) {
      Serial.println("Command: Stop game received.");
    } else if (commandMsg.type == CommandType::PARAM_CHANGE) {
      Serial.println("Command: Parameter Change received.");
    } else {
      Serial.println("Command: Unknown command type received.");
    }
  }
}

/**
 * @brief Computes physics, game state transitions and rendering on Core 1.
 *
 * This function is called at a fixed rate (50Hz) to ensure smooth physics and rendering.
 * It reads the latest sensor data, updates the game engine, renders the display, and updates
 * the shared state for telemetry.
 */
void updateGameStep() {
  // 1. Sensors: Read sensor values (mock or real hardware)
#if MOCK_SENSORS
  SensorData sensorData = sensorManager.readMock();
#else
  SensorData sensorData = sensorManager.read();
#endif

  // Avoid unnecessary updating of the game engine if the game is not running.
  bool isRunningStatus = false;
  {
      MutexLock lock(stateMutex);
      if (lock.isLocked()) {
          isRunningStatus = (sharedState.runningStatus == RunningStatus::RUNNING);
      }
  }

  // 2. Time: Frame timing calculation
  static uint32_t lastFrameMs = 0;
  const uint32_t nowMs = millis();
  if (lastFrameMs == 0) lastFrameMs = nowMs;
  const float dt = (nowMs - lastFrameMs) / 1000.0f;
  lastFrameMs = nowMs;

  // 3. Physics: Advance the active game by one physics frame if running.
  if (isRunningStatus) {
    gameEngine.update(sensorData.gyroscopeX, sensorData.gyroscopeY, dt);
  }

  // 4. Display: Render the current game state to the display.
  if (gameEngine.consumeRedraw()) {
    graphicsManager.forceFullRedraw();
  }
  graphicsManager.update(
      gameEngine.board(),
      play_width,
      play_height,
      gameEngine.ball(),
      gameEngine.cookies().data(),
      (int)gameEngine.cookies().count(),
      gameEngine.state());

  // 5. Telemetry: Update shared state for the network core to publish as telemetry.
  {
    MutexLock lock(stateMutex);
    if (lock.isLocked()) {
      const GameState &gs = gameEngine.state();
      const PhysicsBody &ball = gameEngine.ball();

      sharedState.ballPosX = ball.x;
      sharedState.ballPosY = ball.y;
      sharedState.velocityX = ball.vx;
      sharedState.velocityY = ball.vy;
      sharedState.accX = 0; // Not currently calculated separately from tilt inputs
      sharedState.accY = 0;

      sharedState.cookiesCollected = gs.cookiesCollected;
      sharedState.cookiesRemaining = gs.cookiesRemaining;
      sharedState.currentRound = gs.currentRound;
      sharedState.elapsedTimeSec = gs.elapsedTimeSec;
      sharedState.gameId = gameEngine.activeGameId();
      sharedState.runningStatus = gs.runningStatus;
    }
  }
}

/**
 * @brief Concurrent FreeRTOS network management task pinned to Core 0.
 * Handles MQTT connection, keeps the client alive, and publishes telemetry.
 */
void vNetworkTask(void *pvParameters) {
  TickType_t lastTelemetryTime = xTaskGetTickCount();
  while (true) {
    // Process MQTT callbacks to populate commandQueue with incoming commands.
    // Separated from the connect() call to avoid blocking the game loop for up to 
    // 500ms on network I/O but still ensure keep-alive of the MQTT connection.
    mqttManager.loop();

    // Periodic telemetry publishing at 2Hz.
    TickType_t currentTick = xTaskGetTickCount();
    if ((currentTick - lastTelemetryTime) >= pdMS_TO_TICKS(telemetry_rate_ms)) {
      lastTelemetryTime = currentTick;

      // Keep connection alive every 500ms is sufficient for session maintenance.
      mqttManager.connect();

      SensorData sensorData = sensorManager.getLastData();
      TelemetryData telemetry;

      // Populate hardware information.
      telemetry.client_id = device_id;
      telemetry.hardware = device_hardware;
      telemetry.firmware_version = device_firmware_version;
      telemetry.uptime_ms = millis();
      telemetry.wifi_ssid = wifiManager.getSSID();

      // Populate raw sensor readings.
      telemetry.accel_x = sensorData.accelerometerX;
      telemetry.accel_y = sensorData.accelerometerY;
      telemetry.accel_z = sensorData.accelerometerZ;
      telemetry.gyro_x = sensorData.gyroscopeX;
      telemetry.gyro_y = sensorData.gyroscopeY;
      telemetry.gyro_z = sensorData.gyroscopeZ;
      telemetry.battery_voltage = sensorData.batteryVoltage;
      telemetry.button = sensorData.button;

      // Thread-safe capture of current game progress and physics state.
      {
        MutexLock lock(stateMutex);
        if (lock.isLocked()) {
          telemetry.game_id = sharedState.gameId;
          telemetry.player_name = sharedState.playerName;
          telemetry.target_cookies = sharedState.cookiesCount;
          telemetry.screen_width = display_width;
          telemetry.screen_height = display_height;
          telemetry.wall_thickness_px = sharedState.wallThicknessPx;

          String runningStatusStr = "idle";
          if (sharedState.runningStatus == RunningStatus::RUNNING) {
              runningStatusStr = "running";
          } else if (sharedState.runningStatus == RunningStatus::COMPLETED) {
              runningStatusStr = "completed";
          }
          telemetry.runningStatus = runningStatusStr;
          
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

      // Send telemetry JSON payload to MQTT broker
      String payload = buildTelemetryJson(telemetry);
      mqttManager.publish(mqtt_telemetry_topic, payload.c_str(), mqtt_retain);
    }

    // Short yield to feed the IDLE task and watchdogs.
    // vTaskDelay(): Delays the task for a specified number of ticks. 
    // pdMS_TO_TICKS(): Converts milliseconds to ticks.
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
