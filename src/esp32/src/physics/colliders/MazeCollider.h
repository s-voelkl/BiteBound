#ifndef PHYSICS_MAZE_COLLIDER_H
#define PHYSICS_MAZE_COLLIDER_H

#include <stdint.h>
#include "ICollider.h"

/**
 * @brief Collision environment backed by a MazeManager pixel board.
 *
 * Treats every non-zero board pixel (Wall Type 1 = border/padding, Wall Type 2 =
 * inner maze wall) as solid and every 0 pixel as free corridor. The outer board
 * ring is Wall Type 1, so this collider also keeps the ball inside the play
 * area; Game 1 needs no separate BorderCollider.
 *
 * Used by Game 1. The board pointer (owned by the engine / MazeManager) MUST
 * outlive this collider and is re-filled in place on every maze regeneration.
 */
class MazeCollider : public ICollider
{
public:
    /**
     * @param board  Flat width*height pixel board (0 = free, !=0 = wall).
     * @param width  Board width in pixels.
     * @param height Board height in pixels.
     */
    MazeCollider(const uint8_t *board = nullptr, int width = 0, int height = 0);

    Contact query(const PhysicsBody &body) const override;

private:
    const uint8_t *_board; ///< Not owned; must outlive this collider.
    int _width;
    int _height;
};

#endif // PHYSICS_MAZE_COLLIDER_H
