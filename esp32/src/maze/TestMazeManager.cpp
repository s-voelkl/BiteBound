#include <AUnit.h>
#include "src/maze/MazeManager.h"

// Test that MazeManager holds correct parameters after setup
test(MazeManagerTest, initialization_parameters)
{
    MazeManager maze(240, 260, 6);
    assertEqual(maze.getWidth(), 240);
    assertEqual(maze.getHeight(), 260);
    assertEqual(maze.getWallThickness(), 6);
}

// Test edge cases and invalid parameters to verify robustness
test(MazeManagerTest, invalid_parameters)
{
    // Canvas too small for generation
    MazeManager maze_too_small(10, 10, 6);
    uint8_t small_board[100];
    bool result = maze_too_small.generate(small_board);
    assertFalse(result);

    // Null safety check
    MazeManager maze_normal(240, 260, 6);
    bool result_null = maze_normal.generate(nullptr);
    assertFalse(result_null);
}

// Ensure every pixel in the board maps strictly to 0, 1, or 2
test(MazeManagerTest, valid_pixel_values)
{
    const int w = 60;
    const int h = 60;
    const int t = 6;
    uint8_t board[w * h];

    MazeManager maze(w, h, t);
    bool success = maze.generate(board);
    assertTrue(success);

    for (int i = 0; i < w * h; ++i)
    {
        uint8_t val = board[i];
        bool isValid = (val == 0 || val == 1 || val == 2);
        assertTrue(isValid);
    }
}

// Ensure the outer margins of the board are filled with Wall Type 1
test(MazeManagerTest, boundaries_are_wall_type_1)
{
    const int w = 48;
    const int h = 48;
    const int t = 6;
    uint8_t board[w * h];

    MazeManager maze(w, h, t);
    bool success = maze.generate(board);
    assertTrue(success);

    // Verify top and bottom edges
    for (int x = 0; x < w; ++x)
    {
        assertEqual(board[x], (uint8_t)1);
        assertEqual(board[(h - 1) * w + x], (uint8_t)1);
    }

    // Verify left and right edges
    for (int y = 0; y < h; ++y)
    {
        assertEqual(board[y * w], (uint8_t)1);
        assertEqual(board[y * w + (w - 1)], (uint8_t)1);
    }
}

// Verify that DFS successfully carved open passages (0s)
test(MazeManagerTest, pathways_are_carved)
{
    const int w = 60;
    const int h = 60;
    const int t = 6;
    uint8_t board[w * h];

    MazeManager maze(w, h, t);
    bool success = maze.generate(board);
    assertTrue(success);

    int pathwayPixels = 0;
    for (int i = 0; i < w * h; ++i)
    {
        if (board[i] == 0)
        {
            pathwayPixels++;
        }
    }
    assertTrue(pathwayPixels > 0);
}