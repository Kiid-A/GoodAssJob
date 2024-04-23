#include "Timer.h"

std::atomic_int64_t Timer::numCreated_;

void Timer::restart(Timestamp now)
{
    if(repeat_) {
        expiration_ = addTime(now, interval_);
    } else {
        expiration_ = Timestamp::invalid();
    }
}

