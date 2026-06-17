#include "SensorManager.h"
#include "../../config.h"
#include <Wire.h>

/**
 * @brief Shared SensorManager instance used by the main sketch and tests.
 */
SensorManager sensorManager;

SensorManager::SensorManager()
    : _initialized(false)
{
    // Initialize last data with zeros (or plausible initial values)
    _lastData.accelerometerX = 0.0f;
    _lastData.accelerometerY = 0.0f;
    _lastData.accelerometerZ = default_earth_gravity;
    _lastData.gyroscopeX = 0.0f;
    _lastData.gyroscopeY = 0.0f;
    _lastData.gyroscopeZ = 0.0f;
    _lastData.touchX = 0;
    _lastData.touchY = 0;
    _lastData.touchPressed = false;
    _lastData.batteryVoltage = 3.7f; // Set an average baseline so the EMA doesn't plummet to 0 initially
    _lastData.button = false;
}

void SensorManager::begin()
{
    // Initialize I2C
    Wire.begin(pin_i2c_sda, pin_i2c_scl);

    // 1. Initialize IMU (QMI8658)
    if (!_qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, pin_i2c_sda, pin_i2c_scl))
    {
        Serial.println("[-] QMI8658 IMU not found!");
    }
    else
    {
        Serial.println("[+] QMI8658 IMU successfully started.");
    }

    // 2. Touch-Controller (CST816X) reset and initialization
    pinMode(pin_touch_reset, OUTPUT);
    digitalWrite(pin_touch_reset, LOW);
    delay(20);
    digitalWrite(pin_touch_reset, HIGH);
    delay(50);

    if (!_touch.begin(Wire, CST816X_SLAVE_ADDRESS, pin_i2c_sda, pin_i2c_scl))
    {
        Serial.println("[-] CST816X Touch-Controller not found!");
    }
    else
    {
        Serial.println("[+] CST816X Touch successfully started.");
    }

    // 3. GPIO config for Button and Battery Array
    pinMode(pin_boot_button, INPUT_PULLUP);
    analogReadResolution(12);

    _initialized = true;
    Serial.println("Sensors initialized.");
}

SensorData SensorManager::read()
{
    SensorData rawData;

    // Current time formatted as an ISO 8601 UTC timestamp.
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    rawData.timestamp = String(timeStr);

    // Fetch IMU values (Accelerometer & Gyroscope)
    if (_qmi.isDataReady())
    {
        float acc[3], gyro[3];
        _qmi.getAccelerometer(acc[0], acc[1], acc[2]);
        _qmi.getGyroscope(gyro[0], gyro[1], gyro[2]);

        rawData.accelerometerX = acc[0];
        rawData.accelerometerY = acc[1];
        rawData.accelerometerZ = acc[2];
        rawData.gyroscopeX = gyro[0];
        rawData.gyroscopeY = gyro[1];
        rawData.gyroscopeZ = gyro[2];
    }
    else
    {
        // Fallback to recent filtered measurement components if IMU lags directly between calls
        rawData.accelerometerX = _lastData.accelerometerX;
        rawData.accelerometerY = _lastData.accelerometerY;
        rawData.accelerometerZ = _lastData.accelerometerZ;
        rawData.gyroscopeX = _lastData.gyroscopeX;
        rawData.gyroscopeY = _lastData.gyroscopeY;
        rawData.gyroscopeZ = _lastData.gyroscopeZ;
    }

    // Touch readout
    rawData.touchPressed = _touch.isPressed();
    if (rawData.touchPressed)
    {
        uint16_t x, y;
        _touch.getPoint(x, y);
        rawData.touchX = x;
        rawData.touchY = y;
    }
    else
    {
        rawData.touchX = 0;
        rawData.touchY = 0;
    }

    // Calculate Battery voltage (1:1 voltage divider, resulting in * battery_voltage_multiplier factor)
    int rawAdc = analogRead(pin_battery_adc);
    rawData.batteryVoltage = (rawAdc * adc_voltage_reference / adc_max_resolution) * battery_voltage_multiplier;

    // Boot button state readout
    rawData.button = (digitalRead(pin_boot_button) == LOW);

    // Apply deadzone and filters
    SensorData filteredData = applyDeadzone(rawData, _accDeadzoneThreshold, _gyroDeadzoneThreshold);
    filteredData = applyLowPassFilter(_lastData, filteredData, _emaAlpha);

    // Keep the new values as previous state for the next read
    _lastData = filteredData;

    return filteredData;
}

