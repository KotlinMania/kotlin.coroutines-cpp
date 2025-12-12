#pragma once
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
 * Extended by [CoroutineDispatcher] implementations that have event loop inside and can
 * be asked to process next event from their event queue.
 */
struct EventLoop : CoroutineDispatcher {
    long long use_count = 0;
    bool shared = false;
    std::deque<std::shared_ptr<SchedulerTask>> unconfined_queue;

    virtual ~EventLoop() = default;

    virtual long long process_next_event();
    virtual bool is_empty() const;
    virtual long long next_time() const;

    bool process_unconfined_event();
    virtual bool should_be_processed_from_context() const;
    void dispatch_unconfined(std::shared_ptr<SchedulerTask> task);

    bool is_active() const;
    bool is_unconfined_loop_active() const;
    bool is_unconfined_queue_empty() const;

    static long long delta(bool unconfined);
    void increment_use_count(bool unconfined = false);
    void decrement_use_count(bool unconfined = false);
    virtual void shutdown();

    // CoroutineDispatcher overrides
    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override;
};

struct ThreadLocalEventLoop {
    static std::shared_ptr<EventLoop> get_event_loop();
    static std::shared_ptr<EventLoop> current_or_null();
    static void reset_event_loop();
    static void set_event_loop(std::shared_ptr<EventLoop> eventLoop);

    static thread_local EventLoop* event_loop;
};

/**
 * Event loop that blocks on `process_next_event`.
 * Used by `runBlocking`.
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
    void run();
    void shutdown() override;
};

} // namespace coroutines
} // namespace kotlinx
