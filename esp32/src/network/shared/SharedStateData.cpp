#include "SharedStateData.h"
#include "../../../config.h"

void initSharedState(SharedStateData &state)
{
    memset(&state, 0, sizeof(SharedStateData));
    strncpy(state.playerName, default_player_name, sizeof(state.playerName) - 1);
    state.gameId = default_game_id;
    state.cookiesCount = default_cookies_count;
    state.wallThicknessPx = default_wall_thickness_px;
    state.imuSensitivity = default_imu_sensitivity_multiplier;
    state.bounceRestitution = default_bounce_restitution;
    state.emaAlpha = default_ema_alpha;
    state.deadzoneThreshold = default_deadzone_threshold;
    state.isRunning = false;
}
