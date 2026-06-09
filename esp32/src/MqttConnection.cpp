// required for enabling SSL/TLS support in AsyncMqttClient, before including the library header
#define ASYNC_TCP_SSL_ENABLED 1

#include "MqttConnection.h"
#include "wifi_mqtt_secrets.h"
#include <AsyncMqtt_Generic.h>
#include <WiFiClientSecure.h>

AsyncMqttClient mqttClient;
WiFiClientSecure secureClient;

/**
 * @brief Callback function for successful MQTT connection, verifies connection.
 * This function is called when the MQTT client successfully connects to the broker.
 * It performs a test subscription and a test publish to verify that the connection is working correctly.
 */
void onMqttConnect(bool sessionPresent)
{
    Serial.println("MQTT connected successfully!");

    // Test subscription to verify connection with test topic and qos 1.
    uint16_t packetIdSub = mqttClient.subscribe(mqtt_test_topic.c_str(), mqtt_qos);
    Serial.println("Subscribed at QoS " + String(mqtt_qos) + ", packetId: " + String(packetIdSub) + ", topic: " + mqtt_test_topic);

    // Test publish to verify connection with test topic and qos 1.
    String testPayload = "Test message from " + String(device_id);
    uint16_t packetIdPub = mqttClient.publish(mqtt_test_topic.c_str(), mqtt_qos, false, testPayload.c_str());
    Serial.println("Published at QoS " + String(mqtt_qos) + ", packetId: " + String(packetIdPub) + ", topic: " + mqtt_test_topic);
}

/**
 * @brief Callback function for MQTT disconnection.
 */
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println("MQTT disconnected.");
}

/**
 * @brief Callback function for handling incoming MQTT messages.
 * This function is triggered whenever a message is received on a subscribed topic.
 *
 * @param topic The topic the message was received on.
 * @param payload The message payload.
 * @param properties MQTT message properties.
 * @param len The length of the payload.
 * @param index The index of the current payload chunk.
 * @param total The total size of the payload.
 */
void onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties,
                   const size_t &len, const size_t &index, const size_t &total)
{
    Serial.println("MQTT Message received on topic " + String(topic) + " with payload length " + String(len));
}

/**
 * @brief Setup the MQTT client.
 * Configures the MQTT server and port based on the credentials in wifi_mqtt_secrets.h.
 *
 * @note This function should be called once during the setup phase of the MCU to
 * initialize the MQTT client with the correct server settings and authentication credentials.
 */
void setupMQTT()
{
    Serial.print("Setting up Secure MQTT client: " + String(mqtt_server) + ":" + String(mqtt_port));

    // Set root certificate. Exact time required for TLS certificate validation, as certificates have a validity period.
    secureClient.setCACert(root_ca);

    // callback functions
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);

    // server settings
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCredentials(mqtt_username, mqtt_password);

    // keep alive interval in seconds
    mqttClient.setKeepAlive(mqtt_keep_alive_sec);

    // sometimes explicit secure flag is required for some versions of AsyncMQTT
    mqttClient.setSecure(true);
}

/**
 * @brief Connects or reconnects to the MQTT broker.
 * This function handles connecting to the server asynchronously using the configured credentials.
 */
void connectToMQTT()
{
    if (!mqttClient.connected())
    {
        Serial.print("Connecting to MQTT server: ");
        Serial.print(mqtt_server);
        Serial.print(":");
        Serial.print(mqtt_port);
        Serial.println("...");

        // Attempt to connect (non-blocking)
        mqttClient.connect();
    }
}

/**
 * @brief Publishes data to the configured MQTT topic and subtopic.
 * The full topic is constructed using the base mqtt_topic, device_id, and mqtt_subtopic_data.
 *
 * @param payload The string payload to publish.
 * @return true if published successfully, false otherwise.
 */
bool publishMQTTData(const String &payload)
{
    Serial.println("Publishing to topic [" + String(mqtt_telemetry_topic) + "]: " + String(payload));

    if (mqttClient.connected())
    {
        // topic, qos, retain (not saved by broker for later subscribers), payload
        uint16_t packetIdPub = mqttClient.publish(mqtt_telemetry_topic.c_str(), mqtt_qos, mqtt_retain, payload.c_str());
        // success if packetIdPub > 0, failure if 0
        if (packetIdPub > 0)
        {
            Serial.println("MQTT Publish: Success.");
            return true;
        }
        else
        {
            Serial.println("MQTT Publish: Failed (Packed ID 0).");
            return false;
        }
    }
    else
    {
        Serial.println("MQTT Publish: Failed (not connected).");
        return false;
    }
}