#pragma once
/**
 * @file StacklessCoroutine.hpp
 * @brief Stackless Coroutine Execution Engine
 *
 * This file provides the actual execution machinery for kotlinx.coroutines in C++.
 * It implements stackless coroutines using the Protothreads pattern (switch/case
 * state machines) combined with kcoro_arena's token kernel for zero-spin event
 * dispatch.
 *
 * ## Why Stackless?
 *
 * Kotlin's `suspend` keyword compiles to CPS (Continuation-Passing Style) - the
 * compiler transforms suspend functions into state machines. C++ doesn't have
 * this, so we provide it explicitly:
 *
 * 1. **Stackless** - No stack per coroutine; state lives in heap-allocated records
 * 2. **Zero-spin** - Token kernel for event-driven wake-up (no busy-wait)
 * 3. **Cooperative** - Explicit yield points via macros
 *
 * ## Integration with Kotlin Primitives
 *
 * ```
 * ┌─────────────────────────────────────────────────────────────┐
 * │  User Code                                                   │
 * │  launch(scope) { ... CO_AWAIT(deferred) ... }               │
 * └────────────┬────────────────────────────────────────────────┘
 *              │
 * ┌────────────▼────────────────────────────────────────────────┐
 * │  Kotlin Primitives (Job, Deferred, CancellableContinuation) │
 * │  - State machines, cancellation, parent-child hierarchy     │
 * └────────────┬────────────────────────────────────────────────┘
 *              │
 * ┌────────────▼────────────────────────────────────────────────┐
 * │  StacklessCoroutine (this file)                             │
 * │  - Execution engine: switch/case state machine              │
 * │  - Token kernel: zero-spin dispatch                         │
 * │  - Scheduler: ready queue management                        │
 * └─────────────────────────────────────────────────────────────┘
 * ```
 */

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>

#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/Result.hpp"

namespace kotlinx {
namespace coroutines {
namespace internal {

// ============================================================================
// Forward Declarations
// ============================================================================

class StacklessScheduler;
class StacklessCoroutineBase;
template<typename T> class StacklessCoroutine;

// ============================================================================
// Token Kernel - Zero-Spin Event Dispatch
// ============================================================================

/**
 * Token ID for pending operations.
 */
using TokenId = uint64_t;

/**
 * Payload for token callbacks.
 */
struct TokenPayload {
    void* ptr = nullptr;
    size_t len = 0;
    int status = 0;  // 0=OK, negative=error
};

/**
 * Resume callback signature.
 */
using TokenResumeFn = std::function<void(const TokenPayload&)>;

/**
 * Token registration for async operations.
 * When an operation completes, it posts to this token and the callback fires.
 */
struct Token {
    TokenId id;
    TokenResumeFn callback;
    void* user_ctx;
    std::atomic<bool> fired{false};
};

/**
 * Token Kernel - manages async operation tokens and callbacks.
 *
 * Thread-safe, zero-spin design:
 * - Registering a token is O(1) lock
 * - Posting completion is O(1) lock + condition signal
 * - Worker thread waits on condition (no spin)
 */
class TokenKernel {
public:
    static TokenKernel& instance() {
        static TokenKernel kernel;
        return kernel;
    }

    /**
     * Register a token for later callback.
     * Returns token ID for cancellation/completion.
     */
    TokenId register_token(TokenResumeFn callback, void* user_ctx = nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        TokenId id = next_id_++;
        tokens_[id] = std::make_unique<Token>(Token{id, std::move(callback), user_ctx});
        return id;
    }

    /**
     * Post completion to a token (fires callback on worker thread).
     */
    void post(TokenId id, TokenPayload payload) {
        std::unique_lock<std::mutex> lock(mutex_);
        auto it = tokens_.find(id);
        if (it == tokens_.end()) return;

        auto& token = it->second;
        bool expected = false;
        if (!token->fired.compare_exchange_strong(expected, true)) {
            return;  // Already fired
        }

        ready_queue_.push({id, payload});
        lock.unlock();
        cv_.notify_one();
    }

