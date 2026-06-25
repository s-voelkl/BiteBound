#include <AUnit.h>
#include "GraphicsManager.h"

/**
 * @class MockGFX
 * @brief Test double for Arduino_GFX to track rendering call counts.
 */
class MockGFX : public Arduino_GFX
{
public:
    MockGFX(int16_t w, int16_t h)
        : Arduino_GFX(w, h), drawPixelCount(0), fillRectCount(0), fillScreenCount(0), drawFastHLineCount(0), fillCircleCount(0) {}

    bool begin(int32_t speed = GFX_NOT_DEFINED) override
    {
        return true;
    }

    void writePixelPreclipped(int16_t x, int16_t y, uint16_t color) override
    {
        drawPixelCount++;
    }

    void fillScreen(uint16_t color) override
    {
        fillScreenCount++;
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override
    {
        fillRectCount++;
    }

    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override
    {
        drawFastHLineCount++;
    }

    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override
    {
        fillCircleCount++;
    }

    int drawPixelCount;
    int fillRectCount;
    int fillScreenCount;
    int drawFastHLineCount;
    int fillCircleCount;
};

// Verifies that constructor correctly maps basic state
test(GraphicsManagerTest, Initialization)
{
    GraphicsManager gm(240, 280);
    assertEqual(gm.getWidth(), 240);
    assertEqual(gm.getHeight(), 280);
    assertTrue(gm.isFullRedrawNeeded());
}

// Verifies that forceFullRedraw updates the internal state flag
test(GraphicsManagerTest, ForceFullRedrawFlag)
{
    GraphicsManager gm(240, 280);
    MockGFX mockGfx(240, 280);
    gm.begin(&mockGfx);

    uint8_t dummyGrid[10] = {0};
    PhysicsBody dummyBall;
    Cookie dummyCookies[1];
    GameState dummyState;

    gm.update(dummyGrid, 10, 1, dummyBall, dummyCookies, 0, dummyState);
    assertFalse(gm.isFullRedrawNeeded());

    gm.forceFullRedraw();
    assertTrue(gm.isFullRedrawNeeded());
}

// Verifies transitions between expensive full redraws and fast partial updates
test(GraphicsManagerTest, FullVsPartialDrawTransitions)
{
    GraphicsManager gm(240, 280);
    MockGFX mockGfx(240, 280);
    gm.begin(&mockGfx);

    uint8_t grid[100] = {0};
    grid[5] = 1;
    grid[15] = 2; // Add walls

    PhysicsBody ball;
    ball.x = 50.0f;
    ball.y = 50.0f;
    ball.radius = 4.0f;

    Cookie cookies[2];
    cookies[0] = Cookie(20.0f, 30.0f, 3.0f, true);
    cookies[1] = Cookie(40.0f, 60.0f, 3.0f, true);

    GameState state;
    state.status = "running";
    state.cookiesCollected = 0;
    state.cookiesRemaining = 2;
    state.currentRound = 1;
    state.elapsedTimeSec = 0.0f;

    // First iteration: Trigger Full Redraw
    gm.update(grid, 10, 10, ball, cookies, 2, state);
    assertEqual(mockGfx.fillScreenCount, 1);
    assertFalse(gm.isFullRedrawNeeded());

    int preUpdateDrawPixelCount = mockGfx.drawPixelCount;
    int preUpdateFillCircleCount = mockGfx.fillCircleCount;

    // Tick 2: Move the ball slightly (triggers Partial Redraw)
    ball.x = 52.0f;
    gm.update(grid, 10, 10, ball, cookies, 2, state);

    // Entire screen must NOT clear again
    assertEqual(mockGfx.fillScreenCount, 1);

    // Validate only regional changes were written
    assertTrue(mockGfx.drawPixelCount > preUpdateDrawPixelCount);
    assertTrue(mockGfx.fillCircleCount > preUpdateFillCircleCount);
}

// Verifies that changes to the round metadata force a full redraw
test(GraphicsManagerTest, RoundChangeForcesFullRedraw)
{
    GraphicsManager gm(240, 280);
    MockGFX mockGfx(240, 280);
    gm.begin(&mockGfx);

    uint8_t grid[10] = {0};
    PhysicsBody ball;
    Cookie cookies[1];
    GameState state;
    state.currentRound = 1;

    gm.update(grid, 10, 1, ball, cookies, 0, state);
    assertFalse(gm.isFullRedrawNeeded());

    state.currentRound = 2;
    gm.update(grid, 10, 1, ball, cookies, 0, state);

    assertEqual(mockGfx.fillScreenCount, 2); // Initial loop + round changed
}