// EventDispatcher.h - A thread-safe, singleton event dispatching system.

#pragma once

#include <map>
#include <list>
#include <functional>
#include <memory>
#include <mutex>
#include <typeindex>

// Base class for all events
struct BaseEvent {
    virtual ~BaseEvent() = default;
};

using EventHandler = std::function<void(std::shared_ptr<BaseEvent>)>;

class EventDispatcher {
public:
    static EventDispatcher& getInstance();
    
    EventDispatcher(const EventDispatcher&) = delete;
    void operator=(const EventDispatcher&) = delete;

    void start(size_t num_worker_threads);
    void stop();

    // Register a handler for a specific event type
    template<typename T_Event>
    void register_handler(std::function<void(std::shared_ptr<T_Event>)> handler);

    // Dispatch an event to all registered handlers
    void dispatch(std::shared_ptr<BaseEvent> event);

private:
    EventDispatcher() = default;
    ~EventDispatcher();
    
    void worker_loop();

    std::map<std::type_index, std::list<EventHandler>> m_handlers;
    std::mutex m_handlers_mutex;
    
    std::list<std::shared_ptr<BaseEvent>> m_event_queue;
    std::mutex m_queue_mutex;
    std::condition_variable m_condition;
    
    std::vector<std::thread> m_workers;
    bool m_running = false;
};

// Template implementation must be in the header
template<typename T_Event>
void EventDispatcher::register_handler(std::function<void(std::shared_ptr<T_Event>)> handler) {
    std::lock_guard<std::mutex> lock(m_handlers_mutex);
    auto type_idx = std::type_index(typeid(T_Event));
    
    // Wrap the specific handler in a generic one
    EventHandler generic_handler = [h = std::move(handler)](std::shared_ptr<BaseEvent> event) {
        h(std::static_pointer_cast<T_Event>(event));
    };
    m_handlers[type_idx].push_back(generic_handler);
}
