#include "CookieField.h"
#include "Cookie.h"
#include "../physics/PhysicsBody.h"

#include <AUnit.h>

/**
 * @brief Deterministic spawner: hands out positions from a fixed list, cycling.
 *
 * Lets the tests control exactly where cookies (re)appear, so checkPickup() is
 * fully predictable without any random number generator.
 */
class ScriptedSpawner : public ICookieSpawner
{
public:
    ScriptedSpawner(const Cookie *seq, uint8_t n) : _seq(seq), _n(n), _i(0) {}

    Cookie spawn(const PhysicsBody &) const override
    {
        Cookie c = _seq[_i % _n];
        ++_i;
        return c;
    }

private:
    const Cookie *_seq;
    uint8_t _n;
    mutable uint8_t _i; // mutable: spawn() is const but must advance
};

static PhysicsBody ballAt(float x, float y, float r = 4.0f)
{
    PhysicsBody b;
    b.x = x;
    b.y = y;
    b.radius = r;
    return b;
}

/**
 * @test start() places the requested number of cookies and resets counters.
 */
test(cookie_start_placesVisibleCookiesAndResetsCounters)
{
    Cookie seq[] = {Cookie(100, 100, 3, true), Cookie(50, 50, 3, true), Cookie(200, 180, 3, true)};
    ScriptedSpawner spawner(seq, 3);
    CookieField field;

    field.start(3, 10, spawner, ballAt(120, 140));

    assertEqual((int)field.count(), 3);
    assertEqual((int)field.target(), 10);
    assertEqual((int)field.collected(), 0);
    assertEqual((int)field.remaining(), 10);
    assertFalse(field.finished());
    assertTrue(field.at(0).active);
    assertNear(field.at(0).x, 100.0f, 1e-4f);
}

/**
 * @test A ball far from every cookie collects nothing.
 */
test(cookie_checkPickup_missLeavesScoreUnchanged)
{
    Cookie seq[] = {Cookie(100, 100, 3, true)};
    ScriptedSpawner spawner(seq, 1);
    CookieField field;
    field.start(1, 5, spawner, ballAt(10, 10));

    uint8_t eaten = field.checkPickup(ballAt(10, 10)); // far from (100,100)

    assertEqual((int)eaten, 0);
    assertEqual((int)field.collected(), 0);
}

/**
 * @test On contact the score increases and the cookie reappears at the next
 * spawn position (the core of the respawn design).
 */
test(cookie_checkPickup_hitScoresAndRespawnsAtNextPosition)
{
    Cookie seq[] = {Cookie(100, 100, 3, true), Cookie(50, 50, 3, true)};
    ScriptedSpawner spawner(seq, 2);
    CookieField field;
    field.start(1, 10, spawner, ballAt(0, 0)); // cookie at (100,100)

    uint8_t eaten = field.checkPickup(ballAt(100, 100)); // sit on the cookie

    assertEqual((int)eaten, 1);
    assertEqual((int)field.collected(), 1);
    assertEqual((int)field.remaining(), 9);
    assertTrue(field.at(0).active);          // respawned, not gone
    assertNear(field.at(0).x, 50.0f, 1e-4f); // moved to seq[1]
    assertNear(field.at(0).y, 50.0f, 1e-4f);
}

/**
 * @test Pickup uses the combined radius: touching counts, just-missing does not.
 */
test(cookie_checkPickup_respectsCombinedRadius)
{
    // ball r=4 + cookie r=3 = 7. Center distance exactly 7 -> hit.
    Cookie hitSeq[] = {Cookie(107, 100, 3, true), Cookie(0, 0, 3, true)};
    ScriptedSpawner hitSpawner(hitSeq, 2);
    CookieField hitField;
    hitField.start(1, 10, hitSpawner, ballAt(0, 0));
    assertEqual((int)hitField.checkPickup(ballAt(100, 100)), 1);

    // distance 8 > 7 -> miss.
    Cookie missSeq[] = {Cookie(108, 100, 3, true), Cookie(0, 0, 3, true)};
    ScriptedSpawner missSpawner(missSeq, 2);
    CookieField missField;
    missField.start(1, 10, missSpawner, ballAt(0, 0));
    assertEqual((int)missField.checkPickup(ballAt(100, 100)), 0);
}

/**
 * @test The game is finished once the score reaches the target; the final
 * cookie vanishes instead of respawning.
 */
test(cookie_finished_whenTargetReached)
{
    Cookie seq[] = {Cookie(100, 100, 3, true), Cookie(50, 50, 3, true), Cookie(10, 10, 3, true)};
    ScriptedSpawner spawner(seq, 3);
    CookieField field;
    field.start(1, 2, spawner, ballAt(0, 0)); // target 2

    field.checkPickup(ballAt(100, 100)); // collected 1 -> respawn to (50,50)
    assertFalse(field.finished());

    field.checkPickup(ballAt(50, 50)); // collected 2 -> finished
    assertTrue(field.finished());
    assertEqual((int)field.remaining(), 0);
    assertFalse(field.at(0).active); // last one vanished
}

/**
 * @test A requested visible count above the capacity is clamped.
 */
test(cookie_start_clampsVisibleCountToCapacity)
{
    Cookie seq[] = {Cookie(1, 1, 3, true)};
    ScriptedSpawner spawner(seq, 1);
    CookieField field;

    field.start(10, 100, spawner, ballAt(0, 0)); // 10 > kMaxVisibleCookies

    assertEqual((int)field.count(), (int)kMaxVisibleCookies);
}
