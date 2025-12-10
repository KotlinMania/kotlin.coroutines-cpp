/**
 * @file EventLoop.common.cpp
 * @brief Implementation of EventLoop.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/EventLoop.hpp`.
 */

#include "kotlinx/coroutines/EventLoop.hpp"

namespace kotlinx {
namespace coroutines {

// EventLoop Implementation

long long EventLoop::process_next_event() {
    if (!process_unconfined_event()) return LLONG_MAX;
    return 0;
}

bool EventLoop::is_empty() const {
    return is_unconfined_queue_empty();
}

long long EventLoop::next_time() const {
    if (unconfined_queue.empty()) return LLONG_MAX;
    return 0;
}

bool EventLoop::process_unconfined_event() {
    if (unconfined_queue.empty()) return false;
    auto task = unconfined_queue.front();
    unconfined_queue.pop_front();
    task->run();
    return true;
}

bool EventLoop::should_be_processed_from_context() const { return false; }

void EventLoop::dispatch_unconfined(std::shared_ptr<DispatchedTask> task) {
    unconfined_queue.push_back(task);
}

bool EventLoop::is_active() const { return use_count > 0; }

bool EventLoop::is_unconfined_loop_active() const { return use_count >= delta(true); }

bool EventLoop::is_unconfined_queue_empty() const { return unconfined_queue.empty(); }

long long EventLoop::delta(bool unconfined) { return unconfined ? (1LL << 32) : 1LL; }

void EventLoop::increment_use_count(bool unconfined) {
    use_count += delta(unconfined);
    if (!unconfined) shared = true;
}

void EventLoop::decrement_use_count(bool unconfined) {
    use_count -= delta(unconfined);
    if (use_count > 0) return;
    if (shared) {
        shutdown();
    }
}

void EventLoop::shutdown() {}

void EventLoop::dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const {
    // Implement default dispatch or leave to subclasses
}

// ThreadLocalEventLoop Implementation

thread_local EventLoop* ThreadLocalEventLoop::event_loop = nullptr;

std::shared_ptr<EventLoop> ThreadLocalEventLoop::get_event_loop() {
    static thread_local std::shared_ptr<EventLoop> loop = std::make_shared<EventLoop>(); // Simple default loop
    if (event_loop) return std::shared_ptr<EventLoop>(std::shared_ptr<void>(), event_loop); // Non-owning reference wrapper if raw pointer set?
    return loop;
}

std::shared_ptr<EventLoop> ThreadLocalEventLoop::current_or_null() {
    return event_loop ? std::shared_ptr<EventLoop>(std::shared_ptr<void>(), event_loop) : nullptr;
}

void ThreadLocalEventLoop::reset_event_loop() {
    event_loop = nullptr;
}

void ThreadLocalEventLoop::set_event_loop(std::shared_ptr<EventLoop> eventLoop) {
    // We store raw pointer or handle ownership?
    // thread_local storage of shared_ptr is tricky if we want to set it.
    // For simplify, store raw pointer in thread_local for overriding (runBlocking).
    event_loop = eventLoop.get();
}

// BlockingEventLoop Implementation

BlockingEventLoop::BlockingEventLoop(std::shared_ptr<std::thread> t) : thread(t) {}

void BlockingEventLoop::dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const {
    {
        std::lock_guard<std::mutex> lock(mtx);
        // We need to cast away constness to push to queue if we keep task_queue mutable or use current implementation
        // Actually `dispatch` is const in CoroutineDispatcher parent.
        const_cast<BlockingEventLoop*>(this)->task_queue.push_back(block);
    }
    cv.notify_one();
}

long long BlockingEventLoop::process_next_event() {
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

void BlockingEventLoop::run() {
    while (!quit) {
        if (process_next_event() == LLONG_MAX) {
            // Wait
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]{ return quit || !task_queue.empty(); });
        }
    }
}

void BlockingEventLoop::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        quit = true;
    }
    cv.notify_all();
}

} // namespace coroutines
} // namespace kotlinx