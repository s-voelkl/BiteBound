#include <AUnit.h>
#include "src/physics/MazeCollider.h"
#include "src/physics/PhysicsBody.h"

static const int kW = 20;
static const int kH = 20;

static PhysicsBody ballAt(float x, float y, float r = 4.0f)
{
    PhysicsBody b;
    b.x = x;
    b.y = y;
    b.radius = r;
    return b;
}

// Board: all free corridor except a solid vertical wall at column x = 10.
static void buildWalledBoard(uint8_t *board)
{
    for (int i = 0; i < kW * kH; ++i)
    {
        board[i] = 0;
    }
    for (int y = 0; y < kH; ++y)
    {
        board[y * kW + 10] = 2; // Wall Type 2 column
    }
}

/**
 * @test A body in open corridor, clear of the wall, reports no contact.
 */
test(MazeColliderTest, freeSpaceHasNoContact)
{
    uint8_t board[kW * kH];
    buildWalledBoard(board);
    MazeCollider col(board, kW, kH);

    Contact c = col.query(ballAt(5.0f, 10.0f)); // 5 px from the wall at x=10, r=4
    assertFalse(c.hit);
}

/**
 * @test A body overlapping the wall reports a hit with an outward normal
 * (pointing away from the wall, into free space).
 */
test(MazeColliderTest, overlapReportsOutwardNormal)
{
    uint8_t board[kW * kH];
    buildWalledBoard(board);
    MazeCollider col(board, kW, kH);

    Contact c = col.query(ballAt(8.0f, 10.0f)); // center 2 px left of the wall, r=4
    assertTrue(c.hit);
    assertMore(c.penetration, 0.0f);
    assertLess(c.normal.x, 0.0f); // pushes the ball left, out of the wall
}

/**
 * @test A null board degrades to "no contact" instead of dereferencing.
 */
test(MazeColliderTest, nullBoardIsSafe)
{
    MazeCollider col(nullptr, 0, 0);
    assertFalse(col.query(ballAt(5.0f, 5.0f)).hit);
}
