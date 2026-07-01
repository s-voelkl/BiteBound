/**
 * Struct initialization factories with robust fallbacks.
 */

export function createDefaultDevice(o = {}) {
    return {
        clientId: o.client_id || "—",
        hardware: o.hardware || "—",
        firmwareVersion: o.firmware_version || "—",
        uptimeMs: typeof o.uptime_ms === 'number' ? o.uptime_ms : 0,
        wifiSsid: o.wifi_ssid || "—"
    };
}

export function createDefaultGameConfig(o = {}) {
    return {
        gameId: typeof o.game_id === 'number' ? o.game_id : 1,
        playerName: o.player_name || "—",
        targetCookies: typeof o.target_cookies === 'number' ? o.target_cookies : 0,
        screenWidth: typeof o.screen_width === 'number' ? o.screen_width : 240,
        screenHeight: typeof o.screen_height === 'number' ? o.screen_height : 280,
        wallThicknessPx: typeof o.wall_thickness_px === 'number' ? o.wall_thickness_px : 0
    };
}

export function createDefaultGameState(o = {}) {
    const status = o.runningStatus || "idle";
    return {
        runningStatus: status,
        cookiesCollected: typeof o.cookies_collected === 'number' ? o.cookies_collected : 0,
        cookiesRemaining: typeof o.cookies_remaining === 'number' ? o.cookies_remaining : 0,
        currentRound: typeof o.current_round === 'number' ? o.current_round : 0,
        elapsedTimeSec: typeof o.elapsed_time_sec === 'number' ? o.elapsed_time_sec : 0.0,
        isRunning: status.toLowerCase() === "running",
        isFinished: status.toLowerCase() === "completed"
    };
}

export function createDefaultPhysics(o = {}) {
    return {
        ballPosX: typeof o.ball_pos_x === 'number' ? o.ball_pos_x : 0.0,
        ballPosY: typeof o.ball_pos_y === 'number' ? o.ball_pos_y : 0.0,
        velocityX: typeof o.velocity_x === 'number' ? o.velocity_x : 0.0,
        velocityY: typeof o.velocity_y === 'number' ? o.velocity_y : 0.0,
        accX: typeof o.acc_x === 'number' ? o.acc_x : 0.0,
        accY: typeof o.acc_y === 'number' ? o.acc_y : 0.0,
        collisionDetected: !!o.collision_detected
    };
}

export function createDefaultSensors(o = {}) {
    return {
        accelX: typeof o.accel_x === 'number' ? o.accel_x : 0.0,
        accelY: typeof o.accel_y === 'number' ? o.accel_y : 0.0,
        accelZ: typeof o.accel_z === 'number' ? o.accel_z : 0.0,
        gyroX: typeof o.gyro_x === 'number' ? o.gyro_x : 0.0,
        gyroY: typeof o.gyro_y === 'number' ? o.gyro_y : 0.0,
        gyroZ: typeof o.gyro_z === 'number' ? o.gyro_z : 0.0,
        batteryVoltage: typeof o.battery_voltage === 'number' ? o.battery_voltage : 0.0,
        button: !!o.button
    };
}