#ifndef SHARED_STATE_DATA_H
#define SHARED_STATE_DATA_H

#include <Arduino.h>

/**
 * @brief Synchronized game state and configurations accessed by both CPU cores.
 * This structure is shared between the game logic and the network communication tasks.
 */
struct SharedStateData
{
    // Identity & parameters
    char playerName[32];
    int gameId;
    int cookiesCount;
    int wallThicknessPx;

    // Engine parameter configs
    float imuSensitivity;
    float bounceRestitution;
    float emaAlpha;
    float deadzoneThreshold;

    // Simulation outputs
    float ballPosX;
    float ballPosY;
    float velocityX;
    float velocityY;
    float accX;
    float accY;
    int cookiesCollected;
    int cookiesRemaining;
    int currentRound;
    float elapsedTimeSec;
    bool isRunning;
};

/**
 * @brief Initializer to safely set system defaults on start.
 * This function zeroes out the shared state and sets default values for all fields.
 */
void initSharedState(SharedStateData &state);

#endif // SHARED_STATE_DATA_H
