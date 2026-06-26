#include "PhysicsEngine.h"
#include "../colliders/BorderCollider.h"
#include "../colliders/ICollider.h"
#include "PhysicsBody.h"

#include <math.h>
#include <AUnit.h>

/**
 * @brief Free space: never reports a collision.
 *
 * Used by the pure-integration tests so the engine's motion can be checked in
 * isolation from any environment.
 */
class NullCollider : public ICollider
{
public:
    Contact query(const PhysicsBody &) const override { return Contact(); }
};

/**
 * @brief A single thin, full-height vertical wall slab on x in [x0, x0 + thickness].
 *
 * Lets the tests exercise tunnelling against a wall thinner than the ball could
 * cross in one un-sub-stepped frame. The normal points back towards the side the
 * body approached from.
 */
class VerticalWallCollider : public ICollider
{
public:
    VerticalWallCollider(float x0, float thickness)
        : _x0(x0), _x1(x0 + thickness) {}

    Contact query(const PhysicsBody &b) const override
    {
        Contact c;
        const float left = b.x - b.radius;
        const float right = b.x + b.radius;
        if (right <= _x0 || left >= _x1)
        {
            return c; // no overlap
        }

        const float mid = 0.5f * (_x0 + _x1);
        if (b.x < mid)
        {
            c.normal = Vec2(-1.0f, 0.0f);
            c.penetration = right - _x0;
        }
        else
        {
            c.normal = Vec2(1.0f, 0.0f);
            c.penetration = _x1 - left;
        }
        c.hit = true;
        return c;
    }

private:
    float _x0;
    float _x1;
};

// ---------------------------------------------------------------------------
// Input conditioning: EMA + deadzone + sensitivity
// ---------------------------------------------------------------------------

/**
 * @test The first tilt sample seeds the EMA (pass-through) and is then scaled by
 * the sensitivity.
 */
test(physics_inputAccel_firstSampleScalesBySensitivity)
{
    PhysicsEngine engine;
    PhysicsParams p; // sensitivity 1.25, deadzone 0.05
    engine.setParams(p);
    engine.reset();

    Vec2 a = engine.inputAccel(1.0f, -2.0f);

    assertNear(a.x, 1.25f, 1e-5f);
    assertNear(a.y, -2.5f, 1e-5f);
}

/**
 * @test A tilt below the deadzone is treated as zero acceleration.
 */
test(physics_inputAccel_deadzoneZeroesSmallInput)
{
    PhysicsEngine engine;
    PhysicsParams p; // deadzone 0.05
    engine.setParams(p);
    engine.reset();

    Vec2 a = engine.inputAccel(0.03f, -0.01f);

    assertNear(a.x, 0.0f, 1e-6f);
    assertNear(a.y, 0.0f, 1e-6f);
}

/**
 * @test The EMA blends a new sample with the previous filter state.
 */
test(physics_inputAccel_emaBlendsSecondSample)
{
    PhysicsEngine engine;
    PhysicsParams p;
    p.sensitivity = 1.0f; // isolate the EMA from the scaling
    p.deadzone = 0.0f;
    engine.setParams(p);
    engine.reset();

    engine.inputAccel(0.0f, 0.0f);          // seed the filter at 0
    Vec2 a = engine.inputAccel(1.0f, 0.0f); // 0.25 * 1 + 0.75 * 0 = 0.25

    assertNear(a.x, 0.25f, 1e-5f);
    assertNear(a.y, 0.0f, 1e-6f);
}

/**
 * @test reset() clears the filter so the next sample is again a pass-through.
 */
test(physics_reset_clearsFilterState)
{
    PhysicsEngine engine;
    PhysicsParams p;
    p.sensitivity = 1.0f;
    p.deadzone = 0.0f;
    engine.setParams(p);

    engine.reset();
    engine.inputAccel(10.0f, 0.0f); // seed the filter at 10
    engine.reset();                 // forget it

    Vec2 a = engine.inputAccel(2.0f, 0.0f);

    assertNear(a.x, 2.0f, 1e-5f); // pass-through, not blended with 10
}

// ---------------------------------------------------------------------------
// Integration: semi-implicit Euler, damping, clamp
// ---------------------------------------------------------------------------

/**
 * @test Semi-implicit Euler advances velocity then position with exact values.
 */
test(physics_step_semiImplicitEulerAdvancesPositionAndVelocity)
{
    PhysicsEngine engine;
    PhysicsParams p;
    p.maxSpeed = 1e9f; // disable the clamp for exact arithmetic
    engine.setParams(p);

    NullCollider freeSpace;
    PhysicsBody b; // at origin, at rest
    Vec2 accel(10.0f, 0.0f);
    const float dt = 0.1f;

    // Step 1: v = a*dt = 1.0 ; x = v*dt = 0.1
    engine.step(b, accel, dt, freeSpace);
    assertNear(b.vx, 1.0f, 1e-4f);
    assertNear(b.x, 0.1f, 1e-4f);

    // Step 2: v = 2.0 ; x = 0.1 + 2.0*0.1 = 0.3
    engine.step(b, accel, dt, freeSpace);
    assertNear(b.vx, 2.0f, 1e-4f);
    assertNear(b.x, 0.3f, 1e-4f);
}

