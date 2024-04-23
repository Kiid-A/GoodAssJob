#include "Recommand.h"
#include "../spot/Spot.h"
#include "../user/User.h"

#include <unistd.h>
#include <iostream>

#define debug puts("========================================");
int main()
{
    // User user1(123, "xiaoming");
    // Spot spot1(001, "shan", 55, 4, 5, 32, {SpotType::ART, SpotType::EDUCATION});
    // user1.click(spot1);
    // sleep(5);
    // user1.rate(RateLevel::GOOD);
    // user1.close();
    // printf("spot1 got rate: %.2lf\n", spot1.getRate());
    ScoreList l = {
        {"001", 85.5}, {"002", 85.5}, {"003", 85.5}, {"004", 80.5}, {"005", 89.5},
        {"006", 8.5}, {"007", 95.5}, {"008", 42.5}, {"009", 87.5}, {"010", 88.5},
        {"011", 8.5}, {"012", 5.5}, {"013", 97.5}, {"014", 53.5}, {"015", 87.5},
        {"016", 8.5}, {"017", 75.5}, {"018", 65.5}, {"019", 4.5}, {"020", 86.5},
    };

    Heap A(5, l);
    A.printAll();

    User user((uint64_t)123, "KidA");

    Spot spot_1((uint64_t)001, "shan", 100.92, 4.63, 300, 1000, {SpotType::ART});
    spot_1.update();
}