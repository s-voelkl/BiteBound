#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * @brief Aggregates all telemetry data for a single transmission.
 *
 * Combines device information, game configuration, game state, physics simulation
 * state, and sensor readings into a single structure. This decouples the telemetry
 * serialization from the various components that provide the data.
 */
struct TelemetryData
{
    // Device Information
    String client_id;
    String hardware;
    String firmware_version;
    unsigned long uptime_ms;
    String wifi_ssid;

    // Game Configuration
    int game_id;
    String player_name;
    int target_cookies;
    int screen_width;
    int screen_height;
    int wall_thickness_px;

    // Game State
    String status;
    int cookies_collected;
    int cookies_remaining;
    int current_round;
    float elapsed_time_sec;

    // Physics Simulation State
    float ball_pos_x;
    float ball_pos_y;
    float velocity_x;
    float velocity_y;
    float acc_x;
    float acc_y;
    bool collision_detected;

    // Sensor Readings (from SensorData)
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float battery_voltage;
    bool button;
};

/**
 * @brief Builds a telemetry JSON document from TelemetryData struct.
 *
 * Serializes the TelemetryData struct into a hierarchical JSON structure
 * compatible with the MQTT telemetry topic payload format.
 *
 * @param data The TelemetryData struct containing all telemetry values.
 * @return A serialized JSON string ready to publish via MQTT.
 */
String buildTelemetryJson(const TelemetryData& data);

/**
 * @brief Builds a test telemetry JSON document with mock data.
 *
 * Creates a complete telemetry payload with realistic sample values for testing
 * and development purposes.
 *
 * @return A serialized JSON string with test data ready for inspection or publishing.
 */
String buildTelemetryJsonTest();

#endif // JSON_BUILDER_H
