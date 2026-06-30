// Give VS Code IntelliSense for uibuilder
/// <reference path="../types/uibuilder.d.ts"/>

import { GameConfigConstants, BallTypes, getBallTypeByRestitution } from './app/constants.js';
import { formatUptime, formatElapsedTime, formatBatteryPercentage } from './app/format.js';
import { TelemetryParser } from './app/telemetry.js';
import { GameCommand } from './app/command.js';
import { TelemetryRenderer } from './app/render.js';

// Application State definition mirroring com.example.bitebound.UiState Jvm snapshot
const state = {
    credentials: {
        host: "61775de8f674480cae63ae7690a8ffae.s1.eu.hivemq.cloud",
        port: 8883,
        username: "BiteBound",
        password: "",
        telemetryTopic: "mauc2026/group_03/game/telemetry",
        commandTopic: "mauc2026/group_03/game/command",
        playerName: GameConfigConstants.DEFAULT_PLAYER_NAME,
        gameId: GameConfigConstants.GAME_ID_LABYRINTH,
        cookiesCount: GameConfigConstants.DEFAULT_COOKIES_COUNT,
        wallThickness: GameConfigConstants.DEFAULT_WALL_THICKNESS_PX,
        imuSensitivity: BallTypes.CHOMPER.sensitivity,
        restitution: BallTypes.CHOMPER.restitution,
        emaAlpha: BallTypes.CHOMPER.emaAlpha,
        deadzone: GameConfigConstants.DEFAULT_DEADZONE_THRESHOLD
    },
    connectionState: "Disconnected", // Disconnected | Connecting | Connected | Failed
    messageCount: 0,
    telemetry: null,
    timeoutJob: null,
    connectionError: ""
};

// UI Elements & State Tracking bindings
let renderer;
let selectedBallType = BallTypes.CHOMPER;
let ctrlSelectedBallType = BallTypes.CHOMPER;

/**
 * Validates connection parameters before submission.
 */
function getMissingCredentialsList() {
    const missing = [];
    const pName = document.getElementById('inputPlayerName').value.trim();
    const host = document.getElementById('inputBrokerHost').value.trim();
    const port = parseInt(document.getElementById('inputBrokerPort').value.trim(), 10);
    const user = document.getElementById('inputUsername').value.trim();
    const pass = document.getElementById('inputPassword').value;

    if (!pName) missing.push("Player Name");
    if (!host) missing.push("Broker Host Address");
    if (isNaN(port) || port < 1 || port > 65535) missing.push("Port (1-65535)");
    if (!user) missing.push("Username");
    if (!pass) missing.push("Password");

    return missing;
}

/**
 * Clamps input elements to game thresholds during input.
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

    restrictAndClamp('inputCookiesCount', GameConfigConstants.MAX_COOKIES);
    restrictAndClamp('inputWallThickness', GameConfigConstants.MAX_WALL_THICKNESS);
    restrictAndClamp('ctrlCookiesCount', GameConfigConstants.MAX_COOKIES);
    restrictAndClamp('ctrlWallThickness', GameConfigConstants.MAX_WALL_THICKNESS);
}

/**
 * Dynamic input validation watcher.
 */
function validateConnectionInputs() {
    const missing = getMissingCredentialsList();
    const btn = document.getElementById('btnConnect');
    const notice = document.getElementById('missingFieldsNotice');
    const listSpan = document.getElementById('missingFieldsList');

    if (state.connectionState === 'Connecting') {
        btn.disabled = true;
        btn.textContent = "Connecting…";
        notice.classList.add('hidden');
        return;
    }

    btn.textContent = "Bake the connection 🍪";

    if (missing.length > 0) {
        btn.disabled = true;
        notice.classList.remove('hidden');
        listSpan.textContent = missing.join(", ");
    } else {
        btn.disabled = false;
        notice.classList.add('hidden');
    }
}

