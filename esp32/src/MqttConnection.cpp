#include "MqttConnection.h"
#include "../wifi_mqtt_secrets.h"
#include "../config.h"

#include <PubSubClient.h>
#include <WiFiClientSecure.h>

/**
 * @brief TLS-enabled transport used by the MQTT client.
 *
 * The CA certificate is installed during setupMQTT() so the broker's
 * server certificate can be validated.
 */
static WiFiClientSecure secureClient;

/**
 * @brief PubSubClient instance bound to the secure transport.
 */
PubSubClient mqttClient(secureClient);

/**
 * @brief Handles messages received on subscribed topics.
 *
 * Logs the topic and the payload length to the serial console. The payload
 * is not null-terminated; the provided length must be respected when
 * reading it.
 *
 * @param topic   The topic the message was received on.
 * @param payload Pointer to the raw payload bytes.
 * @param length  Number of bytes in the payload.
 */
static void onMqttMessage(char *topic, byte *payload, unsigned int length)
{
    Serial.println("MQTT message received on topic " + String(topic) +
                   " with payload length " + String(length));

    // Print the payload as a string for debugging purposes.
    // Serial.print("Payload: ");
    // for (unsigned int i = 0; i < length; i++)
    // {
    //     Serial.print((char)payload[i]);
    // }
    // Serial.println();
}

void setupMQTT()
{
    Serial.println("Setting up secure MQTT client: " + String(mqtt_server) + ":" + String(mqtt_port));

    // Install the broker's root CA so the TLS handshake can validate the
    // server certificate. An accurate system clock is required.
    secureClient.setCACert(root_ca);

    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(onMqttMessage);
    mqttClient.setKeepAlive(mqtt_keep_alive_sec);

    // The default PubSubClient buffer (256 bytes) is too small for typical
    // JSON telemetry payloads, so it is enlarged here.
    // The size of the transmitted message can be as big as 950 bytes.
    // Keep a bit of headroom for MQTT overhead and ensure the buffer can hold the entire message.
    mqttClient.setBufferSize(2048);
}

void connectToMQTT()
{
    if (mqttClient.connected())
    {
        return;
    }

    Serial.print("Connecting to MQTT server: ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.println("...");

    if (mqttClient.connect(device_id, mqtt_username, mqtt_password))
    {
        Serial.println("MQTT connected successfully!");

        // Verify the connection by subscribing to the test topic. PubSubClient
        // supports QoS 0 and QoS 1 for subscriptions.
        bool subscribed = mqttClient.subscribe(mqtt_test_topic, mqtt_qos);
        Serial.println("Subscribed at QoS " + String(mqtt_qos) +
                       ", success: " + String(subscribed) +
                       ", topic: " + String(mqtt_test_topic));

        // Verify the connection by publishing a test message. PubSubClient
        // only supports QoS 0 for publishing.
        String testPayload = "Test message from " + String(device_id);
        bool published = mqttClient.publish(mqtt_test_topic, testPayload.c_str(), mqtt_retain);
        Serial.println("Published test message, success: " + String(published) +
                       ", topic: " + String(mqtt_test_topic));
    }
    else
    {
        Serial.println("MQTT connection failed, state: " + String(mqttClient.state()));
    }
}

bool publishMQTTData(const String &payload)
{
    Serial.println("Publishing to topic [" + String(mqtt_telemetry_topic) + "]: " + payload);

    if (!mqttClient.connected())
    {
        Serial.println("MQTT publish failed: client not connected.");
        return false;
    }

    // PubSubClient::publish only supports QoS 0; the configured mqtt_qos and
    // mqtt_retain values are honoured where the API allows.
    bool published = mqttClient.publish(mqtt_telemetry_topic, payload.c_str(), mqtt_retain);
    if (published)
    {
        Serial.println("MQTT publish: success.");
    }
    else
    {
        Serial.println("MQTT publish: failed.");
    }
    return published;
}