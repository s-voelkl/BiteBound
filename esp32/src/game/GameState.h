#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "RunningStatus.h"
#include "../../config.h"

/**
 * @struct GameState
 * @brief Holds volatile metadata used for rendering UI dashboard text.
 */
struct GameState
{
    RunningStatus runningStatus = RunningStatus::IDLE;
    int cookiesCollected = 0;
    int cookiesRemaining = default_cookies_count;
    int currentRound = 1;
    float elapsedTimeSec = 0.0f;
};

#endif // GAME_STATE_H