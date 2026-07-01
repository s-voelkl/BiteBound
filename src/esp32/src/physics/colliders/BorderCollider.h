#ifndef PHYSICS_BORDER_COLLIDER_H
#define PHYSICS_BORDER_COLLIDER_H

#include "ICollider.h"

/**
 * @brief Keeps a body inside an axis-aligned rectangle [0, width] x [0, height].
 *
 * Used by Game 2.
 */
class BorderCollider : public ICollider
{
public:
    /**
     * @brief Constructs the border for a screen of the given size.
     * @param width  Play-field width [px]  (e.g. display_width).
     * @param height Play-field height [px] (e.g. display_height).
     */
    BorderCollider(float width, float height);

    Contact query(const PhysicsBody &body) const override;

private:
    float _width;
    float _height;
};

#endif // PHYSICS_BORDER_COLLIDER_H
