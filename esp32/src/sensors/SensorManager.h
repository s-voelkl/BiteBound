#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SensorQMI8658.hpp>
#include <SensorCST816x.hpp>
#include "../../config.h"

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
    uint16_t touchX;
    uint16_t touchY;
    bool touchPressed;
    float batteryVoltage;
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
     * @brief Mock implementation of read() for testing purposes.
     *
     * @return Random/Mocked SensorData snapshot after applying filters.
     */
    SensorData readMock();

    /**
     * @brief Applies Exponential Moving Average (EMA) low-pass filter.
     */
    SensorData applyLowPassFilter(const SensorData &last, const SensorData &current, float emaAlpha);

    /**
     * @brief Nullifies values below minimum thresholds to avoid sensor noise.
     * Default for acceleration Z is Earth gravity.
     */
    SensorData applyDeadzone(const SensorData &data, float accThreshold, float gyroThreshold);

    /**
     * @brief Indicates whether begin() has completed successfully.
     *
     * @return true once the sensors have been initialised, false otherwise.
     */
    bool isInitialized() const;

    /**
     * @brief Set the EMA Alpha value for the low-pass filter
     */
    void setEmaAlpha(float alpha);

    /**
     * @brief Set the accelerometer deadzone threshold
     */
    void setAccDeadzoneThreshold(float threshold);

    /**
     * @brief Set the gyroscope deadzone threshold
     */
    void setGyroDeadzoneThreshold(float threshold);

private:
    bool _initialized;
    SensorData _lastData;

    // Hardware components
    SensorQMI8658 _qmi;
    SensorCST816x _touch;

    // Configuration parameters for the filters
    float _emaAlpha = default_ema_alpha;
    float _accDeadzoneThreshold = default_acceleration_deadzone_threshold;
    float _gyroDeadzoneThreshold = default_gyro_deadzone_threshold;
};

extern SensorManager sensorManager;

#endif // SENSOR_MANAGER_H
