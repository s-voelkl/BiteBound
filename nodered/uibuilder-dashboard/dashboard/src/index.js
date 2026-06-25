// Give VS Code IntelliSense for uibuilder
/// <reference path="../types/uibuilder.d.ts"/>

import { buildGameCommand } from './app/commands.js';
import { DEFAULT_COOKIES, DEFAULT_GAME_ID, DEFAULT_PLAYER_NAME, DEFAULT_WALL_THICKNESS_PX } from './app/constants.js';
import { render, syncControlsWithTelemetry } from './app/render.js';
import { parseTelemetry } from './app/telemetry.js';

/** @typedef {import('./app/types.js').DashboardState} DashboardState */

/** @type {DashboardState} */
const state = {
    connection: 'connecting',
    telemetry: null,
    messageCount: 0,
    lastError: null,
};

/**
 * Reads game control inputs and applies safe defaults.
 */
function readControlValues() {
    const playerInput = document.querySelector('#player-name');
    const cookiesInput = document.querySelector('#cookies-count');
    const gameIdInput = document.querySelector('#game-id');
    const wallThicknessInput = document.querySelector('#wall-thickness');

    const playerName = playerInput instanceof HTMLInputElement
        ? playerInput.value.trim() || DEFAULT_PLAYER_NAME
        : DEFAULT_PLAYER_NAME;
    const cookiesCount = cookiesInput instanceof HTMLInputElement
        ? toPositiveInt(cookiesInput.value, DEFAULT_COOKIES)
        : DEFAULT_COOKIES;
    const gameId = gameIdInput instanceof HTMLInputElement
        ? toPositiveInt(gameIdInput.value, DEFAULT_GAME_ID)
        : DEFAULT_GAME_ID;
    const wallThicknessPx = wallThicknessInput instanceof HTMLInputElement
        ? toPositiveInt(wallThicknessInput.value, DEFAULT_WALL_THICKNESS_PX)
        : DEFAULT_WALL_THICKNESS_PX;

    return { playerName, cookiesCount, gameId, wallThicknessPx };
}

/**
 * @param {'start'|'stop'} command
 */
function sendCommand(command) {
    const values = readControlValues();
    const payload = buildGameCommand(
        command,
        values.playerName,
        values.gameId,
        values.cookiesCount,
        values.wallThicknessPx,
    );

    uibuilder.send({
        topic: 'game/command',
        payload,
    });
}

function markDirtyOnInput() {
    const selectors = ['#player-name', '#cookies-count', '#game-id', '#wall-thickness'];
    for (const selector of selectors) {
        const element = document.querySelector(selector);
        if (element instanceof HTMLInputElement) {
            element.addEventListener('input', () => {
                element.dataset.dirty = 'true';
            });
        }
    }
}

function bindUiEvents() {
    const startButton = document.querySelector('#start-game');
    const stopButton = document.querySelector('#stop-game');

    if (startButton instanceof HTMLButtonElement) {
        startButton.addEventListener('click', () => {
            sendCommand('start');
        });
    }

    if (stopButton instanceof HTMLButtonElement) {
        stopButton.addEventListener('click', () => {
            sendCommand('stop');
        });
    }

    markDirtyOnInput();
}

function renderAll() {
    render(state);
}

function init() {
    bindUiEvents();

    uibuilder.start();
    state.connection = uibuilder.ioConnected ? 'connected' : 'connecting';
    renderAll();

    // React to Node-RED transport connectivity.
    uibuilder.onChange('ioConnected', (connected) => {
        state.connection = connected ? 'connected' : 'disconnected';
        if (connected) {
            state.lastError = null;
        }
        renderAll();
    });

    // Telemetry arrives via Node-RED MQTT-in -> uibuilder.
    uibuilder.onChange('msg', (msg) => {
        const telemetry = parseTelemetry(msg.payload);
        if (!telemetry) {
            return;
        }

        state.telemetry = telemetry;
        state.messageCount += 1;
        syncControlsWithTelemetry(state);
        renderAll();
    });

    // Capture transport errors so users can diagnose setup issues quickly.
    uibuilder.onChange('sentCtrlMsg', () => {
        // intentionally no-op; keeps ctrl stream active for diagnostics in dev tools
    });
}

/**
 * @param {string} value
 * @param {number} fallback
 */
function toPositiveInt(value, fallback) {
    const parsed = Number.parseInt(value, 10);
    return Number.isInteger(parsed) && parsed > 0 ? parsed : fallback;
}

init();