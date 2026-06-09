#include "../SensorHelper.h"
#include "../../config.h"

#include <ArduinoJson.h>
#include <AUnit.h>

test(SensorHelper_readSensorsMock_returnsValidData)
{
    // Act: Calling readSensors() directly to ensure it cleanly processes and returns a SensorData struct.
    SensorData data = readSensors();

    // Assert: Verify timestamp string is correctly formatted/populated
    assertNotEqual(data.timestamp.length(), (unsigned int)0);

    // Assert: accelerometer and gyroscope values should be floats
    assertTrue(isfinite(data.accelerometerX));
    assertTrue(isfinite(data.accelerometerY));
    assertTrue(isfinite(data.accelerometerZ));

    assertTrue(isfinite(data.gyroscopeX));
    assertTrue(isfinite(data.gyroscopeY));
    assertTrue(isfinite(data.gyroscopeZ));
}