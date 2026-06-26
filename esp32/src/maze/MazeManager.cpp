#include "MazeManager.h"
#include <stack>
#include <algorithm>
#include <cstdlib>

MazeManager::MazeManager(int width, int height, int wallThickness)
    : _width(width), _height(height), _wallThickness(wallThickness) {}

void MazeManager::writeBlock(uint8_t *board, int gx, int gy, uint8_t value, int offsetX, int offsetY)
{
    int startX = gx * _wallThickness + offsetX;
    int startY = gy * _wallThickness + offsetY;

    // Safeguard to prevent out of bounds block generation
    if (startX >= _width || startY >= _height || startX < 0 || startY < 0)
    {
        return;
    }

    for (int y = 0; y < _wallThickness; ++y)
    {
        int py = startY + y;
        if (py >= _height)
            break;

        int rowOffset = py * _width;
        for (int x = 0; x < _wallThickness; ++x)
        {
            int px = startX + x;
            if (px >= _width)
                break;

            board[rowOffset + px] = value;
        }
    }
}

void MazeManager::fillBoard(uint8_t *board, uint8_t value)
{
    int totalPixels = _width * _height;
    for (int i = 0; i < totalPixels; ++i)
    {
        board[i] = value;
    }
}

bool MazeManager::generate(uint8_t *board)
{
    if (!board)
    {
        return false;
    }

    // Ensure the play space can support at least a single path cell surrounded by borders
    if (_wallThickness <= 0 || _width < _wallThickness * 3 || _height < _wallThickness * 3)
    {
        fillBoard(board, 1); // Fill with outer boundaries as a fallback
        return false;
    }

    // Convert display pixels to raw block units
    int gridCols = _width / _wallThickness;
    int gridRows = _height / _wallThickness;

    // The maze requires odd block dimensions to preserve symmetric outer walls
    int mazeCols = (gridCols % 2 == 0) ? gridCols - 1 : gridCols;
    int mazeRows = (gridRows % 2 == 0) ? gridRows - 1 : gridRows;

    // Calculate unused pixel remainders along both axes
    int unusedWidthPx = _width - (mazeCols * _wallThickness);
    int unusedHeightPx = _height - (mazeRows * _wallThickness);

    // Distribute the unused padding symmetrically to the edges
    // This ensures the maze is centered within the play area
    int offsetX = unusedWidthPx / 2;
    int offsetY = unusedHeightPx / 2;

    // Compute dimensions for the internal DFS graph nodes (cells)
    int cellCols = mazeCols / 2;
    int cellRows = mazeRows / 2;

    if (cellCols <= 0 || cellRows <= 0)
    {
        fillBoard(board, 1);
        return false;
    }

    // 1. Fill the entire canvas with Wall Type 1 (outer border & alignment padding)
    fillBoard(board, 1);

    // 2. Fill the active inner grid with Wall Type 2 (uncarved inner walls)
    for (int gy = 1; gy < mazeRows - 1; ++gy)
    {
        for (int gx = 1; gx < mazeCols - 1; ++gx)
        {
            writeBlock(board, gx, gy, 2, offsetX, offsetY);
        }
    }

    // Flat tracking vector for visited status of cells
    std::vector<bool> visited(cellCols * cellRows, false);
    std::stack<Cell> cellStack;

    // Start DFS at top-left cell: (0, 0) in cell coords -> (1, 1) in block coords
    Cell startCell = {0, 0};
    visited[0] = true;
    writeBlock(board, 1, 1, 0, offsetX, offsetY); // Carve open the entry cell
    cellStack.push(startCell);

    // Offsets for Left, Right, Up, Down neighbors
    const int dx[] = {-1, 1, 0, 0};
    const int dy[] = {0, 0, -1, 1};

    while (!cellStack.empty())
    {
        Cell current = cellStack.top();

        // Scan for unvisited neighbor cells
        std::vector<int> validDirections;
        for (int i = 0; i < 4; ++i)
        {
            int nx = current.cx + dx[i];
            int ny = current.cy + dy[i];

            if (nx >= 0 && nx < cellCols && ny >= 0 && ny < cellRows)
            {
                if (!visited[ny * cellCols + nx])
                {
                    validDirections.push_back(i);
                }
            }
        }

        if (!validDirections.empty())
        {
            // Select a random accessible direction
            int randIdx = std::rand() % validDirections.size();
            int dir = validDirections[randIdx];

            Cell neighbor = {current.cx + dx[dir], current.cy + dy[dir]};

            // Flag as visited
            visited[neighbor.cy * cellCols + neighbor.cx] = true;

            // Carve the chosen neighbor cell
            int neighborBlockX = 2 * neighbor.cx + 1;
            int neighborBlockY = 2 * neighbor.cy + 1;
            writeBlock(board, neighborBlockX, neighborBlockY, 0, offsetX, offsetY);

            // Carve the intermediate wall blocking the pathway between them
            int wallBlockX = current.cx + neighbor.cx + 1;
            int wallBlockY = current.cy + neighbor.cy + 1;
            writeBlock(board, wallBlockX, wallBlockY, 0, offsetX, offsetY);

            // Descend into the neighbor cell
            cellStack.push(neighbor);
        }
        else
        {
            // Dead end reached, pop and backtrack
            cellStack.pop();
        }
    }

    return true;
}