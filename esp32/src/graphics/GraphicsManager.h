#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "../../config.h"
#include "../game/Cookie.h"
#include "../game/Cookie.h"
#include "../physics/PhysicsBody.h"
#include "../game/GameState.h"

/**
 * @class GraphicsManager
 * @brief Handles performance-optimized drawing, leveraging dirty tracking and partial updates.
 */
class GraphicsManager
{
public:
    /**
     * @brief Constructor for the Graphics Manager.
     * @param width Width of the display board in pixels.
     * @param height Height of the display board in pixels.
     */
    GraphicsManager(int width = display_width, int height = display_height);

    /**
     * @brief Attaches the physical or canvas-based GFX interface.
     * @param gfx Pointer to the Arduino_GFX display object.
     */
    void begin(Arduino_GFX *gfx);

    /**
     * @brief Forces the engine to fully redraw the background and structures on the next loop tick.
     */
    void forceFullRedraw();

    /**
     * @brief The core rendering function executed every tick (50Hz).
     * Automatically switches between full updates and dynamic partial updates.
     *
     * @param mazeGrid Single-dimension bitmap array mapping states per pixel.
     * @param gridWidth Pixels wide of the active play area.
     * @param gridHeight Pixels high of the active play area.
     * @param ball Sphere object containing the current coordinate states.
     * @param cookies Array of all collectible objects.
     * @param cookieCount Dimension size of the cookies array.
     * @param state Scoreboard meta tracking details.
     */
    void update(
        const uint8_t *mazeGrid,
        int gridWidth,
        int gridHeight,
        const PhysicsBody &ball,
        const Cookie *cookies,
        int cookieCount,
        const GameState &state);

    // Getters for internal tracking state (useful for validating within unit tests)
    bool isFullRedrawNeeded() const { return _needsFullRedraw; }
    PhysicsBody getPrevBallState() const { return _prevBall; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }

private:
    int _width;
    int _height;
    Arduino_GFX *_gfx;
    bool _needsFullRedraw;

    // Local registers tracking changes between consecutive frames
    PhysicsBody _prevBall;
    bool _prevCookieActiveStates[max_rendered_cookies];
    int _prevCookieCount;
    GameState _prevGameState;

    /**
     * @brief Draws the full play area pixel structures from scratch using horizontal line optimization.
     */
    void drawFullMaze(const uint8_t *mazeGrid, int gridWidth, int gridHeight);

    /**
     * @brief Redraws the HUD overlays selectively based on changed metrics.
     */
    void drawUI(const GameState &state, bool forceDraw);

    /**
     * @brief Restores the base maze states over localized bounding boxes to "erase" objects.
     */
    void eraseRegion(int cx, int cy, int radius, const uint8_t *mazeGrid, int gridWidth, int gridHeight);

    /**
     * @brief Draws a single circular cookie.
     */
    void drawCookie(const Cookie &cookie);

    /**
     * @brief Draws the sphere.
     */
    void drawSphere(const PhysicsBody &ball);
};

#endif // GRAPHICS_MANAGER_H