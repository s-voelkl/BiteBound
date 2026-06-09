#include "WifiConnection.h"
#include "wifi_mqtt_secrets.h"
#include <AUnit.h>

test(WifiConnection_connectToWiFi_connectsToWiFiAndReturnsTrue)
{
    // Act: Call the connectToWiFi function
    bool result = connectToWiFi();

    // Assert: Verify that the connection got established successfully
    assertTrue(result);
}