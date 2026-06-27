#ifndef COMMAND_TYPE_H
#define COMMAND_TYPE_H

/**
 * @brief Enum representing the decoded command type.
 *
 * START: Start the game with the provided parameters.
 * STOP: Stop the game and reset the state.
 * PARAM_CHANGE: Update game parameters without starting or stopping the game. This is only optional for later use.
 * UNKNOWN: Unknown command type.
 */
enum class CommandType
{
    START,
    STOP,
    PARAM_CHANGE,
    UNKNOWN
};

#endif // COMMAND_TYPE_H
