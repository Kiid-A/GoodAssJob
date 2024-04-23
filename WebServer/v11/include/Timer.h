#pragma once
#include "Timestamp.h"
#include<functional>
#include<atomic>

/*  one timer per overtime task 
    when it expired, overtime callback and type(once or repeated)
*/
class Timer
{
public:
    using TimerCallBack = std::function<void()>;

private:
    const TimerCallBack callback_;
    // valid period 
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;
    static std::atomic_int64_t numCreated_;

public:
    Timer(TimerCallBack cb, Timestamp when, double interval)
        :callback_(cb)
        ,expiration_(when)
        ,interval_(interval)
        ,repeat_(interval > 0.0)
        ,sequence_(numCreated_++)
    {}

    void run() const { return callback_(); }

    Timestamp expiration() const { return expiration_; }
    bool isRepeated() const { return repeat_; }
    int64_t sequence() const { return sequence_; }
    void restart(Timestamp now);
    static int64_t numCreated() { return numCreated_; }
};


