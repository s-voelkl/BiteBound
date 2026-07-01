#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <Arduino.h>
#include "../shared/CommandMsg.h"

/**
 * @brief Utility for decoding JSON payloads received via network channels.
 */
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

    /**
     * @brief Parses raw binary payload bytes directly into a CommandMsg.
     *
     * Converts raw bytes safely to preserve system memory boundaries before parsing.
     *
     * @param payload Pointer to raw byte buffer.
     * @param length Payload length in bytes.
     * @param outMsg Destination message structure.
     * @return true if payload is parsed and validated successfully.
     */
    static bool parse(const byte *payload, unsigned int length, CommandMsg &outMsg);
};

#endif // COMMAND_PARSER_H