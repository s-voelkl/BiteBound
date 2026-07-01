package com.example.bitebound

import com.example.bitebound.data.BallType
import com.example.bitebound.data.GameCommand
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotEquals
import org.junit.Test

// tests for the json we send to the esp
class GameCommandTest {

    @Test
    fun startCommandHasTheRightShape() {
        val json = JSONObject(
            GameCommand.start(playerName = "Bob", gameId = 2, cookiesCount = 7, wallThicknessPx = 12)
        )
        assertEquals("start", json.getString("command"))
        assertEquals("ANDROID", json.getJSONObject("meta").getString("source_ui"))
        assertEquals("Bob", json.getJSONObject("player").getString("name"))
        assertEquals(2, json.getJSONObject("game").getInt("game_id"))

        val params = json.getJSONObject("parameters")
        assertEquals(7, params.getInt("cookies_count"))
        assertEquals(12, params.getInt("wall_thickness_px"))
    }

    // the chosen ball's physics should actually land in the command we send
    @Test
    fun ballTypePhysicsEndUpInTheCommand() {
        val ball = BallType.SCRAPPY
        val physics = JSONObject(
            GameCommand.start(
                playerName = "X", gameId = 1, cookiesCount = 5, wallThicknessPx = 10,
                imuSensitivity = ball.sensitivity,
                restitution = ball.restitution,
                emaAlpha = ball.emaAlpha,
            )
        ).getJSONObject("physics")

        assertEquals(ball.restitution, physics.getDouble("bounce_restitution"), 0.0001)
        assertEquals(ball.sensitivity, physics.getDouble("imu_sensitivity_multiplier"), 0.0001)
        assertEquals(ball.emaAlpha, physics.getDouble("ema_alpha"), 0.0001)
    }

    @Test
    fun stopAndResumeUseTheRightCommandWord() {
        assertEquals("stop", JSONObject(GameCommand.stop("A", 1, 5, 10)).getString("command"))
        assertEquals("resume", JSONObject(GameCommand.resume("A", 1, 5)).getString("command"))
    }

    // two commands should never share a request id (so the esp can tell them apart)
    @Test
    fun eachCommandGetsItsOwnRequestId() {
        val a = JSONObject(GameCommand.start("A", 1, 5, 10)).getJSONObject("meta").getString("request_id")
        val b = JSONObject(GameCommand.start("A", 1, 5, 10)).getJSONObject("meta").getString("request_id")
        assertNotEquals(a, b)
    }
}