    /**
     * Cancel a token (fires callback with error status).
     */
    void cancel(TokenId id, int reason = -1) {
        post(id, TokenPayload{nullptr, 0, reason});
    }

    /**
     * Start the worker thread (call once at startup).
     */
    void start() {
        if (running_.exchange(true)) return;  // Already running
        worker_ = std::thread([this] { worker_loop(); });
    }

    /**
     * Stop the worker thread (call at shutdown).
     */
    void stop() {
        if (!running_.exchange(false)) return;
        cv_.notify_all();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    ~TokenKernel() {
        stop();
    }

private:
    TokenKernel() = default;

    void worker_loop() {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] {
                return !ready_queue_.empty() || !running_;
            });

            while (!ready_queue_.empty()) {
                auto [id, payload] = ready_queue_.front();
                ready_queue_.pop();

                auto it = tokens_.find(id);
                if (it != tokens_.end()) {
                    auto callback = std::move(it->second->callback);
                    tokens_.erase(it);
                    lock.unlock();

                    // Invoke callback outside lock
                    if (callback) {
                        callback(payload);
                    }

                    lock.lock();
                }
            }
        }
    }

    std::mutex mutex_;
    std::condition_variable cv_;
    std::unordered_map<TokenId, std::unique_ptr<Token>> tokens_;
    std::queue<std::pair<TokenId, TokenPayload>> ready_queue_;
    std::atomic<TokenId> next_id_{1};
    std::atomic<bool> running_{false};
    std::thread worker_;
};

// ============================================================================
// Stackless Coroutine Base
// ============================================================================

/**
 * Coroutine state enum (matches kcoro_arena).
 */
enum class CoroutineState {
    Created,      // Not yet started
    Running,      // Currently executing
    Suspended,    // Waiting for event
    Completed,    // Finished successfully
    Cancelled,    // Cancelled with exception
    Failed        // Failed with exception
};

/**
 * Base class for all stackless coroutines.
 * Contains the state machine and scheduler integration.
 */
class StacklessCoroutineBase : public std::enable_shared_from_this<StacklessCoroutineBase> {
public:
    using StepFn = std::function<void*(StacklessCoroutineBase*)>;

    StacklessCoroutineBase() = default;
    virtual ~StacklessCoroutineBase() = default;

    // State machine
    int line_state = 0;  // Current line number (resumption point)
    CoroutineState state = CoroutineState::Created;

    // Scheduler linkage
    std::shared_ptr<StacklessCoroutineBase> next_in_queue;
    bool in_ready_queue = false;

    // Job integration
    std::shared_ptr<Job> job;
    std::shared_ptr<CoroutineContext> context;

    // Token kernel integration
    TokenId pending_token = 0;
    TokenPayload last_payload;

    // Identity
    uint64_t id = 0;
    const char* name = nullptr;

    /**
     * Execute one step of the coroutine.
     * Returns nullptr if suspended, non-null if complete.
     */
    virtual void* step() = 0;

    /**
     * Check if coroutine is complete.
     */
    bool is_complete() const {
        return state == CoroutineState::Completed ||
               state == CoroutineState::Cancelled ||
               state == CoroutineState::Failed;
    }

    /**
     * Check if coroutine is cancelled.
     */
    bool is_cancelled() const {
        if (job) return job->is_cancelled();
        return state == CoroutineState::Cancelled;
    }

    /**
     * Cancel this coroutine.
     */
    void cancel(std::exception_ptr cause = nullptr) {
        if (job) job->cancel(cause);
        state = CoroutineState::Cancelled;
        if (pending_token != 0) {
            TokenKernel::instance().cancel(pending_token);
        }
    }

protected:
    /**
     * Park the coroutine waiting for a token.
     * Called by suspension macros.
     */
    void park_for_token(TokenId token) {
        pending_token = token;
        state = CoroutineState::Suspended;
        in_ready_queue = false;
    }

