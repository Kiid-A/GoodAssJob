#pragma once

#include <stdio.h>
#include <string>
#include <vector>

#include "../user/User.h"
#include "../enum/enum.h"

class Spot
{
private:
    uint64_t id_;               // can be string
    std::string name_;
    // hot is similar to viewer + rater...
    uint64_t hot_;
    double rate_;
    uint64_t rater_;
    uint64_t viewer_;          
    std::vector<int> types_;    // spot type
    double prefer_;             // user's score

    /* inner maps for future use */

public:
    Spot();

    Spot(uint64_t id, const char* name, uint64_t hot, double rate, 
         uint64_t rater, uint64_t viewer, std::vector<int> types);


    void setPeople(uint64_t p) { rater_ = p; }
    uint64_t getPeople() { return rater_; }

    void setRate(double r) { rate_ = r; }
    // %.2lf is good
    double getRate() { return rate_; }
    
    uint64_t getHot() const { return rater_ + viewer_; }

    std::vector<int> getTypes() { return types_; }

    // they shall be atomic. However, it is a stand-alone version :D
    void update();
    void incViewer() { viewer_++; }
    void incRater() { rater_++; }
};