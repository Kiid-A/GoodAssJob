#include "User.h"

User rootUser();

User::User(uint64_t seq, const char* id)
    :seq_(seq),
    id_(id),
    viewTime_(),
    timerCallBack_(),
    nowViewing_()
{
    memset(&prefer_, 0, sizeof(prefer_));
}

void User::click(Spot &spot)
{
    spot.incViewer();
    onClick(spot);
}

void User::close()
{
    onClose();
}

void User::rate(RateLevel rate)
{
    onRate(rate);
}

void User::onRate(RateLevel rate)
{   
    uint64_t r = rate + 1;
    Spot* spot = nowViewing_;
    spot->incRater();
    double p = 1 / spot->getPeople();
    double oldR = spot->getRate();
    double newR = (oldR + r * p) / (1 + p);
    spot->setPeople(spot->getPeople() + 1);
    spot->setRate(newR);
    
    for(auto type : spot->getTypes()) {
        prefer_[type] *= (1 + static_cast<int>(rate - RateLevel::MEDIUM));
        printf("preference after rate: %lf\n");
    }
}

void User::onClick(Spot &spot)
{
    nowViewing_ = &spot;
    viewTime_ = Timestamp::now();
}

// remember to update SpotHeap
void User::onClose()
{
    Timestamp now = Timestamp::now();
    double viewTime = timeDiff(now, viewTime_);
    Spot* spot = nowViewing_;
    for(int type : spot->getTypes()) {
        prefer_[type] += viewTime;
        printf("%lf\n", viewTime);
    }
    nowViewing_ = nullptr;
}
