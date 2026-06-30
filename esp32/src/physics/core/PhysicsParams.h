#ifndef PHYSICS_PARAMS_H
#define PHYSICS_PARAMS_H

#include "../../../config.h"

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

#endif // PHYSICS_PARAMS_H