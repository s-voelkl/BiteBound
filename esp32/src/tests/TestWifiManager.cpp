#include "../WifiManager.h"

#include <AUnit.h>

/**
 * @test Verifies that WifiManager::connect() returns true once a WiFi
 * connection has been established using the credentials from
 * wifi_mqtt_secrets.h.
 */
test(WifiManager_connect_returnsTrueOnSuccess)
{
    // Act
    bool result = wifiManager.connect();

    // Assert
    assertTrue(result);
    assertTrue(wifiManager.isConnected());
}

/**
 * @test Verifies that WifiManager::localIP() returns a non-empty string once
 * the device is connected to the network.
 */
test(WifiManager_localIP_returnsAddressWhenConnected)
{
    // Arrange: ensure the connection is up.
    wifiManager.connect();

    // Act
    String ip = wifiManager.localIP();

    // Assert
    assertMore(ip.length(), (unsigned int)0);
}
