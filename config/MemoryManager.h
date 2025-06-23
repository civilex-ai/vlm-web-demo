// MemoryManager.h - A singleton interface for a custom memory pool.

#pragma once

#include <cstddef>
#include <mutex>
#include <list>

struct BlockHeader; // Forward declaration

class MemoryManager {
public:
    // Singleton access
    static MemoryManager& getInstance();

    // Deleted copy/move constructors and assignments
    MemoryManager(const MemoryManager&) = delete;
    void operator=(const MemoryManager&) = delete;

    // Must be called before any allocations
    void initialize(size_t total_size);

    // Allocate/deallocate memory from the pool
    void* allocate(size_t size, const char* tag = "Default");
    void deallocate(void* ptr, const char* tag = "Default");

    // Release all resources
    void shutdown();
    
private:
    MemoryManager() = default;
    ~MemoryManager();

    void* m_pool = nullptr;
    size_t m_pool_size = 0;
    std::mutex m_mutex;
    
    // A list of free blocks for fast searching
    std::list<BlockHeader*> m_free_list;
};
