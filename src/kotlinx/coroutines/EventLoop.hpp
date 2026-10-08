#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/EventLoop.common.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt */
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/internal/DispatchedTask.hpp"
#include <deque>
#include <memory>
#include <atomic>
#include <climits>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace kotlinx {
namespace coroutines {

// DispatchedTask is defined as a template in internal/DispatchedTask.hpp
// Use SchedulerTask for type-erased storage in queues

/**
 * Extended by \ref CoroutineDispatcher implementations that have event loop inside and can
 * be asked to process next event from their event queue.
 *
 * It may optionally implement \ref Delay interface and support time-scheduled tasks.
 * It is created or pigged back onto (see \ref ThreadLocalEventLoop)
 * by `run_blocking` and by \ref Dispatchers::get_unconfined.
 *
 * **This an internal API and should not be used from general code.**
 */
// Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:19-120
struct EventLoop : CoroutineDispatcher {

    virtual ~EventLoop() = default;

    /**
     * Processes next event in this event loop.
     *
     * The result of this function is to be interpreted like this:
     * - `<= 0` -- there are potentially more events for immediate processing;
     * - `> 0` -- a number of nanoseconds to wait for next scheduled event;
     * - `LLONG_MAX` -- no more events.
     *
     * **NOTE**: Must be invoked only from the event loop's thread
     *          (no check for performance reasons, may be added in the future).
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:49-52
    virtual long long process_next_event();
    virtual bool is_empty() const;
    virtual long long next_time() const;

    bool process_unconfined_event();
    /**
     * Returns `true` if the invoking `run_blocking(context, block)` that was passed this event loop in its context
     * parameter should call \ref process_next_event for this event loop (otherwise, it will process thread-local one).
     * By default, event loop implementation is thread-local and should not processed in the context
     * (current thread's event loop should be processed instead).
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:74-74
    virtual bool should_be_processed_from_context() const;
    /**
     * Dispatches task whose dispatcher returned `false` from \ref CoroutineDispatcher::is_dispatch_needed
     * into the current event loop.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:80-84
    void dispatch_unconfined(std::shared_ptr<SchedulerTask> task);

    bool is_active() const;
    bool is_unconfined_loop_active() const;
    bool is_unconfined_queue_empty() const;

    void increment_use_count(bool unconfined = false);
    void decrement_use_count(bool unconfined = false);
    virtual void shutdown();

    // CoroutineDispatcher overrides
    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override;

private:
    /**
     * Counts the number of nested `run_blocking` and \ref Dispatchers::get_unconfined that use this event loop.
     */
    long long use_count_ = 0;
    /**
     * Set to true on any use by `run_blocking`, because it potentially leaks this loop to other threads, so
     * this instance must be properly shutdown. We don't need to shutdown event loop that was used solely
     * by \ref Dispatchers::get_unconfined -- it can be left as \ref ThreadLocalEventLoop and reused next time.
     */
    bool shared_ = false;
    /**
     * Queue used by \ref Dispatchers::get_unconfined tasks.
     * These tasks are thread-local for performance and take precedence over the rest of the queue.
     */
    // NOTE(port): A nullable owned deque projects the source's lazily allocated queue.
    std::unique_ptr<std::deque<std::shared_ptr<SchedulerTask>>> unconfined_queue_;
    // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:96-97
    static long long delta(bool unconfined);
};

// Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:122-138
struct ThreadLocalEventLoop {
    static std::shared_ptr<EventLoop> get_event_loop();
    static std::shared_ptr<EventLoop> current_or_null();
    static void reset_event_loop();
    static void set_event_loop(std::shared_ptr<EventLoop> event_loop);

private:
    // NOTE(port): The existing shared C++ owner projects the Native thread-local stored object.
    static thread_local std::shared_ptr<EventLoop> event_loop_;

};

// Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:156-160
long long delay_to_nanos(long long time_millis);
// Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:162-163
long long delay_nanos_to_millis(long long time_nanos);

/**
 * Event loop that blocks on `process_next_event`.
 * Used by `run_blocking`.
 */
struct BlockingEventLoop : public EventLoop {
    std::shared_ptr<std::thread> thread; // The thread running this loop
    std::deque<std::shared_ptr<Runnable>> task_queue;
    mutable std::mutex mtx;
    mutable std::condition_variable cv;
    bool quit = false;

    explicit BlockingEventLoop(std::shared_ptr<std::thread> t);

    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override;
    long long process_next_event() override;
    bool is_empty() const override;
    void run();
    void shutdown() override;
};

} // namespace coroutines
} // namespace kotlinx