/**
 * @test With the default (zero) damping, a coasting ball never loses speed.
 *
 * This is the "must not be permanently damped, otherwise the ball stops" rule.
 */
test(physics_step_noPermanentDampingInFreeSpace)
{
    PhysicsEngine engine;
    PhysicsParams p;
    p.maxSpeed = 1e9f;
    p.linearDamping = 0.0f;
    engine.setParams(p);

    NullCollider freeSpace;
    PhysicsBody b;
    b.vx = 100.0f; // coasting, no input
    Vec2 noAccel(0.0f, 0.0f);

    for (int i = 0; i < 500; ++i)
    {
        engine.step(b, noAccel, 0.02f, freeSpace);
    }

    assertNear(b.vx, 100.0f, 1e-2f); // unchanged: the ball never stalls
    assertMore(b.x, 0.0f);
}

/**
 * @test The optional linear damping does reduce speed when enabled.
 */
test(physics_step_linearDampingReducesSpeedWhenEnabled)
{
    PhysicsEngine engine;
    PhysicsParams p;
    p.maxSpeed = 1e9f;
    p.linearDamping = 2.0f; // opt-in drag
    engine.setParams(p);

    NullCollider freeSpace;
    PhysicsBody b;
    b.vx = 100.0f;
    Vec2 noAccel(0.0f, 0.0f);

    for (int i = 0; i < 50; ++i)
    {
        engine.step(b, noAccel, 0.02f, freeSpace);
    }

    assertLess(b.vx, 100.0f);
    assertMore(b.vx, 0.0f);
}

/**
 * @test A very large acceleration is clamped to maxSpeed.
 */
test(physics_step_clampsToMaxSpeed)
{
    PhysicsEngine engine;
    PhysicsParams p;
    p.maxSpeed = 50.0f;
    engine.setParams(p);

    NullCollider freeSpace;
    PhysicsBody b;
    Vec2 hugeAccel(100000.0f, 0.0f);

    engine.step(b, hugeAccel, 0.02f, freeSpace);

    const float speed = sqrtf(b.vx * b.vx + b.vy * b.vy);
    assertNear(speed, 50.0f, 1e-3f);
}

// ---------------------------------------------------------------------------
// Collision: restitution, tangential preservation, no tunnelling
// ---------------------------------------------------------------------------

/**
 * @test On a wall bounce the normal velocity is reversed and scaled by the
 * restitution, and the ball is pushed back inside the play area.
 */
test(physics_step_bounceReflectsNormalScaledByRestitution)
{
    PhysicsEngine engine;
    PhysicsParams p;
    p.restitution = 0.75f;
    p.maxSpeed = 1e9f;
    engine.setParams(p);

    BorderCollider borders(240.0f, 280.0f);
    PhysicsBody b;
    b.radius = 4.0f;
    b.x = 234.0f; // x + r = 238, just inside the right edge at 240
    b.y = 140.0f;
    b.vx = 60.0f; // moving into the right wall

    engine.step(b, Vec2(0.0f, 0.0f), 0.1f, borders);

    assertNear(b.vx, -45.0f, 1e-3f);                  // -e * 60 = -45
    assertLessOrEqual(b.x + b.radius, 240.0f + 1e-3f); // back inside bounds
}

/**
 * @test A diagonal bounce damps only the normal component; the tangential
 * (along-the-wall) speed is preserved, so the ball keeps moving along the path.
 */
test(physics_step_tangentialVelocityIsPreservedOnBounce)
{
    PhysicsEngine engine;
    PhysicsParams p;
    p.restitution = 0.75f;
    p.maxSpeed = 1e9f;
    engine.setParams(p);

    BorderCollider borders(240.0f, 280.0f);
    PhysicsBody b;
    b.radius = 4.0f;
    b.x = 234.0f;
    b.y = 140.0f;
    b.vx = 60.0f; // normal component (into the wall)
    b.vy = 30.0f; // tangential component (along the wall)

    engine.step(b, Vec2(0.0f, 0.0f), 0.1f, borders);

    assertNear(b.vx, -45.0f, 1e-3f); // normal damped to 75 %
    assertNear(b.vy, 30.0f, 1e-3f);  // tangential untouched
}

/**
 * @test A ball smaller than a thin wall does not tunnel through it, even at a
 * speed that would cross the wall in a single un-sub-stepped frame.
 */
test(physics_step_smallBallDoesNotTunnelThroughThinWall)
{
    PhysicsEngine engine;
    PhysicsParams p;
    p.restitution = 0.75f;
    p.maxSpeed = 1e9f; // do NOT rely on the clamp; prove sub-stepping works
    engine.setParams(p);

    VerticalWallCollider wall(100.0f, 6.0f); // thin 6 px wall (wall_thickness_px)
    PhysicsBody b;
    b.radius = 4.0f; // smaller than the wall thickness
    b.x = 90.0f;
    b.y = 140.0f;
    b.vx = 5000.0f; // would jump 100 px in one frame without sub-stepping

    engine.step(b, Vec2(0.0f, 0.0f), 0.02f, wall);

    assertLessOrEqual(b.x + b.radius, 100.0f + 1e-2f); // never crossed the wall
    assertLess(b.vx, 0.0f);                            // it bounced back
}
