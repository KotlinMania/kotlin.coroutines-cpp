#pragma once
/**
 * @file StacklessBuilders.hpp
 * @brief Working Coroutine Builders using Stackless Execution
 *
 * This file provides fully functional coroutine builders that use the stackless
 * execution engine. These work standalone without Kotlin - pure C++ coroutines
 * using Kotlin's algorithms (Job hierarchy, cancellation, structured concurrency).
 *
 * ## Quick Start
 *
 * ```cpp
 * #include <kotlinx/coroutines/StacklessBuilders.hpp>
 * using namespace kotlinx::coroutines;
 *
 * int main() {
 *     // Initialize runtime
 *     stackless::init();
 *
 *     // Launch a coroutine
 *     auto job = stackless::launch([](auto* coro) {
 *         CO_BEGIN(coro);
 *
 *         std::cout << "Hello ";
 *         CO_DELAY(coro, 100);  // Non-blocking delay
 *         std::cout << "World!" << std::endl;
 *
 *         CO_END(coro);
 *     });
 *
 *     // Run until all coroutines complete
 *     stackless::run();
 *
 *     // Shutdown
 *     stackless::shutdown();
 *     return 0;
 * }
 * ```
 *
 * ## Comparison to Kotlin
 *
 * | Kotlin | C++ Stackless |
 * |--------|---------------|
 * | `launch { ... }` | `stackless::launch([](auto* c) { CO_BEGIN(c); ... CO_END(c); })` |
 * | `async { value }` | `stackless::async<T>([](auto* c) { ... c->complete(value); ... })` |
 * | `delay(100)` | `CO_DELAY(c, 100)` |
 * | `job.join()` | `CO_AWAIT_JOB(c, job)` |
 * | `deferred.await()` | `CO_AWAIT_DEFERRED(c, deferred, result)` |
 * | `yield()` | `CO_YIELD(c)` |
 * | `runBlocking { }` | `stackless::run_blocking([](auto* c) { ... })` |
 *
 * ## Key Differences from Standard Builders.hpp
 *
 * 1. **Actually executes** - Not stubs, real working coroutines
 * 2. **Stackless** - No stack per coroutine, ~100 bytes per coroutine
 * 3. **Explicit yield points** - Must use CO_* macros for suspension
 * 4. **State in structs** - Local variables must be stored explicitly
 */

#include "kotlinx/coroutines/internal/StacklessCoroutine.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace kotlinx {
namespace coroutines {
namespace stackless {

// Re-export types from internal namespace
using internal::StacklessCoroutine;
using internal::StacklessCoroutineBase;
using internal::StacklessScheduler;
using internal::TokenKernel;
using internal::TokenPayload;
using internal::CoroutineState;

// Use parent namespace types explicitly
using ::kotlinx::coroutines::Job;
using ::kotlinx::coroutines::Deferred;
using ::kotlinx::coroutines::ChildJob;
using ::kotlinx::coroutines::DisposableHandle;
using ::kotlinx::coroutines::NoOpDisposableHandle;
using ::kotlinx::coroutines::CoroutineContext;

// ============================================================================
// Runtime Initialization
// ============================================================================

/**
 * Initialize the stackless coroutine runtime.
 * Call once at program startup.
 */
inline void init() {
    internal::init_runtime();
}

/**
 * Shutdown the stackless coroutine runtime.
 * Call at program exit.
 */
inline void shutdown() {
    internal::shutdown_runtime();
}

// ============================================================================
// Stackless Job - Wraps StacklessCoroutine as a Job
// ============================================================================

/**
 * Job implementation backed by a stackless coroutine.
 */
class StacklessJob : public Job, public std::enable_shared_from_this<StacklessJob> {
public:
    explicit StacklessJob(std::shared_ptr<StacklessCoroutineBase> coro)
        : coro_(std::move(coro)) {
        if (coro_) {
            coro_->job = std::static_pointer_cast<Job>(shared_from_this());
        }
    }

    // Job interface
    bool is_active() const override {
        return coro_ && !coro_->is_complete() && !coro_->is_cancelled();
    }

    bool is_completed() const override {
        return coro_ && coro_->is_complete();
    }

    bool is_cancelled() const override {
        return coro_ && coro_->is_cancelled();
    }

    std::exception_ptr get_cancellation_exception() override {
        return cancel_cause_;
    }

    bool start() override {
        if (coro_ && coro_->state == CoroutineState::Created) {
            StacklessScheduler::instance().spawn(coro_);
            return true;
        }
        return false;
    }

    void cancel(std::exception_ptr cause) override {
        cancel_cause_ = cause;
        if (coro_) {
            coro_->cancel(cause);
        }
        // Invoke handlers
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& handler : completion_handlers_) {
            try { handler(cause); } catch (...) {}
        }
        completion_handlers_.clear();
    }

    std::shared_ptr<Job> get_parent() const override {
        return parent_;
    }

    std::vector<std::shared_ptr<Job>> get_children() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return children_;
    }

    std::shared_ptr<DisposableHandle> attach_child(std::shared_ptr<ChildJob> child) override {
        std::lock_guard<std::mutex> lock(mutex_);
        children_.push_back(child);
        return std::make_shared<NoOpDisposableHandle>();
    }

