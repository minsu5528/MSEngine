#include <iostream>
#include "Core/Memory/ObjectPool.h"

int main()
{
    ObjectPool<int> pool;
    int* p = pool.Allocate();
    *p = 42;
    std::cout << *p << std::endl;
    pool.Free(p);
    std::cout << pool.GetAvailableCount() << std::endl;

    return 0;
}