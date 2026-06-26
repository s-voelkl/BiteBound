#ifndef MAZE_COOKIE_SPAWNER_H
#define MAZE_COOKIE_SPAWNER_H

#include <Arduino.h> // for random()
#include <vector>
#include "CookieField.h"            // ICookieSpawner, Cookie, PhysicsBody
#include "../maze/MazeManager.h"    // MazeManager::FreeCell

/**
 * @brief Game 1 spawner: places cookies only on carved maze corridors.
 *
 * The maze counterpart to RectCookieSpawner. Instead of sampling the whole
 * rectangle (which would drop cookies inside walls), it draws exclusively from
 * the corridor-cell centers exposed by MazeManager::getFreeCells(), so a cookie
 * can never land inside a wall.
 *
 * The cell list (owned by the MazeManager) MUST outlive this spawner.
 */
class MazeCookieSpawner : public ICookieSpawner
{
public:
    MazeCookieSpawner(const std::vector<MazeManager::FreeCell> *cells = nullptr,
                      float cookieRadius = 3.0f)
        : _cells(cells), _r(cookieRadius) {}

    Cookie spawn(const PhysicsBody &avoid) const override
    {
        if (_cells == nullptr || _cells->empty())
        {
            return Cookie(0.0f, 0.0f, _r, false); // no maze -> inactive cookie
        }

        const std::vector<MazeManager::FreeCell> &cells = *_cells;
        const float minD = avoid.radius + _r + 6.0f;

        Cookie c(0.0f, 0.0f, _r, true);
        for (int t = 0; t < 20; ++t) // retry until far enough from the ball
        {
            const MazeManager::FreeCell &fc = cells[random((long)cells.size())];
            c.x = fc.x;
            c.y = fc.y;
            const float dx = c.x - avoid.x;
            const float dy = c.y - avoid.y;
            if (dx * dx + dy * dy >= minD * minD)
            {
                break;
            }
        }
        return c;
    }

private:
    const std::vector<MazeManager::FreeCell> *_cells; // owned by MazeManager
    float _r;
};

#endif // MAZE_COOKIE_SPAWNER_H
