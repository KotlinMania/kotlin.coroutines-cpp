/**
 * @file Dispatchers.common.cpp
 * @brief Implementation of Dispatchers (Common).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Dispatchers.common.kt
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

#if __APPLE__
#include <dispatch/dispatch.h>
#include <pthread.h>
#endif

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
        ThreadPoolDispatcher(int threads, std::string name) : name_(name), stop_flag_(false) {
            for (int i = 0; i < threads; ++i) {
                workers_.emplace_back([this] { worker_loop(); });
            }
        }

        ~ThreadPoolDispatcher() {
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                stop_flag_ = true;
            }
            cv_.notify_all();
            for (auto& t : workers_) {
                if (t.joinable()) t.join();
            }
        }

        void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
            auto* self = const_cast<ThreadPoolDispatcher*>(this);
            {
                std::unique_lock<std::mutex> lock(self->queue_mutex_);
                self->tasks_.push(block);
            }
            self->cv_.notify_one();
        }

        std::string to_string() const override {
            return "Dispatchers." + name_;
        }

    private:
        void worker_loop() {
            while (true) {
                std::shared_ptr<Runnable> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    cv_.wait(lock, [this] { return stop_flag_ || !tasks_.empty(); });

                    if (stop_flag_ && tasks_.empty()) return;

                    task = tasks_.front();
                    tasks_.pop();
                }

                try {
                    task->run();
                } catch (...) {
                    // Uncaught exception in dispatcher
                    // TODO: handle via CoroutineExceptionHandler
                }
            }
        }

        std::vector<std::thread> workers_;
        mutable std::queue<std::shared_ptr<Runnable>> tasks_;
        mutable std::mutex queue_mutex_;
        mutable std::condition_variable cv_;
        bool stop_flag_;
        std::string name_;
    };

    // ------------------------------------------------------------------
    // Unconfined Dispatcher
    // ------------------------------------------------------------------
    class UnconfinedDispatcher : public CoroutineDispatcher {
    public:
        // Unconfined runs immediately in the current thread (no dispatch)
        void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
             // In pure C++ transliteration, 'Unconfined' is tricky because we can't always just run()
             // without risking stack overflow or re-entrancy issues.
             // However, the contract is "executes in current thread".
             block->run();
        }

        bool is_dispatch_needed(const CoroutineContext& context) const override {
            return false;
        }

        std::string to_string() const override { return "Dispatchers.Unconfined"; }
    };

#if __APPLE__
    // ------------------------------------------------------------------
    // MacOS/GCD Main Dispatcher Implementation
    // ------------------------------------------------------------------
    class GcdMainDispatcher : public MainCoroutineDispatcher {
    public:
        void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
            dispatch_async(dispatch_get_main_queue(), ^{
                try {
                    block->run();
                } catch (...) {
                    // TODO: Global exception handler
                }
            });
        }

        std::string to_string() const override { return "Dispatchers.Main[GCD]"; }

        MainCoroutineDispatcher& get_immediate() override {
             // GCD doesn't easily support "isCurrentThread" check without referencing specific key.
             // For simplified implementation, we return *this.
             return *this;
        }

        bool is_dispatch_needed(const CoroutineContext& context) const override {
            return pthread_main_np() == 0;
        }
    };
#else
    // Stub for non-Apple platforms
    class StubMainDispatcher : public MainCoroutineDispatcher {
    public:
        void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
            // No main dispatcher available - just run in current thread
            block->run();
        }

        std::string to_string() const override { return "Dispatchers.Main[Stub]"; }

        MainCoroutineDispatcher& get_immediate() override {
             return *this;
        }
    };
#endif

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
#if __APPLE__
    static GcdMainDispatcher instance;
#else
    static StubMainDispatcher instance;
#endif
    return instance;
}

} // namespace coroutines
} // namespace kotlinx
