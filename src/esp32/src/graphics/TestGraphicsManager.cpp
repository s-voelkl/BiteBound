#include <AUnit.h>
#include "GraphicsManager.h"
#include "src/game/RunningStatus.h"

/**
 * @class MockGFX
 * @brief Test double for Arduino_GFX tracking rendering operations via virtual transaction overrides.
 */
class MockGFX : public Arduino_GFX
{
public:
    MockGFX(int16_t w, int16_t h)
        : Arduino_GFX(w, h), beginCount(0), writePixelPreclippedCount(0),
          writeFillRectPreclippedCount(0), writeFastHLineCount(0),
          startWriteCount(0), endWriteCount(0), fullScreenClearCount(0) {}

    bool begin(int32_t speed = GFX_NOT_DEFINED) override
    {
        beginCount++;
        return true;
    }

    // Intercept base pixel drawing (Must override as it is pure virtual)
    void writePixelPreclipped(int16_t x, int16_t y, uint16_t color) override
    {
        writePixelPreclippedCount++;
    }

    // Intercept rectangular area fills (called by fillRect, fillScreen, etc.)
    void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override
    {
        writeFillRectPreclippedCount++;
        // If the rectangle dimensions span the full display, we track it as a screen clear
        if (w == _width && h == _height)
        {
            fullScreenClearCount++;
        }
    }

    // Intercept horizontal lines (used by RLE optimizations)
    void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override
    {
        writeFastHLineCount++;
    }

    // Intercept write transaction blocks
    void startWrite() override
    {
        startWriteCount++;
    }

    void endWrite() override
    {
        endWriteCount++;
    }

    // Counter statistics
    int beginCount;
    int writePixelPreclippedCount;
    int writeFillRectPreclippedCount;
    int writeFastHLineCount;
    int startWriteCount;
    int endWriteCount;
    int fullScreenClearCount;
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

    uint8_t grid[1000] = {0};
    grid[5] = 1;
    grid[15] = 2; // Add walls

    PhysicsBody ball;
    ball.x = 5.0f;
    ball.y = 5.0f;
    ball.radius = 2.0f;

    Cookie cookies[2];
    cookies[0] = Cookie(2.0f, 3.0f, 1.0f, true);
    cookies[1] = Cookie(4.0f, 6.0f, 1.0f, true);

    GameState state;
    state.runningStatus = RunningStatus::RUNNING;
    state.cookiesCollected = 0;
    state.cookiesRemaining = 2;
    state.currentRound = 1;
    state.elapsedTimeSec = 0.0f;

    // First iteration: Trigger Full Redraw
    gm.update(grid, 10, 100, ball, cookies, 2, state);

    // Check that we performed exactly one full screen clear
    assertEqual(mockGfx.fullScreenClearCount, 1);
    assertFalse(gm.isFullRedrawNeeded());

    int preUpdateWritePixelCount = mockGfx.writePixelPreclippedCount;
    int preUpdateFastHLineCount = mockGfx.writeFastHLineCount;

    // Tick 2: Move the ball slightly (triggers Partial Redraw)
    ball.x = 6.0f;
    gm.update(grid, 10, 100, ball, cookies, 2, state);

    // Verify that the screen was NOT cleared again
    assertEqual(mockGfx.fullScreenClearCount, 1);

    // Validate that only regional changes (pixels/lines) were written
    assertTrue(mockGfx.writePixelPreclippedCount > preUpdateWritePixelCount || mockGfx.writeFastHLineCount > preUpdateFastHLineCount);
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

    assertEqual(mockGfx.fullScreenClearCount, 1);

    state.currentRound = 2;
    gm.update(grid, 10, 1, ball, cookies, 0, state);

    // Redraw count should have incremented due to the screen wipe
    assertEqual(mockGfx.fullScreenClearCount, 2);
}

// Verifies that drawLoadingScreen executes and clears the display
test(GraphicsManagerTest, DrawLoadingScreenExecution)
{
    GraphicsManager gm(240, 280);
    MockGFX mockGfx(240, 280);
    gm.begin(&mockGfx);

    // Invoke screen draw
    gm.drawLoadingScreen("Testing System...");

    // Validate that a full screen clear took place
    assertEqual(mockGfx.fullScreenClearCount, 1);

    // Ensure procedural rendering occurred (fills circles for background bite, dough, chips)
    assertTrue(mockGfx.writeFillRectPreclippedCount > 0);
}