package com.example.bitebound.data

import org.json.JSONObject
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.TimeZone
import java.util.UUID

/**
 * Builds the JSON command payloads sent to the ESP32 on
 * `mauc2026/group_03/game/command`. The shape mirrors
 * `tests/mqtt_mock/subscribe_command_example.json`.
 */
object GameCommand {

    fun start(
        playerName: String,
        gameId: Int,
        cookiesCount: Int,
        wallThicknessPx: Int,
        imuSensitivity: Double = GameConfigConstants.DEFAULT_IMU_SENSITIVITY_MULTIPLIER,
        restitution: Double = GameConfigConstants.DEFAULT_BOUNCE_RESTITUTION,
        emaAlpha: Double = GameConfigConstants.DEFAULT_EMA_ALPHA,
        deadzone: Double = GameConfigConstants.DEFAULT_DEADZONE_THRESHOLD,
    ): String = build("start", playerName, gameId, cookiesCount, wallThicknessPx, imuSensitivity, restitution, emaAlpha, deadzone)

    fun stop(
        playerName: String,
        gameId: Int,
        cookiesCount: Int,
        wallThicknessPx: Int,
    ): String = build("stop", playerName, gameId, cookiesCount, wallThicknessPx)

    private fun build(
        command: String,
        playerName: String,
        gameId: Int,
        cookiesCount: Int,
        wallThicknessPx: Int,
        imuSensitivity: Double = GameConfigConstants.DEFAULT_IMU_SENSITIVITY_MULTIPLIER,
        restitution: Double = GameConfigConstants.DEFAULT_BOUNCE_RESTITUTION,
        emaAlpha: Double = GameConfigConstants.DEFAULT_EMA_ALPHA,
        deadzone: Double = GameConfigConstants.DEFAULT_DEADZONE_THRESHOLD,
    ): String {
        val meta = JSONObject()
            .put("source_ui", "ANDROID")
            .put("request_id", UUID.randomUUID().toString())
            .put("timestamp", isoNow())

        val physics = JSONObject()
            .put("imu_sensitivity_multiplier", imuSensitivity)
            .put("bounce_restitution", restitution)
            .put("ema_alpha", emaAlpha)
            .put("deadzone_threshold", deadzone)

        return JSONObject()
            .put("command", command)
            .put("meta", meta)
            .put("player", JSONObject().put("name", playerName))
            .put("game", JSONObject().put("game_id", gameId))
            .put(
                "parameters",
                JSONObject()
                    .put("cookies_count", cookiesCount)
                    .put("wall_thickness_px", wallThicknessPx),
            )
            .put("physics", physics)
            .toString()
    }

    private fun isoNow(): String {
        val fmt = SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.US)
        fmt.timeZone = TimeZone.getTimeZone("UTC")
        return fmt.format(Date())
    }
}
