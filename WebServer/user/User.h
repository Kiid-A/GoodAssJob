#pragma once

#include "../v11/include/Timestamp.h"
#include "../spot/Spot.h"
#include "../rcmd/Heap.h"
#include "../enum/enum.h"

#include <vector>
#include <functional>
#include <string.h>

class Spot;

class User
{   
public:
    using Preference = std::vector<double>; 
    using TimerCallBack = std::function<void(const Spot&)>;
    using ClickCallBack = std::function<void()>;
    using CloseCallBack = std::function<void()>;
    using ViewCallBack = std::function<void()>;

private:
    std::string id_;
    uint64_t seq_;

    double prefer_[sizeof(SpotType)];

    Timestamp viewTime_;
    Spot* nowViewing_;   

    TimerCallBack timerCallBack_;
    ClickCallBack clickCallBack_;
    CloseCallBack closeCallBack_;
    ViewCallBack viewCallBack_;

    void onClose();
    void onClick(Spot &spot);
    void onRate(RateLevel r);
    
public:
    User(uint64_t seq, const char* id);

    void setTimerCallBack(TimerCallBack cb) { timerCallBack_ = cb; } 
    void setClickCallBack(ClickCallBack cb) { clickCallBack_ = cb; }
    void setCloseCallBack(CloseCallBack cb) { closeCallBack_ = cb; }
    void setViewCallBack(ViewCallBack cb) { viewCallBack_ = cb; }

    Spot* getNowView() { return nowViewing_; }
    Timestamp getTimestamp() { return viewTime_; }
    double* getPrefer() { return this->prefer_; }

    // update after every freshing page is OK.
    void updatePrefer();
    // get TopK SpotTypes user like, it is top3 here ?

    void click(Spot &spot);
    void close();
    void rate(RateLevel r);

    int signIn(std::string& passwd);
    int signUp(std::string& passwd);
    int addNew(std::string& details);
    int changePasswd(std::string& passwd);
};