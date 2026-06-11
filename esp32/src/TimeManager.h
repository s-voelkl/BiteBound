#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <time.h>

/**
 * @brief Synchronises the MCU's internal clock with NTP and exposes the
 * current wall-clock time.
 *
 * An accurate clock is required for ISO 8601 timestamps on sensor data and
 * for validating the broker's TLS certificate during MQTT communication.
 * Configuration is supplied through the constructor so the helper can be
 * tested independently of the production configuration.
 */
class TimeManager
{
public:
    /**
     * @brief Constructs a TimeManager with the given NTP configuration.
     *
     * @param ntpServer1        Primary NTP server hostname.
     * @param ntpServer2        Secondary NTP server hostname (fallback).
     * @param gmtOffsetSec      Offset from UTC in seconds (0 for UTC).
     * @param daylightOffsetSec Daylight-saving offset in seconds (0 if unused).
     * @param epochThreshold    Minimum unix epoch value that is considered a valid synchronised time.
     *
     * @note String pointers must remain valid for the lifetime of the instance.
     */
    TimeManager(const char *ntpServer1,
                const char *ntpServer2,
                long gmtOffsetSec,
                int daylightOffsetSec,
                time_t epochThreshold);

    /**
     * @brief Performs an NTP synchronisation and blocks until the system clock
     * holds a plausible wall-clock time.
     *
     * @warning A stable WiFi connection is required. The call blocks
     * indefinitely if no NTP response is ever received.
     */
    void sync();

    /**
     * @brief Returns the current time as a Unix timestamp.
     *
     * @return Seconds since the Unix epoch.
     */
    time_t now() const;

    /**
     * @brief Indicates whether the system clock currently holds a synchronised time.
     *
     * @return true if the clock has been advanced past the configured threshold,
     * false otherwise.
     */
    bool isSynchronized() const;

private:
    const char *_ntpServer1;
    const char *_ntpServer2;
    long _gmtOffsetSec;
    int _daylightOffsetSec;
    time_t _epochThreshold;
};

/**
 * @brief Shared TimeManager instance configured from config.h.
 */
extern TimeManager timeManager;

#endif // TIME_MANAGER_H
