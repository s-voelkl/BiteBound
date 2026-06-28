#include "PhysicsEngine.h"
#include "PhysicsParams.h"

#include <math.h>

/** @brief Maximum contact-resolution iterations per sub-step (corner safety). */
static const int kMaxContactIterations = default_max_contact_iterations;

void PhysicsEngine::setParams(const PhysicsParams &params)
{
    _params = params;
}

const PhysicsParams &PhysicsEngine::params() const
{
    return _params;
}

void PhysicsEngine::reset()
{
    _emaInitialized = false;
    _emaX = 0.0f;
    _emaY = 0.0f;
}

Vec2 PhysicsEngine::inputAccel(float tiltX, float tiltY)
{
    //  EMA low-pass. The first sample seeds the filter (pass-through) so the
    //  ball does not lurch from an initial zero state.
    if (!_emaInitialized)
    {
        _emaX = tiltX;
        _emaY = tiltY;
        _emaInitialized = true;
    }
    else
    {
        const float a = _params.emaAlpha;
        _emaX = a * tiltX + (1.0f - a) * _emaX;
        _emaY = a * tiltY + (1.0f - a) * _emaY;
    }

    //  Deadzone: ignore tiny tilts to stop noise-driven drift when the board
    //  is held flat. This filters the input only; it removes no energy from an
    //  already moving ball.
    const float fx = (fabsf(_emaX) < _params.deadzone) ? 0.0f : _emaX;
    const float fy = (fabsf(_emaY) < _params.deadzone) ? 0.0f : _emaY;

    //  Scale the conditioned tilt to an acceleration.
    return Vec2(fx * _params.sensitivity, fy * _params.sensitivity);
}

bool PhysicsEngine::step(PhysicsBody &body, Vec2 accel, float dt, const ICollider &world)
{
    if (dt <= 0.0f)
    {
        return false;
    }

    // Semi-implicit Euler: integrate the velocity first, then the position.
    body.vx += accel.x * dt;
    body.vy += accel.y * dt;

    // Optional continuous damping (e.g. air resistance). Default 0 -> no permanent damping, so the
    // ball keeps coasting until the player tilts back or it hits a wall.
    if (_params.linearDamping > 0.0f)
    {
        float k = 1.0f - _params.linearDamping * dt;
        if (k < 0.0f)
        {
            k = 0.0f;
        }
        body.vx *= k;
        body.vy *= k;
    }

    // Clamp the speed: keeps the ball controllable
    float speed = sqrtf(body.vx * body.vx + body.vy * body.vy);
    if (speed > _params.maxSpeed && speed > 0.0f)
    {
        const float f = _params.maxSpeed / speed;
        body.vx *= f;
        body.vy *= f;
        speed = _params.maxSpeed;
    }

    // Sub-step so that each increment moves the body by at most half its radius.
    // -> small, fast ball cant pass through walls now!
    float maxStep = body.radius * 0.5f;
    if (maxStep <= 0.0f)
    {
        maxStep = 1.0f;
    }
    int subSteps = (int)ceilf((speed * dt) / maxStep);
    if (subSteps < 1)
    {
        subSteps = 1;
    }
    const float h = dt / (float)subSteps;

    bool collided = false;
    for (int s = 0; s < subSteps; ++s)
    {
        body.x += body.vx * h;
        body.y += body.vy * h;

        // Resolve every obstacle overlapping this position. Iterating handles
        // corners where the body touches two walls at once.
        for (int it = 0; it < kMaxContactIterations; ++it)
        {
            Contact c = world.query(body);
            if (!c.hit)
            {
                break;
            }
            collided = true;

            // Push the body out of the obstacle along the contact normal.
            body.x += c.normal.x * c.penetration;
            body.y += c.normal.y * c.penetration;

            // Reflect only the velocity component moving INTO the wall and scale
            // it by the restitution. The tangential component is left untouched,
            // so the ball keeps sliding along the corridor without losing the
            // speed it carries along the path (no permanent damping).
            const float vn = body.vx * c.normal.x + body.vy * c.normal.y;
            if (vn < 0.0f)
            {
                const float j = (1.0f + _params.restitution) * vn;
                body.vx -= j * c.normal.x;
                body.vy -= j * c.normal.y;
            }
        }
    }

    return collided;
}
