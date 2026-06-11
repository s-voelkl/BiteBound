#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * @brief Aggregates a single snapshot of all sensor readings.
 *
 * Keeping the raw values in a dedicated struct decouples the sensor acquisition
 * code from the consumers that serialise or evaluate the data.
 *
 * @note The exact set of fields can be adjusted as additional sensors are
 * integrated into the project.
 */
struct SensorData
{
    String timestamp;
    float accelerometerX;
    float accelerometerY;
    float accelerometerZ;
    float gyroscopeX;
    float gyroscopeY;
    float gyroscopeZ;
    bool button;
};

/**
 * @brief Owns the sensor peripherals and produces SensorData snapshots.
 *
 * Encapsulates the initialisation and read paths for all on-board sensors so
 * that the rest of the application can interact with sensor data through a
 * single, testable component.
 */
class SensorManager
{
public:
    /**
     * @brief Constructs a SensorManager. No hardware is touched until begin() is called.
     */
    SensorManager();

    /**
     * @brief Initialises the sensor peripherals.
     *
     * Configures pin modes and performs any sensor-specific setup
     * (for example calibration) required before the first read().
     */
    void begin();

    /**
     * @brief Reads the current value of every sensor.
     *
     * @return SensorData snapshot containing the latest readings.
     *
     * @note begin() must have been invoked before the first call.
     */
    SensorData read();

    /**
     * @brief Indicates whether begin() has completed successfully.
     *
     * @return true once the sensors have been initialised, false otherwise.
     */
    bool isInitialized() const;

private:
    bool _initialized;
};

/**
 * @brief Shared SensorManager instance used by the main sketch and tests.
 */
extern SensorManager sensorManager;

#endif // SENSOR_MANAGER_H
