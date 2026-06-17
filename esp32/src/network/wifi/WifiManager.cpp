#include "WifiManager.h"
#include "../../../wifi_mqtt_secrets.h"

#include <WiFi.h>

/**
 * @brief Shared WifiManager instance configured from wifi_mqtt_secrets.h.
 */
WifiManager wifiManager(ssid, password);

WifiManager::WifiManager(const char *ssid, const char *password)
    : _ssid(ssid), _password(password)
{
}

bool WifiManager::connect()
{
    Serial.println("Connecting to WiFi: " + String(_ssid));

    WiFi.begin(_ssid, _password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());
    return true;
}

bool WifiManager::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

String WifiManager::localIP() const
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return String();
    }
    return WiFi.localIP().toString();
}
