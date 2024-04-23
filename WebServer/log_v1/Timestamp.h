#pragma once
#include <string>

/*  Timestamp
    Encapsulation for a timestamp   
    time memo & comparison
*/
class Timestamp
{
private:
    int64_t microSecondsSinceEpoch_;

public:
    static const int microSecondsPerSecond = 1000 * 1000; 

    Timestamp()
        :microSecondsSinceEpoch_(0)
    {}

    explicit Timestamp(int64_t microSecondsSinceEpoch)
        :microSecondsSinceEpoch_(microSecondsSinceEpoch)
    {}

    // @return a timestamp with present time
    static Timestamp now();
    // @return an empty Timestamp
    static Timestamp invalid()
    {
        return Timestamp();
    }

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    std::string toString() const;
    std::string toFormattedString(bool showMicroSeconds = true) const;
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
};

inline bool operator<(Timestamp l, Timestamp r)
{
    return l.microSecondsSinceEpoch() < r.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp l, Timestamp r)
{
    return l.microSecondsSinceEpoch() == r.microSecondsSinceEpoch();
}

inline double timeDiff(Timestamp h, Timestamp l)
{   
    int64_t diff = h.microSecondsSinceEpoch() - l.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::microSecondsPerSecond;
}

inline Timestamp addTime(Timestamp t, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::microSecondsPerSecond);
    return Timestamp(t.microSecondsSinceEpoch() + delta);
}


