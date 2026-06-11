#include "../MqttManager.h"
#include "../../config.h"

#include <ArduinoJson.h>
#include <AUnit.h>

/**
 * @test Verifies that publishing fails when the MQTT client is not connected.
 *
 * The client is disconnected explicitly to guarantee a known starting state.
 */
test(MqttManager_PublishFailsWhenDisconnected)
{
    // Arrange
    mqttManager.disconnect();
    assertFalse(mqttManager.isConnected());

    // Act
    bool result = mqttManager.publish(mqtt_telemetry_topic, "test_payload");

    // Assert
    assertFalse(result);
}

/**
 * @test Verifies that begin() configures the client without establishing a
 * broker connection.
 */
test(MqttManager_BeginConfiguresClientWithoutConnecting)
{
    // Act
    mqttManager.begin();

    // Assert: the client should not yet be connected after setup only.
    assertFalse(mqttManager.isConnected());
}

/**
 * @test Verifies that the MQTT loop can be driven safely even when the
 * client is not connected to a broker.
 */
test(MqttManager_LoopIsSafeWhenDisconnected)
{
    // Arrange
    mqttManager.disconnect();

    // Act / Assert: must not crash or block.
    mqttManager.loop();
    assertFalse(mqttManager.isConnected());
}

/**
 * @test Verifies that an empty payload is rejected when the client is not
 * connected, mirroring the behaviour for non-empty payloads.
 */
test(MqttManager_PublishEmptyPayloadFailsWhenDisconnected)
{
    // Arrange
    mqttManager.disconnect();

    // Act
    bool result = mqttManager.publish(mqtt_telemetry_topic, "");

    // Assert
    assertFalse(result);
}

/**
 * @test Verifies that publishing a JSON-encoded payload behaves the same as
 * publishing any other string when the client is not connected.
 */
test(MqttManager_PublishJsonPayloadFailsWhenDisconnected)
{
    // Arrange
    mqttManager.disconnect();

    StaticJsonDocument<128> doc;
    doc["device_id"] = device_id;
    doc["value"] = 42;
    String payload;
    serializeJson(doc, payload);

    // Act
    bool result = mqttManager.publish(mqtt_telemetry_topic, payload);

    // Assert
    assertFalse(result);
}
