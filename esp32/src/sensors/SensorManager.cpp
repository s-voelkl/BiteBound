#include "SensorManager.h"
#include "../../config.h"
#include <Wire.h>

/**
 * @brief Shared SensorManager instance used by the main sketch and tests.
 */
SensorManager sensorManager;

SensorManager::SensorManager()
    : _initialized(false),
      _imuAvailable(false)
{
    // Initialize last-known values for stable fallback payloads.
    _lastData.accelerometerX = 0.0f;
    _lastData.accelerometerY = 0.0f;
    _lastData.accelerometerZ = default_earth_gravity;
    _lastData.gyroscopeX = 0.0f;
    _lastData.gyroscopeY = 0.0f;
    _lastData.gyroscopeZ = 0.0f;
    _lastData.batteryVoltage = 3.7f;
    _lastData.button = false;
}

void SensorManager::begin()
{
    if (_initialized)
    {
        // Prevent accidental reconfiguration of I2C/IMU if begin() is called twice.
        Serial.println("[-] SensorManager already initialized, skipping begin().");
        return;
    }

    // Initialize I2C
    Wire.begin(pin_i2c_sda, pin_i2c_scl);

    // Initialize and configure the QMI8658 using the same sequence as the vendor sample.
    if (!_qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, pin_i2c_sda, pin_i2c_scl))
    // if (!_qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, IIC_SDA, IIC_SCL))
    {
        Serial.println("[-] QMI8658 IMU not found!");
        _imuAvailable = false;
    }
    else
    {
        Serial.println("[+] QMI8658 IMU successfully started.");

        _qmi.configAccelerometer(
            SensorQMI8658::ACC_RANGE_4G,
            SensorQMI8658::ACC_ODR_1000Hz,
            SensorQMI8658::LPF_MODE_0);

        _qmi.configGyroscope(
            SensorQMI8658::GYR_RANGE_256DPS,
            SensorQMI8658::GYR_ODR_896_8Hz,
            SensorQMI8658::LPF_MODE_3);

        _qmi.enableGyroscope();
        _qmi.enableAccelerometer();

        _imuAvailable = true;
    }

    // GPIO configuration for button and battery sensing remains valid regardless of IMU state.
    pinMode(pin_boot_button, INPUT_PULLUP);
    analogReadResolution(12);

    _initialized = true;
    Serial.println(_imuAvailable ? "Sensors initialized." : "Sensors initialized (IMU unavailable, using fallback values).");
}

SensorData SensorManager::read()
{
    // Start from the previous sample so failed or missing IMU reads still produce a stable payload.
    SensorData rawData = _lastData;

    // Current time formatted as an ISO 8601 UTC timestamp.
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    rawData.timestamp = String(timeStr);

    // Fetch IMU values (accelerometer + gyroscope) only when the driver reports fresh data.
    Serial.print("IMU data ready: ");
    Serial.println(_imuAvailable && _qmi.getDataReady() ? "YES" : "NO");
    if (_imuAvailable && _qmi.getDataReady())
    {
        float accX = rawData.accelerometerX;
        float accY = rawData.accelerometerY;
        float accZ = rawData.accelerometerZ;

        float gyrX = rawData.gyroscopeX;
        float gyrY = rawData.gyroscopeY;
        float gyrZ = rawData.gyroscopeZ;

        if (_qmi.getAccelerometer(accX, accY, accZ))
        {
            Serial.print("Accelerometer: ");
            Serial.print("X=");
            Serial.print(accX);
            Serial.print(" Y=");
            Serial.print(accY);
            Serial.print(" Z=");
            Serial.println(accZ);

            rawData.accelerometerX = accX;
            rawData.accelerometerY = accY;
            rawData.accelerometerZ = accZ;
        }

        if (_qmi.getGyroscope(gyrX, gyrY, gyrZ))
        {
            Serial.print("Gyroscope: ");
            Serial.print("X=");
            Serial.print(gyrX);
            Serial.print(" Y=");
            Serial.print(gyrY);
            Serial.print(" Z=");
            Serial.println(gyrZ);

            rawData.gyroscopeX = gyrX;
            rawData.gyroscopeY = gyrY;
            rawData.gyroscopeZ = gyrZ;
        }
    }

    // Calculate Battery voltage (1:1 voltage divider, resulting in * battery_voltage_multiplier factor)
    int rawAdc = analogRead(pin_battery_adc);
    rawData.batteryVoltage = (rawAdc * adc_voltage_reference / adc_max_resolution) * battery_voltage_multiplier;

    // Boot button state readout
    rawData.button = (digitalRead(pin_boot_button) == LOW);

    // Keep the new values as previous state for the next read.
    _lastData = rawData;

    return rawData;
}

bool SensorManager::isInitialized() const
{
    return _initialized;
}

SensorData SensorManager::readMock()
{
    SensorData rawData;

    // Current time formatted as an ISO 8601 UTC timestamp.
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    rawData.timestamp = String(timeStr);

    // Generate random values for testing
    rawData.accelerometerX = ((random(200) / 100.0f) - 1.0f); // -1.0 to 1.0
    rawData.accelerometerY = ((random(200) / 100.0f) - 1.0f);
    rawData.accelerometerZ = default_earth_gravity + ((random(100) / 100.0f) - 0.5f); // ±0.5 from gravity

    rawData.gyroscopeX = (random(1000) / 10.0f) - 50.0f; // -50 to 50
    rawData.gyroscopeY = (random(1000) / 10.0f) - 50.0f;
    rawData.gyroscopeZ = (random(1000) / 10.0f) - 50.0f;

    float voltageRange = battery_max_voltage_mock - battery_min_voltage_mock;
    rawData.batteryVoltage = battery_min_voltage_mock + (random(voltageRange * 10) / 10.0f);
    rawData.button = false;

    _lastData = rawData;
    return rawData;
}