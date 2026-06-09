#ifndef MQTT_CONNECTION_H
#define MQTT_CONNECTION_H

#include <Arduino.h>
#include <PubSubClient.h>

/**
 * @brief Shared PubSubClient instance.
 *
 * Exposed so that the main sketch and tests can drive the client directly
 * (for example by calling mqttClient.loop() from the Arduino loop).
 */
extern PubSubClient mqttClient;

/**
 * @brief Initialises the MQTT client.
 *
 * Configures the TLS-enabled transport, server address, port, credentials,
 * keep-alive interval and the message callback. The broker address and
 * credentials are read from wifi_mqtt_secrets.h; ports, topics, QoS and
 * keep-alive values are read from config.h.
 *
 * @note Must be invoked once during device setup, after the system time has
 * been synchronised so that TLS certificate validation succeeds.
 */
void setupMQTT();

/**
 * @brief Establishes (or re-establishes) the connection to the MQTT broker.
 *
 * If the client is already connected the call is a no-op. Otherwise a
 * blocking connection attempt is performed using the configured credentials
 * and the device identifier as MQTT client ID. On success the test topic is
 * subscribed and a verification message is published.
 */
void connectToMQTT();

/**
 * @brief Publishes a payload to the configured telemetry topic.
 *
 * @param payload The string payload to publish.
 * @return true if the broker accepted the publish call, false otherwise
 * (for example when the client is not currently connected).
 */
bool publishMQTTData(const String &payload);

#endif // MQTT_CONNECTION_H