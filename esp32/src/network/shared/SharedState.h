#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/**
 * @brief Enum representing the decoded command type.
 */
enum class CommandType
{
    START,
    STOP,
    PARAM_CHANGE,
    UNKNOWN
};

/**
 * @brief Representation of an incoming dashboard command.
 */
struct CommandMsg
{
    CommandType type;
    char playerName[32];
    int gameId;
    int cookiesCount;
    int wallThicknessPx;
    float imuSensitivity;
    float bounceRestitution;
    float emaAlpha;
    float deadzoneThreshold;
    char requestId[36];
};

/**
 * @brief Synchronized game state and configurations accessed by both CPU cores.
 */
struct SharedStateData
{
    // Identity & parameters
    char playerName[32];
    int gameId;
    int cookiesCount;
    int wallThicknessPx;

    // Engine parameter configs
    float imuSensitivity;
    float bounceRestitution;
    float emaAlpha;
    float deadzoneThreshold;

    // Simulation outputs
    float ballPosX;
    float ballPosY;
    float velocityX;
    float velocityY;
    float accX;
    float accY;
    int cookiesCollected;
    int cookiesRemaining;
    int currentRound;
    float elapsedTimeSec;
    bool isRunning;
};

/**
 * @brief Initializer to safely set system defaults on start.
 */
void initSharedState(SharedStateData &state);

/**
 * @brief RAII helper for locking and automatic unlocking of a FreeRTOS Mutex.
 */
class MutexLock
{
public:
    explicit MutexLock(SemaphoreHandle_t mutex, TickType_t timeout = portMAX_DELAY)
        : m_mutex(mutex), m_locked(false)
    {
        if (m_mutex != nullptr)
        {
            if (xSemaphoreTake(m_mutex, timeout) == pdTRUE)
            {
                m_locked = true;
            }
        }
    }

    ~MutexLock()
    {
        if (m_locked && m_mutex != nullptr)
        {
            xSemaphoreGive(m_mutex);
        }
    }

    bool isLocked() const { return m_locked; }

private:
    SemaphoreHandle_t m_mutex;
    bool m_locked;

    // Prevent copying and assignments
    MutexLock(const MutexLock &) = delete;
    MutexLock &operator=(const MutexLock &) = delete;
};

#endif // SHARED_STATE_H