    /**
     * Resume from park with payload.
     * Called by token callback.
     */
    void resume_from_token(const TokenPayload& payload) {
        last_payload = payload;
        pending_token = 0;
        state = CoroutineState::Running;
        // Scheduler will re-enqueue us
    }
};

// ============================================================================
// Stackless Scheduler
// ============================================================================

/**
 * Scheduler for stackless coroutines.
 * Manages ready queue and executes coroutines cooperatively.
 */
class StacklessScheduler {
public:
    static StacklessScheduler& instance() {
        static StacklessScheduler sched;
        return sched;
    }

    /**
     * Enqueue a coroutine to the ready queue.
     */
    void enqueue(std::shared_ptr<StacklessCoroutineBase> coro) {
        if (!coro || coro->in_ready_queue) return;

        std::lock_guard<std::mutex> lock(mutex_);
        coro->in_ready_queue = true;
        ready_queue_.push(std::move(coro));
        cv_.notify_one();
    }

    /**
     * Run until all coroutines complete.
     * This is the main event loop.
     */
    void run() {
        while (true) {
            std::shared_ptr<StacklessCoroutineBase> coro;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (ready_queue_.empty()) {
                    if (active_count_ == 0) break;  // All done
                    cv_.wait(lock, [this] {
                        return !ready_queue_.empty() || active_count_ == 0;
                    });
                    if (ready_queue_.empty()) break;
                }
                coro = ready_queue_.front();
                ready_queue_.pop();
                coro->in_ready_queue = false;
            }

            // Execute one step
            coro->state = CoroutineState::Running;
            void* result = coro->step();

            if (result != nullptr) {
                // Coroutine completed
                coro->state = CoroutineState::Completed;
                decrement_active();
            } else if (coro->state == CoroutineState::Running) {
                // Yielded, re-enqueue
                enqueue(coro);
            }
            // If Suspended, token callback will re-enqueue
        }
    }

    /**
     * Run a single step (for integration with external event loops).
     * Returns true if work was done.
     */
    bool run_one() {
        std::shared_ptr<StacklessCoroutineBase> coro;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (ready_queue_.empty()) return false;
            coro = ready_queue_.front();
            ready_queue_.pop();
            coro->in_ready_queue = false;
        }

        coro->state = CoroutineState::Running;
        void* result = coro->step();

        if (result != nullptr) {
            coro->state = CoroutineState::Completed;
            decrement_active();
        } else if (coro->state == CoroutineState::Running) {
            enqueue(coro);
        }
        return true;
    }

    /**
     * Spawn a new coroutine.
     */
    void spawn(std::shared_ptr<StacklessCoroutineBase> coro) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ++active_count_;
            coro->id = ++next_id_;
        }
        enqueue(std::move(coro));
    }

    /**
     * Get active coroutine count.
     */
    size_t active_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return active_count_;
    }

private:
    void decrement_active() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (active_count_ > 0) --active_count_;
        if (active_count_ == 0) cv_.notify_all();
    }

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::shared_ptr<StacklessCoroutineBase>> ready_queue_;
    size_t active_count_ = 0;
    uint64_t next_id_ = 0;
};

// ============================================================================
// Typed Stackless Coroutine
// ============================================================================

/**
 * Typed stackless coroutine with result.
 */
template<typename T>
class StacklessCoroutine : public StacklessCoroutineBase {
public:
    using Body = std::function<void*(StacklessCoroutine<T>*)>;

    explicit StacklessCoroutine(Body body) : body_(std::move(body)) {}

    void* step() override {
        if (is_complete()) return (void*)1;
        return body_(this);
    }

    /**
     * Complete with a value.
     */
    void complete(T value) {
        result_ = Result<T>::success(std::move(value));
        state = CoroutineState::Completed;
    }

