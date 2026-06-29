#ifndef GAME_GAME1_LABYRINTH_H
#define GAME_GAME1_LABYRINTH_H

#include <vector>
#include "BaseGame.h"
#include "../cookies/MazeCookieSpawner.h"
#include "../../maze/MazeManager.h"
#include "../../physics/colliders/MazeCollider.h"
#include "../../../config.h"

/**
 * @brief Game 1: randomly generated maze.
 *
 * Owns a MazeManager that regenerates a fresh maze into the shared board on
 * every startRound(), a MazeCollider
 * over that board, and the maze cookie spawner (k cookies respawn on corridor).
 */
class Game1Labyrinth : public BaseGame
{
public:
    Game1Labyrinth() : _maze(1, 1, 1) {}

    uint8_t id() const override { return 1; }

    void init(uint8_t *board, int playW, int playH, int wallThickness) override
    {
        _board = board;
        _w = playW;
        _h = playH;
        _t = wallThickness;
        _maze = MazeManager(playW, playH, wallThickness);
        _world = MazeCollider(board, playW, playH);
        // Spawner reads the maze's free-cell list, which is re-filled in place
        // on each generate(); the pointer stays valid across regenerations.
        _spawner = MazeCookieSpawner(&_maze.getFreeCells(), default_cookie_radius);
    }

    const ICollider &collider() const override { return _world; }

protected:
    void buildLevel(const GameConfig &cfg) override
    {
        _maze.generate(_board); // fresh maze for this round

        const std::vector<MazeManager::FreeCell> &cells = _maze.getFreeCells();
        if (!cells.empty())
        {
            _ball.x = cells[0].x; // start on a guaranteed corridor cell
            _ball.y = cells[0].y;
        }
        else
        {
            _ball.x = _w * 0.5f;
            _ball.y = _h * 0.5f;
        }
        _ball.vx = 0.0f;
        _ball.vy = 0.0f;
        // Derive the ball size from the wall thickness so it always fits the
        // corridor: diameter = 70% of a corridor's width.
        _ball.radius = cfg.wallThicknessPx * ball_diameter_wall_ratio / 2.0f;

        _cookies.start(cfg.visibleCookies, cfg.targetCookies, _spawner, _ball);
    }

private:
    MazeManager _maze;
    MazeCollider _world;
    MazeCookieSpawner _spawner;
};

#endif // GAME_GAME1_LABYRINTH_H
