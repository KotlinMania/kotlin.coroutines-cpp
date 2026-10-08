// port-lint: source kotlinx-coroutines-core/common/src/EventLoop.common.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt */
#include "kotlinx/coroutines/EventLoop.hpp"
#include <cassert>
#include <utility>

namespace kotlinx {
    namespace coroutines {
        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:49-52
        long long EventLoop::process_next_event() {
            if (!process_unconfined_event()) return LLONG_MAX;
            return 0;
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:54-54
        bool EventLoop::is_empty() const { return is_unconfined_queue_empty(); }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:56-60
        long long EventLoop::next_time() const {
            if (!unconfined_queue_) return LLONG_MAX;
            return unconfined_queue_->empty() ? LLONG_MAX : 0;
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:62-67
        bool EventLoop::process_unconfined_event() {
            if (!unconfined_queue_ || unconfined_queue_->empty()) return false;
            auto task = std::move(unconfined_queue_->front());
            unconfined_queue_->pop_front();
            task->run();
            return true;
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:74-74
        bool EventLoop::should_be_processed_from_context() const { return false; }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:80-84
        void EventLoop::dispatch_unconfined(std::shared_ptr<SchedulerTask> task) {
            if (!unconfined_queue_) unconfined_queue_ =
                std::make_unique<std::deque<std::shared_ptr<SchedulerTask>>>();
            unconfined_queue_->push_back(std::move(task));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:86-87
        bool EventLoop::is_active() const { return use_count_ > 0; }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:89-90
        bool EventLoop::is_unconfined_loop_active() const { return use_count_ >= delta(true); }

        // May only be used from the event loop's thread
        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:92-94
        bool EventLoop::is_unconfined_queue_empty() const {
            return !unconfined_queue_ || unconfined_queue_->empty();
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:96-97
        long long EventLoop::delta(bool unconfined) { return unconfined ? (1LL << 32) : 1LL; }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:99-102
        void EventLoop::increment_use_count(bool unconfined) {
            use_count_ += delta(unconfined);
            if (!unconfined) shared_ = true;
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:104-112
        void EventLoop::decrement_use_count(bool unconfined) {
            use_count_ -= delta(unconfined);
            if (use_count_ > 0) return;
            assert(use_count_ == 0); // "Extra decrementUseCount"
            if (shared_) {
                // shut it down and remove from ThreadLocalEventLoop
                shutdown();
            }
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:119-119
        void EventLoop::shutdown() {}

        void EventLoop::dispatch(const CoroutineContext& /*context*/,
                                 std::shared_ptr<Runnable> /*block*/) const {
            // The default EventLoop is a degenerate event loop that does not enqueue;
            // dispatch is overridden by every concrete dispatcher (EventLoopImpl,
            // BlockingEventLoop, etc.) that needs a real queue. Upstream's
            // `internal expect open class EventLoop` actual on K/N is empty too.
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:123-123
        thread_local std::shared_ptr<EventLoop> ThreadLocalEventLoop::event_loop_;

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:125-126
        std::shared_ptr<EventLoop> ThreadLocalEventLoop::get_event_loop() {
            if (!event_loop_) event_loop_ = std::make_shared<EventLoop>();
            return event_loop_;
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:128-129
        std::shared_ptr<EventLoop> ThreadLocalEventLoop::current_or_null() {
            return event_loop_;
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:131-133
        void ThreadLocalEventLoop::reset_event_loop() { event_loop_.reset(); }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:135-137
        void ThreadLocalEventLoop::set_event_loop(std::shared_ptr<EventLoop> event_loop) {
            event_loop_ = std::move(event_loop);
        }

        namespace {
        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:147-148
        constexpr long long MS_TO_NS = 1'000'000;
        constexpr long long MAX_MS = LLONG_MAX / MS_TO_NS;
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:156-160
        long long delay_to_nanos(long long time_millis) {
            if (time_millis <= 0) return 0;
            if (time_millis >= MAX_MS) return LLONG_MAX;
            return time_millis * MS_TO_NS;
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/EventLoop.common.kt:162-163
        long long delay_nanos_to_millis(long long time_nanos) {
            return time_nanos / MS_TO_NS;
        }

        // BlockingEventLoop Implementation

        BlockingEventLoop::BlockingEventLoop(std::shared_ptr<std::thread> t) : thread(t) {
        }

        void BlockingEventLoop::dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const {
            {
                std::lock_guard<std::mutex> lock(mtx);
                // We need to cast away constness to push to queue if we keep task_queue mutable or use current implementation
                // Actually `dispatch` is const in CoroutineDispatcher parent.
                const_cast<BlockingEventLoop *>(this)->task_queue.push_back(block);
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

        bool BlockingEventLoop::is_empty() const {
            if (!EventLoop::is_empty()) return false;
            std::lock_guard<std::mutex> lock(mtx);
            return task_queue.empty();
        }

        void BlockingEventLoop::run() {
            while (!quit) {
                if (process_next_event() == LLONG_MAX) {
                    // Wait
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this] { return quit || !task_queue.empty(); });
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