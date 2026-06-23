#include "src/network/time/TimeManager.h"
#include "src/network/wifi-connection/WifiManager.h" 
#include "src/network/mqtt/MqttManager.h"
#include "src/sensors/SensorManager.h"
#include "config.h"
#include <AUnit.h>
#include <ArduinoJson.h>

// Set to 1 to use mock sensor data instead of real hardware
#ifndef MOCK_SENSORS
#define MOCK_SENSORS 0
#endif

// Set to 1 to run AUnit tests; set 0 for main functionality.
// Can be overridden at compile time via -DRUN_TESTS=1 (used by CI).
#ifndef RUN_TESTS
#define RUN_TESTS 0
#endif

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
  // Connect to WiFi
  wifiManager.connect();

  // Synchronize time (required for TLS certificate validation and timestamps).
  timeManager.sync();

  // Initialize sensors 
  sensorManager.begin();

  // Setup MQTT client
  mqttManager.begin();
#endif
}

/**
 * @brief Main loop function for the MCU controller.
 * This function runs continuously after setup.
 */
void loop() {
#if RUN_TESTS
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

  // Create JSON payload
  JsonDocument doc; 
  doc["timestamp"] = sensorData.timestamp;
  doc["accX"] = sensorData.accelerometerX;
  doc["accY"] = sensorData.accelerometerY;
  doc["accZ"] = sensorData.accelerometerZ;
  doc["gyrX"] = sensorData.gyroscopeX;
  doc["gyrY"] = sensorData.gyroscopeY;
  doc["gyrZ"] = sensorData.gyroscopeZ;
  doc["batteryVoltage"] = sensorData.batteryVoltage;
  doc["button"] = sensorData.button;

  String payload;
  serializeJson(doc, payload);

  // Log to Serial output
  Serial.print("Telemetry Payload: ");
  Serial.println(payload);

  // Publish to MQTT Broker
  mqttManager.publish(mqtt_telemetry_topic, payload.c_str(), mqtt_retain);
  
  delay(1000);
#endif
}

