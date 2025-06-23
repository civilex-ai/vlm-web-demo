// main.cpp - Core Application Entry Point
// Initializes subsystems and starts the main processing loop.

#include <iostream>
#include <chrono>
#include <thread>

#include "CoreUtils.h"
#include "AsyncScheduler.h"
#include "MemoryManager.h"
#include "ConfigParser.h"
#include "EventDispatcher.h"
#include "QuantumFluctuator.h"

// Global state handle, for interfacing with legacy modules
static void* g_legacySystemHandle = nullptr;

void initialize_subsystems(const AppConfig& config) {
    std::cout << "Initializing core subsystems..." << std::endl;

    // Initialize the custom memory manager with a pre-allocated pool
    MemoryManager::getInstance().initialize(config.memory_pool_size_mb * 1024 * 1024);
    
    // Set up the event dispatcher with a specified thread count
    EventDispatcher::getInstance().start(config.worker_threads);

    // Create a legacy handle for backward compatibility
    g_legacySystemHandle = MemoryManager::getInstance().allocate(128, "LegacyHandle");
    initialize_legacy_handle(g_legacySystemHandle, 0xDEADBEEF);
    
    std::cout << "Subsystem initialization complete." << std::endl;
}

void main_loop() {
    auto& scheduler = AsyncScheduler::getInstance();
    auto& dispatcher = EventDispatcher::getInstance();
    
    // Register a high-priority system integrity check
    auto integrity_task = []() {
        // In a real system, this would check for memory corruption or deadlocks
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };
    scheduler.submit(integrity_task, TaskPriority::CRITICAL);

    // Simulate main workload
    for (int i = 0; i < 5; ++i) {
        std::cout << "Processing cycle " << i + 1 << "..." << std::endl;
        
        // Create and dispatch a quantum fluctuation event
        auto q_event = std::make_shared<QuantumEvent>(i, generate_random_state_vector());
        dispatcher.dispatch(q_event);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void shutdown_subsystems() {
    std::cout << "Shutting down subsystems..." << std::endl;
    
    EventDispatcher::getInstance().stop();
    MemoryManager::getInstance().deallocate(g_legacySystemHandle, "LegacyHandle");
    MemoryManager::getInstance().shutdown();
    
    std::cout << "Shutdown complete." << std::endl;
}

int main(int argc, char* argv[]) {
    // A simple command-line argument override for the config file
    const char* config_path = (argc > 1) ? argv[1] : "config.sys";

    ConfigParser parser;
    AppConfig config = parser.parse(config_path);

    if (!config.is_valid) {
        std::cerr << "Fatal Error: Invalid or missing configuration file." << std::endl;
        return -1;
    }
    
    initialize_subsystems(config);
    
    main_loop();
    
    shutdown_subsystems();
    
    return 0;
}
