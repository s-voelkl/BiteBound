#include "JsonBuilder.h"

String buildTelemetryJson(const TelemetryData &data)
{
    JsonDocument doc;

    // Device Information
    JsonObject device = doc["device"].to<JsonObject>();
    device["client_id"] = data.client_id;
    device["hardware"] = data.hardware;
    device["firmware_version"] = data.firmware_version;
    device["uptime_ms"] = data.uptime_ms;
    device["wifi_ssid"] = data.wifi_ssid;

    // Game Configuration
    JsonObject config = doc["config"].to<JsonObject>();
    config["game_id"] = data.game_id;
    config["player_name"] = data.player_name;
    config["target_cookies"] = data.target_cookies;
    config["screen_width"] = data.screen_width;
    config["screen_height"] = data.screen_height;
    config["wall_thickness_px"] = data.wall_thickness_px;

    // Game State
    JsonObject state = doc["state"].to<JsonObject>();
    state["status"] = data.status;
    state["cookies_collected"] = data.cookies_collected;
    state["cookies_remaining"] = data.cookies_remaining;
    state["current_round"] = data.current_round;
    state["elapsed_time_sec"] = data.elapsed_time_sec;

    // Physics Simulation State
    JsonObject physics = doc["physics"].to<JsonObject>();
    physics["ball_pos_x"] = data.ball_pos_x;
    physics["ball_pos_y"] = data.ball_pos_y;
    physics["velocity_x"] = data.velocity_x;
    physics["velocity_y"] = data.velocity_y;
    physics["acc_x"] = data.acc_x;
    physics["acc_y"] = data.acc_y;
    physics["collision_detected"] = data.collision_detected;

    // Sensor Readings
    JsonObject sensors = doc["sensors"].to<JsonObject>();
    sensors["accel_x"] = data.accel_x;
    sensors["accel_y"] = data.accel_y;
    sensors["accel_z"] = data.accel_z;
    sensors["gyro_x"] = data.gyro_x;
    sensors["gyro_y"] = data.gyro_y;
    sensors["gyro_z"] = data.gyro_z;
    sensors["battery_voltage"] = data.battery_voltage;
    sensors["button"] = data.button;

    // Serialize to JSON string
    String payload;
    serializeJson(doc, payload);
    return payload;
}

String buildTelemetryJsonTest()
{
    TelemetryData testData;

    // Device Information
    testData.client_id = "BiteBound_ESP32_G03";
    testData.hardware = "Waveshare ESP32-S3 1.69inch";
    testData.firmware_version = "1.0.0";
    testData.uptime_ms = 745200;
    testData.wifi_ssid = "MyWiFiNetwork";

    // Game Configuration
    testData.game_id = 1;
    testData.player_name = "Player 1";
    testData.target_cookies = 15;
    testData.screen_width = 240;
    testData.screen_height = 280;
    testData.wall_thickness_px = 6;

    // Game State
    testData.status = "running";
    testData.cookies_collected = 4;
    testData.cookies_remaining = 11;
    testData.current_round = 2;
    testData.elapsed_time_sec = 42.8;

    // Physics Simulation State
    testData.ball_pos_x = 112.45;
    testData.ball_pos_y = 145.2;
    testData.velocity_x = 1.85;
    testData.velocity_y = -0.92;
    testData.acc_x = 0.15;
    testData.acc_y = -0.34;
    testData.collision_detected = false;

    // Sensor Readings
    testData.accel_x = 0.12;
    testData.accel_y = -0.08;
    testData.accel_z = 9.81;
    testData.gyro_x = 0.02;
    testData.gyro_y = -0.01;
    testData.gyro_z = 0.005;
    testData.battery_voltage = 4.2;
    testData.button = false;

    return buildTelemetryJson(testData);
}
