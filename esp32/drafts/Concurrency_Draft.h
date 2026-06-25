#ifndef CONCURRENCY_DRAFT_H
#define CONCURRENCY_DRAFT_H

// =============================================================================
// DRAFT BLUEPRINT — dual-core wiring (FreeRTOS).
//
// This file is a REFERENCE implementation, not production code. It is inert
// because nothing #includes it. When you wire the game up, move these pieces
// into the right places:
//   - the parse helper  -> MqttManager::onMessage() in MqttManager.cpp
//   - the queue handles + setupConcurrency() + the two tasks -> esp32.ino
// The TODO markers show where the (not-yet-built) game/display/sensor parts plug
// in. It intentionally references symbols that do not exist yet, so do NOT
// include it as-is — it will not compile until those parts exist.
//
// Model (see also the data-flow diagram):
//   Core 0 (network): WiFi/TLS + MQTT. Parses commands -> commandQueue.
//                     Reads telemetryQueue -> builds JSON -> publishes ~2 Hz.
//   Core 1 (game):    sensors -> PhysicsEngine -> game logic -> render, 50 Hz.
//                     Drains commandQueue, writes the telemetry snapshot.
//
// The command channel is always a FreeRTOS queue. For the telemetry snapshot you
// pick ONE of two transports via TELEMETRY_USE_MUTEX (see below): a 1-element
// overwrite queue (recommended) or a shared struct guarded by a mutex
// (SemaphoreHandle_t, as sketched in the architecture.puml).
// =============================================================================

#include <Arduino.h>
#include <ArduinoJson.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "GameCommand_Draft.h"
#include "TelemetrySnapshot_Draft.h"
#include "../physics/PhysicsEngine.h"
#include "../physics/BorderCollider.h"
#include "CookieField.h"
#include "RectCookieSpawner.h"
#include "../network/mqtt/MqttManager.h"
#include "../network/wifi/WifiManager.h"
#include "../network/time/TimeManager.h"
#include "../sensors/SensorManager.h"
#include "../../config.h"

// -----------------------------------------------------------------------------
// Telemetry transport: pick ONE mechanism for the Core 1 -> Core 0 snapshot.
//   0 = 1-element overwrite queue (recommended: FreeRTOS does the locking, the
//       producer never blocks, and the consumer always gets a consistent copy).
//   1 = shared struct guarded by a mutex (matches the puml's SemaphoreHandle_t).
// The command channel (Core 0 -> Core 1) is always a queue.
// -----------------------------------------------------------------------------
#ifndef TELEMETRY_USE_MUTEX
#define TELEMETRY_USE_MUTEX 0
#endif

// -----------------------------------------------------------------------------
// Cross-core channels. Created once in setupConcurrency().
// -----------------------------------------------------------------------------
QueueHandle_t commandQueue = nullptr; // Core 0 -> Core 1, discrete events

#if TELEMETRY_USE_MUTEX
SemaphoreHandle_t telemetryMutex = nullptr; // guards gTelemetry
TelemetrySnapshot gTelemetry;               // shared "latest" snapshot
#else
QueueHandle_t telemetryQueue = nullptr; // Core 1 -> Core 0, latest snapshot only
#endif

// -----------------------------------------------------------------------------
// Telemetry transport helpers. The tasks call these; the mechanism stays hidden.
//
// GOLDEN RULE for the mutex variant: hold the lock ONLY for the plain copy, never
// while building JSON or publishing. Those are slow (string work / TLS) and would
// stall the other core for as long as the lock is held, defeating the whole
// point of splitting the work across two cores.
// -----------------------------------------------------------------------------
inline void publishSnapshot(const TelemetrySnapshot &snap) // called on Core 1
{
#if TELEMETRY_USE_MUTEX
    if (telemetryMutex && xSemaphoreTake(telemetryMutex, portMAX_DELAY) == pdTRUE)
    {
        gTelemetry = snap; // copy in, then release immediately
        xSemaphoreGive(telemetryMutex);
    }
#else
    if (telemetryQueue)
    {
        xQueueOverwrite(telemetryQueue, &snap);
    }
#endif
}

inline bool readSnapshot(TelemetrySnapshot &out) // called on Core 0
{
#if TELEMETRY_USE_MUTEX
    if (telemetryMutex && xSemaphoreTake(telemetryMutex, portMAX_DELAY) == pdTRUE)
    {
        out = gTelemetry; // copy out under the lock ...
        xSemaphoreGive(telemetryMutex);
        return true; // ... and build the JSON AFTER releasing it (in the caller)
    }
    return false;
#else
    return telemetryQueue && xQueuePeek(telemetryQueue, &out, 0) == pdTRUE;
#endif
}

