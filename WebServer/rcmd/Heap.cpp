#include "Heap.h"

Heap::Heap(size_t size, ScoreList list)
    :size_(size),
    heapNodes_(size + 1)
{
    for (int i = 1; i <= size; ++i) {
        heapNodes_[i] = list[i - 1];
    }
    adjust();

    for (size_t i = size; i < list.size(); ++i) {
        push(list[i]);
    }
}

void Heap::adjust()
{
    for (size_t j = size_; j > 1; --j) {
        size_t i = j;
        while (heapNodes_[i].second < heapNodes_[i >> 1].second && i > 1) {
            std::swap(heapNodes_[i], heapNodes_[i >> 1]);
            i >>= 1;
        }
    }
}

void Heap::push(pair node)
{
    if (node.second > heapNodes_[1].second) {
        std::swap(node, heapNodes_[1]);
        adjust();
    }
}

void Heap::printAll()
{
    for (size_t i = 1; i <= size_; ++i) {
        printf("spot: %s rate: %.3f ", heapNodes_[i].first.c_str(), heapNodes_[i].second);
    }
    puts("");
}

