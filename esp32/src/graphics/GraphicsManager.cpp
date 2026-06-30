#include "GraphicsManager.h"
#include "src/game/RunningStatus.h"

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
    bool statusChanged = state.runningStatus != _prevGameState.runningStatus;
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
        _gfx->setCursor(30, 5);
        _gfx->print("Round:");
        _gfx->print(state.currentRound);
        // Show collected / target. There's no target field in GameState, but
        // remaining = target - collected, so collected + remaining = target.
        _gfx->print(" Cookies:");
        _gfx->print(state.cookiesCollected);
        _gfx->print("/");
        _gfx->print(state.cookiesCollected + state.cookiesRemaining);

        // Print stopwatch timer
        // _gfx->print(" Time:");
        // _gfx->print((int)state.elapsedTimeSec);
        // _gfx->print("s");

        // Align runningStatus string
        // _gfx->setCursor(_width - 60, 5);
        String runningStatusStr;
        switch (state.runningStatus)
        {
        case RunningStatus::IDLE:
            runningStatusStr = "idle";
            break;
        case RunningStatus::RUNNING:
            runningStatusStr = "running";
            break;
        case RunningStatus::COMPLETED:
            runningStatusStr = "completed";
            break;
        default:
            runningStatusStr = "unknown";
            break;
        }
        _gfx->print(" ");
        _gfx->print(runningStatusStr);
    }
}

void GraphicsManager::eraseRegion(int cx, int cy, int radius, const uint8_t *mazeGrid, int gridWidth, int gridHeight)
{
    if (!_gfx || !mazeGrid)
        return;

    // Map play-area relative cy to screen coordinates
    int sy = cy + ui_header_height;

    // Define localized bounding box surrounding the shape
    int minX = max(0, cx - radius - 1);
    int maxX = min(_width - 1, cx + radius + 1);
    int minY = max(ui_header_height, sy - radius - 1);
    int maxY = min(_height - 1, sy + radius + 1);

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
    _gfx->fillCircle((int16_t)cookie.x, (int16_t)(cookie.y + ui_header_height), (int16_t)cookie.radius, color_cookie);
}

void GraphicsManager::drawSphere(const PhysicsBody &ball)
{
    if (!_gfx)
        return;
    _gfx->fillCircle((int16_t)ball.x, (int16_t)(ball.y + ui_header_height), (int16_t)ball.radius, color_sphere);
}

void GraphicsManager::drawPauseOverlay()
{
    if (!_gfx)
        return;

    const int barWidth = 14;
    const int barHeight = 56;
    const int gap = 16;

    int centerX = _width / 2;
    int centerY = ui_header_height + (_height - ui_header_height) / 2;
    int top = centerY - barHeight / 2;

    int leftX = centerX - gap / 2 - barWidth;
    int rightX = centerX + gap / 2;

    _gfx->fillRect(leftX, top, barWidth, barHeight, color_frosting_white);
    _gfx->fillRect(rightX, top, barWidth, barHeight, color_frosting_white);
}