/**
 * Initializes form fields on load using values retrieved from localStorage.
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

    // Assign back to UI inputs
    document.getElementById('inputPlayerName').value = state.credentials.playerName;
    document.getElementById('inputBrokerHost').value = state.credentials.host;
    document.getElementById('inputBrokerPort').value = state.credentials.port;
    document.getElementById('inputUsername').value = state.credentials.username;
    document.getElementById('inputPassword').value = state.credentials.password;
    document.getElementById('inputTelemetryTopic').value = state.credentials.telemetryTopic;
    document.getElementById('inputCommandTopic').value = state.credentials.commandTopic;
    document.getElementById('inputCookiesCount').value = state.credentials.cookiesCount;
    document.getElementById('inputWallThickness').value = state.credentials.wallThickness;

    // Toggle presets
    setGameModeSelection(state.credentials.gameId);
    setBallPresetSelection(getBallTypeByRestitution(state.credentials.restitution));
}

function setGameModeSelection(modeId) {
    state.credentials.gameId = modeId;
    
    const elements = [
        { btn: 'btnModeLabyrinth', formGroup: 'wallThicknessFormGroup' },
        { btn: 'ctrlModeLabyrinth', formGroup: 'ctrlWallThicknessFormGroup' }
    ];

    elements.forEach(item => {
        const btnLabyrinth = document.getElementById(item.btn);
        const btnTray = document.getElementById(item.btn.replace('Labyrinth', 'Flatland'));
        const formGroup = document.getElementById(item.formGroup);

        if (modeId === GameConfigConstants.GAME_ID_LABYRINTH) {
            btnLabyrinth?.classList.add('active');
            btnTray?.classList.remove('active');
            formGroup?.classList.remove('hidden');
        } else {
            btnLabyrinth?.classList.remove('active');
            btnTray?.classList.add('active');
            formGroup?.classList.add('hidden');
        }
    });
}

function setBallPresetSelection(ball, isControlGroup = false) {
    if (isControlGroup) {
        ctrlSelectedBallType = ball;
        ['Chonk', 'Chomper', 'Scrappy'].forEach(b => {
            const btn = document.getElementById('ctrlBall' + b);
            if (ball.label === b) btn?.classList.add('active');
            else btn?.classList.remove('active');
        });
    } else {
        selectedBallType = ball;
        state.credentials.restitution = ball.restitution;
        state.credentials.imuSensitivity = ball.sensitivity;
        state.credentials.emaAlpha = ball.emaAlpha;
        
        ['Chonk', 'Chomper', 'Scrappy'].forEach(b => {
            const btn = document.getElementById('btnBall' + b);
            if (ball.label === b) btn?.classList.add('active');
            else btn?.classList.remove('active');
        });
    }
}

/**
 * Handles accordions UI.
 */
function setupAccordions() {
    const registerAccordion = (toggleId, panelId) => {
        const toggle = document.getElementById(toggleId);
        const panel = document.getElementById(panelId);
        toggle.addEventListener('click', () => {
            panel.classList.toggle('collapsed');
            const collapsed = panel.classList.contains('collapsed');
            toggle.querySelector('.icon').textContent = collapsed ? '▼' : '▲';
        });
    };

    registerAccordion('toggleServerConnection', 'panelServerConnection');
    registerAccordion('toggleTopics', 'panelTopics');
}

/**
 * Manages game UI switching based on active state.
 */
function updateScreenVisibility() {
    const connScreen = document.getElementById('connectionScreen');
    const dashScreen = document.getElementById('dashboardScreen');

    if (state.connectionState === 'Connected') {
        connScreen.classList.add('hidden');
        dashScreen.classList.remove('hidden');
    } else {
        connScreen.classList.remove('hidden');
        dashScreen.classList.add('hidden');
    }
}

/**
 * Updates progress ring variables programmatically.
 */
function updateCookieProgressRing(collected, target) {
    const circle = document.getElementById('scoreCircle');
    const labelCollected = document.getElementById('textCollected');
    const labelTarget = document.getElementById('textTarget');
    const labelPercent = document.getElementById('textPercent');

    if (!circle) return;

    const fraction = target > 0 ? Math.min(Math.max(collected / target, 0), 1) : 0;
    const percent = Math.round(fraction * 100);

    // Circle Circumference definition: 2 * PI * r = 596.90
    const circumference = 596.90;
    const offset = circumference - (fraction * circumference);
    circle.style.strokeDashoffset = offset;

    labelCollected.textContent = collected;
    labelTarget.textContent = `of ${target}`;
    labelPercent.textContent = `${percent}%`;
}

