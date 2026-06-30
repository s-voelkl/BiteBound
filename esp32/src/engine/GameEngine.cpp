#include "GameEngine.h"

void GameEngine::begin(int playWidth, int playHeight)
{
    _playW = playWidth;
    _playH = playHeight;
    if (_board == nullptr)
    {
        _board = new uint8_t[(size_t)playWidth * playHeight];
    }
    // Both games share one play board; only the active game writes it.
    _game1.init(_board, playWidth, playHeight, _config.wallThicknessPx);
    _game2.init(_board, playWidth, playHeight, _config.wallThicknessPx);
    _status = RunningStatus::IDLE;
    _state.runningStatus = RunningStatus::IDLE;
}

void GameEngine::applyConfig(const GameConfig &cfg)
{
    _config = cfg;
    _physics.setParams(cfg.physics);
}

void GameEngine::chooseGameMode(uint8_t gameId)
{
    _active = (gameId == 1) ? static_cast<IGame *>(&_game1)
                            : static_cast<IGame *>(&_game2);
    _config.gameId = gameId;
    _physics.reset();        // drop carried-over input filter state
    _active->start(_config); // game resets its own round to 1 + builds the level
    afterBuild();
}

void GameEngine::nextRound()
{
    if (_active == nullptr)
    {
        return;
    }
    _physics.reset();
    _active->nextRound(_config); // game bumps its own round counter + rebuilds
    afterBuild();
}

void GameEngine::afterBuild()
{
    _elapsedSec = 0.0f;
    _status = RunningStatus::RUNNING;
    _needsFullRedraw = true; // freshly built level -> renderer repaints everything

    _state.runningStatus = RunningStatus::RUNNING;
    _state.currentRound = (int)_active->round();
    _state.cookiesCollected = (int)_active->cookies().collected();
    _state.cookiesRemaining = (int)_active->cookies().remaining();
    _state.elapsedTimeSec = 0.0f;
}

void GameEngine::update(float tiltX, float tiltY, float dt)
{
    if (_status != RunningStatus::RUNNING || _active == nullptr || dt <= 0.0f)
    {
        return;
    }

    const Vec2 accel = _physics.inputAccel(tiltX, tiltY);
    _physics.step(_active->body(), accel, dt, _active->collider());
    _active->onResolved(); // collect overlapped cookies
    _elapsedSec += dt;

    _state.cookiesCollected = (int)_active->cookies().collected();
    _state.cookiesRemaining = (int)_active->cookies().remaining();
    _state.currentRound = (int)_active->round();
    _state.elapsedTimeSec = _elapsedSec;

    if (_active->finished())
    {
        _status = RunningStatus::COMPLETED;
        _state.runningStatus = RunningStatus::COMPLETED;
    }
}

bool GameEngine::consumeRedraw()
{
    const bool needed = _needsFullRedraw;
    _needsFullRedraw = false;
    return needed;
}