void GraphicsManager::drawCompletedOverlay()
{
    if (!_gfx)
        return;

    // A small banner in the middle of the play area announcing the finished round.
    const int boxW = 150;
    const int boxH = 38;
    int centerX = _width / 2;
    int centerY = ui_header_height + (_height - ui_header_height) / 2;
    int boxX = centerX - boxW / 2;
    int boxY = centerY - boxH / 2;

    _gfx->fillRect(boxX, boxY, boxW, boxH, color_dark_cocoa);
    _gfx->drawRect(boxX, boxY, boxW, boxH, color_honey);

    _gfx->setTextColor(color_honey);
    _gfx->setTextSize(2);
    // "ROUND DONE" is 10 chars; size-2 glyphs are ~12px wide -> ~120px total.
    _gfx->setCursor(centerX - 60, centerY - 7);
    _gfx->print("ROUND DONE");
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
    bool statusChanged = state.runningStatus != _prevGameState.runningStatus;
    bool roundChanged = state.currentRound != _prevGameState.currentRound;

    if (statusChanged || roundChanged)
    {
        _needsFullRedraw = true;
    }

    // While paused (idle) the ball is frozen. Draw one full frame with the pause
    // bars on top, then leave the screen untouched on later ticks so the partial
    // update path doesn't erase the overlay.
    bool paused = (state.runningStatus == RunningStatus::IDLE);
    bool completed = (state.runningStatus == RunningStatus::COMPLETED);
    // Both paused and completed freeze the ball; draw one full frame with the
    // matching overlay, then leave the screen alone on later ticks so the partial
    // update path doesn't erase it.
    if ((paused || completed) && !_needsFullRedraw)
    {
        return;
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
            _prevCookies[i] = cookies[i];
        }
        _prevCookieCount = min(cookieCount, max_rendered_cookies);

        drawSphere(ball);
        drawUI(state, true);

        // Lay the pause bars / round-complete banner over everything else.
        if (paused)
        {
            drawPauseOverlay();
        }
        else if (completed)
        {
            drawCompletedOverlay();
        }

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

        // 3. Process cookie changes: erase eaten ones and (re)draw new or respawned
        // ones. A respawn keeps the cookie active but moves it to a new spot, so we
        // also have to treat a position change as "needs redraw" - that case was
        // missing before, which is why respawned cookies never showed up.
        for (int i = 0; i < cookieCount && i < max_rendered_cookies; ++i)
        {
            const Cookie &cur = cookies[i];
            const Cookie &prev = _prevCookies[i];
            bool wasActive = (i < _prevCookieCount) ? _prevCookieActiveStates[i] : false;
            bool isActive = cur.active;
            bool moved = ((int)prev.x != (int)cur.x) || ((int)prev.y != (int)cur.y);

            // Erase the old drawing if the cookie was visible and has now gone or moved.
            if (wasActive && (!isActive || moved))
            {
                eraseRegion((int)prev.x, (int)prev.y, (int)prev.radius, mazeGrid, gridWidth, gridHeight);
            }
            // Draw at the current spot if it is visible and just appeared or moved.
            if (isActive && (!wasActive || moved))
            {
                drawCookie(cur);
            }

            _prevCookieActiveStates[i] = isActive;
            _prevCookies[i] = cur;
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

void GraphicsManager::drawLoadingScreen(const char *statusText)
{
    if (!_gfx)
        return;

    // 1. Clear display with rich Dark Cocoa background
    _gfx->fillScreen(color_dark_cocoa);

    // Coordinate variables for center alignment
    int16_t centerX = _width / 2;
    int16_t centerY = _height / 2 - 15;
    int16_t baseRadius = 38;

    // 2. Procedural Cookie Rendering
    // Baked outer edge
    _gfx->fillCircle(centerX, centerY, baseRadius, color_cookie_golden);
    // Soft golden cookie dough center
    _gfx->fillCircle(centerX, centerY, baseRadius - 3, color_cookie_dough);

    // Scattered dark chocolate chips
    _gfx->fillCircle(centerX - 14, centerY - 12, 4, color_chocolate_chip);
    _gfx->fillCircle(centerX + 16, centerY - 16, 3, color_chocolate_chip);
    _gfx->fillCircle(centerX - 10, centerY + 14, 3, color_chocolate_chip);
    _gfx->fillCircle(centerX + 12, centerY + 12, 4, color_chocolate_chip);
    _gfx->fillCircle(centerX - 2, centerY - 2, 5, color_chocolate_chip);
    _gfx->fillCircle(centerX + 14, centerY - 2, 3, color_chocolate_chip);
    _gfx->fillCircle(centerX - 16, centerY + 1, 3, color_chocolate_chip);

    // Thematic "Bite" taken out of the right edge (using background color overlap)
    _gfx->fillCircle(centerX + baseRadius - 4, centerY + 6, 12, color_dark_cocoa);
    _gfx->fillCircle(centerX + baseRadius - 8, centerY - 6, 10, color_dark_cocoa);

    // 3. Render Title "BiteBound" with clear drop shadow
    _gfx->setTextSize(3);

    // Offset Drop-Shadow (Chocolate Chip color)
    _gfx->setTextColor(color_chocolate_chip);
    _gfx->setCursor(centerX - 78 + 2, centerY + 55 + 2);
    _gfx->print("BiteBound");

    // Foreground Text (Warm Frosting White)
    _gfx->setTextColor(color_frosting_white);
    _gfx->setCursor(centerX - 78, centerY + 55);
    _gfx->print("BiteBound");

    // 4. Render Loading Status (Honey glow accent)
    _gfx->setTextSize(1);
    _gfx->setTextColor(color_honey);

    // Center-align the string (6 pixels per character average width at size 1)
    int statusLen = strlen(statusText);
    int16_t statusX = centerX - (statusLen * 6) / 2;
    _gfx->setCursor(statusX, centerY + 100);
    _gfx->print(statusText);
}