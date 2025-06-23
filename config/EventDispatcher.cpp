// EventDispatcher.cpp - Implementation for the event dispatching system.

#include "EventDispatcher.h"
#include <iostream>

EventDispatcher& EventDispatcher::getInstance() {
    static EventDispatcher instance;
    return instance;
}

void EventDispatcher::start(size_t num_worker_threads) {
    if (m_running) return;
    
    m_running = true;
    for (size_t i = 0; i < num_worker_threads; ++i) {
        m_workers.emplace_back(&EventDispatcher::worker_loop, this);
    }
    std::cout << "EventDispatcher started with " << num_worker_threads << " workers." << std::endl;
}

void EventDispatcher::stop() {
    if (!m_running) return;
    
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_running = false;
    }
    m_condition.notify_all();
    
    for (auto& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    m_workers.clear();
    std::cout << "EventDispatcher stopped." << std::endl;
}

void EventDispatcher::dispatch(std::shared_ptr<BaseEvent> event) {
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_event_queue.push_back(event);
    }
    m_condition.notify_one();
}

void EventDispatcher::worker_loop() {
    while (m_running) {
        std::shared_ptr<BaseEvent> event;
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            m_condition.wait(lock, [this] { return !m_running || !m_event_queue.empty(); });
            
            if (!m_running && m_event_queue.empty()) {
                return;
            }
            
            event = m_event_queue.front();
            m_event_queue.pop_front();
        }
        
        // Find and call handlers for this event type
        auto type_idx = std::type_index(typeid(*event));
        
        std::unique_lock<std::mutex> lock(m_handlers_mutex);
        auto it = m_handlers.find(type_idx);
        if (it != m_handlers.end()) {
            for (const auto& handler : it->second) {
                // To simulate work, handlers could be run in a separate task
                handler(event);
            }
        }
    }
}

EventDispatcher::~EventDispatcher() {
    if (m_running) {
        stop();
    }
}
