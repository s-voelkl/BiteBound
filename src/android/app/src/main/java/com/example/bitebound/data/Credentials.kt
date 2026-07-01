package com.example.bitebound.data

import android.content.Context
import androidx.core.content.edit

/**
 * Holds the broker login plus the current game setup. Host, port and topics are
 * pre-filled with our group's HiveMQ Cloud values (same broker the Node-RED
 * dashboard uses), so normally you only have to add the username and password.
 */
data class Credentials(
    val host: String = DEFAULT_HOST,
    val port: Int = DEFAULT_PORT,
    val username: String = DEFAULT_USERNAME,
    val password: String = "",
    val telemetryTopic: String = DEFAULT_TELEMETRY_TOPIC,
    val commandTopic: String = DEFAULT_COMMAND_TOPIC,
    val playerName: String = GameConfigConstants.DEFAULT_PLAYER_NAME,
    val gameId: Int = GameConfigConstants.GAME_ID_LABYRINTH,
    val cookiesCount: Int = GameConfigConstants.DEFAULT_COOKIES_COUNT,
    val wallThickness: Int = GameConfigConstants.DEFAULT_WALL_THICKNESS_PX,
    val imuSensitivity: Double = GameConfigConstants.DEFAULT_IMU_SENSITIVITY_MULTIPLIER,
    val restitution: Double = GameConfigConstants.DEFAULT_BOUNCE_RESTITUTION,
    val emaAlpha: Double = GameConfigConstants.DEFAULT_EMA_ALPHA,
    val deadzone: Double = GameConfigConstants.DEFAULT_DEADZONE_THRESHOLD,
) {
    val isComplete: Boolean
        get() = host.isNotBlank() && port in 1..65535 &&
            username.isNotBlank() && password.isNotBlank() &&
            telemetryTopic.isNotBlank() && commandTopic.isNotBlank() &&
            playerName.isNotBlank()

    companion object {
        const val DEFAULT_HOST = "61775de8f674480cae63ae7690a8ffae.s1.eu.hivemq.cloud"
        const val DEFAULT_PORT = 8883
        const val DEFAULT_USERNAME = "BiteBound"
        const val DEFAULT_TELEMETRY_TOPIC = "mauc2026/group_03/game/telemetry"
        const val DEFAULT_COMMAND_TOPIC = "mauc2026/group_03/game/command"
    }
}

/**
 * Saves and loads [Credentials] in SharedPreferences so you don't have to type
 * everything in again on the next app start.
 */
class CredentialsStore(context: Context) {
    private val prefs = context.getSharedPreferences("bitebound_mqtt", Context.MODE_PRIVATE)

    fun load(): Credentials = Credentials(
        host = prefs.getString(KEY_HOST, Credentials.DEFAULT_HOST)!!,
        port = prefs.getInt(KEY_PORT, Credentials.DEFAULT_PORT),
        username = prefs.getString(KEY_USERNAME, Credentials.DEFAULT_USERNAME)!!,
        password = prefs.getString(KEY_PASSWORD, "")!!,
        telemetryTopic = prefs.getString(KEY_TELEMETRY, Credentials.DEFAULT_TELEMETRY_TOPIC)!!,
        commandTopic = prefs.getString(KEY_COMMAND, Credentials.DEFAULT_COMMAND_TOPIC)!!,
        playerName = prefs.getString(KEY_PLAYER_NAME, GameConfigConstants.DEFAULT_PLAYER_NAME)!!,
        gameId = prefs.getInt(KEY_GAME_ID, GameConfigConstants.GAME_ID_LABYRINTH),
        cookiesCount = prefs.getInt(KEY_COOKIES_COUNT, GameConfigConstants.DEFAULT_COOKIES_COUNT),
        wallThickness = prefs.getInt(KEY_WALL_THICKNESS, GameConfigConstants.DEFAULT_WALL_THICKNESS_PX),
        imuSensitivity = prefs.getFloat(KEY_IMU_SENSITIVITY, GameConfigConstants.DEFAULT_IMU_SENSITIVITY_MULTIPLIER.toFloat()).toDouble(),
        restitution = prefs.getFloat(KEY_RESTITUTION, GameConfigConstants.DEFAULT_BOUNCE_RESTITUTION.toFloat()).toDouble(),
        emaAlpha = prefs.getFloat(KEY_EMA_ALPHA, GameConfigConstants.DEFAULT_EMA_ALPHA.toFloat()).toDouble(),
        deadzone = prefs.getFloat(KEY_DEADZONE, GameConfigConstants.DEFAULT_DEADZONE_THRESHOLD.toFloat()).toDouble(),
    )

    fun save(credentials: Credentials) {
        prefs.edit {
            putString(KEY_HOST, credentials.host)
                .putInt(KEY_PORT, credentials.port)
                .putString(KEY_USERNAME, credentials.username)
                .putString(KEY_PASSWORD, credentials.password)
                .putString(KEY_TELEMETRY, credentials.telemetryTopic)
                .putString(KEY_COMMAND, credentials.commandTopic)
                .putString(KEY_PLAYER_NAME, credentials.playerName)
                .putInt(KEY_GAME_ID, credentials.gameId)
                .putInt(KEY_COOKIES_COUNT, credentials.cookiesCount)
                .putInt(KEY_WALL_THICKNESS, credentials.wallThickness)
                .putFloat(KEY_IMU_SENSITIVITY, credentials.imuSensitivity.toFloat())
                .putFloat(KEY_RESTITUTION, credentials.restitution.toFloat())
                .putFloat(KEY_EMA_ALPHA, credentials.emaAlpha.toFloat())
                .putFloat(KEY_DEADZONE, credentials.deadzone.toFloat())
        }
    }

    private companion object {
        const val KEY_HOST = "host"
        const val KEY_PORT = "port"
        const val KEY_USERNAME = "username"
        const val KEY_PASSWORD = "password"
        const val KEY_TELEMETRY = "telemetry_topic"
        const val KEY_COMMAND = "command_topic"
        const val KEY_PLAYER_NAME = "player_name"
        const val KEY_GAME_ID = "game_id"
        const val KEY_COOKIES_COUNT = "cookies_count"
        const val KEY_WALL_THICKNESS = "wall_thickness"
        const val KEY_IMU_SENSITIVITY = "imu_sensitivity"
        const val KEY_RESTITUTION = "restitution"
        const val KEY_EMA_ALPHA = "ema_alpha"
        const val KEY_DEADZONE = "deadzone"
    }
}
