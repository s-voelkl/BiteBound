package com.example.bitebound.data

import org.json.JSONObject

/**
 * The telemetry JSON coming from the ESP32, parsed into plain Kotlin objects.
 *
 * Every field falls back to a default if it's missing, so a half-broken message
 * just shows zeros on the dashboard instead of crashing the whole thing.
 */
data class Telemetry(
    val device: Device,
    val config: GameConfig,
    val state: GameState,
    val physics: Physics,
    val sensors: Sensors,
    val receivedAtMillis: Long = System.currentTimeMillis(),
) {
    companion object {
        fun parse(raw: String): Telemetry? = try {
            val root = JSONObject(raw)
            Telemetry(
                device = Device.from(root.optJSONObject("device")),
                config = GameConfig.from(root.optJSONObject("config")),
                state = GameState.from(root.optJSONObject("state")),
                physics = Physics.from(root.optJSONObject("physics")),
                sensors = Sensors.from(root.optJSONObject("sensors")),
            )
        } catch (_: Exception) {
            null
        }
    }
}

data class Device(
    val clientId: String,
    val hardware: String,
    val firmwareVersion: String,
    val uptimeMs: Long,
    val wifiSsid: String,
) {
    companion object {
        fun from(o: JSONObject?): Device {
            val j = o ?: JSONObject()
            return Device(
                clientId = j.optString("client_id", "—"),
                hardware = j.optString("hardware", "—"),
                firmwareVersion = j.optString("firmware_version", "—"),
                uptimeMs = j.optLong("uptime_ms", 0L),
                wifiSsid = j.optString("wifi_ssid", "—"),
            )
        }
    }
}

data class GameConfig(
    val gameId: Int,
    val playerName: String,
    val targetCookies: Int,
    val screenWidth: Int,
    val screenHeight: Int,
    val wallThicknessPx: Int,
) {
    companion object {
        fun from(o: JSONObject?): GameConfig {
            val j = o ?: JSONObject()
            return GameConfig(
                gameId = j.optInt("game_id", 1),
                playerName = j.optString("player_name", "—"),
                targetCookies = j.optInt("target_cookies", 0),
                screenWidth = j.optInt("screen_width", 240),
                screenHeight = j.optInt("screen_height", 280),
                wallThicknessPx = j.optInt("wall_thickness_px", 0),
            )
        }
    }
}

data class GameState(
    val runningStatus: String,
    val cookiesCollected: Int,
    val cookiesRemaining: Int,
    val currentRound: Int,
    val elapsedTimeSec: Double,
) {
    val isRunning: Boolean get() = runningStatus.equals("running", ignoreCase = true)
    val isFinished: Boolean get() = runningStatus.equals("completed", ignoreCase = true)

    companion object {
        fun from(o: JSONObject?): GameState {
            val j = o ?: JSONObject()
            return GameState(
                runningStatus = j.optString("runningStatus", "idle"),
                cookiesCollected = j.optInt("cookies_collected", 0),
                cookiesRemaining = j.optInt("cookies_remaining", 0),
                currentRound = j.optInt("current_round", 0),
                elapsedTimeSec = j.optDouble("elapsed_time_sec", 0.0),
            )
        }
    }
}

data class Physics(
    val ballPosX: Double,
    val ballPosY: Double,
    val velocityX: Double,
    val velocityY: Double,
    val accX: Double,
    val accY: Double,
    val collisionDetected: Boolean,
) {
    companion object {
        fun from(o: JSONObject?): Physics {
            val j = o ?: JSONObject()
            return Physics(
                ballPosX = j.optDouble("ball_pos_x", 0.0),
                ballPosY = j.optDouble("ball_pos_y", 0.0),
                velocityX = j.optDouble("velocity_x", 0.0),
                velocityY = j.optDouble("velocity_y", 0.0),
                accX = j.optDouble("acc_x", 0.0),
                accY = j.optDouble("acc_y", 0.0),
                collisionDetected = j.optBoolean("collision_detected", false),
            )
        }
    }
}

data class Sensors(
    val accelX: Double,
    val accelY: Double,
    val accelZ: Double,
    val gyroX: Double,
    val gyroY: Double,
    val gyroZ: Double,
    val batteryVoltage: Double,
    val button: Boolean,
) {
    companion object {
        fun from(o: JSONObject?): Sensors {
            val j = o ?: JSONObject()
            return Sensors(
                accelX = j.optDouble("accel_x", 0.0),
                accelY = j.optDouble("accel_y", 0.0),
                accelZ = j.optDouble("accel_z", 0.0),
                gyroX = j.optDouble("gyro_x", 0.0),
                gyroY = j.optDouble("gyro_y", 0.0),
                gyroZ = j.optDouble("gyro_z", 0.0),
                batteryVoltage = j.optDouble("battery_voltage", 0.0),
                button = j.optBoolean("button", false),
            )
        }
    }
}