bool SensorManager::isInitialized() const
{
    return _initialized;
}

void SensorManager::setEmaAlpha(float alpha)
{
    _emaAlpha = alpha;
}

void SensorManager::setAccDeadzoneThreshold(float threshold)
{
    _accDeadzoneThreshold = threshold;
}

void SensorManager::setGyroDeadzoneThreshold(float threshold)
{
    _gyroDeadzoneThreshold = threshold;
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

    rawData.touchX = random(240);
    rawData.touchY = random(280);
    rawData.touchPressed = (random(10) > 8); // 10% chance

    float voltageRange = battery_max_voltage_mock - battery_min_voltage_mock;
    rawData.batteryVoltage = battery_min_voltage_mock + (random(voltageRange * 10) / 10.0f);
    rawData.button = false;

    // Apply deadzones and filters
    SensorData filteredData = applyDeadzone(rawData, _accDeadzoneThreshold, _gyroDeadzoneThreshold);
    filteredData = applyLowPassFilter(_lastData, filteredData, _emaAlpha);

    _lastData = filteredData;
    return filteredData;
}

SensorData SensorManager::applyLowPassFilter(const SensorData &last, const SensorData &current, float emaAlpha)
{
    SensorData result = current; // Preserve timestamp, buttons, etc.

    // EMA smoothing: EMA = alpha * new + (1 - alpha) * old
    result.accelerometerX = emaAlpha * current.accelerometerX + (1.0f - emaAlpha) * last.accelerometerX;
    result.accelerometerY = emaAlpha * current.accelerometerY + (1.0f - emaAlpha) * last.accelerometerY;
    result.accelerometerZ = emaAlpha * current.accelerometerZ + (1.0f - emaAlpha) * last.accelerometerZ;

    result.gyroscopeX = emaAlpha * current.gyroscopeX + (1.0f - emaAlpha) * last.gyroscopeX;
    result.gyroscopeY = emaAlpha * current.gyroscopeY + (1.0f - emaAlpha) * last.gyroscopeY;
    result.gyroscopeZ = emaAlpha * current.gyroscopeZ + (1.0f - emaAlpha) * last.gyroscopeZ;

    result.batteryVoltage = emaAlpha * current.batteryVoltage + (1.0f - emaAlpha) * last.batteryVoltage;

    return result;
}

SensorData SensorManager::applyDeadzone(const SensorData &data, float accThreshold, float gyroThreshold)
{
    SensorData result = data;

    // Default values if within deadzone (mostly 0.0, except Accel Z which assumes Earth gravity normally)
    if (fabs(result.accelerometerX) < accThreshold)
    {
        result.accelerometerX = 0.0f;
    }
    if (fabs(result.accelerometerY) < accThreshold)
    {
        result.accelerometerY = 0.0f;
    }
    // Acceleration Z should be evaluated towards Earth gravity in an upright state
    if (fabs(result.accelerometerZ - default_earth_gravity) < accThreshold)
    {
        result.accelerometerZ = default_earth_gravity;
    }

    if (fabs(result.gyroscopeX) < gyroThreshold)
    {
        result.gyroscopeX = 0.0f;
    }
    if (fabs(result.gyroscopeY) < gyroThreshold)
    {
        result.gyroscopeY = 0.0f;
    }
    if (fabs(result.gyroscopeZ) < gyroThreshold)
    {
        result.gyroscopeZ = 0.0f;
    }

    return result;
}