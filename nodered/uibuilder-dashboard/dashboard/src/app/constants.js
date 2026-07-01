/**
 * BiteBound Game Configuration Constants.
 */
export const GameConfigConstants = {
    DEFAULT_IMU_SENSITIVITY_MULTIPLIER: 100.0,
    DEFAULT_BOUNCE_RESTITUTION: 0.3,
    DEFAULT_EMA_ALPHA: 0.5,
    DEFAULT_DEADZONE_THRESHOLD: 0.04,
    DEFAULT_MAX_SPEED: 300.0, // px/s

    GAME_ID_LABYRINTH: 1,
    GAME_ID_FLATLAND: 2,

    DEFAULT_PLAYER_NAME: "Cookie-Monster",
    DEFAULT_COOKIES_COUNT: 10,
    DEFAULT_WALL_THICKNESS_PX: 10,
    MAX_COOKIES: 20,
    MAX_WALL_THICKNESS: 20,
    TELEMETRY_TIMEOUT_MS: 60000,

    TOPIC_COMMAND: "mauc2026/group_03/game/command",
    TOPIC_TELEMETRY: "mauc2026/group_03/game/telemetry"
};

/**
 * Ball configurations mapping to specific movement feels.
 */
export const BallTypes = {
    CHONK: {
        label: "Chonk",
        restitution: 0.15,
        sensitivity: 55.0,
        emaAlpha: 0.3
    },
    CHOMPER: {
        label: "Chomper",
        restitution: 0.40,
        sensitivity: 80.0,
        emaAlpha: 0.5
    },
    SCRAPPY: {
        label: "Scrappy",
        restitution: 0.75,
        sensitivity: 100.0,
        emaAlpha: 0.6
    }
};

/**
 * Helper to match stored parameters back to closest pre-defined BallType
 */
export function getBallTypeByRestitution(value) {
    const list = Object.values(BallTypes);
    return list.reduce((prev, curr) => 
        Math.abs(curr.restitution - value) < Math.abs(prev.restitution - value) ? curr : prev, 
        BallTypes.CHOMPER
    );
}