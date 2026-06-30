package com.example.bitebound.data

/**
 * Default values copied over from esp32/config.h. Kept in one spot so we send
 * the same numbers the firmware starts with instead of hard-coding them all
 * over the UI.
 */
object GameConfigConstants {
    // mirrored from config.h
    const val DEFAULT_IMU_SENSITIVITY_MULTIPLIER = 100.0
    const val DEFAULT_BOUNCE_RESTITUTION = 0.3
    const val DEFAULT_EMA_ALPHA = 0.5
    const val DEFAULT_DEADZONE_THRESHOLD = 0.04
    const val DEFAULT_MAX_SPEED = 300.0 // px/s
    
    const val GAME_ID_LABYRINTH = 1
    const val GAME_ID_FLATLAND = 2
    
    const val DEFAULT_PLAYER_NAME = "Cookie-Monster"
    const val DEFAULT_COOKIES_COUNT = 10
    const val DEFAULT_WALL_THICKNESS_PX = 10
    const val MAX_COOKIES = 20
    const val MAX_WALL_THICKNESS = 20
    const val TELEMETRY_TIMEOUT_MS = 60000L
}

/**
 * Instead of typing raw physics numbers the user picks a ball. Each ball maps to
 * a feel: restitution = how much it bounces, sensitivity = how strongly a tilt
 * pushes it (lower sensitivity = heavier ball, you have to hold the tilt longer
 * to get it moving). Numbers are picked by feel, not from a real physics table.
 */
enum class BallType(
    val label: String,
    val restitution: Double,
    val sensitivity: Double,
    val emaAlpha: Double,
) {
    // emaAlpha: lower = more input lag (the tilt has to build up over more frames
    // before the ball reacts), which adds to the "heavy, hold-it-longer" feel.
    CHONK("Chonk", 0.15, 55.0, 0.3),
    CHOMPER("Chomper", 0.40, 80.0, 0.5),
    SCRAPPY("Scrappy", 0.75, 100.0, 0.6);

    companion object {
        val DEFAULT = CHOMPER

        /** Figure out which ball a stored restitution belongs to (used on load). */
        fun fromRestitution(value: Double): BallType =
            entries.minByOrNull { kotlin.math.abs(it.restitution - value) } ?: DEFAULT
    }
}
