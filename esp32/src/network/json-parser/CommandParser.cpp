#include "CommandParser.h"
#include "../../../config.h"
#include <ArduinoJson.h>

bool CommandParser::parse(const String &jsonPayload, CommandMsg &outMsg)
{
    memset(&outMsg, 0, sizeof(CommandMsg));
    outMsg.type = CommandType::UNKNOWN;

    // Use a fixed size static buffer to minimize dynamic heap fragmentation
    StaticJsonDocument<768> doc;
    DeserializationError error = deserializeJson(doc, jsonPayload);

    if (error)
    {
        Serial.print(F("JSON Deserialization Failure: "));
        Serial.println(error.f_str());
        return false;
    }

    const char *commandStr = doc["command"];
    if (!commandStr)
    {
        return false;
    }

    // Assignment of command type based on the string value
    // "start" --> CommandType::START
    // "stop" --> CommandType::STOP
    // "resume" --> CommandType::RESUME
    // "param_change" --> CommandType::PARAM_CHANGE
    // else --> CommandType::UNKNOWN
    if (strcmp(commandStr, "start") == 0)
    {
        outMsg.type = CommandType::START;
    }
    else if (strcmp(commandStr, "stop") == 0)
    {
        outMsg.type = CommandType::STOP;
    }
    else if (strcmp(commandStr, "resume") == 0)
    {
        outMsg.type = CommandType::RESUME;
    }
    else if (strcmp(commandStr, "param_change") == 0)
    {
        outMsg.type = CommandType::PARAM_CHANGE;
    }
    else
    {
        outMsg.type = CommandType::UNKNOWN;
    }

    // Request ID for identifying the request.
    const char *reqId = doc["meta"]["request_id"];
    if (reqId)
    {
        strncpy(outMsg.requestId, reqId, sizeof(outMsg.requestId) - 1);
    }

    // Player name for identifying the player.
    const char *playerName = doc["player"]["name"];
    if (playerName)
    {
        strncpy(outMsg.playerName, playerName, sizeof(outMsg.playerName) - 1);
    }
    else
    {
        strncpy(outMsg.playerName, default_player_name, sizeof(outMsg.playerName) - 1);
    }

    outMsg.gameId = doc["game"]["game_id"] | default_game_id;
    outMsg.cookiesCount = doc["parameters"]["cookies_count"] | default_cookies_count;
    outMsg.wallThicknessPx = doc["parameters"]["wall_thickness_px"] | default_wall_thickness_px;

    outMsg.imuSensitivity = doc["physics"]["imu_sensitivity_multiplier"] | default_imu_sensitivity_multiplier;
    outMsg.bounceRestitution = doc["physics"]["bounce_restitution"] | default_bounce_restitution;
    outMsg.emaAlpha = doc["physics"]["ema_alpha"] | default_ema_alpha;
    outMsg.deadzoneThreshold = doc["physics"]["deadzone_threshold"] | default_deadzone_threshold;

    return true;
}

bool CommandParser::parse(const byte *payload, unsigned int length, CommandMsg &outMsg)
{
    if (payload == nullptr || length == 0)
    {
        return false;
    }

    String jsonStr;
    jsonStr.reserve(length + 1);
    for (unsigned int i = 0; i < length; ++i)
    {
        jsonStr += (char)payload[i];
    }

    return parse(jsonStr, outMsg);
}