/**
 * Emits active game details back to the Node-RED broker flow.
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
 * Triggers UI updates from background telemetry timeouts.
 */
function startTelemetryTimeout() {
    stopTelemetryTimeout();
    state.timeoutJob = setTimeout(() => {
        handleDisconnect();
        state.connectionState = "Failed";
        state.connectionError = "Telemetry Timeout — No responses received from the ESP32 in 60s.";
        showConnectionError();
    }, GameConfigConstants.TELEMETRY_TIMEOUT_MS);
}

function stopTelemetryTimeout() {
    if (state.timeoutJob) {
        clearTimeout(state.timeoutJob);
        state.timeoutJob = null;
    }
}

function showConnectionError() {
    const box = document.getElementById('connectionErrorBox');
    if (box && state.connectionError) {
        box.textContent = `🔥 Burnt batch: ${state.connectionError}`;
        box.classList.remove('hidden');
    } else if (box) {
        box.classList.add('hidden');
    }
}

function handleDisconnect() {
    stopTelemetryTimeout();
    state.connectionState = "Disconnected";
    state.telemetry = null;
    state.messageCount = 0;
    state.connectionError = "";
    
    // Hide diagnostic panel metrics
    document.getElementById('activeTelemetryGrid').classList.add('hidden');
    document.getElementById('placeholderOven').classList.remove('hidden');

    updateScreenVisibility();
    validateConnectionInputs();
    showConnectionError();
}

/**
 * Setup entry points and WebSockets listener threads on DOM completion.
 */
