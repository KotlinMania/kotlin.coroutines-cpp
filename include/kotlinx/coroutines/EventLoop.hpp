#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include <deque>
#include <memory>
#include <atomic>
#include <climits>

namespace kotlinx {
namespace coroutines {

class DispatchedTask : public Runnable {
public:
    virtual void run() override = 0;
};

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
        task->run(); 
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

    // ThreadLocal State
    struct ThreadLocalEventLoop {
        static std::shared_ptr<EventLoop> get_event_loop() {
            static thread_local std::shared_ptr<EventLoop> loop = std::make_shared<EventLoop>(); // Simple default loop
            if (event_loop) return std::shared_ptr<EventLoop>(std::shared_ptr<void>(), event_loop); // Non-owning reference wrapper if raw pointer set?
            return loop;
        }
        
        static std::shared_ptr<EventLoop> current_or_null() {
             return event_loop ? std::shared_ptr<EventLoop>(std::shared_ptr<void>(), event_loop) : nullptr;
        }
        
        static void reset_event_loop() {
            event_loop = nullptr;
        }
        
        static void set_event_loop(std::shared_ptr<EventLoop> eventLoop) {
             // We store raw pointer or handle ownership?
             // thread_local storage of shared_ptr is tricky if we want to set it.
             // For simplify, store raw pointer in thread_local for overriding (runBlocking).
             event_loop = eventLoop.get();
        }
        
        static thread_local EventLoop* event_loop; 
    };
    
    // Define the static member
    inline thread_local EventLoop* ThreadLocalEventLoop::event_loop = nullptr;

    /**
     * Event loop that blocks on `process_next_event`.
     * Used by `runBlocking`.
     */
    struct BlockingEventLoop : public EventLoop {
        std::shared_ptr<std::thread> thread; // The thread running this loop
        
        BlockingEventLoop(std::shared_ptr<std::thread> t) : thread(t) {}
        
        // Queue for dispatched tasks
        std::deque<std::shared_ptr<Runnable>> task_queue;
        mutable std::mutex mtx;
        mutable std::condition_variable cv;
        bool quit = false;

        void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
             {
                 std::lock_guard<std::mutex> lock(mtx);
                 // We need to cast away constness to push to queue if we keep task_queue mutable or use current implementation
                 // Actually `dispatch` is const in CoroutineDispatcher parent.
                 const_cast<BlockingEventLoop*>(this)->task_queue.push_back(block);
             }
             cv.notify_one();
        }

        long long process_next_event() override {
            // Process unconfined first
            if (EventLoop::process_unconfined_event()) return 0;
            
            std::shared_ptr<Runnable> task;
            {
                std::unique_lock<std::mutex> lock(mtx);
                if (task_queue.empty()) return LLONG_MAX; // No events
                task = task_queue.front();
                task_queue.pop_front();
            }
            task->run();
            return 0;
        }
        
        // Specific block method for runBlocking
        void run() {
             while (!quit) {
                 if (process_next_event() == LLONG_MAX) {
                     // Wait
                     std::unique_lock<std::mutex> lock(mtx);
                     cv.wait(lock, [this]{ return quit || !task_queue.empty(); });
                 }
             }
        }
        
        void shutdown() override {
             {
                 std::lock_guard<std::mutex> lock(mtx);
                 quit = true;
             }
             cv.notify_all();
        }
    };

} // namespace coroutines
} // namespace kotlinx
