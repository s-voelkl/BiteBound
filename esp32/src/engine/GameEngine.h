#ifndef ENGINE_GAME_ENGINE_H
#define ENGINE_GAME_ENGINE_H

#include <stdint.h>
#include "GameConfig.h"
#include "../physics/PhysicsEngine.h"
#include "../game/GameState.h"
#include "../game/IGame.h"
#include "../game/Game1Labyrinth.h"
#include "../game/Game2Flatland.h"

/**
 * @brief Orchestrates both games behind one shared engine
 *
 * The active game is selected via
 * chooseGameMode(); the per-frame tick (input -> physics -> pickup -> telemetry
 * meta) runs here so the games stay thin.
 *
 * Each game owns its OWN round counter; the engine only decides when to start
 * vs. advance:
 *  - chooseGameMode(id): selects the game and start()s it -> the game's round
 *    resets to 1 (so changing the mode selection always resets the count),
 *    rebuilds the level (Game 1 -> new maze) and requests a full redraw.
 *  - nextRound(): advances the active game, which bumps its own counter (Game 2
 *    counts rounds too) and rebuilds the level (Game 1 -> new maze).
 */
class GameEngine
{
public:
    /** @brief Allocates the shared play board and initializes both games. */
    void begin(int playWidth, int playHeight);

    /** @brief Applies configuration (physics params, cookie counts, etc.). */
    void applyConfig(const GameConfig &cfg);

    /** @brief Selects a game (1/2), resets the round to 1 and starts it. */
    void chooseGameMode(uint8_t gameId);

    /** @brief Advances to the next round of the active game (regenerates Game 1s maze). */
    void nextRound();

    /**
     * @brief Advances the active game by one frame.
     * @param tiltX In-plane tilt input (e.g. accelerometer X).
     * @param tiltY In-plane tilt input (e.g. accelerometer Y).
     * @param dt    Time step [s].
     */
    void update(float tiltX, float tiltY, float dt);

    /** @brief Current scoreboard meta (status, round, score, time). */
    const GameState &state() const { return _state; }

    /** @brief Returns and clears the "needs full redraw" flag. */
    bool consumeRedraw();

    // Telemetry/ Status getter
    const uint8_t *board() const { return _board; }
    int playWidth() const { return _playW; }
    int playHeight() const { return _playH; }
    const PhysicsBody &ball() const { return _active->body(); }
    const CookieField &cookies() const { return _active->cookies(); }
    uint8_t activeGameId() const { return _active ? _active->id() : 0; }
    uint16_t round() const { return _active ? _active->round() : 0; }
    bool isRunning() const { return _status == Status::Running; }

private:
    enum class Status : uint8_t { Idle, Running, Completed };

    /** @brief Post-build step for chooseGameMode/nextRound: status + meta. */
    void afterBuild();

    PhysicsEngine _physics;
    GameConfig _config;
    GameState _state;

    Game1Labyrinth _game1;
    Game2Flatland _game2;
    IGame *_active = nullptr;

    uint8_t *_board = nullptr;
    int _playW = 0;
    int _playH = 0;

    float _elapsedSec = 0.0f;
    Status _status = Status::Idle;
    bool _needsFullRedraw = false;
};

#endif // ENGINE_GAME_ENGINE_H
