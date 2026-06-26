#ifndef GAME_STATE_H
#define GAME_STATE_H

/**
 * @struct GameState
 * @brief Holds volatile metadata used for rendering UI dashboard text.
 */
struct GameState
{
    const char *status = "idle"; // "idle", "running", "completed"
    int cookiesCollected = 0;
    int cookiesRemaining = 10;
    int currentRound = 1;
    float elapsedTimeSec = 0.0f;
};

#endif // GAME_STATE_H