// =============================================================================
// Core 0: parse an incoming MQTT payload into a GameCommand.
// Belongs in MqttManager::onMessage(). Returns false on malformed/unknown input.
// =============================================================================
inline bool parseCommand(const byte *payload, unsigned int length, GameCommand &out)
{
    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, payload, length))
    {
        return false; // ignore malformed JSON
    }

    const char *c = doc["command"] | "";
    if (strcmp(c, "start") == 0)
    {
        out.type = CommandType::Start;
    }
    else if (strcmp(c, "stop") == 0)
    {
        out.type = CommandType::Stop;
    }
    else
    {
        return false; // unknown command
    }

    out.gameId = doc["game"]["game_id"] | 1;
    out.cookiesCount = doc["parameters"]["cookies_count"] | 10;
    out.wallThicknessPx = doc["parameters"]["wall_thickness_px"] | 6;

    const char *name = doc["player"]["name"] | "Player";
    snprintf(out.playerName, sizeof(out.playerName), "%s", name); // safe, truncates

    JsonObjectConst p = doc["physics"];
    out.physics.sensitivity = p["imu_sensitivity_multiplier"] | 1.25f;
    out.physics.restitution = p["bounce_restitution"] | 0.75f;
    out.physics.emaAlpha = p["ema_alpha"] | 0.25f;
    out.physics.deadzone = p["deadzone_threshold"] | 0.05f;
    // maxSpeed / linearDamping are not in the payload -> keep PhysicsParams defaults.

    return true;
}

// =============================================================================
// Core 0: build the full telemetry JSON from a snapshot + device-static fields.
// =============================================================================
inline String buildTelemetryJson(const TelemetrySnapshot &s)
{
    StaticJsonDocument<1024> doc;

    JsonObject device = doc.createNestedObject("device");
    device["client_id"] = device_id;
    device["hardware"] = "Waveshare ESP32-S3 1.69inch"; // TODO: move to config.h
    device["firmware_version"] = "0.1.0";               // TODO: move to config.h
    device["uptime_ms"] = millis();
    device["wifi_ssid"] = WiFi.SSID();

    JsonObject config = doc.createNestedObject("config");
    config["game_id"] = s.gameId;
    config["player_name"] = s.playerName;
    config["target_cookies"] = s.targetCookies;
    config["screen_width"] = display_width;
    config["screen_height"] = display_height;
    config["wall_thickness_px"] = s.wallThicknessPx;

    JsonObject state = doc.createNestedObject("state");
    const char *status = "idle";
    if (s.status == GameStatus::Running)
        status = "running";
    else if (s.status == GameStatus::Finished)
        status = "finished";
    state["status"] = status;
    state["cookies_collected"] = s.cookiesCollected;
    state["cookies_remaining"] = s.cookiesRemaining;
    state["current_round"] = s.currentRound;
    state["elapsed_time_sec"] = s.elapsedSec;

    JsonObject physics = doc.createNestedObject("physics");
    physics["ball_pos_x"] = s.ballX;
    physics["ball_pos_y"] = s.ballY;
    physics["velocity_x"] = s.velX;
    physics["velocity_y"] = s.velY;
    physics["acc_x"] = s.accX;
    physics["acc_y"] = s.accY;
    physics["collision_detected"] = s.collision;

    JsonObject sensors = doc.createNestedObject("sensors");
    sensors["accel_x"] = s.accelX;
    sensors["accel_y"] = s.accelY;
    sensors["accel_z"] = s.accelZ;
    sensors["gyro_x"] = s.gyroX;
    sensors["gyro_y"] = s.gyroY;
    sensors["gyro_z"] = s.gyroZ;

    String json;
    serializeJson(doc, json);
    return json;
}

// =============================================================================
// Core 1: minimal game state for this draft (Game 2 / Flatland style).
// Replace with the GameEngine + IGame structure once it exists.
// =============================================================================
struct DraftGameState
{
    PhysicsBody ball;
    CookieField cookies;
    RectCookieSpawner spawner;
    GameStatus status = GameStatus::Idle;
    uint8_t gameId = 2;
    uint8_t wallThicknessPx = 6;
    char playerName[kMaxPlayerNameLen] = {0};
    uint32_t startMs = 0;
};

// Core 1: apply one command to engine + game state.
inline void applyCommand(const GameCommand &cmd, PhysicsEngine &engine, DraftGameState &g)
{
    switch (cmd.type)
    {
    case CommandType::Start:
        engine.setParams(cmd.physics);
        engine.reset();
        g.gameId = cmd.gameId;
        g.wallThicknessPx = cmd.wallThicknessPx;
        snprintf(g.playerName, sizeof(g.playerName), "%s", cmd.playerName);
        // Center the ball, then spawn the (1-3) respawning cookies.
        g.ball.x = display_width * 0.5f;
        g.ball.y = display_height * 0.5f;
        g.ball.vx = g.ball.vy = 0.0f;
        g.spawner = RectCookieSpawner(display_width, display_height, 3.0f);
        g.cookies.start(3, cmd.cookiesCount, g.spawner, g.ball); // target = cookies_count
        g.status = GameStatus::Running;
        g.startMs = millis();
        // TODO: for Game 1 select a MazeCookieSpawner + generate the maze.
        break;
    case CommandType::Stop:
        g.status = GameStatus::Finished;
        break;
    default:
        break;
    }
}

