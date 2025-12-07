/**
 * @file CoroutineDispatcher.cpp
 * @brief Implementation of CoroutineDispatcher and its helpers.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/CoroutineDispatcher.hpp`.
 *
 * This file contains the implementation of:
 * - `LimitedDispatcher` (internal helper for limited parallelism)
 * - `CoroutineDispatcher::limited_parallelism`
 * - `CoroutineDispatcher::intercept_continuation` logic
 */

#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/DispatchedContinuation.hpp"
#include <mutex>
#include <queue>
#include <atomic>
#include <iostream>

namespace kotlinx {
namespace coroutines {

// Internal LimitedDispatcher implementation
class LimitedDispatcher : public CoroutineDispatcher {
    std::shared_ptr<CoroutineDispatcher> dispatcher;
    int parallelism;
    std::string name;
    
    // Semantics:
    // We need to limit concurrent tasks.
    // Simplified implementation: internal queue + atomic counter.
    // Real implementation would be more complex lock-free.
    
    struct Task {
        std::shared_ptr<Runnable> block;
        CoroutineContext context; // copy for safety
    };
    
    mutable std::mutex lock;
    mutable std::queue<Task> queue;
    mutable int running_workers = 0;

public:
    LimitedDispatcher(std::shared_ptr<CoroutineDispatcher> dispatcher, int parallelism, std::string name)
        : dispatcher(dispatcher), parallelism(parallelism), name(name) {}

    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
        {
            std::lock_guard<std::mutex> g(lock);
            if (running_workers < parallelism) {
                running_workers++;
                // Dispatch directly to the underlying dispatcher
                // wrapper to decrement on completion
                auto worker = std::make_shared<Worker>(const_cast<LimitedDispatcher*>(this)->shared_from_this(), block);
                dispatcher->dispatch(context, worker);
                return;
            }
            // Queue it
            queue.push({block, context}); // This is copying context, might be expensive, but safe
        }
    }
    
    void on_worker_complete() {
        // Check queue for more work
        std::shared_ptr<Runnable> next_block;
        CoroutineContext next_context; // This is tricky, CoroutineContext is abstract base usually handled via shared_ptr
        // But here we stored it by value? No, context is likely implicitly handling shared data.
        // wait, CoroutineContext is a class. 
        
        {
            std::lock_guard<std::mutex> g(lock);
            if (queue.empty()) {
                running_workers--;
                return;
            }
            auto task = queue.front();
            queue.pop();
            next_block = task.block;
            // next_context = task.context; 
            // We need to dispatch this.
            
            // Re-dispatch:
            // running_workers stays same (we took one out, put one in)
            auto worker = std::make_shared<Worker>(shared_from_this(), next_block);
            dispatcher->dispatch(task.context, worker);
        }
    }
    
    struct Worker : public Runnable {
        std::shared_ptr<CoroutineDispatcher> parent; // actually LimitedDispatcher
        std::shared_ptr<Runnable> wrapped;
        
        Worker(std::shared_ptr<CoroutineDispatcher> parent, std::shared_ptr<Runnable> wrapped) 
            : parent(parent), wrapped(wrapped) {}
            
        void run() override {
            try {
                wrapped->run();
            } catch (...) {
                // handle
            }
            // Notify completion
            // We need to cast back to LimitedDispatcher
            auto limited = std::static_pointer_cast<LimitedDispatcher>(parent);
            limited->on_worker_complete();
        }
    };
    
    std::string to_string() const override {
        return name.empty() ? "LimitedDispatcher" : name;
    }
};

// CoroutineDispatcher implementation

// Template method intercept_continuation is in header

// Explicit instantiation for common types if needed, or keep in header if possible.
// But we defined it in header as template.

void CoroutineDispatcher::release_intercepted_continuation(std::shared_ptr<ContinuationBase> continuation) {
    auto dispatched = std::dynamic_pointer_cast<DispatchedContinuationBase>(continuation);
    if (dispatched) {
        dispatched->release();
    }
}

std::shared_ptr<CoroutineDispatcher> CoroutineDispatcher::limited_parallelism(int parallelism, const std::string& name) {
    if (parallelism <= 0) throw std::invalid_argument("Parallelism must be positive");
    return std::make_shared<LimitedDispatcher>(shared_from_this(), parallelism, name);
}

std::string CoroutineDispatcher::to_string() const {
    return "CoroutineDispatcher";
}

} // namespace coroutines
} // namespace kotlinx