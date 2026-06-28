/**
 * Shared JSDoc typedefs for the BiteBound dashboard.
 * These keep the code self-documenting without requiring a TS build step.
 */

/**
 * @typedef {Object} TelemetryDevice
 * @property {string} client_id
 * @property {string} hardware
 * @property {string} firmware_version
 * @property {number} uptime_ms
 * @property {string} wifi_ssid
 */

/**
 * @typedef {Object} TelemetryConfig
 * @property {number} game_id
 * @property {string} player_name
 * @property {number} target_cookies
 * @property {number} screen_width
 * @property {number} screen_height
 * @property {number} wall_thickness_px
 */

/**
 * @typedef {Object} TelemetryState
 * @property {string} runningStatus
 * @property {number} cookies_collected
 * @property {number} cookies_remaining
 * @property {number} current_round
 * @property {number} elapsed_time_sec
 */

/**
 * @typedef {Object} TelemetryPhysics
 * @property {number} ball_pos_x
 * @property {number} ball_pos_y
 * @property {number} velocity_x
 * @property {number} velocity_y
 * @property {number} acc_x
 * @property {number} acc_y
 * @property {boolean} collision_detected
 */

/**
 * @typedef {Object} TelemetrySensors
 * @property {number} accel_x
 * @property {number} accel_y
 * @property {number} accel_z
 * @property {number} gyro_x
 * @property {number} gyro_y
 * @property {number} gyro_z
 * @property {number} battery_voltage
 * @property {boolean} button
 */

/**
 * @typedef {Object} Telemetry
 * @property {TelemetryDevice} device
 * @property {TelemetryConfig} config
 * @property {TelemetryState} state
 * @property {TelemetryPhysics} physics
 * @property {TelemetrySensors} sensors
 */

/**
 * @typedef {'connected'|'connecting'|'disconnected'} DashboardConnection
 */

/**
 * @typedef {Object} DashboardState
 * @property {DashboardConnection} connection
 * @property {Telemetry|null} telemetry
 * @property {number} messageCount
 * @property {string|null} lastError
 */

export {};
