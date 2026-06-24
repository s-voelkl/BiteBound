#ifndef PHYSICS_ENGINE_H
#define PHYSICS_ENGINE_H

#include "Vec2.h"
#include "PhysicsBody.h"
#include "ICollider.h"
#include "../../config.h"

/**
 * @brief Tunable physics parameters.
 *
 * These parameters can be adjusted via MQTT on runtime
 */
struct PhysicsParams
{
    // The "imu_sensitivity_multiplier" config setting; more granular adjustment to movement sensitivity
    float sensitivity = default_imu_sensitivity_multiplier;

    // Bounce Factor applied to the object; 0 means no bounce; 1 means perfect bounce
    float restitution = default_bounce_restitution;

    // EMA smoothing alpha; Higher means more responsive
    float emaAlpha = default_ema_alpha;

    // Input deadzone: tilt below this value will not be registered
    float deadzone = default_deadzone_threshold;

    // The maximum value the ball can travel per second [px/s]
    float maxSpeed = default_max_speed;

    /** Continuous drag [1/s]. Default 0 means NO permanent damping, so the ball
     *  never stalls on its own; energy is only lost on collisions. */
    float linearDamping = default_linear_damping;
};

/**
 * @brief Shared, game-agnostic 2D physics engine.
 *
 * Holds the tuning parameters and the input filter state, and provides the
 * integration and collision-resolution algorithms.
 *
 * Energy is conserved except at collisions: there is no per-frame friction by
 * default, and a bounce only damps the velocity component normal to the wall,
 * so a ball scraping along a corridor keeps its speed along the path.
 */
class PhysicsEngine
{
public:
    /** @brief Replaces the active tuning parameters. */
    void setParams(const PhysicsParams &params);

    /** @brief Returns the active tuning parameters. */
    const PhysicsParams &params() const;

    /**
     * @brief Resets the input filter state.
     *
     * Should be called on restart or game switch to not carry over ema values
     */
    void reset();

    /**
     * @brief Conditions a raw tilt reading into an acceleration vector.
     *
     * Pipeline: EMA low-pass -> deadzone -> sensitivity scaling. The engine is
     * intentionally decoupled from SensorData; the game passes the in-plane
     * accelerometer components (e.g. accelerometerX / accelerometerY) directly.
     * This keeps the raw SensorData untouched for telemetry.
     *
     * @param tiltX Raw in-plane tilt input on X (e.g. accelerometer X).
     * @param tiltY Raw in-plane tilt input on Y.
     * @return Acceleration [px/s^2] to feed into step().
     */
    Vec2 inputAccel(float tiltX, float tiltY);

    /**
     * @brief Advances a body by one frame and resolves collisions.
     *
     * Uses semi-implicit Euler (velocity first, then position), clamps the speed
     * to maxSpeed, and sub-steps the motion so a small ball cannot tunnel
     * through a thin wall in a single frame. On contact only the normal velocity
     * component is reflected and scaled by the restitution; the tangential
     * component is preserved.
     *
     * @param body  Body to advance (modified in place).
     * @param accel Acceleration [px/s^2], typically from inputAccel().
     * @param dt    Time step [s].
     * @param world Collision environment.
     * @return true if at least one collision was resolved this frame (maps to
     *         the telemetry field physics.collision_detected).
     */
    bool step(PhysicsBody &body, Vec2 accel, float dt, const ICollider &world);

private:
    PhysicsParams _params;
    float _emaX = 0.0f;
    float _emaY = 0.0f;
    bool _emaInitialized = false;
};

#endif // PHYSICS_ENGINE_H
