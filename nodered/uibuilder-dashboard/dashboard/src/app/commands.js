import {
    COMMAND_PHYSICS_DEFAULTS,
    DEFAULT_COOKIES,
    DEFAULT_GAME_ID,
    DEFAULT_PLAYER_NAME,
    DEFAULT_WALL_THICKNESS_PX,
} from './constants.js';

/**
 * @param {'start'|'stop'} command
 * @param {string} playerName
 * @param {number} gameId
 * @param {number} cookiesCount
 * @param {number} wallThicknessPx
 */
export function buildGameCommand(command, playerName, gameId, cookiesCount, wallThicknessPx) {
    return {
        command,
        meta: {
            source_ui: 'UIBUILDER',
            request_id: crypto.randomUUID(),
            timestamp: new Date().toISOString().replace(/\.\d{3}Z$/, 'Z'),
        },
        player: {
            name: (playerName || DEFAULT_PLAYER_NAME).trim() || DEFAULT_PLAYER_NAME,
        },
        game: {
            game_id: Number.isFinite(gameId) ? gameId : DEFAULT_GAME_ID,
        },
        parameters: {
            cookies_count: Number.isFinite(cookiesCount) ? cookiesCount : DEFAULT_COOKIES,
            wall_thickness_px: Number.isFinite(wallThicknessPx)
                ? wallThicknessPx
                : DEFAULT_WALL_THICKNESS_PX,
        },
        physics: { ...COMMAND_PHYSICS_DEFAULTS },
    };
}
