#ifndef MAZE_MANAGER_H
#define MAZE_MANAGER_H

#include <stdint.h>
#include <vector>

/**
 * @class MazeManager
 * @brief Handles procedural generation of a maze using a randomized Depth-First Search (DFS) algorithm.
 *
 * This class slices the play-field into a block grid where each block is of size wallThickness x wallThickness.
 * Pathways and walls are created from these blocks. The output is written into a flat 1D pixel array
 * of size (width * height), mapping pixels to:
 *
 * - 0: Empty (passageway)

 * - 1: Wall Type 1 (outer borders / solid boundary padding)
 *
 * - 2: Wall Type 2 (inner procedural maze walls)
 */
class MazeManager
{
public:
    /**
     * @brief Constructs the MazeManager.
     * @param width The width of the play area in pixels.
     * @param height The height of the play area in pixels.
     * @param wallThickness The thickness of walls and passages in pixels.
     */
    MazeManager(int width, int height, int wallThickness);

    /**
     * @brief Generates the procedural maze into the provided flat buffer.
     * @param board Pre-allocated flat pixel buffer of size (width * height).
     * @return true if generation was successful, false otherwise.
     */
    bool generate(uint8_t *board);

    /** @brief Returns the configured play-field width in pixels. */
    int getWidth() const { return _width; }

    /** @brief Returns the configured play-field height in pixels. */
    int getHeight() const { return _height; }

    /** @brief Returns the configured wall thickness in pixels. */
    int getWallThickness() const { return _wallThickness; }

private:
    int _width;         ///< Width of the maze area in pixels.
    int _height;        ///< Height of the maze area in pixels.
    int _wallThickness; ///< Thickness of the maze components in pixels.

    /**
     * @struct Cell
     * @brief Coordinates of a pathway hub in the DFS grid.
     */
    struct Cell
    {
        int cx; ///< Column index in the cell grid.
        int cy; ///< Row index in the cell grid.

        bool operator==(const Cell &other) const
        {
            return cx == other.cx && cy == other.cy;
        }
    };

    /**
     * @brief Sets all pixels within a specific grid block to a target value.
     * @param board Flat pixel array.
     * @param gx Grid X coordinate in blocks.
     * @param gy Grid Y coordinate in blocks.
     * @param value The pixel state (0, 1, or 2).
     */
    void writeBlock(uint8_t *board, int gx, int gy, uint8_t value);

    /**
     * @brief Fills the entire flat buffer with a target pixel value.
     * @param board Flat pixel array.
     * @param value The pixel state (0, 1, or 2).
     */
    void fillBoard(uint8_t *board, uint8_t value);
};

#endif // MAZE_MANAGER_H