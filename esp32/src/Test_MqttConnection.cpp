#include "../config.h"
#include < ArduinoJson.h>
#include "MqttConnection.h"
#include <AsyncMqtt_Generic.h>
#include <AUnit.h>

extern AsyncMqttClient mqttClient;

test(MqttConnection_PublishFailsWhenDisconnected)
{
    // Arrange: ensure client is disconnected
    // Act
    bool result = publishMQTTData("test_payload");

    // Assert
    assertFalse(result);
}
