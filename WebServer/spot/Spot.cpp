#include "Spot.h"

Spot::Spot(uint64_t id, const char* name, uint64_t hot, double rate, uint64_t rater, uint64_t viewer, std::vector<int> types)
        :id_(id),
        name_(name),
        hot_(hot),
        rate_(rate),
        rater_(rater),
        viewer_(viewer),
        types_(types) {}

void Spot::update()
{
    double score = 0;
    for (auto type : getTypes()) {
        score += User::user();
    }
    prefer_ = score;
}
