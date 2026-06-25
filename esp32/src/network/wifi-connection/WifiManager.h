#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

/**
 * @brief Manages the device's WiFi connection lifecycle.
 *
 * Wraps the underlying WiFi driver and exposes a small, testable API for
 * connecting to a configured access point and querying connection state.
 * Credentials are supplied through the constructor so the class can be
 * unit-tested independently of the production configuration.
 */
class WifiManager
{
public:
    /**
     * @brief Constructs a WifiManager with the given credentials.
     *
     * @param ssid     Network SSID. Must remain valid for the lifetime of this instance.
     * @param password Network password. Must remain valid for the lifetime of this instance.
     */
    WifiManager(const char *ssid, const char *password);

    /**
     * @brief Establishes a connection to the configured WiFi network.
     *
     * Blocks until the connection is established. A stable WiFi connection is
     * required for the rest of the device to operate, so blocking here is
     * acceptable.
     *
     * @return true once the connection has been established.
     *
     * @warning If the credentials are incorrect or the network is unreachable
     * the call will block indefinitely.
     */
    bool connect();

    /**
     * @brief Indicates whether the device currently holds a WiFi connection.
     *
     * @return true if connected, false otherwise.
     */
    bool isConnected() const;

    /**
     * @brief Returns the IP address currently assigned by the access point.
     *
     * @return The local IP address as a string, or an empty string when not connected.
     */
    String localIP() const;

    /**
     * @brief Returns the SSID of the currently connected WiFi network.
     *
     * @return The SSID as a string, or an empty string when not connected.
     */
    String getSSID() const;

private:
    const char *_ssid;
    const char *_password;
};

/**
 * @brief Shared WifiManager instance configured from wifi_mqtt_secrets.h.
 *
 * Exposed so the main sketch and tests can drive WiFi connectivity through a
 * single, well-known entry point.
 */
extern WifiManager wifiManager;

#endif // WIFI_MANAGER_H