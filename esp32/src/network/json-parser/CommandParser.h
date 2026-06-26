#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <Arduino.h>
#include "../shared/CommandMsg.h"

class CommandParser
{
public:
    /**
     * @brief Parses JSON string input command payloads into a CommandMsg.
     * @param jsonPayload Raw string input payload received from MQTT.
     * @param outMsg Output CommandMsg struct parameter.
     * @return true if string deserialization and schema matches.
     */
    static bool parse(const String &jsonPayload, CommandMsg &outMsg);
};

#endif // COMMAND_PARSER_H