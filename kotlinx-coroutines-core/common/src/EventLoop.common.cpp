#include <string>
#include <deque>
#include <atomic>
#include <memory>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>
#include <condition_variable>
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp" 

namespace kotlinx {
namespace coroutines {

// Forward classes
class DispatchedTask;
class DelayedTask;

/**
 * Extended by [CoroutineDispatcher] implementations that have event loop inside and can
 * be asked to process next event from their event queue.
 */
struct EventLoop : CoroutineDispatcher {
    long long use_count = 0;
    bool shared = false;
    std::deque<std::shared_ptr<DispatchedTask>> unconfined_queue;

    virtual ~EventLoop() = default;

    virtual long long process_next_event() {
        if (!process_unconfined_event()) return LLONG_MAX;
        return 0;
    }

    virtual bool is_empty() const {
        return is_unconfined_queue_empty();
    }

    virtual long long next_time() const {
        if (unconfined_queue.empty()) return LLONG_MAX;
        return 0;
    }

    bool process_unconfined_event() {
        if (unconfined_queue.empty()) return false;
        auto task = unconfined_queue.front();
        unconfined_queue.pop_front();
        // task->run(); 
        return true;
    }

    virtual bool should_be_processed_from_context() const { return false; }

    void dispatch_unconfined(std::shared_ptr<DispatchedTask> task) {
        unconfined_queue.push_back(task);
    }

    bool is_active() const { return use_count > 0; }
    
    bool is_unconfined_loop_active() const { return use_count >= delta(true); }

    bool is_unconfined_queue_empty() const { return unconfined_queue.empty(); }

    static long long delta(bool unconfined) { return unconfined ? (1LL << 32) : 1LL; }

    void increment_use_count(bool unconfined = false) {
        use_count += delta(unconfined);
        if (!unconfined) shared = true;
    }

    void decrement_use_count(bool unconfined = false) {
        use_count -= delta(unconfined);
        if (use_count > 0) return;
        if (shared) {
            shutdown();
        }
    }

    virtual void shutdown() {}
    
    // CoroutineDispatcher overrides
    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
        // Implement default dispatch or leave to subclasses
    }
};

struct ThreadLocalEventLoop {
    // In actual C++, this would be a thread_local static member
    // For fsyntax-only check, just declaring the interface is enough
    static std::shared_ptr<EventLoop> get_event_loop();
    static std::shared_ptr<EventLoop> current_or_null();
    static void reset_event_loop();
    static void set_event_loop(std::shared_ptr<EventLoop> eventLoop);
};

class DispatchedTask : public Runnable {
public:
    virtual void run() override = 0; 
};

} // namespace coroutines
} // namespace kotlinx