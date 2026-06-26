#include "MazeCollider.h"
#include <math.h>

MazeCollider::MazeCollider(const uint8_t *board, int width, int height)
    : _board(board), _width(width), _height(height)
{
}

Contact MazeCollider::query(const PhysicsBody &body) const
{
    Contact best;
    if (_board == nullptr || _width <= 0 || _height <= 0)
    {
        return best;
    }

    const float r = body.radius;

    // Only the wall pixels under the body's bounding box can be in contact.
    int x0 = (int)floorf(body.x - r);
    int x1 = (int)ceilf(body.x + r);
    int y0 = (int)floorf(body.y - r);
    int y1 = (int)ceilf(body.y + r);
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > _width - 1) x1 = _width - 1;
    if (y1 > _height - 1) y1 = _height - 1;

    float maxPen = 0.0f;
    Vec2 normal(0.0f, -1.0f); // fallback if the center sits exactly on a wall pixel

    for (int py = y0; py <= y1; ++py)
    {
        const int row = py * _width;
        for (int px = x0; px <= x1; ++px)
        {
            if (_board[row + px] == 0)
            {
                continue; // free corridor
            }

            // Closest point of the 1x1 wall cell [px, px+1] x [py, py+1] to the center.
            float nx = body.x;
            if (nx < px) nx = (float)px;
            else if (nx > px + 1) nx = (float)(px + 1);
            float ny = body.y;
            if (ny < py) ny = (float)py;
            else if (ny > py + 1) ny = (float)(py + 1);

            const float dx = body.x - nx;
            const float dy = body.y - ny;
            const float d2 = dx * dx + dy * dy;
            if (d2 < r * r)
            {
                const float d = sqrtf(d2);
                const float pen = r - d;
                if (pen > maxPen)
                {
                    maxPen = pen;
                    if (d > 1e-4f)
                    {
                        normal = Vec2(dx / d, dy / d); // points out of the wall
                    }
                    else
                    {
                        normal = Vec2(0.0f, -1.0f);
                    }
                }
            }
        }
    }

    if (maxPen > 0.0f)
    {
        best.hit = true;
        best.penetration = maxPen;
        best.normal = normal;
    }
    return best;
}
