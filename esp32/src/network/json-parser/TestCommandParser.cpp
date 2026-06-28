#include <AUnit.h>
#include "CommandParser.h"
#include "../../../config.h"

test(CommandParser_ParseStartSuccess)
{
    String payload = String("{") +
                     "\"command\":\"start\"," +
                     "\"meta\":{\"source_ui\":\"NODE_RED\",\"request_id\":\"id1\"," +
                     "\"timestamp\":\"2026-06-26T11:50:00Z\"}," +
                     "\"player\":{\"name\":\"" + default_player_name + "\"}," +
                     "\"game\":{\"game_id\":" + String(default_game_id) + "}," +
                     "\"parameters\":{" +
                     "\"cookies_count\":" + String(default_cookies_count) + "," +
                     "\"wall_thickness_px\":" + String(default_wall_thickness_px) +
                     "}," +
                     "\"physics\":{" +
                     "\"imu_sensitivity_multiplier\":" + String(default_imu_sensitivity_multiplier) + "," +
                     "\"bounce_restitution\":" + String(default_bounce_restitution) + "," +
                     "\"ema_alpha\":" + String(default_ema_alpha) + "," +
                     "\"deadzone_threshold\":" + String(default_deadzone_threshold) +
                     "}" +
                     "}";

    CommandMsg msg;
    bool success = CommandParser::parse(payload, msg);

    assertTrue(success);
    assertTrue(msg.type == CommandType::START);
    assertEqual(strcmp(msg.playerName, default_player_name), 0);
    assertEqual(msg.cookiesCount, default_cookies_count);
    assertEqual(msg.wallThicknessPx, default_wall_thickness_px);
    assertEqual(msg.imuSensitivity, default_imu_sensitivity_multiplier);
    assertEqual(msg.bounceRestitution, default_bounce_restitution);
    assertEqual(msg.emaAlpha, default_ema_alpha);
    assertNear(msg.deadzoneThreshold, default_deadzone_threshold, 1e-4f);
    assertEqual(strcmp(msg.requestId, "id1"), 0);
}

test(CommandParser_ParseByteArraySuccess)
{
    String payload = String("{") +
                     "\"command\":\"start\"," +
                     "\"meta\":{\"source_ui\":\"NODE_RED\",\"request_id\":\"id_byte\"," +
                     "\"timestamp\":\"2026-06-26T11:50:00Z\"}," +
                     "\"player\":{\"name\":\"" + default_player_name + "\"}," +
                     "\"game\":{\"game_id\":" + String(default_game_id) + "}," +
                     "\"parameters\":{" +
                     "\"cookies_count\":" + String(default_cookies_count) + "," +
                     "\"wall_thickness_px\":" + String(default_wall_thickness_px) +
                     "}," +
                     "\"physics\":{" +
                     "\"imu_sensitivity_multiplier\":" + String(default_imu_sensitivity_multiplier) + "," +
                     "\"bounce_restitution\":" + String(default_bounce_restitution) + "," +
                     "\"ema_alpha\":" + String(default_ema_alpha) + "," +
                     "\"deadzone_threshold\":" + String(default_deadzone_threshold) +
                     "}" +
                     "}";

    // Extract raw bytes and length from the String instance
    const byte *byteData = reinterpret_cast<const byte *>(payload.c_str());
    unsigned int length = payload.length();

    CommandMsg msg;
    bool success = CommandParser::parse(byteData, length, msg);

    // Verify properties decode identically through the byte array interface
    assertTrue(success);
    assertTrue(msg.type == CommandType::START);
    assertEqual(strcmp(msg.playerName, default_player_name), 0);
    assertEqual(msg.cookiesCount, default_cookies_count);
    assertEqual(msg.wallThicknessPx, default_wall_thickness_px);
    assertEqual(msg.imuSensitivity, default_imu_sensitivity_multiplier);
    assertEqual(msg.bounceRestitution, default_bounce_restitution);
    assertEqual(msg.emaAlpha, default_ema_alpha);
    assertNear(msg.deadzoneThreshold, default_deadzone_threshold, 1e-4f);
    assertEqual(strcmp(msg.requestId, "id_byte"), 0);
}

test(CommandParser_ParseByteArrayNullOrEmpty)
{
    CommandMsg msg;

    // Verify rejection of null pointers
    bool successNull = CommandParser::parse(nullptr, 12, msg);
    assertFalse(successNull);

    // Verify rejection of zero-length buffers
    const byte dummyBuffer[] = {'{', '}'};
    bool successZeroLen = CommandParser::parse(dummyBuffer, 0, msg);
    assertFalse(successZeroLen);
}

test(CommandParser_ParseInvalidCommand)
{
    String payload = "{\"command\":\"invalid_action\"}";
    CommandMsg msg;
    bool success = CommandParser::parse(payload, msg);

    assertTrue(success);
    assertTrue(msg.type == CommandType::UNKNOWN);
}

test(CommandParser_ParseMalformedJson)
{
    String payload = "{\"command\":\"start\", malformed_json";
    CommandMsg msg;
    bool success = CommandParser::parse(payload, msg);

    assertFalse(success);
}