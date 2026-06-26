#include <AUnit.h>
#include "SharedState.h"
#include "../../../config.h"

test(SharedState_InitDefaults)
{
    SharedStateData state;
    initSharedState(state);

    assertEqual(state.gameId, default_game_id);
    assertEqual(state.cookiesCount, default_cookies_count);
    assertEqual(state.wallThicknessPx, default_wall_thickness_px);
    assertEqual(state.imuSensitivity, default_imu_sensitivity);
    assertEqual(state.bounceRestitution, default_bounce_restitution);
    assertEqual(state.emaAlpha, default_ema_alpha);
    assertEqual(state.deadzoneThreshold, default_deadzone_threshold);
    assertFalse(state.isRunning);
    assertEqual(strcmp(state.playerName, default_player_name), 0);
}

test(SharedState_MutexLockSuccess)
{
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    assertNotEqual(mutex, (SemaphoreHandle_t) nullptr);

    {
        MutexLock lock(mutex);
        assertTrue(lock.isLocked());
    } // Mutex auto-released here

    vSemaphoreDelete(mutex);
}