#include "ObjectPool.h"
#include <iostream>
#include <chrono>


int main()
{
    // ���� Objectpool ����
    Objectpool<int> objectPool;
    auto startObjectPool = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000000; ++i)
    {
        int* obj = objectPool.New();
        objectPool.clear(obj);
    }

    auto endObjectPool = std::chrono::high_resolution_clock::now();
    auto durationObjectPool = std::chrono::duration_cast<std::chrono::milliseconds>(endObjectPool - startObjectPool);
    std::cout << "Objectpool performance: " << durationObjectPool.count() << " milliseconds" << std::endl;

    // ���� malloc ����
    auto startMalloc = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000000; ++i)
    {
        int* obj = (int*)malloc(sizeof(int));
        free(obj);
    }

    auto endMalloc = std::chrono::high_resolution_clock::now();
    auto durationMalloc = std::chrono::duration_cast<std::chrono::milliseconds>(endMalloc - startMalloc);
    std::cout << "Malloc performance: " << durationMalloc.count() << " milliseconds" << std::endl;

    return 0;
}