window.addEventListener('DOMContentLoaded', () => {
    renderer = new TelemetryRenderer();

    // Event listeners for credential setup
    document.getElementById('btnModeLabyrinth').addEventListener('click', () => setGameModeSelection(1));
    document.getElementById('btnModeFlatland').addEventListener('click', () => setGameModeSelection(2));
    document.getElementById('ctrlModeLabyrinth').addEventListener('click', () => setGameModeSelection(1));
    document.getElementById('ctrlModeFlatland').addEventListener('click', () => setGameModeSelection(2));

    document.getElementById('btnBallChonk').addEventListener('click', () => setBallPresetSelection(BallTypes.CHONK));
    document.getElementById('btnBallChomper').addEventListener('click', () => setBallPresetSelection(BallTypes.CHOMPER));
    document.getElementById('btnBallScrappy').addEventListener('click', () => setBallPresetSelection(BallTypes.SCRAPPY));
    
    document.getElementById('ctrlBallChonk').addEventListener('click', () => setBallPresetSelection(BallTypes.CHONK, true));
    document.getElementById('ctrlBallChomper').addEventListener('click', () => setBallPresetSelection(BallTypes.CHOMPER, true));
    document.getElementById('ctrlBallScrappy').addEventListener('click', () => setBallPresetSelection(BallTypes.SCRAPPY, true));

    // Form validation bindings
    ['inputPlayerName', 'inputBrokerHost', 'inputBrokerPort', 'inputUsername', 'inputPassword'].forEach(id => {
        document.getElementById(id).addEventListener('input', validateConnectionInputs);
    });

    // Eye toggle for password visibility
    document.getElementById('btnTogglePassword').addEventListener('click', () => {
        const input = document.getElementById('inputPassword');
        const isPass = input.type === "password";
        input.type = isPass ? "text" : "password";
    });

    // Connect button action
    document.getElementById('btnConnect').addEventListener('click', () => {
        state.connectionState = "Connecting";
        state.connectionError = "";
        showConnectionError();
        validateConnectionInputs();

        // Save back coordinates to cache
        state.credentials.playerName = document.getElementById('inputPlayerName').value.trim();
        state.credentials.host = document.getElementById('inputBrokerHost').value.trim();
        state.credentials.port = parseInt(document.getElementById('inputBrokerPort').value.trim(), 10) || 8883;
        state.credentials.username = document.getElementById('inputUsername').value.trim();
        state.credentials.password = document.getElementById('inputPassword').value;
        state.credentials.telemetryTopic = document.getElementById('inputTelemetryTopic').value.trim();
        state.credentials.commandTopic = document.getElementById('inputCommandTopic').value.trim();
        state.credentials.cookiesCount = parseInt(document.getElementById('inputCookiesCount').value, 10) || 10;
        state.credentials.wallThickness = parseInt(document.getElementById('inputWallThickness').value, 10) || 10;

        localStorage.setItem('bitebound_credentials', JSON.stringify(state.credentials));

        // Push configuration payload to trigger broker subscriptions in Node-RED
        window.uibuilder.send({
            payload: state.credentials,
            topic: 'bitebound/system/credentials'
        });

        // Initialize dashboard form controls
        document.getElementById('ctrlPlayerName').value = state.credentials.playerName;
        document.getElementById('ctrlCookiesCount').value = state.credentials.cookiesCount;
        document.getElementById('ctrlWallThickness').value = state.credentials.wallThickness;
        setBallPresetSelection(getBallTypeByRestitution(state.credentials.restitution), true);

        // Start active tracking
        startTelemetryTimeout();
    });

    // Disconnect button action
    document.getElementById('btnDisconnect').addEventListener('click', handleDisconnect);

    // Modal dialogue flows
    const newGameModal = document.getElementById('newGameModal');
    document.getElementById('btnNewGame').addEventListener('click', () => {
        newGameModal.classList.remove('hidden');
    });
    document.getElementById('btnConfirmCancel').addEventListener('click', () => {
        newGameModal.classList.add('hidden');
    });
    document.getElementById('btnConfirmStart').addEventListener('click', () => {
        newGameModal.classList.add('hidden');
        triggerStartCommand();
    });

    // Pause / Resume toggle button
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

    // Init UI Elements
    setupInputClamping();
    loadStoredCredentials();
    setupAccordions();
    validateConnectionInputs();

    // Connect Node-RED UI Builder WebSocket interface events [5.2]
    window.uibuilder.onChange('msg', (msg) => {
        if (!msg || !msg.payload) return;

        // Skip non-telemetry logs and configuration echo parameters
        if (msg.topic && msg.topic.indexOf('telemetry') === -1) return;

        const telemetry = TelemetryParser.parse(msg.payload);
        if (!telemetry) return;

        // Valid telemetry package received -> update state
        if (state.connectionState !== 'Connected') {
            state.connectionState = "Connected";
            updateScreenVisibility();
            triggerStartCommand(); // Auto-start play matching MainActivity JVM initialization
        }

        state.messageCount++;
        state.telemetry = telemetry;
        startTelemetryTimeout();

        // Reveal telemetry-bound layouts
        document.getElementById('placeholderOven').classList.add('hidden');
        document.getElementById('activeTelemetryGrid').classList.remove('hidden');

        // Render parameters and stats
        const statusText = telemetry.state.runningStatus.charAt(0).toUpperCase() + telemetry.state.runningStatus.slice(1);
        document.getElementById('connectionBannerText').textContent = `${statusText} · ${state.messageCount} updates`;

        // Configure connection indicator coloring matching ConnectionBanner
        const bannerDot = document.querySelector('.status-dot');
        if (telemetry.state.isRunning) {
            bannerDot.style.backgroundColor = 'var(--mint-green)';
        } else if (telemetry.state.runningStatus === 'completed') {
            bannerDot.style.backgroundColor = 'var(--honey)';
        } else {
            bannerDot.style.backgroundColor = 'var(--text-muted)';
        }

        // Handle game-over/next round notifications
        const completeBanner = document.getElementById('roundCompleteAnnouncement');
        if (telemetry.state.isFinished) {
            completeBanner.classList.remove('hidden');
        } else {
            completeBanner.classList.add('hidden');
        }

        // Render progress ring
        updateCookieProgressRing(telemetry.state.cookiesCollected, telemetry.config.targetCookies);
        document.getElementById('gameStateSubline').textContent = 
            `Player: ${telemetry.config.playerName}  ·  Remaining: ${telemetry.state.cookiesRemaining}`;

        // Configure interactive pause toggle styling
        const playBtn = document.getElementById('btnPauseResume');
        playBtn.disabled = false;
        if (telemetry.state.isRunning) {
            playBtn.textContent = "Pause";
            playBtn.className = "btn btn-danger flex-btn";
        } else {
            playBtn.textContent = "Resume";
            playBtn.className = "btn btn-warning flex-btn";
        }

        // Draw and update metrics
        renderer.update(telemetry, state.connectionState);
    });
});