    /**
     * Complete with an exception.
     */
    void complete_exceptionally(std::exception_ptr ex) {
        result_ = Result<T>::failure(ex);
        state = CoroutineState::Failed;
    }

    /**
     * Get the result (blocks if not complete).
     */
    Result<T> get_result() const {
        return result_;
    }

    /**
     * Get the result value or throw.
     */
    T get_or_throw() const {
        return result_.get_or_throw();
    }

private:
    Body body_;
    Result<T> result_;
};

/**
 * Specialization for void.
 */
template<>
class StacklessCoroutine<void> : public StacklessCoroutineBase {
public:
    using Body = std::function<void*(StacklessCoroutine<void>*)>;

    explicit StacklessCoroutine(Body body) : body_(std::move(body)) {}

    void* step() override {
        if (is_complete()) return (void*)1;
        return body_(this);
    }

    void complete() {
        result_ = Result<void>::success();
        state = CoroutineState::Completed;
    }

    void complete_exceptionally(std::exception_ptr ex) {
        result_ = Result<void>::failure(ex);
        state = CoroutineState::Failed;
    }

    Result<void> get_result() const {
        return result_;
    }

private:
    Body body_;
    Result<void> result_;
};

// ============================================================================
// Coroutine Macros - Protothreads Style
// ============================================================================

/**
 * Begin a stackless coroutine body.
 * Use at the start of the body lambda.
 */
#define CO_BEGIN(coro) \
    switch ((coro)->line_state) { \
        case 0:

/**
 * End a stackless coroutine body.
 * Use at the end of the body lambda.
 */
#define CO_END(coro) \
    } \
    (coro)->line_state = 0; \
    return (void*)1;

/**
 * Yield to scheduler and resume later.
 * Other coroutines get a chance to run.
 */
#define CO_YIELD(coro) \
    do { \
        (coro)->line_state = __LINE__; \
        return nullptr; \
        case __LINE__:; \
    } while (0)

/**
 * Suspend until condition is true.
 * WARNING: This polls - use CO_AWAIT for event-driven waiting.
 */
#define CO_WAIT_UNTIL(coro, condition) \
    do { \
        (coro)->line_state = __LINE__; \
        case __LINE__: \
        if (!(condition)) return nullptr; \
    } while (0)

/**
 * Check for cancellation and exit if cancelled.
 */
#define CO_CHECK_CANCELLED(coro) \
    do { \
        if ((coro)->is_cancelled()) { \
            (coro)->state = kotlinx::coroutines::internal::CoroutineState::Cancelled; \
            return (void*)1; \
        } \
    } while (0)

/**
 * Await an async operation.
 * The operation should call the provided callback when complete.
 *
 * Usage:
 *   CO_AWAIT(coro, result, {
 *       // Start async operation
 *       start_async([callback](Value v) {
 *           callback(TokenPayload{&v, sizeof(v), 0});
 *       });
 *   });
 *   // result now contains the payload
 */
#define CO_AWAIT(coro, result_var, async_block) \
    do { \
        (coro)->line_state = __LINE__; \
        { \
            auto __coro_ptr = (coro)->shared_from_this(); \
            auto __token_id = kotlinx::coroutines::internal::TokenKernel::instance().register_token( \
                [__coro_ptr](const kotlinx::coroutines::internal::TokenPayload& p) { \
                    auto* __c = static_cast<std::decay_t<decltype(*(coro))>*>(__coro_ptr.get()); \
                    __c->last_payload = p; \
                    __c->pending_token = 0; \
                    __c->state = kotlinx::coroutines::internal::CoroutineState::Running; \
                    kotlinx::coroutines::internal::StacklessScheduler::instance().enqueue(__coro_ptr); \
                }); \
            (coro)->pending_token = __token_id; \
            auto callback = [__token_id](const kotlinx::coroutines::internal::TokenPayload& p) { \
                kotlinx::coroutines::internal::TokenKernel::instance().post(__token_id, p); \
            }; \
            async_block \
        } \
        (coro)->state = kotlinx::coroutines::internal::CoroutineState::Suspended; \
        return nullptr; \
        case __LINE__:; \
        result_var = (coro)->last_payload; \
    } while (0)

