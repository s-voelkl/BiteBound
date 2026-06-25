#include <AUnit.h>
#include "GraphicsManager.h"

/**
 * @class MockGFX
 * @brief Test double for Arduino_GFX tracking rendering operations via virtual transaction overrides.
 */
class MockGFX : public Arduino_GFX
{
public:
    MockGFX(int16_t w, int16_t h)
        : Arduino_GFX(w, h), beginCount(0), writePixelPreclippedCount(0),
          writeFillRectPreclippedCount(0), writeFastHLineCount(0), startWriteCount(0), endWriteCount(0) {}

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

    // Counters for unit test validation
    int beginCount;
    int writePixelPreclippedCount;
    int writeFillRectPreclippedCount;
    int writeFastHLineCount;
    int startWriteCount;
    int endWriteCount;
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

    // Check that we performed initial screen wipes and fills
    assertTrue(mockGfx.writeFillRectPreclippedCount > 0);
    assertFalse(gm.isFullRedrawNeeded());

    int preUpdateWritePixelCount = mockGfx.writePixelPreclippedCount;
    int preUpdateFastHLineCount = mockGfx.writeFastHLineCount;
    int preUpdateFillRectCount = mockGfx.writeFillRectPreclippedCount;

    // Tick 2: Move the ball slightly (triggers Partial Redraw)
    ball.x = 52.0f;
    gm.update(grid, 10, 10, ball, cookies, 2, state);

    // The screen should NOT have been cleared again (fill rect counts should stay identical)
    assertEqual(mockGfx.writeFillRectPreclippedCount, preUpdateFillRectCount);

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

    int initialFillCount = mockGfx.writeFillRectPreclippedCount;

    state.currentRound = 2;
    gm.update(grid, 10, 1, ball, cookies, 0, state);

    // Redraw count should have incremented due to the screen wipe
    assertTrue(mockGfx.writeFillRectPreclippedCount > initialFillCount);
}