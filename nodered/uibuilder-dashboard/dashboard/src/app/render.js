import { DEFAULT_COOKIES, DEFAULT_PLAYER_NAME, DEFAULT_WALL_THICKNESS_PX } from './constants.js';
import { formatDuration, formatNumber, formatStatus, progressFraction } from './format.js';

/** @typedef {import('./types.js').DashboardState} DashboardState */

/**
 * @param {DashboardState} state
 */
export function render(state) {
    renderConnection(state);
    renderHero(state);
    renderBoard(state);
    renderSensors(state);
    renderDevice(state);
}

/**
 * @param {DashboardState} state
 */
function renderConnection(state) {
    const badge = document.querySelector('#connection-status');
    const counter = document.querySelector('#message-count');
    const error = document.querySelector('#connection-error');

    if (!(badge instanceof HTMLElement) || !(counter instanceof HTMLElement) || !(error instanceof HTMLElement)) {
        return;
    }

    badge.classList.remove('connected', 'connecting', 'disconnected');
    badge.classList.add(state.connection);

    const label = {
        connected: 'Connected',
        connecting: 'Connecting',
        disconnected: 'Disconnected',
    }[state.connection];

    badge.textContent = label;
    counter.textContent = `${state.messageCount} updates`;
    error.textContent = state.lastError || 'No connection errors.';
}

/**
 * @param {DashboardState} state
 */
function renderHero(state) {
    const telemetry = state.telemetry;
    const fallbackTarget = DEFAULT_COOKIES;
    const collected = telemetry?.state.cookies_collected ?? 0;
    const target = telemetry?.config.target_cookies ?? fallbackTarget;

    setText('#cookies-collected', String(collected));
    setText('#cookies-target', String(target));
    setText('#round-value', String(telemetry?.state.current_round ?? 0));
    setText('#time-value', formatDuration(telemetry?.state.elapsed_time_sec ?? 0));
    setText('#running-status-value', formatStatus(telemetry?.state.runningStatus ?? 'idle'));

    const player = telemetry?.config.player_name || DEFAULT_PLAYER_NAME;
    const remaining = telemetry?.state.cookies_remaining ?? target;
    setText('#player-summary', `Player: ${player} · Remaining: ${remaining}`);

    const ring = document.querySelector('#cookie-progress');
    if (ring instanceof HTMLElement) {
        const percent = Math.round(progressFraction(collected, target) * 100);
        ring.style.setProperty('--progress', `${percent}%`);
    }
}

/**
 * @param {DashboardState} state
 */
function renderBoard(state) {
    const telemetry = state.telemetry;
    if (!telemetry) {
        setText('#board-empty', 'Waiting for telemetry from the ESP32.');
        setText('#board-pos-x', '0');
        setText('#board-pos-y', '0');
        setText('#board-vel-x', '0');
        setText('#board-vel-y', '0');
        setText('#board-acc-x', '0');
        setText('#board-collision', 'No');
        return;
    }

    setText('#board-empty', '');

    const width = Math.max(1, telemetry.config.screen_width || 240);
    const height = Math.max(1, telemetry.config.screen_height || 280);
    const x = clamp(telemetry.physics.ball_pos_x / width, 0, 1);
    const y = clamp(telemetry.physics.ball_pos_y / height, 0, 1);

    const ball = document.querySelector('#board-ball');
    if (ball instanceof HTMLElement) {
        ball.style.left = `${x * 100}%`;
        ball.style.top = `${y * 100}%`;
        ball.classList.toggle('collision', Boolean(telemetry.physics.collision_detected));
    }

    setText('#board-pos-x', formatNumber(telemetry.physics.ball_pos_x, 0));
    setText('#board-pos-y', formatNumber(telemetry.physics.ball_pos_y, 0));
    setText('#board-vel-x', formatNumber(telemetry.physics.velocity_x, 2));
    setText('#board-vel-y', formatNumber(telemetry.physics.velocity_y, 2));
    setText('#board-acc-x', formatNumber(telemetry.physics.acc_x, 2));
    setText('#board-collision', telemetry.physics.collision_detected ? 'Yes' : 'No');
}

/**
 * @param {DashboardState} state
 */
function renderSensors(state) {
    const telemetry = state.telemetry;

    setText('#sensor-acc-x', formatNumber(telemetry?.sensors.accel_x ?? 0, 2));
    setText('#sensor-acc-y', formatNumber(telemetry?.sensors.accel_y ?? 0, 2));
    setText('#sensor-acc-z', formatNumber(telemetry?.sensors.accel_z ?? 0, 2));
    setText('#sensor-gyro-x', formatNumber(telemetry?.sensors.gyro_x ?? 0, 3));
    setText('#sensor-gyro-y', formatNumber(telemetry?.sensors.gyro_y ?? 0, 3));
    setText('#sensor-gyro-z', formatNumber(telemetry?.sensors.gyro_z ?? 0, 3));
    setText('#sensor-battery', formatNumber(telemetry?.sensors.battery_voltage ?? 0, 3));
    setText('#sensor-button', telemetry?.sensors.button ? 'Pressed' : 'Released');
}

/**
 * @param {DashboardState} state
 */
function renderDevice(state) {
    const telemetry = state.telemetry;

    setText('#device-client-id', telemetry?.device.client_id ?? '—');
    setText('#device-hardware', telemetry?.device.hardware ?? '—');
    setText('#device-firmware', telemetry?.device.firmware_version ?? '—');
    setText('#device-wifi', telemetry?.device.wifi_ssid ?? '—');
    setText('#device-uptime', formatDuration((telemetry?.device.uptime_ms ?? 0) / 1000));
}

/**
 * @param {string} selector
 * @param {string} value
 */
function setText(selector, value) {
    const element = document.querySelector(selector);
    if (element instanceof HTMLElement) {
        element.textContent = value;
    }
}

/**
 * @param {number} value
 * @param {number} min
 * @param {number} max
 * @returns {number}
 */
function clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
}

/**
 * Syncs controls to telemetry defaults unless the user has already typed.
 * @param {DashboardState} state
 */
export function syncControlsWithTelemetry(state) {
    const telemetry = state.telemetry;
    if (!telemetry) return;

    const player = document.querySelector('#player-name');
    const cookies = document.querySelector('#cookies-count');
    const gameId = document.querySelector('#game-id');
    const wallThickness = document.querySelector('#wall-thickness');

    if (player instanceof HTMLInputElement && !player.dataset.dirty) {
        player.value = telemetry.config.player_name || DEFAULT_PLAYER_NAME;
    }
    if (cookies instanceof HTMLInputElement && !cookies.dataset.dirty) {
        cookies.value = String(telemetry.config.target_cookies || DEFAULT_COOKIES);
    }
    if (gameId instanceof HTMLInputElement && !gameId.dataset.dirty) {
        gameId.value = String(telemetry.config.game_id || 1);
    }
    if (wallThickness instanceof HTMLInputElement && !wallThickness.dataset.dirty) {
        wallThickness.value = String(telemetry.config.wall_thickness_px || DEFAULT_WALL_THICKNESS_PX);
    }
}
