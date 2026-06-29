#include <iostream>

struct MemoryMetrics {
    size_t totalAllocated = 0;
    size_t totalFreed = 0;
    size_t currentUsage = 0;
    int allocationCount = 0;

    void PrintReport() {
        std::cout << "\n--- Memory Report ---" << std::endl;
        std::cout << "Total Allocated: " << totalAllocated << " bytes" << std::endl;
        std::cout << "Total Freed:     " << totalFreed << " bytes" << std::endl;
        std::cout << "Current Leaks:   " << currentUsage << " bytes" << std::endl;
        std::cout << "Alloc Count:     " << allocationCount << std::endl;
        if (currentUsage == 0) std::cout << "Result: CLEAN" << std::endl;
        else std::cout << "Result: MEMORY LEAK DETECTED!" << std::endl;
    }
};

static MemoryMetrics g_metrics;

void* operator new(size_t size) {
    g_metrics.totalAllocated += size;
    g_metrics.currentUsage += size;
    g_metrics.allocationCount++;

    // 실제 메모리 할당은 OS의 malloc에게 맡김
    void* ptr = malloc(size);
    // std::cout << "Allocating " << size << " bytes at " << ptr << std::endl;
    return ptr;
}

void operator delete(void* ptr, size_t size) noexcept {
    g_metrics.totalFreed += size;
    g_metrics.currentUsage -= size;
    g_metrics.allocationCount--;

    free(ptr);
}

void* operator new[](size_t size) {
    g_metrics.totalAllocated += size;
    g_metrics.currentUsage += size;
    g_metrics.allocationCount++;

    // 실제 메모리 할당은 OS의 malloc에게 맡김
    void* ptr = malloc(size);
    // std::cout << "Allocating " << size << " bytes at " << ptr << std::endl;
    return ptr;
}

void operator delete[](void* ptr, size_t size) noexcept {
    g_metrics.totalFreed += size;
    g_metrics.currentUsage -= size;
    g_metrics.allocationCount--;

    free(ptr);
}