#include <AUnit.h>
#include "JsonBuilder.h"

test(JsonBuilderBasicStructure)
{
    TelemetryData data;
    data.client_id = "TEST_DEVICE";
    data.hardware = "TestHardware";
    data.firmware_version = "1.0.0";
    data.uptime_ms = 1000;
    data.wifi_ssid = "TestSSID";

    data.game_id = 1;
    data.player_name = "Test";
    data.target_cookies = 10;
    data.screen_width = 240;
    data.screen_height = 280;
    data.wall_thickness_px = 6;

    data.runningStatus = "running";
    data.cookies_collected = 5;
    data.cookies_remaining = 5;
    data.current_round = 1;
    data.elapsed_time_sec = 30.5f;

    data.ball_pos_x = 100.0f;
    data.ball_pos_y = 150.0f;
    data.velocity_x = 2.5f;
    data.velocity_y = -1.5f;
    data.acc_x = 0.1f;
    data.acc_y = -0.2f;

    data.accel_x = 0.5f;
    data.accel_y = -0.3f;
    data.accel_z = 9.81f;
    data.gyro_x = 0.01f;
    data.gyro_y = -0.02f;
    data.gyro_z = 0.005f;
    data.battery_voltage = 4.0f;
    data.button = false;

    String payload = buildTelemetryJson(data);

    // Parse the JSON to verify structure
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    assertTrue(!error);
    assertTrue(doc["device"].is<JsonObject>());
    assertTrue(doc["config"].is<JsonObject>());
    assertTrue(doc["state"].is<JsonObject>());
    assertTrue(doc["physics"].is<JsonObject>());
    assertTrue(doc["sensors"].is<JsonObject>());
}

test(JsonBuilderTestMethod)
{
    String payload = buildTelemetryJsonTest();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    assertTrue(!error);

    // Verify device information
    assertEqual(doc["device"]["client_id"].as<String>(), "BiteBound_ESP32_G03");
    assertEqual(doc["device"]["hardware"].as<String>(), "Waveshare ESP32-S3 1.69inch");
    assertEqual(doc["device"]["firmware_version"].as<String>(), "1.0.0");
    assertEqual(doc["device"]["uptime_ms"].as<unsigned long>(), (unsigned long)745200);
    assertEqual(doc["device"]["wifi_ssid"].as<String>(), "MyWiFiNetwork");

    // Verify config
    assertEqual(doc["config"]["game_id"].as<int>(), 1);
    assertEqual(doc["config"]["player_name"].as<String>(), "Player 1");
    assertEqual(doc["config"]["target_cookies"].as<int>(), 15);

    // Verify state
    assertEqual(doc["state"]["runningStatus"].as<String>(), "running");
    assertEqual(doc["state"]["cookies_collected"].as<int>(), 4);
    assertEqual(doc["state"]["cookies_remaining"].as<int>(), 11);

    // Verify physics
    assertTrue(abs(doc["physics"]["ball_pos_x"].as<float>() - 112.45f) < 0.01f);
    assertTrue(abs(doc["physics"]["velocity_x"].as<float>() - 1.85f) < 0.01f);

    // Verify sensors
    assertTrue(abs(doc["sensors"]["battery_voltage"].as<float>() - 4.2f) < 0.01f);
    assertEqual(doc["sensors"]["button"].as<bool>(), false);
}

test(JsonBuilderPayloadNotEmpty)
{
    TelemetryData data = {};
    String payload = buildTelemetryJson(data);

    assertTrue(payload.length() > 0);
}

test(JsonBuilderAllFieldsPresent)
{
    TelemetryData data;
    data.client_id = "TEST";
    data.hardware = "TEST";
    data.firmware_version = "1.0";
    data.uptime_ms = 100;
    data.wifi_ssid = "TEST";
    data.game_id = 1;
    data.player_name = "Test";
    data.target_cookies = 10;
    data.screen_width = 240;
    data.screen_height = 280;
    data.wall_thickness_px = 6;
    data.runningStatus = "running";
    data.cookies_collected = 5;
    data.cookies_remaining = 5;
    data.current_round = 1;
    data.elapsed_time_sec = 30.0f;
    data.ball_pos_x = 100.0f;
    data.ball_pos_y = 150.0f;
    data.velocity_x = 1.0f;
    data.velocity_y = -1.0f;
    data.acc_x = 0.1f;
    data.acc_y = -0.1f;
    data.accel_x = 0.0f;
    data.accel_y = 0.0f;
    data.accel_z = 9.81f;
    data.gyro_x = 0.0f;
    data.gyro_y = 0.0f;
    data.gyro_z = 0.0f;
    data.battery_voltage = 4.0f;
    data.button = false;

    String payload = buildTelemetryJson(data);
    JsonDocument doc;
    deserializeJson(doc, payload);

    // Check all required fields are present
    assertTrue(doc["device"]["client_id"].is<String>());
    assertTrue(doc["device"]["hardware"].is<String>());
    assertTrue(doc["device"]["firmware_version"].is<String>());
    assertTrue(doc["device"]["uptime_ms"].is<unsigned long>());
    assertTrue(doc["device"]["wifi_ssid"].is<String>());

    assertTrue(doc["config"]["game_id"].is<int>());
    assertTrue(doc["config"]["player_name"].is<String>());
    assertTrue(doc["config"]["target_cookies"].is<int>());
    assertTrue(doc["config"]["screen_width"].is<int>());
    assertTrue(doc["config"]["screen_height"].is<int>());
    assertTrue(doc["config"]["wall_thickness_px"].is<int>());

    assertTrue(doc["state"]["runningStatus"].is<String>());
    assertTrue(doc["state"]["cookies_collected"].is<int>());
    assertTrue(doc["state"]["cookies_remaining"].is<int>());
    assertTrue(doc["state"]["current_round"].is<int>());
    assertTrue(doc["state"]["elapsed_time_sec"].is<float>());

    assertTrue(doc["physics"]["ball_pos_x"].is<float>());
    assertTrue(doc["physics"]["ball_pos_y"].is<float>());
    assertTrue(doc["physics"]["velocity_x"].is<float>());
    assertTrue(doc["physics"]["velocity_y"].is<float>());
    assertTrue(doc["physics"]["acc_x"].is<float>());
    assertTrue(doc["physics"]["acc_y"].is<float>());

    assertTrue(doc["sensors"]["accel_x"].is<float>());
    assertTrue(doc["sensors"]["accel_y"].is<float>());
    assertTrue(doc["sensors"]["accel_z"].is<float>());
    assertTrue(doc["sensors"]["gyro_x"].is<float>());
    assertTrue(doc["sensors"]["gyro_y"].is<float>());
    assertTrue(doc["sensors"]["gyro_z"].is<float>());
    assertTrue(doc["sensors"]["battery_voltage"].is<float>());
    assertTrue(doc["sensors"]["button"].is<bool>());
}
