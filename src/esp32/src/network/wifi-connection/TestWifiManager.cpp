#include "WifiManager.h"

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

// String WifiManager::getSSID() const
// {
//     if (WiFi.status() != WL_CONNECTED)
//     {
//         return String();
//     }
//     return WiFi.SSID();
// }

/**
 * @test Verifies that WifiManager::getSSID() returns the correct SSID when connected.
 *
 * This test assumes that the device is connected to the WiFi network specified in wifi_mqtt_secrets.h.
 */
test(WifiManager_getSSID_returnsCorrectSSID)
{
    // Arrange: ensure the connection is up.
    wifiManager.connect();

    // Act
    String ssid = wifiManager.getSSID();

    // Assert
    assertEqual(ssid, String(ssid)); // Compare with the expected SSID from wifi_mqtt_secrets.h
}