/**
 * BiteBound Game Orchestrator & UI Controller
 * Establishes real-time reactive bindings between the Node-RED UI Builder 
 * environment and the local HTML rendering layers.
 */

/// <reference path="../types/uibuilder.d.ts"/>

import { GameConfigConstants, BallTypes, getBallTypeByRestitution } from './app/constants.js';
import { formatUptime, formatElapsedTime } from './app/format.js';
import { TelemetryParser } from './app/telemetry.js';
import { GameCommand } from './app/command.js';
import { TelemetryRenderer } from './app/render.js';

// Application State definition
const state = {
    credentials: {
        playerName: GameConfigConstants.DEFAULT_PLAYER_NAME,
        gameId: GameConfigConstants.GAME_ID_LABYRINTH,
        cookiesCount: GameConfigConstants.DEFAULT_COOKIES_COUNT,
        wallThickness: GameConfigConstants.DEFAULT_WALL_THICKNESS_PX,
        imuSensitivity: BallTypes.CHOMPER.sensitivity,
        restitution: BallTypes.CHOMPER.restitution,
        emaAlpha: BallTypes.CHOMPER.emaAlpha,
        deadzone: GameConfigConstants.DEFAULT_DEADZONE_THRESHOLD
    },
    connectionState: "Connected",
    messageCount: 0,
    telemetry: null,
    timeoutJob: null
};

let renderer;
let ctrlSelectedBallType = BallTypes.CHOMPER;

/**
 * Bounds text element ranges on key presses to keep inputs clean.
 */
function setupInputClamping() {
    const restrictAndClamp = (elementId, maxVal) => {
        const el = document.getElementById(elementId);
        if (!el) return;
        el.addEventListener('input', () => {
            const digits = el.value.replace(/\D/g, '');
            if (digits === '') {
                el.value = '';
                return;
            }
            const val = parseInt(digits, 10);
            el.value = Math.min(val, maxVal).toString();
        });
    };

    restrictAndClamp('ctrlCookiesCount', GameConfigConstants.MAX_COOKIES);
    restrictAndClamp('ctrlWallThickness', GameConfigConstants.MAX_WALL_THICKNESS);
}

/**
 * Restores configuration defaults on page loading.
 */
function loadStoredCredentials() {
    const stored = localStorage.getItem('bitebound_credentials');
    if (stored) {
        try {
            state.credentials = { ...state.credentials, ...JSON.parse(stored) };
        } catch (e) {
            console.error("Failed to parse local stored credentials", e);
        }
    }

    // Assign directly to dashboard UI inputs
    const inputPlayer = document.getElementById('ctrlPlayerName');
    const inputCookies = document.getElementById('ctrlCookiesCount');
    const inputWall = document.getElementById('ctrlWallThickness');

    if (inputPlayer) inputPlayer.value = state.credentials.playerName;
    if (inputCookies) inputCookies.value = state.credentials.cookiesCount;
    if (inputWall) inputWall.value = state.credentials.wallThickness;

    // Set presets
    setGameModeSelection(state.credentials.gameId);
    setBallPresetSelection(getBallTypeByRestitution(state.credentials.restitution));
}

/**
 * Toggles configuration inputs and disables layout parameters depending on active mode.
 * Mode 1: Dough Maze (Labyrinth)
 * Mode 2: Baking Tray (Flatland) -> Disables the wall thickness parameter
 * @param {number} modeId - Game mode identifier constant.
 */
function setGameModeSelection(modeId) {
    state.credentials.gameId = modeId;
    
    const isLabyrinth = (modeId === GameConfigConstants.GAME_ID_LABYRINTH);
    const btnLabyrinth = document.getElementById('ctrlModeLabyrinth');
    const btnTray = document.getElementById('ctrlModeFlatland');
    const inputWall = document.getElementById('ctrlWallThickness');
    
    if (isLabyrinth) {
        btnLabyrinth?.classList.add('active');
        btnTray?.classList.remove('active');
        if (inputWall) {
            inputWall.disabled = false;
            inputWall.placeholder = "10";
        }
    } else {
        btnLabyrinth?.classList.remove('active');
        btnTray?.classList.add('active');
        if (inputWall) {
            inputWall.disabled = true;
            inputWall.value = "";
            inputWall.placeholder = "N/A (Baking Tray)";
        }
    }
}

/**
 * Assigns active kinetic preset values for physical movements.
 */
function setBallPresetSelection(ball) {
    ctrlSelectedBallType = ball;
    ['Chonk', 'Chomper', 'Scrappy'].forEach(b => {
        const btn = document.getElementById('ctrlBall' + b);
        if (ball.label === b) btn?.classList.add('active');
        else btn?.classList.remove('active');
    });
}

/**
 * Updates progress ring variables programmatically.
 * Calculation scale uses the adjusted 180px ring diameter
 */
function updateCookieProgressRing(collected, target) {
    const circle = document.getElementById('scoreCircle');
    const labelCollected = document.getElementById('textCollected');
    const labelTarget = document.getElementById('textTarget');
    const labelPercent = document.getElementById('textPercent');

    if (!circle) return;

    const fraction = target > 0 ? Math.min(Math.max(collected / target, 0), 1) : 0;
    const percent = Math.round(fraction * 100);

    // Circumference: 2 * PI * r = 2 * 3.14159 * 78 = 490.08
    const circumference = 490.08;
    const offset = circumference - (fraction * circumference);
    circle.style.strokeDashoffset = offset;

    labelCollected.textContent = collected;
    labelTarget.textContent = `of ${target}`;
    labelPercent.textContent = `${percent}%`;
}

/**
 * Emits dynamic game properties over the WebSocket pipeline to the broker.
 */
