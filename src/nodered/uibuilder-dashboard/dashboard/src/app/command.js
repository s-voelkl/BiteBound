import { GameConfigConstants } from './constants.js';
import { isoNow, generateUUID } from './format.js';

/**
 * Builds and dispatches commands via uibuilder over WebSockets.
 */
export class GameCommand {
    /**
     * Constructs a 'start' payload.
     */
    static start({
        playerName = GameConfigConstants.DEFAULT_PLAYER_NAME,
        gameId = GameConfigConstants.GAME_ID_LABYRINTH,
        cookiesCount = GameConfigConstants.DEFAULT_COOKIES_COUNT,
        wallThicknessPx = GameConfigConstants.DEFAULT_WALL_THICKNESS_PX,
        imuSensitivity = GameConfigConstants.DEFAULT_IMU_SENSITIVITY_MULTIPLIER,
        restitution = GameConfigConstants.DEFAULT_BOUNCE_RESTITUTION,
        emaAlpha = GameConfigConstants.DEFAULT_EMA_ALPHA,
        deadzone = GameConfigConstants.DEFAULT_DEADZONE_THRESHOLD
    } = {}) {
        return this.build("start", playerName, gameId, cookiesCount, wallThicknessPx, imuSensitivity, restitution, emaAlpha, deadzone);
    }

    /**
     * Constructs a 'stop' payload.
     */
    static stop({
        playerName = GameConfigConstants.DEFAULT_PLAYER_NAME,
        gameId = GameConfigConstants.GAME_ID_LABYRINTH,
        cookiesCount = GameConfigConstants.DEFAULT_COOKIES_COUNT,
        wallThicknessPx = GameConfigConstants.DEFAULT_WALL_THICKNESS_PX
    } = {}) {
        return this.build("stop", playerName, gameId, cookiesCount, wallThicknessPx);
    }

    /**
     * Constructs a 'resume' payload.
     */
    static resume({
        playerName = GameConfigConstants.DEFAULT_PLAYER_NAME,
        gameId = GameConfigConstants.GAME_ID_LABYRINTH,
        cookiesCount = GameConfigConstants.DEFAULT_COOKIES_COUNT,
        wallThicknessPx = 6
    } = {}) {
        return this.build("resume", playerName, gameId, cookiesCount, wallThicknessPx);
    }

    /**
     * Unified builder matching ESP32 firmware expectations.
     */
    static build(
        command,
        playerName,
        gameId,
        cookiesCount,
        wallThicknessPx,
        imuSensitivity = GameConfigConstants.DEFAULT_IMU_SENSITIVITY_MULTIPLIER,
        restitution = GameConfigConstants.DEFAULT_BOUNCE_RESTITUTION,
        emaAlpha = GameConfigConstants.DEFAULT_EMA_ALPHA,
        deadzone = GameConfigConstants.DEFAULT_DEADZONE_THRESHOLD
    ) {
        return {
            command: command,
            meta: {
                source_ui: "NODERED",
                request_id: generateUUID(),
                timestamp: isoNow()
            },
            player: {
                name: playerName
            },
            game: {
                game_id: gameId
            },
            parameters: {
                cookies_count: cookiesCount,
                wall_thickness_px: wallThicknessPx
            },
            physics: {
                imu_sensitivity_multiplier: imuSensitivity,
                bounce_restitution: restitution,
                ema_alpha: emaAlpha,
                deadzone_threshold: deadzone
            }
        };
    }

    /**
     * Publishes command directly into UI Builder topmost output port.
     * @param {object} uibuilder - Global uibuilder runtime instance.
     * @param {string} commandType - Command code ('start', 'stop', 'resume').
     * @param {object} params - UI-modified parameters.
     */
    static send(uibuilder, commandType, params) {
        let payload;
        switch (commandType) {
            case 'start':
                payload = this.start(params);
                break;
            case 'stop':
                payload = this.stop(params);
                break;
            case 'resume':
                payload = this.resume(params);
                break;
            default:
                console.error('Unsupported UI command:', commandType);
                return;
        }

        // Emit over UI Builder WebSocket layer
        uibuilder.send({
            payload: payload,
            topic: GameConfigConstants.TOPIC_COMMAND
        });
    }
}