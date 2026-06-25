#include "src/network/time/TimeManager.h"
#include "src/network/wifi-connection/WifiManager.h"
#include "src/network/mqtt/MqttManager.h"
#include "src/network/json-builder/JsonBuilder.h"
#include "src/sensors/SensorManager.h"
#include "src/graphics/GraphicsManager.h"
#include "src/physics/PhysicsBody.h"
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

// Play space calculations
const int play_width = display_width;
const int play_height = display_height - ui_header_height;

// Memory allocation for the game board bitmap
uint8_t *gameBoard = nullptr;

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
  // pinMode(pin_lcd_bl, OUTPUT);
  // digitalWrite(pin_lcd_bl, HIGH);
  graphicsManager.begin(gfx);

  // Connect to WiFi
  graphicsManager.drawLoadingScreen("Connecting WiFi...");
  wifiManager.connect();

  // Synchronize time (required for TLS certificate validation and timestamps).
  graphicsManager.drawLoadingScreen("Syncing Time...");
  timeManager.sync();

  // Initialize sensors
  graphicsManager.drawLoadingScreen("Initializing Sensors...");
  sensorManager.begin();

  // Setup MQTT client
  graphicsManager.drawLoadingScreen("Starting MQTT...");
  mqttManager.begin();

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

  // Ensure the MQTT connection is active
  mqttManager.connect();

  // Handle MQTT communication and keep alive
  mqttManager.loop();

  // Read sensor values (mock or real based on definition)
#if MOCK_SENSORS
  Serial.println("Reading mock sensor data...");
  SensorData sensorData = sensorManager.readMock();
#else
  Serial.println("Reading real sensor data...");
  SensorData sensorData = sensorManager.read();
#endif

  // Populate telemetry data struct from sensors and configuration
  TelemetryData telemetry;

  // Device Information
  telemetry.client_id = device_id;
  telemetry.hardware = device_hardware;
  telemetry.firmware_version = device_firmware_version;
  telemetry.uptime_ms = millis();
  telemetry.wifi_ssid = wifiManager.getSSID();

  // Game Configuration
  telemetry.game_id = 1;
  telemetry.player_name = "Player 1";
  telemetry.target_cookies = default_cookies_count;
  telemetry.screen_width = display_width;
  telemetry.screen_height = display_height;
  telemetry.wall_thickness_px = default_wall_thickness_px;

  // Game State
  telemetry.status = "running";
  telemetry.cookies_collected = 0;
  telemetry.cookies_remaining = 10;
  telemetry.current_round = 1;
  telemetry.elapsed_time_sec = 0.0f;

  // Physics Simulation State
  telemetry.ball_pos_x = 112.45f;
  telemetry.ball_pos_y = 145.2f;
  telemetry.velocity_x = 1.85f;
  telemetry.velocity_y = -0.92f;
  telemetry.acc_x = 0.15f;
  telemetry.acc_y = -0.34f;

  // Sensor Readings (from SensorManager)
  telemetry.accel_x = sensorData.accelerometerX;
  telemetry.accel_y = sensorData.accelerometerY;
  telemetry.accel_z = sensorData.accelerometerZ;
  telemetry.gyro_x = sensorData.gyroscopeX;
  telemetry.gyro_y = sensorData.gyroscopeY;
  telemetry.gyro_z = sensorData.gyroscopeZ;
  telemetry.battery_voltage = sensorData.batteryVoltage;
  telemetry.button = sensorData.button;

  // Build JSON payload
  String payload = buildTelemetryJson(telemetry);

  // Log to Serial output
  // Serial.print("Telemetry Payload: ");
  // Serial.println(payload);

  // Publish to MQTT Broker
  mqttManager.publish(mqtt_telemetry_topic, payload.c_str(), mqtt_retain);

  delay(1000);
#endif
}
