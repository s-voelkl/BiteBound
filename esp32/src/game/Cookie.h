#ifndef COOKIE_H
#define COOKIE_H

/**
 * @brief A single collectible cookie.
 *
 * @param x      Center X [px].
 * @param y      Center Y [px].
 * @param radius Collision radius [px].
 * @param active True when the cookie is visible and can be collected.
 */
struct Cookie
{
    float x;
    float y;
    float radius;
    bool active; // false once collected (until it respawns)

    Cookie(float x = 0.0f, float y = 0.0f, float radius = 3.0f, bool active = false)
        : x(x), y(y), radius(radius), active(active) {}
};

#endif // COOKIE_H
