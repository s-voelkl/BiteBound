#include <AUnit.h>
#include "src/engine/GameEngine.h"

static const int kPW = 60;
static const int kPH = 60;

// True when the ball's center sits on a free corridor pixel.
static bool ballOnCorridor(const GameEngine &eng)
{
    int px = (int)eng.ball().x;
    int py = (int)eng.ball().y;
    return eng.board()[py * eng.playWidth() + px] == 0;
}

/**
 * @test Choosing Game 1 starts a running round 1, builds a maze, and places the
 * ball and all cookies on corridor cells; the round triggers a full redraw.
 */
test(GameEngineTest, chooseGame1BuildsMazeRoundOne)
{
    GameEngine eng;
    eng.begin(kPW, kPH);
    eng.chooseGameMode(1);

    assertEqual((int)eng.activeGameId(), 1);
    assertEqual((int)eng.round(), 1);
    assertTrue(eng.isRunning());
    assertEqual(eng.state().currentRound, 1);
    assertMore((int)eng.cookies().count(), 0);
    assertTrue(ballOnCorridor(eng));
    for (uint8_t i = 0; i < eng.cookies().count(); ++i)
    {
        const Cookie &c = eng.cookies().at(i);
        assertEqual((int)eng.board()[(int)c.y * eng.playWidth() + (int)c.x], 0);
    }
    assertTrue(eng.consumeRedraw());        // first read sees the redraw...
    assertFalse(eng.consumeRedraw());       // ...and it clears.
}

/**
 * @test nextRound() increments the round, regenerates the level, and keeps the
 * ball on a corridor (Game 1 gets a fresh maze each round).
 */
test(GameEngineTest, nextRoundRegeneratesGame1)
{
    GameEngine eng;
    eng.begin(kPW, kPH);
    eng.chooseGameMode(1);

    eng.nextRound();
    assertEqual((int)eng.round(), 2);
    assertTrue(eng.isRunning());
    assertTrue(eng.consumeRedraw());
    assertTrue(ballOnCorridor(eng));
}

/**
 * @test Choosing Game 2 resets the round to 1, opens the board (all free), and
 * centers the ball; nextRound() still advances the counter for Game 2.
 */
test(GameEngineTest, game2OpensBoardAndCountsRounds)
{
    GameEngine eng;
    eng.begin(kPW, kPH);
    eng.chooseGameMode(2);

    assertEqual((int)eng.activeGameId(), 2);
    assertEqual((int)eng.round(), 1);
    assertNear(eng.ball().x, kPW * 0.5f, 1e-3f);
    assertNear(eng.ball().y, kPH * 0.5f, 1e-3f);

    bool allFree = true;
    for (int i = 0; i < eng.playWidth() * eng.playHeight(); ++i)
    {
        if (eng.board()[i] != 0)
        {
            allFree = false;
            break;
        }
    }
    assertTrue(allFree);

    eng.nextRound();
    assertEqual((int)eng.round(), 2); // Game 2 counts rounds too
}

/**
 * @test Each game owns its round counter and selecting a mode resets it to 1,
 * so changing (or re-selecting) the mode always restarts at round 1.
 */
test(GameEngineTest, modeChangeResetsRoundCounter)
{
    GameEngine eng;
    eng.begin(kPW, kPH);

    eng.chooseGameMode(1);
    eng.nextRound();
    eng.nextRound();
    assertEqual((int)eng.round(), 3); // Game 1 at round 3

    eng.chooseGameMode(2);
    assertEqual((int)eng.round(), 1); // switching modes resets to 1

    eng.nextRound();
    assertEqual((int)eng.round(), 2); // Game 2 advances independently

    eng.chooseGameMode(1);
    assertEqual((int)eng.round(), 1); // back to Game 1 -> reset again
}

/**
 * @test The ball never leaves the play area under sustained tilt.
 */
test(GameEngineTest, ballStaysInBounds)
{
    GameEngine eng;
    eng.begin(kPW, kPH);
    eng.chooseGameMode(1);

    for (int s = 0; s < 300; ++s)
    {
        eng.update(0.5f, -0.4f, 0.02f);
        assertMoreOrEqual(eng.ball().x, 0.0f);
        assertLessOrEqual(eng.ball().x, (float)kPW);
        assertMoreOrEqual(eng.ball().y, 0.0f);
        assertLessOrEqual(eng.ball().y, (float)kPH);
    }
}
