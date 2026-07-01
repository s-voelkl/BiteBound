#ifndef GAME_GAME2_FLATLAND_H
#define GAME_GAME2_FLATLAND_H

#include <string.h>
#include "BaseGame.h"
#include "../cookies/RectCookieSpawner.h"
#include "../../physics/colliders/BorderCollider.h"
#include "../../../config.h"

/**
 * @brief Game 2: an open play-field limited only by the screen edges.
 *
 * Uses a BorderCollider and the rectangle cookie spawner (k cookies respawn on
 * pickup). No maze; the shared board is cleared to free space for rendering.
 */
class Game2Flatland : public BaseGame
{
public:
    Game2Flatland() : _world(1.0f, 1.0f) {}

    uint8_t id() const override { return 2; }

    void init(uint8_t *board, int playW, int playH, int wallThickness) override
    {
        _board = board;
        _w = playW;
        _h = playH;
        _t = wallThickness;
        _world = BorderCollider((float)playW, (float)playH);
    }

    const ICollider &collider() const override { return _world; }

protected:
    void buildLevel(const GameConfig &cfg) override
    {
        if (_board != nullptr)
        {
            memset(_board, 0, (size_t)_w * _h); // open field: all free
        }
        _ball.x = _w * 0.5f;
        _ball.y = _h * 0.5f;
        _ball.vx = 0.0f;
        _ball.vy = 0.0f;
        // Open field has no corridor to scale to - use fixed, clearly visible sizes.
        _ball.radius = default_flatland_ball_radius;
        _spawner = RectCookieSpawner((float)_w, (float)_h, default_flatland_cookie_radius);
        _cookies.start(cfg.visibleCookies, cfg.targetCookies, _spawner, _ball);
    }

private:
    BorderCollider _world;
    RectCookieSpawner _spawner;
};

#endif // GAME_GAME2_FLATLAND_H
