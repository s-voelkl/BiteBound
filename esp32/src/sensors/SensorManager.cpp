#include "SensorManager.h"
#include "../../config.h"

/**
 * @brief Shared SensorManager instance used by the main sketch and tests.
 */
SensorManager sensorManager;

SensorManager::SensorManager()
    : _initialized(false)
{
}

void SensorManager::begin()
{
    // pinMode(PIN, INPUT);

    _initialized = true;
    Serial.println("Sensors initialized.");
}

SensorData SensorManager::read()
{
    SensorData data;

    // Current time formatted as an ISO 8601 UTC timestamp.
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    data.timestamp = String(timeStr);

    // Accelerometer (placeholder values until the hardware is wired in).
    data.accelerometerX = 0.0f;
    data.accelerometerY = 0.0f;
    data.accelerometerZ = 0.0f;

    // Gyroscope (placeholder values until the hardware is wired in).
    data.gyroscopeX = 0.0f;
    data.gyroscopeY = 0.0f;
    data.gyroscopeZ = 0.0f;

    data.button = false;

    return data;
}

bool SensorManager::isInitialized() const
{
    return _initialized;
}

// see README.md and config.h:
// TODO: EMA for smoothing
// TODO: threshold reduction with dead zones
