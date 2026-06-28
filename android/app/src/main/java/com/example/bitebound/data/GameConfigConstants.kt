package com.example.bitebound.data

/**
 * Constants mirrored from esp32/config.h and default values for the Android app.
 */
object GameConfigConstants {
    // From config.h
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
    const val MAX_COOKIES = 1000
    
    const val MIN_WALL_THICKNESS = 5
    const val MAX_WALL_THICKNESS = 40

    const val TELEMETRY_TIMEOUT_MS = 30000L
}
