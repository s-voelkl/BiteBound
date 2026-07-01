#ifndef GAME_IGAME_H
#define GAME_IGAME_H

#include <stdint.h>
#include "../../physics/core/PhysicsBody.h"
#include "../../physics/colliders/ICollider.h"
#include "../cookies/CookieField.h"
#include "../../engine/GameConfig.h"

/**
 * @brief A single game mode.
 *
 * Games carry ONLY what differs between modes: the collision world, where the
 * ball/cookies live, and how a round is (re)built. The shared per-frame tick
 * (input -> physics -> pickup -> telemetry) lives in GameEngine, so there is no
 * duplicated game loop here.
 */
class IGame
{
public:
    virtual ~IGame() = default;

    /** @brief Stable id (1 = Labyrinth, 2 = Flatland). */
    virtual uint8_t id() const = 0;

    /**
     * @brief One-time setup: bind the shared play-field board and dimensions.
     * @param board         Engine-owned flat play board (width*height).
     * @param playW         Play-field width in pixels.
     * @param playH         Play-field height in pixels.
     * @param wallThickness Maze block size in pixels.
     */
    virtual void init(uint8_t *board, int playW, int playH, int wallThickness) = 0;

    /**
     * @brief Starts the game at round 1 and builds the level.
     *
     * Called when this mode is selected, so the round counter resets to 1 every
     * time the game-mode selection changes. Game 1 regenerates the maze here.
     */
    virtual void start(const GameConfig &cfg) = 0;

    /**
     * @brief Advances this game to its next round and rebuilds the level.
     *
     * Increments the game's OWN round counter (each mode counts independently)
     * and regenerates the level (Game 1 -> a fresh maze).
     */
    virtual void nextRound(const GameConfig &cfg) = 0;

    /** @brief This game's current round number (1-based; 0 before start()). */
    virtual uint16_t round() const = 0;

    /** @brief The ball; engine integrates it against collider() each frame. */
    virtual PhysicsBody &body() = 0;
    virtual const PhysicsBody &body() const = 0;

    /** @brief Collision environment for this game's world. */
    virtual const ICollider &collider() const = 0;

    /** @brief Post-physics hook: collect cookies the ball now overlaps. */
    virtual void onResolved() = 0;

    /** @brief True once the win condition (all target cookies collected) is met. */
    virtual bool finished() const = 0;

    /** @brief Cookie field, for rendering and telemetry. */
    virtual const CookieField &cookies() const = 0;
};

#endif // GAME_IGAME_H
