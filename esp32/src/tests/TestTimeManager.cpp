#include "../TimeManager.h"
#include "../WifiManager.h"
#include "../../config.h"

#include <time.h>
#include <WiFi.h>
#include <AUnit.h>

/**
 * @test Verifies that TimeManager::sync() advances the system clock past the
 * configured plausibility threshold and reports the helper as synchronised.
 */
test(TimeManager_sync_synchronisesSystemClock)
{
    // Arrange: ensure WiFi is connected before attempting NTP synchronisation.
    if (!wifiManager.isConnected())
    {
        wifiManager.connect();
    }

    // Act
    timeManager.sync();

    // Assert: the system clock should be past the configured threshold.
    assertTrue(timeManager.isSynchronized());
    assertTrue(timeManager.now() >= static_cast<time_t>(time_epoch_2020_Jan_1));
}
