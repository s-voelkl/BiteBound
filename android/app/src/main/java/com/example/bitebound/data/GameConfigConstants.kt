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
    
    const val DEFAULT_PLAYER_NAME = "Cookie-Lover"
    const val DEFAULT_COOKIES_COUNT = 10
    const val DEFAULT_WALL_THICKNESS_PX = 10
    
    const val MIN_COOKIES = 1
    const val MAX_COOKIES = 20
    
    const val MIN_WALL_THICKNESS = 5
    const val MAX_WALL_THICKNESS = 40

    const val TELEMETRY_TIMEOUT_MS = 30000L
}

/**
 * Instead of letting the user type a raw restitution number we let them pick a
 * ball. Each ball just maps to a bounce value we drop into the start command.
 * The numbers are picked by feel (gummy bounces a lot, steel barely), not from
 * any real physics table.
 */
enum class BallType(val label: String, val restitution: Double) {
    STEEL("Steel", 0.30),
    FOOTBALL("Football", 0.60),
    GUMMY("Gummy", 0.90);

    companion object {
        val DEFAULT = STEEL

        /** Figure out which ball a stored restitution belongs to (used on load). */
        fun fromRestitution(value: Double): BallType =
            entries.minByOrNull { kotlin.math.abs(it.restitution - value) } ?: DEFAULT
    }
}
