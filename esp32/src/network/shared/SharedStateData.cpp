#include "SharedStateData.h"
#include "../../../config.h"
#include "src/game/RunningStatus.h"

void initSharedState(SharedStateData &state)
{
    memset(&state, 0, sizeof(SharedStateData));

    state.gameId = default_game_id;

    // Strings must be copied with strncpy into character arrays using strncpy.
    strncpy(state.playerName, default_player_name, sizeof(state.playerName) - 1);

    state.playerName[sizeof(state.playerName) - 1] = '\0';
    state.cookiesCount = default_cookies_count;
    state.wallThicknessPx = default_wall_thickness_px;
    state.imuSensitivity = default_imu_sensitivity_multiplier;
    state.bounceRestitution = default_bounce_restitution;
    state.emaAlpha = default_ema_alpha;
    state.deadzoneThreshold = default_deadzone_threshold;
    state.runningStatus = RunningStatus::IDLE;
}
