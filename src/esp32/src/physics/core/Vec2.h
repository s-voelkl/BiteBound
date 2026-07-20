#ifndef PHYSICS_VEC2_H
#define PHYSICS_VEC2_H

/**
 * @brief Minimal 2D vector used throughout the physics engine.
 *
 * @param x X component.
 * @param y Y component.
 *
 */
struct Vec2
{
    float x;
    float y;

    Vec2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

#endif // PHYSICS_VEC2_H