    void join() override {
        // Blocking join - wait for completion
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return is_completed() || is_cancelled(); });
    }

    std::shared_ptr<DisposableHandle> invoke_on_completion(
        std::function<void(std::exception_ptr)> handler
    ) override {
        return invoke_on_completion(false, false, std::move(handler));
    }

    std::shared_ptr<DisposableHandle> invoke_on_completion(
        bool /*on_cancelling*/,
        bool invoke_immediately,
        std::function<void(std::exception_ptr)> handler
    ) override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!is_completed() && !is_cancelled()) {
                completion_handlers_.push_back(handler);
                return std::make_shared<NoOpDisposableHandle>();
            }
        }
        // Already complete
        if (invoke_immediately) {
            handler(cancel_cause_);
        }
        return std::make_shared<NoOpDisposableHandle>();
    }

    CoroutineContext::Key* key() const override {
        static CoroutineContext::Key job_key;
        return &job_key;
    }

    // Called when coroutine completes
    void on_complete() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& handler : completion_handlers_) {
            try { handler(nullptr); } catch (...) {}
        }
        completion_handlers_.clear();
        cv_.notify_all();
    }

    void set_parent(std::shared_ptr<Job> parent) {
        parent_ = std::move(parent);
    }

private:
    std::shared_ptr<StacklessCoroutineBase> coro_;
    std::shared_ptr<Job> parent_;
    std::vector<std::shared_ptr<Job>> children_;
    std::vector<std::function<void(std::exception_ptr)>> completion_handlers_;
    std::exception_ptr cancel_cause_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

// ============================================================================
// Stackless Deferred - Async with result
// ============================================================================

/**
 * Deferred implementation backed by a stackless coroutine.
 */
template<typename T>
class StacklessDeferred : public Deferred<T>, public std::enable_shared_from_this<StacklessDeferred<T>> {
public:
    using CoroType = StacklessCoroutine<T>;

    explicit StacklessDeferred(std::shared_ptr<CoroType> coro)
        : coro_(std::move(coro)) {}

    // Deferred interface
    T get_completed() const override {
        if (coro_) {
            return coro_->get_or_throw();
        }
        throw std::runtime_error("Deferred not completed");
    }

    std::exception_ptr get_completion_exception_or_null() const override {
        if (coro_ && coro_->get_result().is_failure()) {
            return coro_->get_result().exception_or_null();
        }
        return nullptr;
    }

    T await() override {
        // Blocking await
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return coro_ && coro_->is_complete(); });
        return get_completed();
    }

    // Job interface
    bool is_active() const override {
        return coro_ && !coro_->is_complete() && !coro_->is_cancelled();
    }

    bool is_completed() const override {
        return coro_ && coro_->is_complete();
    }

    bool is_cancelled() const override {
        return coro_ && coro_->is_cancelled();
    }

    std::exception_ptr get_cancellation_exception() override {
        return cancel_cause_;
    }

    bool start() override {
        if (coro_ && coro_->state == CoroutineState::Created) {
            auto base_coro = std::static_pointer_cast<StacklessCoroutineBase>(coro_);
            StacklessScheduler::instance().spawn(base_coro);
            return true;
        }
        return false;
    }

    void cancel(std::exception_ptr cause) override {
        cancel_cause_ = cause;
        if (coro_) {
            coro_->cancel(cause);
        }
    }

    std::shared_ptr<Job> get_parent() const override { return nullptr; }
    std::vector<std::shared_ptr<Job>> get_children() const override { return {}; }
    std::shared_ptr<DisposableHandle> attach_child(std::shared_ptr<ChildJob>) override {
        return std::make_shared<NoOpDisposableHandle>();
    }

    void join() override {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return is_completed() || is_cancelled(); });
    }

    std::shared_ptr<DisposableHandle> invoke_on_completion(
        std::function<void(std::exception_ptr)> handler
    ) override {
        return invoke_on_completion(false, false, std::move(handler));
    }

    std::shared_ptr<DisposableHandle> invoke_on_completion(
        bool, bool invoke_immediately,
        std::function<void(std::exception_ptr)> handler
    ) override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!is_completed() && !is_cancelled()) {
                completion_handlers_.push_back(handler);
                return std::make_shared<NoOpDisposableHandle>();
            }
        }
        if (invoke_immediately) {
            handler(cancel_cause_);
        }
        return std::make_shared<NoOpDisposableHandle>();
    }

    CoroutineContext::Key* key() const override {
        static CoroutineContext::Key key;
        return &key;
    }

    void on_complete() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& handler : completion_handlers_) {
            try { handler(nullptr); } catch (...) {}
        }
        completion_handlers_.clear();
        cv_.notify_all();
    }

private:
    std::shared_ptr<CoroType> coro_;
    std::vector<std::function<void(std::exception_ptr)>> completion_handlers_;
    std::exception_ptr cancel_cause_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

// ============================================================================
// Builder Functions
// ============================================================================

