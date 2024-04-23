#pragma once

#include "Timestamp.h"
#include "../spot/Spot.h"
#include "../user/User.h"
#include "Heap.h"

#include <vector>
#include <functional>
#include <string.h>

/*  we hope the "quit" operation will pass a cb, which help us calc time 
    recommand TopK spots 
*/

class Spot;
class User;
using pair = std::pair<std::string, double>;
using ScoreList = std::vector<pair>;     // id + score

class Recommand
{
public:
    using SpotList = std::vector<Spot>; 
    
private:
    void updateUser();   

public:
    Recommand();
    ~Recommand();

    // set a callback on connection to sort spots, rcmd topK spots with prefer type
    SpotList passiveFilter(User user);
    // select SpotTypes and sort algorithm
    SpotList activeFilter();
};


