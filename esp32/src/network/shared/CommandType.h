#ifndef COMMAND_TYPE_H
#define COMMAND_TYPE_H

/**
 * @brief Enum representing the decoded command type.
 *
 * START: Start the game with the provided parameters.
 * STOP: Stop (pause) the game, keeping the current progress.
 * RESUME: Continue a paused game right where it left off.
 * PARAM_CHANGE: Update game parameters without starting or stopping the game. This is only optional for later use.
 * UNKNOWN: Unknown command type.
 */
enum class CommandType
{
    START,
    STOP,
    RESUME,
    PARAM_CHANGE,
    UNKNOWN
};

#endif // COMMAND_TYPE_H
