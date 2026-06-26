#include "CookieField.h"

void CookieField::start(uint8_t visibleCount, uint16_t target,
                        const ICookieSpawner &spawner, const PhysicsBody &ball)
{
    _spawner = &spawner;
    _target = target;
    _collected = 0;

    if (visibleCount > kMaxVisibleCookies)
    {
        visibleCount = kMaxVisibleCookies;
    }
    _count = visibleCount;

    for (uint8_t i = 0; i < _count; ++i)
    {
        _cookies[i] = spawner.spawn(ball);
    }
}

uint8_t CookieField::checkPickup(const PhysicsBody &ball)
{
    if (_spawner == nullptr)
    {
        return 0;
    }

    uint8_t eaten = 0;
    for (uint8_t i = 0; i < _count; ++i)
    {
        Cookie &c = _cookies[i];
        if (!c.active)
        {
            continue;
        }

        // Circle-circle overlap, squared to avoid a sqrt.
        const float dx = ball.x - c.x;
        const float dy = ball.y - c.y;
        const float r = ball.radius + c.radius;
        if (dx * dx + dy * dy <= r * r)
        {
            ++_collected;
            ++eaten;

            if (_collected < _target)
            {
                c = _spawner->spawn(ball); // reappear elsewhere
            }
            else
            {
                c.active = false; // target reached -> last cookie vanishes
            }
        }
    }
    return eaten;
}