/**
 * Launch a coroutine that returns Unit (void).
 *
 * Usage:
 * ```cpp
 * auto job = stackless::launch([](auto* coro) {
 *     CO_BEGIN(coro);
 *     // ... coroutine body ...
 *     CO_END(coro);
 * });
 * ```
 *
 * @param body The coroutine body function
 * @return Job handle for cancellation and joining
 */
inline std::shared_ptr<Job> launch(
    std::function<void*(StacklessCoroutine<void>*)> body
) {
    auto coro = std::make_shared<StacklessCoroutine<void>>(std::move(body));

    // Cast to base for scheduler
    auto base_coro = std::static_pointer_cast<StacklessCoroutineBase>(coro);
    auto job = std::make_shared<StacklessJob>(base_coro);

    StacklessScheduler::instance().spawn(base_coro);
    return job;
}

/**
 * Launch a coroutine that computes a result.
 *
 * Usage:
 * ```cpp
 * auto deferred = stackless::async<int>([](auto* coro) {
 *     CO_BEGIN(coro);
 *     // ... compute value ...
 *     coro->complete(42);
 *     CO_END(coro);
 * });
 *
 * int result = deferred->await();  // Blocking
 * // OR use CO_AWAIT_DEFERRED in another coroutine
 * ```
 *
 * @tparam T The result type
 * @param body The coroutine body function
 * @return Deferred<T> handle for awaiting the result
 */
template<typename T>
std::shared_ptr<Deferred<T>> async(
    std::function<void*(StacklessCoroutine<T>*)> body
) {
    auto coro = std::make_shared<StacklessCoroutine<T>>(std::move(body));
    auto deferred = std::make_shared<StacklessDeferred<T>>(coro);

    // Cast to base for scheduler
    auto base_coro = std::static_pointer_cast<StacklessCoroutineBase>(coro);
    StacklessScheduler::instance().spawn(base_coro);
    return deferred;
}

/**
 * Run coroutines until all complete.
 * This is the main event loop.
 *
 * Usage:
 * ```cpp
 * stackless::launch(...);
 * stackless::launch(...);
 * stackless::run();  // Blocks until both complete
 * ```
 */
inline void run() {
    StacklessScheduler::instance().run();
}

/**
 * Run a single step of the scheduler.
 * Useful for integration with external event loops.
 *
 * @return true if work was done, false if queue was empty
 */
inline bool run_one() {
    return StacklessScheduler::instance().run_one();
}

/**
 * Run a coroutine and block until it completes.
 *
 * Usage:
 * ```cpp
 * int result = stackless::run_blocking<int>([](auto* coro) {
 *     CO_BEGIN(coro);
 *     coro->complete(42);
 *     CO_END(coro);
 * });
 * ```
 *
 * @tparam T The result type
 * @param body The coroutine body
 * @return The computed result
 */
template<typename T>
T run_blocking(std::function<void*(StacklessCoroutine<T>*)> body) {
    auto coro = std::make_shared<StacklessCoroutine<T>>(std::move(body));
    auto deferred = std::make_shared<StacklessDeferred<T>>(coro);

    // Cast to base and spawn
    auto base_coro = std::static_pointer_cast<StacklessCoroutineBase>(coro);
    StacklessScheduler::instance().spawn(base_coro);

    // Run scheduler until this coroutine completes
    while (!coro->is_complete()) {
        if (!StacklessScheduler::instance().run_one()) {
            // No work available, but coroutine not complete
            // This could happen if waiting for external events
            std::this_thread::yield();
        }
    }

    return coro->get_or_throw();
}

/**
 * run_blocking specialization for void.
 */
inline void run_blocking(std::function<void*(StacklessCoroutine<void>*)> body) {
    auto coro = std::make_shared<StacklessCoroutine<void>>(std::move(body));

    // Cast to base and spawn
    auto base_coro = std::static_pointer_cast<StacklessCoroutineBase>(coro);
    StacklessScheduler::instance().spawn(base_coro);

    while (!coro->is_complete()) {
        if (!StacklessScheduler::instance().run_one()) {
            std::this_thread::yield();
        }
    }
}

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Get the current number of active coroutines.
 */
inline size_t active_count() {
    return StacklessScheduler::instance().active_count();
}

/**
 * Create a simple scope for structured concurrency.
 * All coroutines launched within the scope are children.
 */
class CoroutineScope {
public:
    ~CoroutineScope() {
        // Cancel all children on scope exit
        for (auto& child : children_) {
            if (child && child->is_active()) {
                child->cancel();
            }
        }
    }

    std::shared_ptr<Job> launch(std::function<void*(StacklessCoroutine<void>*)> body) {
        auto job = stackless::launch(std::move(body));
        children_.push_back(job);
        return job;
    }

    template<typename T>
    std::shared_ptr<Deferred<T>> async(std::function<void*(StacklessCoroutine<T>*)> body) {
        auto deferred = stackless::async<T>(std::move(body));
        children_.push_back(deferred);
        return deferred;
    }

    void cancel_all() {
        for (auto& child : children_) {
            if (child) child->cancel();
        }
    }

private:
    std::vector<std::shared_ptr<Job>> children_;
};

} // namespace stackless
} // namespace coroutines
} // namespace kotlinx
