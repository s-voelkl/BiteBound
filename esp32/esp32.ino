#include "src/network/time/TimeManager.h"
#include "src/network/wifi-connection/WifiManager.h"
#include "src/network/mqtt/MqttManager.h"
#include "src/network/json-builder/JsonBuilder.h"
#include "src/sensors/SensorManager.h"
#include "src/graphics/GraphicsManager.h"
#include "src/engine/GameEngine.h"
#include "config.h"

#include <AUnit.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>

// Set to 1 to use mock sensor data instead of real hardware
#ifndef MOCK_SENSORS
#define MOCK_SENSORS 0
#endif

// Set to 1 to run AUnit tests; set 0 for main functionality.
// Can be overridden at compile time via -DRUN_TESTS=1 (used by CI).
#ifndef RUN_TESTS
#define RUN_TESTS 0
#endif

// Global Graphics Setup
Arduino_DataBus *bus = new Arduino_ESP32SPI(pin_lcd_dc, pin_lcd_cs, pin_lcd_sck, pin_lcd_mosi);
Arduino_GFX *gfx = new Arduino_ST7789(bus, pin_lcd_rst /* RST */,0 /* rotation */,
  true /* IPS */, display_width, display_height, 0, 20, 0, 0);
GraphicsManager graphicsManager(display_width, display_height);

// Play space calculations (HUD occupies the top ui_header_height pixels)
const int play_width = display_width;
const int play_height = display_height - ui_header_height;

// Game orchestration: owns physics and both games, and drives the active one
// (each game tracks its own round counter).
GameEngine gameEngine;

/**
 * @brief Main setup function for the MCU controller.
 * This function initializes the serial communication, connects to WiFi and MQTT.
 */
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 BiteBound Hardware starting...");

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

  // Initialize the game engine and start Game 1 (Labyrinth) by default.
  // chooseGameMode() generates the first maze and flags a full redraw.
  graphicsManager.drawLoadingScreen("Creating Maze...");
  gameEngine.begin(play_width, play_height);
  gameEngine.chooseGameMode(default_game_id);

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

  // Ensure the MQTT connection is active and serviced.
  mqttManager.connect();
  mqttManager.loop();

  // Frame timing for the physics step.
  static uint32_t lastFrameMs = 0;
  static uint32_t lastTelemetryMs = 0;
  const uint32_t nowMs = millis();

  // Initialize timing on first call
  if (lastFrameMs == 0) {
    lastFrameMs = nowMs;
  }

  // Maintain a stable game loop at the tick rate defined in config.h.
  if (nowMs - lastFrameMs < game_tick_rate_ms) {
    delay(1);
    return;
  }

  // Time delta (how much time elapsed since the last PhysicsEngine step())
  const float dt = (nowMs - lastFrameMs) / 1000.0f;
  lastFrameMs = nowMs;

  // Read sensor values (mock or real based on definition)
#if MOCK_SENSORS
  SensorData sensorData = sensorManager.readMock();
#else
  SensorData sensorData = sensorManager.read();
#endif

  // 1) Advance the active game by one frame (tilt -> physics -> cookie pickup).
  gameEngine.update(sensorData.gyroscopeX, sensorData.gyroscopeY, dt);

  // 2) Render. A freshly built round requests a full redraw of the maze/HUD.
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

  // 3) Publish telemetry at a throttled rate (~2 Hz).
  if (nowMs - lastTelemetryMs >= telemetry_rate_ms) {
    lastTelemetryMs = nowMs;

    const GameState &gs = gameEngine.state();
    const PhysicsBody &ball = gameEngine.ball();

    TelemetryData telemetry;

    // Device Information
    telemetry.client_id = device_id;
    telemetry.hardware = device_hardware;
    telemetry.firmware_version = device_firmware_version;
    telemetry.uptime_ms = millis();
    telemetry.wifi_ssid = wifiManager.getSSID();

    // Game Configuration
    telemetry.game_id = gameEngine.activeGameId();
    telemetry.player_name = default_player_name;
    telemetry.target_cookies = (int)gameEngine.cookies().target();
    telemetry.screen_width = display_width;
    telemetry.screen_height = display_height;
    telemetry.wall_thickness_px = default_wall_thickness_px;

    String runningStatusStr = "idle";
    if (gs.runningStatus == RunningStatus::RUNNING) {
      runningStatusStr = "running";
    } else if (gs.runningStatus == RunningStatus::COMPLETED) {
      runningStatusStr = "completed";
    } 
    // Game State (from the engine)
    telemetry.runningStatus = runningStatusStr;
    telemetry.cookies_collected = gs.cookiesCollected;
    telemetry.cookies_remaining = gs.cookiesRemaining;
    telemetry.current_round = gs.currentRound;
    telemetry.elapsed_time_sec = gs.elapsedTimeSec;

    // Physics Simulation State (from the active ball)
    telemetry.ball_pos_x = ball.x;
    telemetry.ball_pos_y = ball.y;
    telemetry.velocity_x = ball.vx;
    telemetry.velocity_y = ball.vy;
    telemetry.acc_x = 0.0f;
    telemetry.acc_y = 0.0f;

    // Sensor Readings (from SensorManager)
    telemetry.accel_x = sensorData.accelerometerX;
    telemetry.accel_y = sensorData.accelerometerY;
    telemetry.accel_z = sensorData.accelerometerZ;
    telemetry.gyro_x = sensorData.gyroscopeX;
    telemetry.gyro_y = sensorData.gyroscopeY;
    telemetry.gyro_z = sensorData.gyroscopeZ;
    telemetry.battery_voltage = sensorData.batteryVoltage;
    telemetry.button = sensorData.button;

    String payload = buildTelemetryJson(telemetry);
    mqttManager.publish(mqtt_telemetry_topic, payload.c_str(), mqtt_retain);
  }
#endif
}
