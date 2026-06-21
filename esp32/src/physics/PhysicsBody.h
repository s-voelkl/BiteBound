#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H

/**
 * @brief Dynamic state of a single circular body (the ball).
 *
 * Every game has its own PhysicsBody, while the PhysicsEngine is shared!
 *
 * The radius of the Body is kept smaller than the corridor it passes through
 * for more precise movement
 */
struct PhysicsBody
{
    float x = 0.0f;      ///< Center position X [px], origin top-left.
    float y = 0.0f;      ///< Center position Y [px].
    float vx = 0.0f;     ///< Velocity X [px/s].
    float vy = 0.0f;     ///< Velocity Y [px/s].
    float radius = 4.0f; ///< Collision radius [px]; smaller than the path width.
};

#endif // PHYSICS_BODY_H
