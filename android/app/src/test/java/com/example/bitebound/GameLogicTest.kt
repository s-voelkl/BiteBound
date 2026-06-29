package com.example.bitebound

import com.example.bitebound.data.BallType
import com.example.bitebound.data.GameState
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class GameLogicTest {

    private fun toggleLabel(state: GameState) = if (state.isRunning) "Pause" else "Resume"

    @Test
    fun idleShowsResume() {
        val state = GameState("idle", 0, 0, 1, 0.0)
        assertEquals("Resume", toggleLabel(state))
    }

    @Test
    fun runningShowsPause() {
        val state = GameState("running", 2, 8, 1, 5.0)
        assertEquals("Pause", toggleLabel(state))
    }

    @Test
    fun completedRoundIsFinished() {
        val state = GameState("completed", 10, 0, 3, 60.0)
        assertTrue(state.isFinished)
        assertFalse(state.isRunning)
    }

    // steel should be heavier -> lower sensitivity than the others
    @Test
    fun steelIsHeavier() {
        assertTrue(BallType.STEEL.sensitivity < BallType.FOOTBALL.sensitivity)
        assertTrue(BallType.STEEL.sensitivity < BallType.GUMMY.sensitivity)
    }

    // gummy bounces more than steel
    @Test
    fun gummyBouncesMore() {
        assertTrue(BallType.GUMMY.restitution > BallType.STEEL.restitution)
    }

    // picking a ball and reading it back should give the same ball
    @Test
    fun ballRoundTrips() {
        assertEquals(BallType.GUMMY, BallType.fromRestitution(BallType.GUMMY.restitution))
        assertEquals(BallType.STEEL, BallType.fromRestitution(BallType.STEEL.restitution))
    }
}
