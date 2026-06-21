#ifndef COOKIE_H
#define COOKIE_H

/**
 * @brief A single collectible cookie.
 *
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
