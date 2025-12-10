/**
 * @file Dispatchers.common.cpp
 * @brief Implementation of Dispatchers (Common).
 */

#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/MainCoroutineDispatcher.hpp"
#include <mutex>
#include <thread>
#include <vector>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <iostream>

namespace kotlinx {
namespace coroutines {

namespace {

    // ------------------------------------------------------------------
    // Internal Helper Implementation: Simple ThreadPool Dispatcher
    // ------------------------------------------------------------------
    // In a real production app, this would be a sophisticated Scheduler.
    // For now, it's a basic thread pool to satisfy the 'Default' and 'IO' contract.

    class ThreadPoolDispatcher : public CoroutineDispatcher {
    public:
        ThreadPoolDispatcher(int threads, std::string name) : name(name), stop_flag(false) {
            for (int i = 0; i < threads; ++i) {
                workers.emplace_back([this] { worker_loop(); });
            }
        }

        ~ThreadPoolDispatcher() {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                stop_flag = true;
            }
            cv.notify_all();
            for (auto& t : workers) {
                if (t.joinable()) t.join();
            }
        }

        void dispatch(CoroutineContext* context, std::shared_ptr<Runnable> block) override {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                tasks.push(block);
            }
            cv.notify_one();
        }

        std::string to_string() const override {
            return "Dispatchers." + name;
        }

    private:
        void worker_loop() {
            while (true) {
                std::shared_ptr<Runnable> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    cv.wait(lock, [this] { return stop_flag || !tasks.empty(); });
                    
                    if (stop_flag && tasks.empty()) return;
                    
                    task = tasks.front();
                    tasks.pop();
                }
                
                try {
                    task->run();
                } catch (...) {
                    // Uncaught exception in dispatcher
                    // TODO: handle via CoroutineExceptionHandler
                }
            }
        }

        std::vector<std::thread> workers;
        std::queue<std::shared_ptr<Runnable>> tasks;
        std::mutex queue_mutex;
        std::condition_variable cv;
        bool stop_flag;
        std::string name;
    };

    // ------------------------------------------------------------------
    // Unconfined Dispatcher
    // ------------------------------------------------------------------
    class UnconfinedDispatcher : public CoroutineDispatcher {
    public:
        // Unconfined runs immediately in the current thread (no dispatch)
        void dispatch(CoroutineContext* context, std::shared_ptr<Runnable> block) override {
             // In pure C++ transliteration, 'Unconfined' is tricky because we can't always just run()
             // without risking stack overflow or re-entrancy issues. 
             // However, the contract is "executes in current thread".
             block->run(); 
        }

        bool is_dispatch_needed(CoroutineContext* context) override {
            return false;
        }
        
        std::string to_string() const override { return "Dispatchers.Unconfined"; }
    };

    // ------------------------------------------------------------------
    // MacOS/GCD Main Dispatcher Implementation
    // ------------------------------------------------------------------
    #include <dispatch/dispatch.h>

    class GcdMainDispatcher : public MainCoroutineDispatcher {
    public:
        void dispatch(CoroutineContext* context, std::shared_ptr<Runnable> block) override {
            dispatch_async(dispatch_get_main_queue(), ^{
                // Wrap weak ptr or strong ptr?
                // For now, capture shared_ptr by value to keep it alive during execution
                try {
                    block->run();
                } catch (...) {
                    // TODO: Global exception handler
                }
            });
        }
        
        std::string to_string() const override { return "Dispatchers.Main[GCD]"; }
        
        MainCoroutineDispatcher* get_immediate() override { 
             // GCD doesn't easily support "isCurrentThread" check without referencing specific key.
             // If we are on main thread, run immediately?
             if (pthread_main_np()) {
                 // We are on main thread. But we need a separate "Immediate" dispatcher object 
                 // to conform to the API which might return a different dispatcher.
                 // For simplified implementation, we can return 'this' but strict immediate dispatch 
                 // requires distinct logic in 'dispatch'.
                 return this;
             }
             return this;
        }
        
        // TODO: is_dispatch_needed logic using pthread_main_np()
        bool is_dispatch_needed(CoroutineContext* context) override {
            return !pthread_main_np();
        }
    };

} // anonymous namespace

// ------------------------------------------------------------------
// Dispatchers Singleton Accessors
// ------------------------------------------------------------------

CoroutineDispatcher& Dispatchers::get_default() {
    static ThreadPoolDispatcher instance(std::thread::hardware_concurrency(), "Default");
    return instance;
}

CoroutineDispatcher& Dispatchers::get_io() {
    // IO usually has more threads. 
    static ThreadPoolDispatcher instance(std::max((int)std::thread::hardware_concurrency() * 2, 64), "IO");
    return instance;
}

CoroutineDispatcher& Dispatchers::get_unconfined() {
    static UnconfinedDispatcher instance;
    return instance;
}

MainCoroutineDispatcher& Dispatchers::get_main() {
    static GcdMainDispatcher instance;
    return instance;
}

} // namespace coroutines
} // namespace kotlinx
