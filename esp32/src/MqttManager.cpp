#include "MqttManager.h"
#include "../wifi_mqtt_secrets.h"
#include "../config.h"

/**
 * @brief Pointer to the instance whose member callback should receive
 * incoming MQTT messages. Only one active instance is supported at a time,
 * which is sufficient for this single-broker device.
 */
MqttManager *MqttManager::_activeInstance = nullptr;

/**
 * @brief Shared MqttManager instance configured from wifi_mqtt_secrets.h and
 * config.h.
 */
MqttManager mqttManager(mqtt_server,
                        static_cast<uint16_t>(mqtt_port),
                        device_id,
                        mqtt_username,
                        mqtt_password,
                        root_ca);

MqttManager::MqttManager(const char *server,
                         uint16_t port,
                         const char *clientId,
                         const char *username,
                         const char *password,
                         const char *rootCa)
    : _client(_secureClient),
      _server(server),
      _port(port),
      _clientId(clientId),
      _username(username),
      _password(password),
      _rootCa(rootCa)
{
}

void MqttManager::begin()
{
    Serial.println("Setting up secure MQTT client: " + String(_server) + ":" + String(_port));

    _activeInstance = this;

    // Install the broker's root CA so the TLS handshake can validate the
    // server certificate. An accurate system clock is required.
    _secureClient.setCACert(_rootCa);

    _client.setServer(_server, _port);
    _client.setCallback(&MqttManager::messageTrampoline);
    _client.setKeepAlive(mqtt_keep_alive_sec);

    // The default PubSubClient buffer (256 bytes) is too small for typical
    // JSON telemetry payloads. Telemetry messages can reach roughly 950 bytes;
    // a 2 KiB buffer keeps comfortable headroom for MQTT framing overhead.
    _client.setBufferSize(2048);
}

bool MqttManager::connect()
{
    if (_client.connected())
    {
        return true;
    }

    Serial.print("Connecting to MQTT server: ");
    Serial.print(_server);
    Serial.print(":");
    Serial.print(_port);
    Serial.println("...");

    if (_client.connect(_clientId, _username, _password))
    {
        Serial.println("MQTT connected successfully!");

        // Verify the connection by subscribing to the test topic. PubSubClient
        // supports QoS 0 and QoS 1 for subscriptions.
        bool subscribed = _client.subscribe(mqtt_test_topic, mqtt_qos);
        Serial.println("Subscribed at QoS " + String(mqtt_qos) +
                       ", success: " + String(subscribed) +
                       ", topic: " + String(mqtt_test_topic));

        // Verify the connection by publishing a test message. PubSubClient
        // only supports QoS 0 for publishing; the configured retain flag is
        // honoured.
        String testPayload = "Test message from " + String(_clientId);
        bool published = _client.publish(mqtt_test_topic, testPayload.c_str(), mqtt_retain);
        Serial.println("Published test message, success: " + String(published) +
                       ", topic: " + String(mqtt_test_topic));

        return true;
    }

    Serial.println("MQTT connection failed, state: " + String(_client.state()));
    return false;
}

void MqttManager::loop()
{
    _client.loop();
}

bool MqttManager::isConnected()
{
    return _client.connected();
}

bool MqttManager::publish(const char *topic, const String &payload, bool retain)
{
    Serial.println("Publishing to topic [" + String(topic) + "]: " + payload);

    if (!_client.connected())
    {
        Serial.println("MQTT publish failed: client not connected.");
        return false;
    }

    bool published = _client.publish(topic, payload.c_str(), retain);
    Serial.println(published ? "MQTT publish: success." : "MQTT publish: failed.");
    return published;
}

bool MqttManager::subscribe(const char *topic, uint8_t qos)
{
    return _client.subscribe(topic, qos);
}

void MqttManager::disconnect()
{
    _client.disconnect();
}

int MqttManager::state()
{
    return _client.state();
}

void MqttManager::messageTrampoline(char *topic, byte *payload, unsigned int length)
{
    if (_activeInstance != nullptr)
    {
        _activeInstance->onMessage(topic, payload, length);
    }
}

void MqttManager::onMessage(char *topic, byte *payload, unsigned int length)
{
    (void)payload; // Payload is not consumed in the default implementation.
    Serial.println("MQTT message received on topic " + String(topic) +
                   " with payload length " + String(length));
}
