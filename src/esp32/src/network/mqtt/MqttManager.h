#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

/**
 * @brief Manages the secure MQTT connection to the configured broker.
 *
 * Encapsulates the TLS-enabled transport, the PubSubClient, the broker
 * credentials and the message callback into a single, testable component.
 * Configuration values that are intrinsic to this device (test topic, QoS,
 * keep-alive interval) are read from config.h, while broker-specific values
 * (host, port, credentials, CA certificate) are injected via the constructor.
 */
class MqttManager
{
public:
    /**
     * @brief Constructs an MqttManager with the broker connection parameters.
     *
     * @param server   Broker hostname.
     * @param port     Broker port.
     * @param clientId MQTT client identifier used during CONNECT.
     * @param username Broker username.
     * @param password Broker password.
     * @param rootCa   PEM-encoded CA certificate used to validate the broker.
     *
     * @note All pointers must remain valid for the lifetime of the instance.
     */
    MqttManager(const char *server,
                uint16_t port,
                const char *clientId,
                const char *username,
                const char *password,
                const char *rootCa);

    /**
     * @brief Initialises the underlying MQTT client.
     *
     * Installs the CA certificate on the TLS transport, configures the broker
     * address, the keep-alive interval, the message callback and the publish
     * buffer size. Must be invoked once during device setup, after the system
     * clock has been synchronised so TLS certificate validation can succeed.
     */
    void begin();

    /**
     * @brief Establishes (or re-establishes) the connection to the broker.
     *
     * If the client is already connected the call is a no-op. Otherwise a
     * blocking connection attempt is performed using the configured
     * credentials and client identifier. On success a verification subscribe
     * and publish are issued against the test topic defined in config.h.
     *
     * @return true if the client is connected after the call, false otherwise.
     */
    bool connect();

    /**
     * @brief Drives the MQTT client's network loop.
     *
     * Must be invoked on every iteration of the Arduino loop() to service
     * incoming traffic and honour the configured keep-alive interval.
     */
    void loop();

    /**
     * @brief Indicates whether the MQTT client is currently connected.
     *
     * @return true if connected, false otherwise.
     */
    bool isConnected();

    /**
     * @brief Publishes a payload to the given topic.
     *
     * @param topic   Topic to publish on.
     * @param payload Payload to publish.
     * @param retain  Retain flag forwarded to the broker.
     * @return true if the broker accepted the publish call, false otherwise
     * (for example when the client is not currently connected).
     */
    bool publish(const char *topic, const String &payload, bool retain = false);

    /**
     * @brief Subscribes to the given topic.
     *
     * @param topic Topic to subscribe to.
     * @param qos   Requested quality of service (0 or 1).
     * @return true on success, false otherwise.
     */
    bool subscribe(const char *topic, uint8_t qos = 0);

    /**
     * @brief Disconnects the MQTT client from the broker.
     */
    void disconnect();

    /**
     * @brief Returns the underlying PubSubClient state code.
     *
     * @return The most recent PubSubClient state.
     */
    int state();

private:
    /**
     * @brief Static trampoline that forwards PubSubClient callbacks to the
     * currently active instance.
     *
     * PubSubClient does not portably support std::function, so a static
     * adapter is used to dispatch to a member function on the active instance.
     */
    static void messageTrampoline(char *topic, byte *payload, unsigned int length);

    /**
     * @brief Handles a single message received on a subscribed topic.
     *
     * Logs the topic and payload length to the serial console. The payload is
     * not null-terminated; the provided length must be respected when reading
     * it.
     *
     * If the topic matches the configured command topic, the payload is parsed
     * as a command message and the resulting CommandMsg is enqueued for the
     * main loop to process using the shared state. Else the message is not further processed.
     */
    void onMessage(char *topic, byte *payload, unsigned int length);

    static MqttManager *_activeInstance;

    WiFiClientSecure _secureClient;
    PubSubClient _client;
    const char *_server;
    uint16_t _port;
    const char *_clientId;
    const char *_username;
    const char *_password;
    const char *_rootCa;
};

/**
 * @brief Shared MqttManager instance configured from wifi_mqtt_secrets.h and
 * config.h.
 *
 * Exposed so the main sketch and tests can drive MQTT through a single,
 * well-known entry point.
 */
extern MqttManager mqttManager;

#endif // MQTT_MANAGER_H
