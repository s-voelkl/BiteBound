/** @typedef {import('./types.js').Telemetry} Telemetry */

/**
 * Creates a full telemetry object with safe defaults.
 * @returns {Telemetry}
 */
function emptyTelemetry() {
    return {
        device: {
            client_id: '—',
            hardware: '—',
            firmware_version: '—',
            uptime_ms: 0,
            wifi_ssid: '—',
        },
        config: {
            game_id: 1,
            player_name: '—',
            target_cookies: 0,
            screen_width: 240,
            screen_height: 280,
            wall_thickness_px: 6,
        },
        state: {
            status: 'idle',
            cookies_collected: 0,
            cookies_remaining: 0,
            current_round: 0,
            elapsed_time_sec: 0,
        },
        physics: {
            ball_pos_x: 0,
            ball_pos_y: 0,
            velocity_x: 0,
            velocity_y: 0,
            acc_x: 0,
            acc_y: 0,
            collision_detected: false,
        },
        sensors: {
            accel_x: 0,
            accel_y: 0,
            accel_z: 0,
            gyro_x: 0,
            gyro_y: 0,
            gyro_z: 0,
            battery_voltage: 0,
            button: false,
        },
    };
}

/**
 * @param {unknown} payload
 * @returns {Telemetry|null}
 */
export function parseTelemetry(payload) {
    try {
        const source = typeof payload === 'string' ? JSON.parse(payload) : payload;
        if (!source || typeof source !== 'object') return null;

        const base = emptyTelemetry();
        const root = /** @type {Record<string, any>} */ (source);

        const telemetry = {
            ...base,
            device: { ...base.device, ...(root.device || {}) },
            config: { ...base.config, ...(root.config || {}) },
            state: { ...base.state, ...(root.state || {}) },
            physics: { ...base.physics, ...(root.physics || {}) },
            sensors: { ...base.sensors, ...(root.sensors || {}) },
        };

        if (!('device' in root) || !('config' in root) || !('state' in root)) return null;
        return /** @type {Telemetry} */ (telemetry);
    } catch (error) {
        return null;
    }
}
