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
 * snapshot with finite numeric values and a non-empty timestamp.
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
}
