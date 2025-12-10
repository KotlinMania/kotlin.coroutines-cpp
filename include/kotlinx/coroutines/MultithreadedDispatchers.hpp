#pragma once
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include <string>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <atomic>

namespace kotlinx {
namespace coroutines {

class CloseableCoroutineDispatcher : public CoroutineDispatcher {
public:
    virtual void close() = 0;
};

// Artisan implementation of a ThreadPool
class ExecutorCoroutineDispatcherImpl : public CloseableCoroutineDispatcher {
    std::string name_;
    int nThreads_;
    std::vector<std::thread> workers_;
    std::deque<std::shared_ptr<Runnable>> taskQueue_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> closed_;

    void worker_loop() {
        while (true) {
            std::shared_ptr<Runnable> task;
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                condition_.wait(lock, [this] { return closed_ || !taskQueue_.empty(); });
                
                if (closed_ && taskQueue_.empty()) return;
                
                if (!taskQueue_.empty()) {
                    task = taskQueue_.front();
                    taskQueue_.pop_front();
                }
            }
            if (task) {
                try {
                    task->run();
                } catch (const std::exception& e) {
                    std::cerr << "Exception in worker thread: " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "Unknown exception in worker thread" << std::endl;
                }
            }
        }
    }

public:
    ExecutorCoroutineDispatcherImpl(int nThreads, std::string name) 
        : name_(name), nThreads_(nThreads), closed_(false) {
        for (int i = 0; i < nThreads; ++i) {
            workers_.emplace_back(&ExecutorCoroutineDispatcherImpl::worker_loop, this);
        }
    }

    ~ExecutorCoroutineDispatcherImpl() {
        close();
        for (auto& t : workers_) {
            if (t.joinable()) t.join();
        }
    }

    std::string to_string() const override {
        return name_;
    }

    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
        if (closed_) return; // Reject or thread?
        {
            // Mutable access needed for mutex/queue
            auto self = const_cast<ExecutorCoroutineDispatcherImpl*>(this);
            std::lock_guard<std::mutex> lock(self->queueMutex_);
            self->taskQueue_.push_back(block);
        }
        // Notify one worker
        auto self = const_cast<ExecutorCoroutineDispatcherImpl*>(this);
        self->condition_.notify_one();
    }

    void close() override {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            closed_ = true;
        }
        condition_.notify_all();
    }
};

inline CloseableCoroutineDispatcher* new_fixed_thread_pool_context(int n_threads, const std::string& name) {
    return new ExecutorCoroutineDispatcherImpl(n_threads, name);
}

} // namespace coroutines
} // namespace kotlinx
