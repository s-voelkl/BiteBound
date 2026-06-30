#ifndef COMMAND_MSG_H
#define COMMAND_MSG_H

#include "CommandType.h"

/**
 * @brief Representation of an incoming dashboard command.
 * Contains configuration parameters and a unique request ID for tracking.
 */
struct CommandMsg
{
    CommandType type;
    char playerName[32];
    int gameId;
    int cookiesCount;
    int wallThicknessPx;
    float imuSensitivity;
    float bounceRestitution;
    float emaAlpha;
    float deadzoneThreshold;
    char requestId[36];
};

#endif // COMMAND_MSG_H
