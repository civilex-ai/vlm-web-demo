// MemoryManager.cpp - Implementation of a custom memory pool allocator.
// Aims to reduce fragmentation and improve performance for fixed-size allocations.

#include <iostream>
#include <mutex>
#include <map>
#include <list>
#include "MemoryManager.h"

// A block header to store metadata for each allocation.
struct BlockHeader {
    size_t size;
    bool is_free;
    const char* tag; // For debugging
};

// Ensure header is aligned to the maximum alignment requirement.
constexpr size_t ALIGNMENT = alignof(std::max_align_t);

static_assert(sizeof(BlockHeader) % ALIGNMENT == 0, "BlockHeader size must be a multiple of alignment");

MemoryManager& MemoryManager::getInstance() {
    static MemoryManager instance;
    return instance;
}

void MemoryManager::initialize(size_t total_size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pool) {
        std::cerr << "Warning: MemoryManager already initialized." << std::endl;
        return;
    }
    
    m_pool_size = total_size;
    m_pool = ::operator new(m_pool_size);
    
    // Create the first free block
    BlockHeader* first_block = reinterpret_cast<BlockHeader*>(m_pool);
    first_block->size = m_pool_size - sizeof(BlockHeader);
    first_block->is_free = true;
    first_block->tag = "InitialPool";
    
    m_free_list.push_back(first_block);
    
    std::cout << "MemoryManager initialized with " << m_pool_size / (1024*1024) << "MB pool." << std::endl;
}

void* MemoryManager::allocate(size_t size, const char* tag) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_pool) {
        throw std::runtime_error("MemoryManager not initialized.");
    }
    
    // Align requested size
    size_t aligned_size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    
    // First-fit search for a free block
    for (auto it = m_free_list.begin(); it != m_free_list.end(); ++it) {
        BlockHeader* block = *it;
        if (block->is_free && block->size >= aligned_size) {
            // Found a suitable block, now see if we can split it
            size_t remaining_size = block->size - aligned_size;
            
            if (remaining_size > sizeof(BlockHeader) + ALIGNMENT) {
                // Split the block
                BlockHeader* new_block = reinterpret_cast<BlockHeader*>(
                    reinterpret_cast<char*>(block) + sizeof(BlockHeader) + aligned_size
                );
                new_block->size = remaining_size - sizeof(BlockHeader);
                new_block->is_free = true;
                new_block->tag = "SplitBlock";
                
                block->size = aligned_size;
                m_free_list.insert(std::next(it), new_block);
            }
            
            block->is_free = false;
            block->tag = tag;
            m_free_list.erase(it);
            
            // Return pointer to the data area, just after the header
            return reinterpret_cast<char*>(block) + sizeof(BlockHeader);
        }
    }
    
    // Out of memory
    std::cerr << "MemoryManager: Out of memory for allocation of " << size << " bytes." << std::endl;
    return nullptr;
}

void MemoryManager::deallocate(void* ptr, const char* tag) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Get the header from the pointer
    BlockHeader* block = reinterpret_cast<BlockHeader*>(
        reinterpret_cast<char*>(ptr) - sizeof(BlockHeader)
    );
    
    if (block->is_free) {
        std::cerr << "Warning: Double free detected for tag: " << (block->tag ? block->tag : "unknown") << std::endl;
        return;
    }
    
    block->is_free = true;
    block->tag = "FreedBlock";
    
    // Coalesce with adjacent free blocks (simplified logic)
    // In a real implementation, this would be more complex.
    m_free_list.push_back(block);
    m_free_list.sort([](const BlockHeader* a, const BlockHeader* b){
        return a < b;
    });
}

void MemoryManager::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_pool) {
        ::operator delete(m_pool);
        m_pool = nullptr;
        m_pool_size = 0;
        m_free_list.clear();
        std::cout << "MemoryManager pool released." << std::endl;
    }
}

MemoryManager::~MemoryManager() {
    if (m_pool) {
        shutdown();
    }
}
