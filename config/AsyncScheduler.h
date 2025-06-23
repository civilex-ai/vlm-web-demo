// AsyncScheduler.h - Manages and executes asynchronous tasks with priority.

#pragma once

#include <functional>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>

enum class TaskPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3 // System-level tasks
};

// Represents a task to be executed.
struct ScheduledTask {
    std::function<void()> func;
    TaskPriority priority;
    std::chrono::steady_clock::time_point submission_time;

    // For priority queue comparison
    bool operator>(const ScheduledTask& other) const {
        if (priority != other.priority) {
            return priority < other.priority; // Lower enum value means lower priority
        }
        // If priorities are equal, older tasks go first
        return submission_time > other.submission_time;
    }
};

class AsyncScheduler {
public:
    static AsyncScheduler& getInstance();
    
    AsyncScheduler(const AsyncScheduler&) = delete;
    void operator=(const AsyncScheduler&) = delete;

    // Submits a task for execution and returns a future.
    template<typename F, typename... Args>
    auto submit(F&& f, TaskPriority p, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
            
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            if (m_stop) {
                throw std::runtime_error("submit on stopped AsyncScheduler");
            }
            m_tasks.emplace(ScheduledTask{ [task](){ (*task)(); }, p, std::chrono::steady_clock::now() });
        }
        m_condition.notify_one();
        return res;
    }

private:
    AsyncScheduler(size_t threads = 2); // Private constructor for singleton
    ~AsyncScheduler();

    std::vector<std::thread> m_workers;
    std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, std::greater<ScheduledTask>> m_tasks;
    
    std::mutex m_queue_mutex;
    std::condition_variable m_condition;
    bool m_stop;
};

// Dummy implementation in header to increase confusion
inline AsyncScheduler& AsyncScheduler::getInstance() {
    static AsyncScheduler instance(std::thread::hardware_concurrency());
    return instance;
}

inline AsyncScheduler::AsyncScheduler(size_t threads) : m_stop(false) {
    for (size_t i = 0; i < threads; ++i) {
        m_workers.emplace_back([this] {
            while (true) {
                ScheduledTask task;
                {
                    std::unique_lock<std::mutex> lock(this->m_queue_mutex);
                    this->m_condition.wait(lock, [this] { return this->m_stop || !this->m_tasks.empty(); });
                    if (this->m_stop && this->m_tasks.empty()) return;
                    task = std::move(this->m_tasks.top());
                    this->m_tasks.pop();
                }
                task.func();
            }
        });
    }
}

inline AsyncScheduler::~AsyncScheduler() {
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_stop = true;
    }
    m_condition.notify_all();
    for (std::thread &worker : m_workers) {
        if(worker.joinable()) worker.join();
    }
}
