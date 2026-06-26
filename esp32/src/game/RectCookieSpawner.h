#ifndef RECT_COOKIE_SPAWNER_H
#define RECT_COOKIE_SPAWNER_H

#include <Arduino.h> // for random()
#include "CookieField.h"

/**
 * @brief Game 2 spawner: a random position inside [0, width] x [0, height],
 * kept a margin away from the edges and away from the ball.
 * TODO: Currently, spawning in a wall is possible!
 */
class RectCookieSpawner : public ICookieSpawner
{
public:
    RectCookieSpawner(float width = 0.0f, float height = 0.0f, float cookieRadius = 3.0f)
        : _w(width), _h(height), _r(cookieRadius) {}

    Cookie spawn(const PhysicsBody &avoid) const override
    {
        const float m = _r + 2.0f; // margin from the edges
        Cookie c(0.0f, 0.0f, _r, true);
        for (int t = 0; t < 20; ++t) // retry until far enough from the ball
        {
            c.x = m + (random(1001) / 1000.0f) * (_w - 2.0f * m);
            c.y = m + (random(1001) / 1000.0f) * (_h - 2.0f * m);
            const float dx = c.x - avoid.x;
            const float dy = c.y - avoid.y;
            const float minD = avoid.radius + _r + 6.0f;
            if (dx * dx + dy * dy >= minD * minD)
            {
                break;
            }
        }
        return c;
    }

private:
    float _w;
    float _h;
    float _r;
};

#endif // RECT_COOKIE_SPAWNER_H
