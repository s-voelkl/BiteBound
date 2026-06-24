#ifndef TELEMETRY_SNAPSHOT_DRAFT_H
#define TELEMETRY_SNAPSHOT_DRAFT_H

// =============================================================================
// DRAFT. The latest game state, produced by Core 1 and consumed by Core 0.
// Inert until #included (see note in GameCommand_Draft.h).
//
// This snapshot holds only the GAME-DYNAMIC data. Device-static fields of the
// telemetry payload (client_id, hardware, firmware_version, uptime, wifi_ssid,
// screen_width/height) are added by Core 0 when it builds the JSON, because it
// already knows them from config.h / WiFi. Keeping them out of the snapshot
// keeps it small and the cross-core copy cheap.
// =============================================================================

#include <stdint.h>
#include "GameCommand_Draft.h" // for kMaxPlayerNameLen

/** Coarse game lifecycle state. Mapped to the telemetry "status" string on Core 0. */
enum class GameStatus : uint8_t
{
    Idle = 0,  // no game running
    Running,   // game in progress
    Finished   // all cookies collected / stopped
};

/**
 * @brief Latest game state for telemetry. POD, queue-safe (see GameCommand).
 *
 * Maps to the "config", "state", "physics" and "sensors" blocks of the
 * telemetry payload. Written every frame by Core 1 (xQueueOverwrite), read
 * ~2 Hz by Core 0 (xQueuePeek).
 */
struct TelemetrySnapshot
{
    // ---- config (chosen via the last Start command) ----
    uint8_t gameId = 1;
    char playerName[kMaxPlayerNameLen] = {0};
    uint16_t targetCookies = 10;
    uint8_t wallThicknessPx = 6;

    // ---- state ----
    GameStatus status = GameStatus::Idle;
    uint16_t cookiesCollected = 0;
    uint16_t cookiesRemaining = 0;
    uint16_t currentRound = 1;
    float elapsedSec = 0.0f;

    // ---- physics (from the PhysicsBody + last step() result) ----
    float ballX = 0.0f;
    float ballY = 0.0f;
    float velX = 0.0f;
    float velY = 0.0f;
    float accX = 0.0f; // last input acceleration applied this frame
    float accY = 0.0f;
    bool collision = false;

    // ---- sensors (RAW, straight from SensorData) ----
    float accelX = 0.0f;
    float accelY = 0.0f;
    float accelZ = 0.0f;
    float gyroX = 0.0f;
    float gyroY = 0.0f;
    float gyroZ = 0.0f;
};

#endif // TELEMETRY_SNAPSHOT_DRAFT_H
