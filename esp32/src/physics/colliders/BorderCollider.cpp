#include "BorderCollider.h"

BorderCollider::BorderCollider(float width, float height)
    : _width(width), _height(height)
{
}

Contact BorderCollider::query(const PhysicsBody &body) const
{
    Contact best;

    // Penetration depth against each edge (positive => the circle overlaps it).
    const float penLeft = body.radius - body.x;
    const float penRight = (body.x + body.radius) - _width;
    const float penTop = body.radius - body.y;
    const float penBottom = (body.y + body.radius) - _height;

    float maxPen = 0.0f;

    if (penLeft > maxPen)
    {
        maxPen = penLeft;
        best.normal = Vec2(1.0f, 0.0f);
    }
    if (penRight > maxPen)
    {
        maxPen = penRight;
        best.normal = Vec2(-1.0f, 0.0f);
    }
    if (penTop > maxPen)
    {
        maxPen = penTop;
        best.normal = Vec2(0.0f, 1.0f);
    }
    if (penBottom > maxPen)
    {
        maxPen = penBottom;
        best.normal = Vec2(0.0f, -1.0f);
    }

    if (maxPen > 0.0f)
    {
        best.penetration = maxPen;
        best.hit = true;
    }
    return best;
}
