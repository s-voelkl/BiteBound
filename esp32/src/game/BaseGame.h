#ifndef GAME_BASE_GAME_H
#define GAME_BASE_GAME_H

#include "IGame.h"

/**
 * @brief Shared state and behavior for both game modes
 *
 * Holds the ball and the cookie field and implements the parts that are
 * identical between games (pickup, win check, accessors). Concrete games only
 * add their world/collider and spawner and implement id()/init()/startRound().
 */
class BaseGame : public IGame
{
public:
    // Round management is identical for every mode: selecting a mode restarts it
    // at round 1, advancing bumps this game's own counter. Only buildLevel()
    // (the level/maze/cookie setup) differs per game.
    void start(const GameConfig &cfg) override
    {
        _round = 1;
        buildLevel(cfg);
    }
    void nextRound(const GameConfig &cfg) override
    {
        _round++;
        buildLevel(cfg);
    }
    uint16_t round() const override { return _round; }

    PhysicsBody &body() override { return _ball; }
    const PhysicsBody &body() const override { return _ball; }
    void onResolved() override { _cookies.checkPickup(_ball); }
    bool finished() const override { return _cookies.finished(); }
    const CookieField &cookies() const override { return _cookies; }

protected:
    static constexpr float kCookieRadius = 3.0f;

    /** @brief (Re)builds this game's level for the current round. */
    virtual void buildLevel(const GameConfig &cfg) = 0;

    uint16_t _round = 0; ///< This game's own round counter (0 until start()).
    PhysicsBody _ball;
    CookieField _cookies;
    uint8_t *_board = nullptr; ///< Engine-owned; not freed here.
    int _w = 0;
    int _h = 0;
    int _t = 1;
};

#endif // GAME_BASE_GAME_H
