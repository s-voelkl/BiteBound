#include "GraphicsManager.h"

GraphicsManager::GraphicsManager(int width, int height)
    : _width(width), _height(height), _gfx(nullptr), _needsFullRedraw(true), _prevCookieCount(0)
{
    memset(_prevCookieActiveStates, 0, sizeof(_prevCookieActiveStates));
}

void GraphicsManager::begin(Arduino_GFX *gfx)
{
    _gfx = gfx;
    _needsFullRedraw = true;
}

void GraphicsManager::forceFullRedraw()
{
    _needsFullRedraw = true;
}

void GraphicsManager::drawFullMaze(const uint8_t *mazeGrid, int gridWidth, int gridHeight)
{
    if (!_gfx || !mazeGrid)
        return;

    // Scanline-based run length optimizer to compress draw commands sent via SPI
    for (int gy = 0; gy < gridHeight; ++gy)
    {
        int sy = gy + ui_header_height;
        int gx = 0;

        while (gx < gridWidth)
        {
            uint8_t state = mazeGrid[gy * gridWidth + gx];
            int runLength = 1;

            while (gx + runLength < gridWidth && mazeGrid[gy * gridWidth + gx + runLength] == state)
            {
                runLength++;
            }

            uint16_t color = color_background;
            if (state == 1)
            {
                color = color_wall_type_1;
            }
            else if (state == 2)
            {
                color = color_wall_type_2;
            }

            if (runLength == 1)
            {
                _gfx->drawPixel(gx, sy, color);
            }
            else
            {
                _gfx->drawFastHLine(gx, sy, runLength, color);
            }
            gx += runLength;
        }
    }
}

void GraphicsManager::drawUI(const GameState &state, bool forceDraw)
{
    if (!_gfx)
        return;

    // Prevent screen flicker by executing heavy character renders only when data shifts
    bool statusChanged = strcmp(state.status, _prevGameState.status) != 0;
    bool scoreChanged = state.cookiesCollected != _prevGameState.cookiesCollected ||
                        state.cookiesRemaining != _prevGameState.cookiesRemaining;
    bool roundChanged = state.currentRound != _prevGameState.currentRound;
    bool timeChanged = abs(state.elapsedTimeSec - _prevGameState.elapsedTimeSec) >= 1.0f;

    if (forceDraw || statusChanged || scoreChanged || roundChanged || timeChanged)
    {
        // Redraw HUD boundary and reset text settings
        _gfx->fillRect(0, 0, _width, ui_header_height, color_background);
        _gfx->drawFastHLine(0, ui_header_height - 1, _width, color_wall_type_1);
        _gfx->setTextColor(color_ui_text);
        _gfx->setTextSize(1);

        // Print scoring
        _gfx->setCursor(4, 5);
        _gfx->print("Round:");
        _gfx->print(state.currentRound);
        _gfx->print(" Cookies:");
        _gfx->print(state.cookiesCollected);
        _gfx->print("/");
        _gfx->print(state.cookiesRemaining);

        // Print stopwatch timer
        _gfx->print(" Time:");
        _gfx->print((int)state.elapsedTimeSec);
        _gfx->print("s");

        // Align status string
        _gfx->setCursor(_width - 60, 5);
        _gfx->print(state.status);
    }
}

void GraphicsManager::eraseRegion(int cx, int cy, int radius, const uint8_t *mazeGrid, int gridWidth, int gridHeight)
{
    if (!_gfx || !mazeGrid)
        return;

    // Define localized bounding box surrounding the shape
    int minX = max(0, cx - radius - 1);
    int maxX = min(_width - 1, cx + radius + 1);
    int minY = max(ui_header_height, cy - radius - 1);
    int maxY = min(_height - 1, cy + radius + 1);

    for (int y = minY; y <= maxY; ++y)
    {
        int gridY = y - ui_header_height;
        if (gridY < 0 || gridY >= gridHeight)
            continue;

        for (int x = minX; x <= maxX; ++x)
        {
            if (x < 0 || x >= gridWidth)
                continue;

            uint8_t pixelState = mazeGrid[gridY * gridWidth + x];
            uint16_t color = color_background;

            if (pixelState == 1)
            {
                color = color_wall_type_1;
            }
            else if (pixelState == 2)
            {
                color = color_wall_type_2;
            }

            _gfx->drawPixel(x, y, color);
        }
    }
}

