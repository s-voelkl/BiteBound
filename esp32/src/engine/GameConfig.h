#ifndef ENGINE_GAME_CONFIG_H
#define ENGINE_GAME_CONFIG_H

#include <stdint.h>
#include "../physics/core/PhysicsEngine.h" // PhysicsParams
#include "../../config.h"

/**
 * @brief Transport-agnostic game configuration applied by the engine.
 *
 * Filled from an MQTT command (or defaults).
 */
struct GameConfig
{
    uint8_t gameId = 1;                                  ///< 1 = Labyrinth, 2 = Flatland.
    uint16_t targetCookies = default_cookies_count;      ///< Score needed to win a round.
    uint8_t visibleCookies = default_max_visible_cookies; ///< k cookies on screen at once.
    uint8_t wallThicknessPx = default_wall_thickness_px;  ///< Maze block size (Game 1).
    PhysicsParams physics;                               ///< Tunable physics parameters.
};

#endif // ENGINE_GAME_CONFIG_H
