package com.example.bitebound

import com.example.bitebound.data.GameCommand
import com.example.bitebound.data.Telemetry
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class TelemetryTest {

    private val sample = """
        {
          "device": {"client_id": "BiteBound_ESP32_G03", "hardware": "Waveshare ESP32-S3",
                     "firmware_version": "1.0.0", "uptime_ms": 745200, "wifi_ssid": "MyWiFi"},
          "config": {"game_id": 1, "player_name": "Player 1", "target_cookies": 15,
                     "screen_width": 240, "screen_height": 280, "wall_thickness_px": 6},
          "state": {"runningStatus": "running", "cookies_collected": 4, "cookies_remaining": 11,
                    "current_round": 2, "elapsed_time_sec": 42.8},
          "physics": {"ball_pos_x": 112.45, "ball_pos_y": 145.2, "velocity_x": 1.85,
                      "velocity_y": -0.92, "acc_x": 0.15, "acc_y": -0.34, "collision_detected": false},
          "sensors": {"accel_x": 0.12, "accel_y": -0.08, "accel_z": 9.81,
                      "gyro_x": 0.02, "gyro_y": -0.01, "gyro_z": 0.005}
        }
    """.trimIndent()

    @Test
    fun parsesFullTelemetry() {
        val t = Telemetry.parse(sample)!!
        assertEquals("BiteBound_ESP32_G03", t.device.clientId)
        assertEquals(15, t.config.targetCookies)
        assertEquals(4, t.state.cookiesCollected)
        assertTrue(t.state.isRunning)
        assertEquals(112.45, t.physics.ballPosX, 0.001)
        assertFalse(t.physics.collisionDetected)
        assertEquals(9.81, t.sensors.accelZ, 0.001)
    }

    @Test
    fun missingFieldsFallBackToDefaults() {
        val t = Telemetry.parse("""{"state": {"status": "idle"}}""")!!
        assertEquals("idle", t.state.runningStatus)
        assertEquals(0, t.state.cookiesCollected)
        assertEquals("—", t.device.clientId)
    }

    @Test
    fun invalidJsonReturnsNull() {
        assertNull(Telemetry.parse("not json"))
    }

    @Test
    fun startCommandHasExpectedShape() {
        val json = JSONObject(GameCommand.start(playerName = "Alex", gameId = 1, cookiesCount = 10, wallThicknessPx = 6))
        assertEquals("start", json.getString("command"))
        assertEquals("ANDROID", json.getJSONObject("meta").getString("source_ui"))
        assertEquals("Alex", json.getJSONObject("player").getString("name"))
        assertEquals(10, json.getJSONObject("parameters").getInt("cookies_count"))
    }
}