// Core 1: assemble the telemetry snapshot from current state.
inline TelemetrySnapshot makeSnapshot(const DraftGameState &g, const SensorData &s,
                                      Vec2 lastAccel, bool collided)
{
    TelemetrySnapshot snap;
    snap.gameId = g.gameId;
    snprintf(snap.playerName, sizeof(snap.playerName), "%s", g.playerName);
    snap.targetCookies = g.cookies.target();
    snap.wallThicknessPx = g.wallThicknessPx;

    snap.status = g.status;
    snap.cookiesCollected = g.cookies.collected();
    snap.cookiesRemaining = g.cookies.remaining();
    snap.currentRound = 1;
    snap.elapsedSec = (g.status == GameStatus::Running) ? (millis() - g.startMs) / 1000.0f : 0.0f;

    snap.ballX = g.ball.x;
    snap.ballY = g.ball.y;
    snap.velX = g.ball.vx;
    snap.velY = g.ball.vy;
    snap.accX = lastAccel.x;
    snap.accY = lastAccel.y;
    snap.collision = collided;

    snap.accelX = s.accelerometerX;
    snap.accelY = s.accelerometerY;
    snap.accelZ = s.accelerometerZ;
    snap.gyroX = s.gyroscopeX;
    snap.gyroY = s.gyroscopeY;
    snap.gyroZ = s.gyroscopeZ;
    return snap;
}

// =============================================================================
// Core 0 task: network. Pinned to core 0 (where the WiFi stack lives).
// =============================================================================
inline void networkTask(void *)
{
    wifiManager.connect();
    timeManager.sync();
    mqttManager.begin();

    TickType_t last = xTaskGetTickCount();
    for (;;)
    {
        mqttManager.connect(); // also (re)subscribes; see note below
        mqttManager.loop();    // services incoming -> onMessage -> parseCommand -> queue

        TelemetrySnapshot snap;
        if (readSnapshot(snap)) // copies out (lock already released); JSON built unlocked
        {
            mqttManager.publish(mqtt_telemetry_topic, buildTelemetryJson(snap), mqtt_retain);
        }

        vTaskDelayUntil(&last, pdMS_TO_TICKS(500)); // ~2 Hz
    }
}

// =============================================================================
// Core 1 task: game loop @ 50 Hz. Pinned to core 1.
// =============================================================================
inline void gameTask(void *)
{
    sensorManager.begin();
    // TODO: displayManager.begin();

    PhysicsEngine engine;
    BorderCollider borders(display_width, display_height); // Game 2 environment
    DraftGameState g;

    const TickType_t period = pdMS_TO_TICKS(20); // 50 Hz
    TickType_t last = xTaskGetTickCount();
    uint32_t prevMs = millis();

    for (;;)
    {
        // 1) drain all pending commands
        GameCommand cmd;
        while (commandQueue && xQueueReceive(commandQueue, &cmd, 0) == pdTRUE)
        {
            applyCommand(cmd, engine, g);
        }

        // 2) sensors + physics (only while running)
        const uint32_t nowMs = millis();
        const float dt = (nowMs - prevMs) / 1000.0f;
        prevMs = nowMs;

        SensorData s = sensorManager.read();
        Vec2 accel(0.0f, 0.0f);
        bool collided = false;
        if (g.status == GameStatus::Running && dt > 0.0f)
        {
            accel = engine.inputAccel(s.accelerometerX, s.accelerometerY);
            collided = engine.step(g.ball, accel, dt, borders);
            g.cookies.checkPickup(g.ball); // score++ and respawn on contact
            if (g.cookies.finished())
            {
                g.status = GameStatus::Finished;
            }
        }

        // 3) render
        // TODO: displayManager.draw(g.ball, cookies, HUD ...);

        // 4) publish latest snapshot (newest wins)
        TelemetrySnapshot snap = makeSnapshot(g, s, accel, collided);
        publishSnapshot(snap);

        vTaskDelayUntil(&last, period);
    }
}

// =============================================================================
// Call once from setup(): create the channels and start both tasks.
// =============================================================================
inline void setupConcurrency()
{
    commandQueue = xQueueCreate(8, sizeof(GameCommand));
#if TELEMETRY_USE_MUTEX
    telemetryMutex = xSemaphoreCreateMutex(); // shared-struct variant
#else
    telemetryQueue = xQueueCreate(1, sizeof(TelemetrySnapshot)); // length 1 = latest only
#endif

    xTaskCreatePinnedToCore(networkTask, "net", 8192, nullptr, 1, nullptr, 0); // Core 0
    xTaskCreatePinnedToCore(gameTask, "game", 8192, nullptr, 2, nullptr, 1);   // Core 1
}

// NOTE — two changes still needed in the existing code to make this live:
//   1. MqttManager must subscribe to mqtt_command_topic (today it only subscribes
//      to the test topic) and call parseCommand() in onMessage(), then
//      xQueueSend(commandQueue, &cmd, 0).
//   2. esp32.ino setup() (non-test branch) should call setupConcurrency() instead
//      of the current inline connect/begin sequence, and loop() can stay empty.

#endif // CONCURRENCY_DRAFT_H
