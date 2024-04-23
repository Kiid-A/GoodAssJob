#include "Timestamp.h"
#include <chrono>
#include <inttypes.h>

static_assert(sizeof(Timestamp) == sizeof(int64_t),
    "Timestamp should be same size as int64_t");

std::string Timestamp::toString() const 
{   
    char buf[32] = {0};
    int64_t seconds = microSecondsSinceEpoch_ / microSecondsPerSecond;
    int64_t microSeconds = microSecondsSinceEpoch_ % microSecondsPerSecond;
    snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microSeconds);
    return buf;
}

std::string Timestamp::toFormattedString(bool showMicroSeconds) const
{
    char buf[64] = { 0 };
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / microSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (showMicroSeconds)
    {
        int microseconds = static_cast<int>(microSecondsSinceEpoch_ % microSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
            microseconds);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

Timestamp Timestamp::now()
{
    auto counts = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    return Timestamp(counts);
}