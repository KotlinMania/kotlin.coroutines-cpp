#pragma once
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <iostream>

namespace kotlinx {
namespace coroutines {
namespace internal {

class ThreadPoolDispatcher : public CoroutineDispatcher {
public:
    ThreadPoolDispatcher(int nThreads, std::string name = "ThreadPool") 
        : name(name), stop(false) 
    {
        for(int i = 0; i < nThreads; ++i) {
            workers.emplace_back([this] {
                for(;;) {
                    std::shared_ptr<Runnable> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    try {
                        task->run();
                    } catch (const std::exception& e) {
                        std::cerr << "Exception in worker thread: " << e.what() << std::endl;
                    } catch (...) {
                         std::cerr << "Unknown exception in worker thread" << std::endl;
                    }
                }
            });
        }
    }

    ~ThreadPoolDispatcher() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker: workers) {
            if (worker.joinable()) worker.join();
        }
    }

    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.push(block);
        }
        condition.notify_one();
    }
    
    // Support minimal limited parallelism by just returning this (naive) or new pool
    // For full support we'd need a wrapper that limits concurrency on top of this pool.
    
    std::string to_string() const override {
        return name;
    }

private:
    std::string name;
    mutable std::vector<std::thread> workers;
    mutable std::queue<std::shared_ptr<Runnable>> tasks;
    
    mutable std::mutex queue_mutex;
    mutable std::condition_variable condition;
    bool stop;
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
