#ifndef PHYSICS_ICOLLIDER_H
#define PHYSICS_ICOLLIDER_H

#include "Vec2.h"
#include "PhysicsBody.h"

/**
 * @brief Result of a collision query for a body against an environment.
 */
struct Contact
{
    Vec2 normal;            ///< Normal vector pointing OUT of the obstacle (into free space).
    float penetration = 0;  ///< Overlap depth [px]; >= 0 when hit.
    bool hit = false;       ///< True when the body overlaps an obstacle.
};

/**
 * @brief Strategy interface for the collision environment of a game.
 *
 */
class ICollider
{
public:
    virtual ~ICollider() = default;

    /**
     * @brief Returns the deepest current contact for the given body.
     *
     * @param body Body to test against the environment.
     * @return Contact with hit == false when the body is in free space.
     */
    virtual Contact query(const PhysicsBody &body) const = 0;
};

#endif // PHYSICS_ICOLLIDER_H
