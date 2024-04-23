#pragma once

#include "Recommand.h"
#include "../user/User.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

class User;
class Spot;

/*  Mainly worked upon topK
    passive rcmder for specific user.
    init after connection, and update Spot & Heap on rate/click
    sort spot according to user's topK preference
    calc fomula: hot * rate * SUM(TopK types)
    btw, frequent flushing is annoying. so we only update TopK after fresh page
*/

class Heap
{
public:
    using pair = std::pair<std::string, double>;
    using ScoreList = std::vector<std::pair<std::string, double>>;

private:
    uint64_t size_;     // 1 ~ k
    ScoreList heapNodes_;

public:
    Heap(uint64_t size, ScoreList list);

    ~Heap()
    {
        ScoreList().swap(heapNodes_);
    }   

    void push(pair node);
    void pop();
    pair top() { return heapNodes_[1]; }

    void adjust();
    
    void printAll();

    // stream user they may like
    void filter(User user);

};



