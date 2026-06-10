# Overview about the used prompts

## 1. Random DFS Maze Generation Algorithm

Prompt:

```txt
Act as a computer science expert. For an embedded IoT game ("BiteBound"), I need to design a maze generation algorithm for a grid-mapped display space (conceptually 240x280 with variable wall thickness).

Provide an architectural breakdown of a Randomized Depth-First Search (DFS) algorithm with backtracking. Your response must include:
- Definition and core concepts of Randomized DFS in maze generation.
- A step-by-step description of cell visitation and backtracking mechanics.
- Structured, language-agnostic pseudo-code of the generation loop, stack operations, and random neighbor selection.

STRICT REQUIREMENT: Do not generate any functional source code (e.g., C++, Python). Provide only high-level logical pseudo-code and analysis.
Keep your answer concise, technical, and focused on algorithmic design principles.
```

Answer with Gemini Flash 3.5 Thinking:

```txt
BITEBOUND: RANDOMIZED DFS MAZE GENERATION ARCHITECTURE

1. CORE CONCEPTS & GRID MAPPING
Randomized Depth-First Search (DFS) generates a "perfect maze" (one unique path between any two points, no loops) by computing a spanning tree over a logical grid graph.

To adapt this to a 240x280 pixel space:

- Abstract the pixel canvas into a logical grid where:
- Total Unit Size (U) = Cell Width + Wall Thickness.
- Columns = 240 / U, Rows = 280 / U.

Memory Optimization: In embedded IoT systems, recursion risks call-stack overflow. Use an iterative approach with an explicit array-based stack in static RAM.

Cell State: Track visitation and wall states (North, South, East, West) using bitmasks packed into a single byte per cell.

2. CELL VISITATION & BACKTRACKING MECHANICS
The algorithm carves a path through solid walls using a random walk, backtracking when it encounters a dead end.

Step-by-Step Execution:

- Initialize: Mark all cells as unvisited with all walls intact. Pick a starting cell as current_cell and mark it visited.
- Evaluate: Check the four orthogonal neighbors of current_cell. Filter out out-of-bounds or already visited cells.
- Advance: If unvisited neighbors exist, select one at random. Push current_cell onto the backtrack stack. Break the wall between current_cell and the chosen neighbor. Move to the neighbor and mark it visited.
- Backtrack: If no unvisited neighbors exist, pop a cell from the backtrack stack and make it the current_cell.
- Terminate: Repeat steps 2-4 until the backtrack stack is empty.

3. LOGICAL PSEUDO-CODE

FUNCTION GenerateMaze(rows, cols):
Create 2D array 'grid' of size [rows][cols] (all visited = false, all walls intact)
Create array-based 'stack'
current_cell = [0, 0]
mark current_cell as visited

LOOP WHILE true:
    neighbors = GetUnvisitedNeighbors(current_cell, grid, rows, cols)
    
    IF neighbors is not empty THEN
        next_cell = RandomSelect(neighbors)
        PUSH current_cell TO stack
        RemoveWallsBetween(current_cell, next_cell, grid)
        current_cell = next_cell
        mark current_cell as visited
    ELSE IF stack is not empty THEN
        current_cell = POP from stack
    ELSE
        BREAK LOOP (Maze complete)
    END IF
END LOOP
END FUNCTION

FUNCTION GetUnvisitedNeighbors(cell, grid, rows, cols):
unvisited = empty list
directions = [[-1,0], [1,0], [0,-1], [0,1]] // North, South, West, East

FOR EACH dir IN directions:
    check_row = cell.row + dir[0]
    check_col = cell.col + dir[1]
    
    IF check_row within bounds AND check_col within bounds THEN
        IF grid[check_row][check_col].visited is false THEN
            ADD [check_row, check_col] TO unvisited
        END IF
    END IF
END FOR
RETURN unvisited
END FUNCTION

4. EMBEDDED DESIGN TIP
For high performance on low-power displays, update the screen incrementally by drawing the "carved" paths during step 3, rather than redrawing the whole grid every loop cycle.
```
