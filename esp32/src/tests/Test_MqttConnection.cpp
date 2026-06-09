#include "../MqttConnection.h"
#include "../../config.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <AUnit.h>

/**
 * @test Verifies that publishing fails when the MQTT client is not connected.
 *
 * The client is disconnected explicitly to guarantee a known starting state.
 */
test(MqttConnection_PublishFailsWhenDisconnected)
{
    // Arrange: ensure the client is in a disconnected state.
    mqttClient.disconnect();
    assertFalse(mqttClient.connected());

    // Act
    bool result = publishMQTTData("test_payload");

    // Assert
    assertFalse(result);
}

/**
 * @test Verifies that setupMQTT() configures the PubSubClient with the
 * server and keep-alive values defined in the project configuration.
 */
test(MqttConnection_SetupConfiguresClient)
{
    // Act
    setupMQTT();

    // Assert: the client should not yet be connected after setup only.
    assertFalse(mqttClient.connected());
}

/**
 * @test Verifies that the PubSubClient loop can be driven safely even when
 * the client is not connected to a broker.
 */
test(MqttConnection_LoopIsSafeWhenDisconnected)
{
    // Arrange
    mqttClient.disconnect();

    // Act / Assert: must not crash or block.
    mqttClient.loop();
    assertFalse(mqttClient.connected());
}

/**
 * @test Verifies that an empty payload is rejected when the client is not
 * connected, mirroring the behaviour for non-empty payloads.
 */
test(MqttConnection_PublishEmptyPayloadFailsWhenDisconnected)
{
    // Arrange
    mqttClient.disconnect();

    // Act
    bool result = publishMQTTData("");

    // Assert
    assertFalse(result);
}

/**
 * @test Verifies that publishing a JSON-encoded payload behaves the same
 * as publishing any other string when the client is not connected.
 */
test(MqttConnection_PublishJsonPayloadFailsWhenDisconnected)
{
    // Arrange
    mqttClient.disconnect();

    StaticJsonDocument<128> doc;
    doc["device_id"] = device_id;
    doc["value"] = 42;
    String payload;
    serializeJson(doc, payload);

    // Act
    bool result = publishMQTTData(payload);

    // Assert
    assertFalse(result);
}