void GraphicsManager::drawCookie(const Cookie &cookie)
{
    if (!_gfx || !cookie.active)
        return;
    _gfx->fillCircle((int16_t)cookie.x, (int16_t)cookie.y, (int16_t)cookie.radius, color_cookie);
}

void GraphicsManager::drawSphere(const PhysicsBody &ball)
{
    if (!_gfx)
        return;
    _gfx->fillCircle((int16_t)ball.x, (int16_t)ball.y, (int16_t)ball.radius, color_sphere);
}

void GraphicsManager::update(
    const uint8_t *mazeGrid,
    int gridWidth,
    int gridHeight,
    const PhysicsBody &ball,
    const Cookie *cookies,
    int cookieCount,
    const GameState &state)
{
    if (!_gfx)
        return;

    // Detect structural system shifts requiring a complete redraw
    bool statusChanged = strcmp(state.status, _prevGameState.status) != 0;
    bool roundChanged = state.currentRound != _prevGameState.currentRound;

    if (statusChanged || roundChanged)
    {
        _needsFullRedraw = true;
    }

    if (_needsFullRedraw)
    {
        // Redraw display from scratch
        _gfx->fillScreen(color_background);
        drawFullMaze(mazeGrid, gridWidth, gridHeight);

        for (int i = 0; i < cookieCount && i < max_rendered_cookies; ++i)
        {
            drawCookie(cookies[i]);
            _prevCookieActiveStates[i] = cookies[i].active;
        }
        _prevCookieCount = min(cookieCount, max_rendered_cookies);

        drawSphere(ball);
        drawUI(state, true);

        // Store structures
        _prevBall = ball;
        _prevGameState = state;
        _needsFullRedraw = false;
    }
    else
    {
        // High-Performance Partial Update (50Hz Mode)

        // 1. Erase the sphere's previous position
        eraseRegion((int)_prevBall.x, (int)_prevBall.y, (int)_prevBall.radius, mazeGrid, gridWidth, gridHeight);

        // 2. Prevent visual clipping of active cookies overlapping the erased sphere area
        for (int i = 0; i < cookieCount && i < max_rendered_cookies; ++i)
        {
            if (cookies[i].active)
            {
                float dx = cookies[i].x - _prevBall.x;
                float dy = cookies[i].y - _prevBall.y;
                float distSq = dx * dx + dy * dy;
                float overlapDist = cookies[i].radius + _prevBall.radius + 1.0f;
                if (distSq <= overlapDist * overlapDist)
                {
                    drawCookie(cookies[i]);
                }
            }
        }

        // 3. Process changes in cookie states (erase newly collected, render respawned)
        for (int i = 0; i < cookieCount && i < max_rendered_cookies; ++i)
        {
            bool wasActive = (i < _prevCookieCount) ? _prevCookieActiveStates[i] : false;
            bool isActive = cookies[i].active;

            if (wasActive && !isActive)
            {
                eraseRegion((int)cookies[i].x, (int)cookies[i].y, (int)cookies[i].radius, mazeGrid, gridWidth, gridHeight);
            }
            else if (!wasActive && isActive)
            {
                drawCookie(cookies[i]);
            }
            _prevCookieActiveStates[i] = isActive;
        }
        _prevCookieCount = min(cookieCount, max_rendered_cookies);

        // 4. Render the ball in its new coordinates
        drawSphere(ball);

        // 5. Update the HUD
        drawUI(state, false);

        // Save current state as baseline for the next tick
        _prevBall = ball;
        _prevGameState = state;
    }
}