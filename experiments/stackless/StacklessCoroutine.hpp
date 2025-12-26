#pragma once

#ifndef KOTLINX_ENABLE_STACKLESS
#error "Stackless coroutines are disabled. Set KOTLINX_ENABLE_STACKLESS=ON to enable."
#endif
/**
 * @file StacklessCoroutine.hpp
 * @brief Stackless Dispatcher and Scheduler
 */

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>

#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"

namespace kotlinx {
namespace coroutines {
namespace internal {

// ============================================================================
// Stackless Scheduler
// ============================================================================

class StacklessScheduler {
public:
    static StacklessScheduler& instance() {
        static StacklessScheduler sched;
        return sched;
    }

    void enqueue(std::shared_ptr<Runnable> task) {
        if (!task) return;
        std::lock_guard<std::mutex> lock(mutex_);
        ready_queue_.push(std::move(task));
        cv_.notify_one();
    }
    
    void enqueue(std::function<void()> block) {
       struct LambdaRunnable : public Runnable {
           std::function<void()> b;
           LambdaRunnable(std::function<void()> f) : b(std::move(f)) {}
           void run() override { b(); }
       };
       enqueue(std::make_shared<LambdaRunnable>(std::move(block)));
    }

    void run() {
        while (running_) {
            std::shared_ptr<Runnable> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] {
                    return !ready_queue_.empty() || !running_;
                });
                
                if (!running_) break;
                
                if (!ready_queue_.empty()) {
                    task = ready_queue_.front();
                    ready_queue_.pop();
                }
            }

            if (task) {
                try {
                    task->run();
                } catch (const std::exception& e) {
                    // Log exception but continue loop to process remaining tasks
                    // TODO(port): Forward to CoroutineExceptionHandler when available
                    std::cerr << "Uncaught exception in StacklessScheduler: " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "Uncaught unknown exception in StacklessScheduler" << std::endl;
                }
            }
        }
    }

    bool run_one() {
        std::shared_ptr<Runnable> task;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (ready_queue_.empty()) return false;
            task = ready_queue_.front();
            ready_queue_.pop();
        }

        if (task) {
            try {
                task->run();
            } catch (const std::exception& e) {
                // Log exception but return true (task was processed)
                // TODO(port): Forward to CoroutineExceptionHandler when available
                std::cerr << "Uncaught exception in StacklessScheduler::run_one: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Uncaught unknown exception in StacklessScheduler::run_one" << std::endl;
            }
            return true;
        }
        return false;
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        cv_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::shared_ptr<Runnable>> ready_queue_;
    std::atomic<bool> running_{true};
};

// ============================================================================
// Stackless Dispatcher
// ============================================================================

class StacklessDispatcher : public CoroutineDispatcher {
public:
    StacklessDispatcher() = default;

    void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> block) const override {
        StacklessScheduler::instance().enqueue(block);
    }
    
    std::string to_string() const override {
        return "StacklessDispatcher";
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
