#pragma once
#include <iostream>
#include <array>

enum class ResourceType { Buffer, Shader, Texture, Count };

struct ResourceCounts {
    int created = 0;
    int destroyed = 0;
};

struct MemoryMetrics {
    size_t totalAllocated = 0;
    size_t totalFreed = 0;
    size_t currentUsage = 0;
    int allocationCount = 0;

    std::array<ResourceCounts, static_cast<size_t>(ResourceType::Count)> resourceCounters;

    size_t currentGPUMemory = 0;   // 지금 GPU에 올라가 있는 총 바이트
    size_t peakGPUMemory = 0;      // 지금까지 관측된 최댓값

    void OnResourceCreated(ResourceType type, size_t byteSize)
    {
        resourceCounters[static_cast<size_t>(type)].created++;
        currentGPUMemory += byteSize;
        if (currentGPUMemory > peakGPUMemory) peakGPUMemory = currentGPUMemory;
    }

    void OnResourceDestroyed(ResourceType type, size_t byteSize)
    {
        resourceCounters[static_cast<size_t>(type)].destroyed++;
        currentGPUMemory -= byteSize;
    }

    void PrintReport() {
        std::cout << "\n[ Memory Report ]" << std::endl;
        std::cout << "Total Allocated:      " << totalAllocated << " bytes" << std::endl;
        std::cout << "Total Freed:          " << totalFreed << " bytes" << std::endl;
        std::cout << "Current Leaks:        " << currentUsage << " bytes" << std::endl;
        std::cout << "Alloc Count:          " << allocationCount << std::endl;
        if (currentUsage == 0) std::cout << "Result: CLEAN" << std::endl;
        else std::cout << "Result: MEMORY LEAK DETECTED!" << std::endl;

        std::cout << "\n[Resource Report]" << std::endl;
        std::cout << "Buffers created:      " << resourceCounters[0].created << ", " << "destroyed: " << resourceCounters[0].destroyed << std::endl;
        std::cout << "Shaders created:      " << resourceCounters[1].created << ", " << "destroyed: " << resourceCounters[1].destroyed << std::endl;
        std::cout << "Textures created:     " << resourceCounters[2].created << ", " << "destroyed: " << resourceCounters[2].destroyed << std::endl;
        std::cout << "Peak GPU memory:      " << peakGPUMemory << std::endl;
        std::cout << "Buffers Outstandind:  " << resourceCounters[0].created - resourceCounters[0].destroyed << std::endl;
        std::cout << "Shaders Outstandind:  " << resourceCounters[1].created - resourceCounters[1].destroyed << std::endl;
        std::cout << "Textures Outstandind: " << resourceCounters[2].created - resourceCounters[2].destroyed << std::endl;
        
    }
};

inline MemoryMetrics g_metrics;

inline void* operator new(size_t size) {
    g_metrics.totalAllocated += size;
    g_metrics.currentUsage += size;
    g_metrics.allocationCount++;

    // 실제 메모리 할당은 OS의 malloc에게 맡김
    void* ptr = malloc(size);
    // std::cout << "Allocating " << size << " bytes at " << ptr << std::endl;
    return ptr;
}

inline void operator delete(void* ptr, size_t size) noexcept {
    g_metrics.totalFreed += size;
    g_metrics.currentUsage -= size;
    g_metrics.allocationCount--;

    free(ptr);
}

inline void* operator new[](size_t size) {
    g_metrics.totalAllocated += size;
    g_metrics.currentUsage += size;
    g_metrics.allocationCount++;

    // 실제 메모리 할당은 OS의 malloc에게 맡김
    void* ptr = malloc(size);
    // std::cout << "Allocating " << size << " bytes at " << ptr << std::endl;
    return ptr;
}

inline void operator delete[](void* ptr, size_t size) noexcept {
    g_metrics.totalFreed += size;
    g_metrics.currentUsage -= size;
    g_metrics.allocationCount--;

    free(ptr);
}