function triggerStartCommand() {
    const pName = document.getElementById('ctrlPlayerName').value.trim() || "Cookie-Monster";
    const cookies = parseInt(document.getElementById('ctrlCookiesCount').value, 10) || 10;
    const wall = parseInt(document.getElementById('ctrlWallThickness').value, 10) || 10;
    
    const params = {
        playerName: pName,
        gameId: state.credentials.gameId,
        cookiesCount: cookies,
        wallThicknessPx: wall,
        imuSensitivity: ctrlSelectedBallType.sensitivity,
        restitution: ctrlSelectedBallType.restitution,
        emaAlpha: ctrlSelectedBallType.emaAlpha,
        deadzone: state.credentials.deadzone
    };

    GameCommand.send(window.uibuilder, 'start', params);
}

/**
 * Initializes listeners on page load.
 */
window.addEventListener('DOMContentLoaded', () => {
    renderer = new TelemetryRenderer();

    // Attach control element triggers
    document.getElementById('ctrlModeLabyrinth').addEventListener('click', () => setGameModeSelection(1));
    document.getElementById('ctrlModeFlatland').addEventListener('click', () => setGameModeSelection(2));

    document.getElementById('ctrlBallChonk').addEventListener('click', () => setBallPresetSelection(BallTypes.CHONK));
    document.getElementById('ctrlBallChomper').addEventListener('click', () => setBallPresetSelection(BallTypes.CHOMPER));
    document.getElementById('ctrlBallScrappy').addEventListener('click', () => setBallPresetSelection(BallTypes.SCRAPPY));

    // Modal Confirmation Handlers
    const newGameModal = document.getElementById('newGameModal');
    document.getElementById('btnNewGame').addEventListener('click', () => {
        newGameModal.classList.remove('hidden');
    });
    document.getElementById('btnConfirmCancel').addEventListener('click', () => {
        newGameModal.classList.add('hidden');
    });
    document.getElementById('btnConfirmStart').addEventListener('click', () => {
        newGameModal.classList.add('hidden');
        
        // Cache parameters locally
        state.credentials.playerName = document.getElementById('ctrlPlayerName').value.trim();
        state.credentials.cookiesCount = parseInt(document.getElementById('ctrlCookiesCount').value, 10) || 10;
        state.credentials.wallThickness = parseInt(document.getElementById('ctrlWallThickness').value, 10) || 10;
        localStorage.setItem('bitebound_credentials', JSON.stringify(state.credentials));

        triggerStartCommand();
    });

    // Pause & Resume Control Handlers
    const pauseResumeBtn = document.getElementById('btnPauseResume');
    pauseResumeBtn.addEventListener('click', () => {
        const isRunning = state.telemetry && state.telemetry.state.isRunning;
        if (isRunning) {
            const params = {
                playerName: document.getElementById('ctrlPlayerName').value.trim() || state.credentials.playerName,
                gameId: state.credentials.gameId,
                cookiesCount: parseInt(document.getElementById('ctrlCookiesCount').value, 10) || 10,
                wallThicknessPx: parseInt(document.getElementById('ctrlWallThickness').value, 10) || 10
            };
            GameCommand.send(window.uibuilder, 'stop', params);
        } else {
            const pName = document.getElementById('ctrlPlayerName').value.trim() || state.credentials.playerName;
            GameCommand.send(window.uibuilder, 'resume', { playerName: pName });
        }
    });

    setupInputClamping();
    loadStoredCredentials();

    // Listen to WebSocket messages
    window.uibuilder.onChange('msg', (msg) => {
        if (!msg || !msg.payload) return;

        // Skip configuration echo notifications
        if (msg.topic && msg.topic.indexOf('telemetry') === -1) return;

        const telemetry = TelemetryParser.parse(msg.payload);
        if (!telemetry) return;

        state.messageCount++;
        state.telemetry = telemetry;

        // Reveal active views
        document.getElementById('placeholderOven').classList.add('hidden');
        document.getElementById('activeTelemetryGrid').classList.remove('hidden');

        // Render configuration headers and statuses
        const statusText = telemetry.state.runningStatus.charAt(0).toUpperCase() + telemetry.state.runningStatus.slice(1);
        document.getElementById('connectionBannerText').textContent = `${statusText} · ${state.messageCount} updates`;

        const bannerDot = document.querySelector('.status-dot');
        if (telemetry.state.isRunning) {
            bannerDot.style.backgroundColor = 'var(--mint-green)';
        } else if (telemetry.state.runningStatus === 'completed') {
            bannerDot.style.backgroundColor = 'var(--honey)';
        } else {
            bannerDot.style.backgroundColor = 'var(--text-muted)';
        }

        // Toggle round banner announcements
        const completeBanner = document.getElementById('roundCompleteAnnouncement');
        if (telemetry.state.isFinished) {
            completeBanner.classList.remove('hidden');
        } else {
            completeBanner.classList.add('hidden');
        }

        updateCookieProgressRing(telemetry.state.cookiesCollected, telemetry.config.targetCookies);
        document.getElementById('gameStateSubline').textContent = 
            `Player: ${telemetry.config.playerName}  ·  Remaining: ${telemetry.state.cookiesRemaining}`;

        // Toggle command button state
        const playBtn = document.getElementById('btnPauseResume');
        playBtn.disabled = false;
        if (telemetry.state.isRunning) {
            playBtn.textContent = "Pause";
            playBtn.className = "btn btn-danger btn-compact";
        } else {
            playBtn.textContent = "Resume";
            playBtn.className = "btn btn-warning btn-compact";
        }

        // Execute rendering loop
        renderer.update(telemetry, state.connectionState);
    });
});