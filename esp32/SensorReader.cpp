#include "SensorReader.h"
#include "config.h"

// define pins: none

/**
 * @brief Initialize all sensors.
 * This function sets up the pin modes and initializes any sensor-specific settings.
 *
 * @note Depending on the sensors used, additional initialization steps may be required (e.g., calibration).
 */
void initSensors()
{
    // pinMode(PIN, INPUT);

    Serial.println("Sensors initialized.");
}

/**
 * @brief Read all sensor values.
 * This function reads the current values from all sensors and returns them in a SensorData struct.
 *
 * @return SensorData struct containing the latest readings from all sensors.
 *
 * @note The function assumes that the sensors are properly initialized and that the pin modes are set correctly.
 */
SensorData readSensors()
{
    SensorData data;

    // current time as ISO 8601 string
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    data.timestamp = String(timeStr);

    // accelerometer
    data.accelerometerX = 0.0;
    data.accelerometerY = 0.0;
    data.accelerometerZ = 0.0;

    // gyroscope
    data.gyroscopeX = 0.0;
    data.gyroscopeY = 0.0;
    data.gyroscopeZ = 0.0;

    return data;
}

// see README.md and config.h:
// TODO: EMA for smoothing
// TODO: threshold reduction with dead zones