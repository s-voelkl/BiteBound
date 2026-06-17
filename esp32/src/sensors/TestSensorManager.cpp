#include "SensorManager.h"
#include "../../config.h"

#include <ArduinoJson.h>
#include <AUnit.h>

/**
 * @test Verifies that SensorManager::begin() marks the reader as initialised.
 */
test(SensorManager_begin_marksReaderInitialised)
{
    // Act
    sensorManager.begin();

    // Assert
    assertTrue(sensorManager.isInitialized());
}

/**
 * @test Verifies that SensorManager::read() returns a populated SensorData
 * snapshot with finite numeric values, extended types and a non-empty timestamp.
 */
test(SensorManager_read_returnsValidData)
{
    // Arrange
    sensorManager.begin();

    // Act: read() must cleanly process and return a SensorData snapshot.
    SensorData data = sensorManager.read();

    // Assert: the timestamp string should be populated.
    assertNotEqual(data.timestamp.length(), (unsigned int)0);

    // Assert: accelerometer and gyroscope values should be finite floats.
    assertTrue(isfinite(data.accelerometerX));
    assertTrue(isfinite(data.accelerometerY));
    assertTrue(isfinite(data.accelerometerZ));

    assertTrue(isfinite(data.gyroscopeX));
    assertTrue(isfinite(data.gyroscopeY));
    assertTrue(isfinite(data.gyroscopeZ));

    // Assert: Extended properties exist
    assertEqual(data.touchX, (uint16_t)0);
    assertEqual(data.touchY, (uint16_t)0);
    assertFalse(data.touchPressed);
    assertTrue(data.batteryVoltage > 0.0f);
}

/**
 * @test Verifies that deadzone correctly filters small variations
 * and preserves distinct movements outside the threshold.
 */
test(SensorManager_applyDeadzone)
{
    SensorData testData;
    // Values strictly inside the deadzone
    testData.accelerometerX = 0.03f;
    testData.accelerometerY = -0.04f;
    testData.accelerometerZ = default_earth_gravity - 0.02f;
    testData.gyroscopeX = 1.0f;
    testData.gyroscopeY = -1.5f;

    SensorData result = sensorManager.applyDeadzone(testData, 0.05f, 2.0f);

    assertEqual(result.accelerometerX, 0.0f);
    assertEqual(result.accelerometerY, 0.0f);
    assertEqual(result.accelerometerZ, default_earth_gravity);
    assertEqual(result.gyroscopeX, 0.0f);
    assertEqual(result.gyroscopeY, 0.0f);

    // Values outside the deadzone
    testData.accelerometerX = 0.1f;
    testData.accelerometerY = -0.1f;
    testData.accelerometerZ = default_earth_gravity + 0.5f;
    testData.gyroscopeX = 3.0f;

    result = sensorManager.applyDeadzone(testData, 0.05f, 2.0f);

    assertEqual(result.accelerometerX, 0.1f);
    assertEqual(result.accelerometerY, -0.1f);
    assertEqual(result.accelerometerZ, default_earth_gravity + 0.5f);
    assertEqual(result.gyroscopeX, 3.0f);
}

/**
 * @test Verifies that the EMA blends old and new measurements correctly
 * and leaves un-filtered properties untouched.
 */
test(SensorManager_applyLowPassFilter)
{
    SensorData lastData;
    lastData.accelerometerX = 0.0f;
    lastData.gyroscopeY = 10.0f;
    lastData.touchX = 50;

    SensorData currentData;
    currentData.accelerometerX = 1.0f;
    currentData.gyroscopeY = 20.0f;
    currentData.touchX = 150;

    // alpha = 0.2 means: 0.2 * new + 0.8 * old
    SensorData result = sensorManager.applyLowPassFilter(lastData, currentData, 0.2f);

    // Filtered vars
    assertTrue(fabs(result.accelerometerX - 0.2f) < 0.001f);
    assertTrue(fabs(result.gyroscopeY - 12.0f) < 0.001f);

    // Un-filtered vars should mirror currentData directly (unless another process added it to the EMA filter)
    assertEqual(result.touchX, 150);
}

/**
 * @test Verifies that readMock() generates values within expected ranges and applies filters.
 */
test(SensorManager_readMock_returnsValidData)
{
    sensorManager.begin();
    SensorData data = sensorManager.readMock();

    assertNotEqual(data.timestamp.length(), (unsigned int)0);
    assertTrue(isfinite(data.accelerometerX));
    assertTrue(isfinite(data.accelerometerZ));

    // Battery must fall around the mocked valid ranges
    assertTrue(data.batteryVoltage >= battery_min_voltage_mock && data.batteryVoltage <= battery_max_voltage_mock);

    // Touch screens are restricted bounds
    assertTrue(data.touchX >= 0 && data.touchX <= display_width);
    assertTrue(data.touchY >= 0 && data.touchY <= display_height);
}
