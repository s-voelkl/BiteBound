#ifndef SENSOR_READER_H
#define SENSOR_READER_H

#include <Arduino.h>
#include <DHT.h>
#include <ArduinoJson.h>

/**
 * @brief Struct to hold all sensor data together for easy access and transmission.
 * This allows to keep the sensor reading logic separate from the data handling logic.
 *
 * @note The specific sensors and their data types can be adjusted based on the actual sensors used in the project.
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
};

/**
 * @brief Initialize all sensors.
 * This function sets up the pin modes and initializes any sensor-specific settings.
 *
 * @note Depending on the sensors used, additional initialization steps may be required (e.g., calibration).
 */
void initSensors();

/**
 * @brief Read all sensor values.
 * This function reads the current values from all sensors and returns them in a SensorData struct.
 *
 * @return SensorData struct containing the latest readings from all sensors.
 *
 * @note The function assumes that the sensors are properly initialized and that the pin modes are set correctly.
 */
SensorData readSensors();

#endif // SENSOR_READER_H