#include "WifiConnection.h"
#include "MqttConnection.h"
#include "SensorReader.h"
#include "TimeHelper.h"
#include <AUnit.h>

// Set to 1 to run AUnit tests; set 0 for main functionality.
#define RUN_TESTS 0

/**
 * @brief Main setup function for the MCU controller.
 * This function initializes the serial communication, connects to WiFi and MQTT.
 */
void setup() {
  Serial.begin(115200, SERIAL_8N1);
  
#if RUN_TESTS 
  // serial initialization delay for test output
  delay(3000); 
  Serial.println("Starting AUnit tests...");
#else
  // Connect to WiFi
  connectToWiFi();

  // Synchronize time (required for TLS certificate validation and timestamps).
  syncTime();

  // Initialize sensors 
  initSensors();

  // Setup MQTT client
  setupMQTT();
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
  connectToMQTT();

  // Handle MQTT communication and keep alive
  loopMQTT();

  // Read sensor values. 
  SensorData sensorData = readSensors();

  // Publish to MQTT Broker
  publishMQTTData("payload_placeholder"); // TODO: json payload
  
  delay(t_delay_ms);
#endif
}
