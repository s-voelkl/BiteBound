#include "TimeManager.h"
#include "../../../config.h"

/**
 * @brief Shared TimeManager instance configured from config.h.
 */
TimeManager timeManager(ntp_server_1,
                        ntp_server_2,
                        static_cast<long>(gmt_offset_sec),
                        static_cast<int>(daylight_offset_sec),
                        static_cast<time_t>(time_epoch_2020_Jan_1));

TimeManager::TimeManager(const char *ntpServer1,
                         const char *ntpServer2,
                         long gmtOffsetSec,
                         int daylightOffsetSec,
                         time_t epochThreshold)
    : _ntpServer1(ntpServer1),
      _ntpServer2(ntpServer2),
      _gmtOffsetSec(gmtOffsetSec),
      _daylightOffsetSec(daylightOffsetSec),
      _epochThreshold(epochThreshold)
{
}

void TimeManager::sync()
{
    Serial.print("Synchronizing time...");

    // Configure the NTP client with two servers for redundancy.
    configTime(_gmtOffsetSec, _daylightOffsetSec, _ntpServer1, _ntpServer2);

    // Block until the system clock advances past the configured plausibility
    // threshold (e.g. 2020-01-01), which indicates a successful NTP response.
    time_t current = time(nullptr);
    while (current < _epochThreshold)
    {
        delay(500);
        Serial.print(".");
        current = time(nullptr);
    }
    Serial.println("\nTime synchronized.");
}

time_t TimeManager::now() const
{
    return time(nullptr);
}

bool TimeManager::isSynchronized() const
{
    return time(nullptr) >= _epochThreshold;
}