/**
 * Await a Deferred<T> value.
 *
 * Usage:
 *   T value;
 *   CO_AWAIT_DEFERRED(coro, deferred, value);
 */
#define CO_AWAIT_DEFERRED(coro, deferred, result_var) \
    do { \
        if ((deferred)->is_completed()) { \
            result_var = (deferred)->get_completed(); \
        } else { \
            (coro)->line_state = __LINE__; \
            { \
                auto __coro_ptr = (coro)->shared_from_this(); \
                auto __token_id = kotlinx::coroutines::internal::TokenKernel::instance().register_token( \
                    [__coro_ptr](const kotlinx::coroutines::internal::TokenPayload&) { \
                        kotlinx::coroutines::internal::StacklessScheduler::instance().enqueue(__coro_ptr); \
                    }); \
                (coro)->pending_token = __token_id; \
                (deferred)->invoke_on_completion([__token_id](std::exception_ptr) { \
                    kotlinx::coroutines::internal::TokenKernel::instance().post(__token_id, {}); \
                }); \
            } \
            (coro)->state = kotlinx::coroutines::internal::CoroutineState::Suspended; \
            return nullptr; \
            case __LINE__:; \
            result_var = (deferred)->get_completed(); \
        } \
    } while (0)

/**
 * Await a Job completion (like join()).
 */
#define CO_AWAIT_JOB(coro, job) \
    do { \
        if ((job)->is_completed()) { \
            /* Already done */ \
        } else { \
            (coro)->line_state = __LINE__; \
            { \
                auto __coro_ptr = (coro)->shared_from_this(); \
                auto __token_id = kotlinx::coroutines::internal::TokenKernel::instance().register_token( \
                    [__coro_ptr](const kotlinx::coroutines::internal::TokenPayload&) { \
                        kotlinx::coroutines::internal::StacklessScheduler::instance().enqueue(__coro_ptr); \
                    }); \
                (coro)->pending_token = __token_id; \
                (job)->invoke_on_completion([__token_id](std::exception_ptr) { \
                    kotlinx::coroutines::internal::TokenKernel::instance().post(__token_id, {}); \
                }); \
            } \
            (coro)->state = kotlinx::coroutines::internal::CoroutineState::Suspended; \
            return nullptr; \
            case __LINE__:; \
        } \
    } while (0)

/**
 * Delay for a duration (non-blocking).
 */
#define CO_DELAY(coro, milliseconds) \
    do { \
        (coro)->line_state = __LINE__; \
        { \
            auto __coro_ptr = (coro)->shared_from_this(); \
            auto __token_id = kotlinx::coroutines::internal::TokenKernel::instance().register_token( \
                [__coro_ptr](const kotlinx::coroutines::internal::TokenPayload&) { \
                    kotlinx::coroutines::internal::StacklessScheduler::instance().enqueue(__coro_ptr); \
                }); \
            (coro)->pending_token = __token_id; \
            std::thread([__token_id, ms = (milliseconds)]() { \
                std::this_thread::sleep_for(std::chrono::milliseconds(ms)); \
                kotlinx::coroutines::internal::TokenKernel::instance().post(__token_id, {}); \
            }).detach(); \
        } \
        (coro)->state = kotlinx::coroutines::internal::CoroutineState::Suspended; \
        return nullptr; \
        case __LINE__:; \
    } while (0)

// ============================================================================
// Initialization
// ============================================================================

/**
 * Initialize the coroutine runtime.
 * Call once at program startup.
 */
inline void init_runtime() {
    TokenKernel::instance().start();
}

/**
 * Shutdown the coroutine runtime.
 * Call at program exit.
 */
inline void shutdown_runtime() {
    TokenKernel::instance().stop();
}

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
