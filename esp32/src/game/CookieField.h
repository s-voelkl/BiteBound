#ifndef COOKIE_FIELD_H
#define COOKIE_FIELD_H

#include <stdint.h>
#include "Cookie.h"
#include "../physics/PhysicsBody.h" // for PhysicsBody in checkPickup
#include "../../config.h"

// Max count of cookies
static const int kMaxVisibleCookies = default_max_visible_cookies;

/**
 * @brief Produces a fresh cookie at a valid position for the current game.
 *
 * Game 2 supplies a rectangle spawner, Game 1 a maze-cell spawner. CookieField
 * depends only on this interface, so the pickup/respawn logic stays shared and
 * unit-testable.
 */
class ICookieSpawner
{
public:
    virtual ~ICookieSpawner() = default;

    /**
     * @brief Returns a new active cookie at a valid position.
     * @param avoid A body the new cookie should not overlap (the ball).
     */
    virtual Cookie spawn(const PhysicsBody &avoid) const = 0;
};

/**
 * @brief A small set of cookies that respawn on contact.
 *
 * Keeps a few visible cookies (1-3). When the ball touches one, the score
 * increases and that cookie reappears at a new position from the injected
 * spawner. The game is won once collected() reaches target(). Reusable in both games!
 * Only the CookieField itself knows the current score and target; the game loop
 * queries it via collected() and remaining().
 */
class CookieField
{
public:
    /**
     * @brief (Re)starts the field for a new round.
     *
     * @param visibleCount Cookies on screen at once (clamped to kMaxVisibleCookies).
     * @param target       Score needed to win (e.g. cmd.cookiesCount).
     * @param spawner      Position source. MUST outlive this CookieField.
     * @param ball         Current ball, so the initial cookies avoid it.
     */
    void start(uint8_t visibleCount, uint16_t target,
               const ICookieSpawner &spawner, const PhysicsBody &ball);

    /**
     * @brief Checks contact; on a hit, scores and respawns the eaten cookie.
     * @return Number of cookies eaten in this call (usually 0 or 1).
     */
    uint8_t checkPickup(const PhysicsBody &ball);

    uint16_t target() const { return _target; }
    uint16_t collected() const { return _collected; }
    uint16_t remaining() const { return (_target > _collected) ? (_target - _collected) : 0; }
    bool finished() const { return _collected >= _target; }

    uint8_t count() const { return _count; } // visible cookies (for rendering)
    const Cookie &at(uint8_t i) const { return _cookies[i]; }

private:
    Cookie _cookies[kMaxVisibleCookies]; // fixed size -> no heap (MCU-friendly)
    uint8_t _count = 0;
    uint16_t _target = 0;
    uint16_t _collected = 0;
    const ICookieSpawner *_spawner = nullptr; // owned by the game, must outlive us
};

#endif // COOKIE_FIELD_H
