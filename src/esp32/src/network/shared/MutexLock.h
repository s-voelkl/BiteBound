#ifndef MUTEX_LOCK_H
#define MUTEX_LOCK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/**
 * @brief RAII helper for locking and automatic unlocking of a FreeRTOS Mutex.
 * This class ensures that the mutex is properly released when the object goes out of scope.
 */
class MutexLock
{
public:
    /**
     * @brief Constructs a MutexLock and attempts to acquire the given mutex.
     * @param mutex The FreeRTOS mutex handle to lock.
     * @param timeout The maximum time to wait for the mutex (default: portMAX_DELAY).
     * @return None. The lock state can be checked with `isLocked()`.
     */
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

    /**
     * @brief Destructor that releases the mutex if it was successfully acquired.
     * This ensures that the mutex is always released when the MutexLock object goes out of scope.
     */
    ~MutexLock()
    {
        if (m_locked && m_mutex != nullptr)
        {
            xSemaphoreGive(m_mutex);
        }
    }

    /**
     * @brief Checks if the mutex was successfully acquired.
     * @return true if the mutex is currently locked by this MutexLock instance, false otherwise.
     */
    bool isLocked() const { return m_locked; }

private:
    SemaphoreHandle_t m_mutex;
    bool m_locked;

    // Prevent copying and assignments
    MutexLock(const MutexLock &) = delete;
    MutexLock &operator=(const MutexLock &) = delete;
};

#endif // MUTEX_LOCK_H
