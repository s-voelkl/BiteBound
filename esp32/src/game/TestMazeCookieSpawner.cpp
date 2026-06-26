#include <AUnit.h>
#include <vector>

#include "src/game/MazeCookieSpawner.h"
#include "src/game/CookieField.h"
#include "src/maze/MazeManager.h"
#include "src/physics/PhysicsBody.h"

// Shared small maze geometry: even wall thickness keeps cell centers integral,
// so a pixel center maps exactly onto a board index.
static const int kW = 60;
static const int kH = 60;
static const int kT = 6;
static const uint8_t kVisible = 3; // k cookies visible/respawning at once

static PhysicsBody ballAt(float x, float y, float r = 4.0f)
{
    PhysicsBody b;
    b.x = x;
    b.y = y;
    b.radius = r;
    return b;
}

// Returns the board value (0/1/2) under a cookie's center.
static uint8_t boardValueUnder(const uint8_t *board, const Cookie &c)
{
    int px = (int)c.x;
    int py = (int)c.y;
    return board[py * kW + px];
}

/**
 * @test Every cookie from spawn() lands on a passable corridor pixel (value 0),
 * never inside a wall. This is the core fix for cookies-spawning-in-the-maze.
 */
test(MazeCookieSpawnerTest, spawn_alwaysLandsOnCorridor)
{
    uint8_t board[kW * kH];
    MazeManager maze(kW, kH, kT);
    assertTrue(maze.generate(board));
    assertMore((int)maze.getFreeCells().size(), 0);

    MazeCookieSpawner spawner(&maze.getFreeCells(), 3.0f);
    PhysicsBody ball = ballAt(kW * 0.5f, kH * 0.5f);

    for (int i = 0; i < 50; ++i)
    {
        Cookie c = spawner.spawn(ball);
        assertTrue(c.active);
        assertEqual((int)boardValueUnder(board, c), 0); // on corridor, not a wall
    }
}

/**
 * @test On pickup the spawner respawns at a fresh corridor cell.
 */
test(MazeCookieSpawnerTest, spawn_respawnsOnCorridorAfterPickup)
{
    uint8_t board[kW * kH];
    MazeManager maze(kW, kH, kT);
    assertTrue(maze.generate(board));

    MazeCookieSpawner spawner(&maze.getFreeCells(), 3.0f);
    CookieField field;
    field.start(kVisible, 10, spawner, ballAt(kW * 0.5f, kH * 0.5f));

    // Every visible cookie starts on a corridor cell.
    for (uint8_t i = 0; i < field.count(); ++i)
    {
        assertTrue(field.at(i).active);
        assertEqual((int)boardValueUnder(board, field.at(i)), 0);
    }

    // Sit the ball on a cookie -> at least one is collected (overlapping cookies
    // sharing a cell are allowed) and every respawned cookie stays on a corridor.
    uint8_t eaten = field.checkPickup(ballAt(field.at(0).x, field.at(0).y));
    assertMoreOrEqual((int)eaten, 1);
    for (uint8_t i = 0; i < field.count(); ++i)
    {
        if (field.at(i).active)
        {
            assertEqual((int)boardValueUnder(board, field.at(i)), 0);
        }
    }
}

/**
 * @test With no maze cells available the spawner degrades safely to an inactive
 * cookie instead of dereferencing an empty list.
 */
test(MazeCookieSpawnerTest, spawn_withoutCellsIsInactive)
{
    std::vector<MazeManager::FreeCell> empty;
    MazeCookieSpawner spawnerNull(nullptr, 3.0f);
    MazeCookieSpawner spawnerEmpty(&empty, 3.0f);

    assertFalse(spawnerNull.spawn(ballAt(10, 10)).active);
    assertFalse(spawnerEmpty.spawn(ballAt(10, 10)).active);
}
