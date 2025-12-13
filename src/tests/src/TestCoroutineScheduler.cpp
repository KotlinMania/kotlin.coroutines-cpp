#include <string>
#include "kotlinx/coroutines/selects/Select.hpp"
// Transliterated from Kotlin to C++ - kotlinx.coroutines.test.TestCoroutineScheduler
// Original package: kotlinx.coroutines.test
//
// TODO: Import statements removed; fully qualify types or add appropriate includes
// TODO: suspend functions translated as normal functions; coroutine semantics NOT implemented
// TODO: Kotlin atomic operations (kotlinx.atomicfu) need C++ std::atomic equivalents
// TODO: Kotlin companion object pattern translated to static members
// TODO: ThreadSafeHeap and related synchronization primitives need platform-specific implementations
// TODO: Annotations (@ExperimentalCoroutinesApi, @JvmField) preserved as comments

#include <atomic>
#include <functional>
#include <memory>
#include <chrono>
#include <algorithm>
#include <stdexcept>

namespace kotlinx {
    namespace coroutines {
        namespace test {
            // package kotlinx.coroutines.test

            // import kotlinx.atomicfu.*
            // import kotlinx.coroutines.*
            // import kotlinx.coroutines.channels.*
            // import kotlinx.coroutines.channels.Channel.Factory.CONFLATED
            // import kotlinx.coroutines.internal.*
            // import kotlinx.coroutines.selects.*
            // import kotlin.coroutines.*
            // import kotlin.jvm.*
            // import kotlin.time.*
            // import kotlin.time.Duration.Companion.milliseconds

            /**
 * This is a scheduler for coroutines used in tests, providing the delay-skipping behavior.
 *
 * [Test dispatchers][TestDispatcher] are parameterized with a scheduler. Several dispatchers can share the
 * same scheduler, in which case their knowledge about the virtual time will be synchronized. When the dispatchers
 * require scheduling an event at a later point in time, they notify the scheduler, which will establish the order of
 * the tasks.
 *
 * The scheduler can be queried to advance the time (via [advanceTimeBy]), run all the scheduled tasks advancing the
 * virtual time as needed (via [advanceUntilIdle]), or run the tasks that are scheduled to run as soon as possible but
 * haven't yet been dispatched (via [runCurrent]).
 */
            class TestCoroutineScheduler : AbstractCoroutineContextElement, CoroutineContext::Element {
            public:
                /** @suppress */
                class Key : CoroutineContext::Key<TestCoroutineScheduler> {
                };

                static Key KEY;

            private:
                /** This heap stores the knowledge about which dispatchers are interested in which moments of virtual time. */
                // TODO: all the synchronization is done via a separate lock, so a non-thread-safe priority queue can be used.
                ThreadSafeHeap<TestDispatchEvent<void *> > *events_;

                /** Establishes that [currentTime] can't exceed the time of the earliest event in [events]. */
                SynchronizedObject *lock_;

                /** This counter establishes some order on the events that happen at the same virtual time. */
                std::atomic<int64_t> count_;

                /** The current virtual time in milliseconds. */
                // @ExperimentalCoroutinesApi
                int64_t current_time_;

            public:
                TestCoroutineScheduler() : AbstractCoroutineContextElement(KEY),
                                           events_(new ThreadSafeHeap<TestDispatchEvent<void *> >()),
                                           lock_(new SynchronizedObject()),
                                           count_(0),
                                           current_time_(0) {
                }

                // @ExperimentalCoroutinesApi
                int64_t current_time() const {
                    return synchronized(*lock_, [this]() { return current_time_; });
                }

            private:
                /** A channel for notifying about the fact that a foreground work dispatch recently happened. */
                Channel<void> *dispatch_events_foreground_;

                /** A channel for notifying about the fact that a dispatch recently happened. */
                Channel<void> *dispatch_events_;

            public:
                TestCoroutineScheduler() {
                    dispatch_events_foreground_ = new Channel<void>(Channel<void>::CONFLATED);
                    dispatch_events_ = new Channel<void>(Channel<void>::CONFLATED);
                }

                /**
     * Registers a request for the scheduler to notify [dispatcher] at a virtual moment [timeDeltaMillis] milliseconds
     * later via [TestDispatcher.processEvent], which will be called with the provided [marker] object.
     *
     * Returns the handler which can be used to cancel the registration.
     */
                template<typename T>
                DisposableHandle *register_event(
                    TestDispatcher *dispatcher,
                    int64_t time_delta_millis,
                    T *marker,
                    const CoroutineContext &context,
                    std::function<bool(T *)> is_cancelled
                ) {
                    if (time_delta_millis < 0) {
                        throw std::invalid_argument(
                            "Attempted scheduling an event earlier in time (with the time delta " +
                            std::to_string(time_delta_millis) + ")");
                    }
                    check_scheduler_in_context(*this, context);
                    int64_t count = count_.fetch_add(1);
                    bool is_foreground = context[BackgroundWork::Key] == nullptr;
                    return synchronized(*lock_, [&]() {
                        int64_t time = add_clamping(current_time_, time_delta_millis);
                        auto *event = new TestDispatchEvent<void *>(dispatcher, count, time,
                                                                    static_cast<void *>(marker),
                                                                    is_foreground, [is_cancelled, marker]() {
                                                                        return is_cancelled(marker);
                                                                    });
                        events_->add_last(event);
                        /** can't be moved above: otherwise, [onDispatchEventForeground] or [onDispatchEvent] could consume the
             * token sent here before there's actually anything in the event queue. */
                        send_dispatch_event(context);
                        return new DisposableHandle([this, event]() {
                            synchronized(*lock_, [&]() {
                                events_->remove(event);
                            });
                        });
                    });
                }

                /**
     * Runs the next enqueued task, advancing the virtual time to the time of its scheduled awakening,
     * unless [condition] holds.
     */
                bool try_run_next_task_unless(std::function<bool()> condition) {
                    TestDispatchEvent<void *> *event = synchronized(*lock_, [&]() -> TestDispatchEvent<void *> * {
                        if (condition()) return nullptr;
                        auto *evt = events_->remove_first_or_null();
                        if (evt == nullptr) return nullptr;
                        if (current_time_ > evt->time)
                            current_time_ahead_of_events();
                        current_time_ = evt->time;
                        return evt;
                    });
                    if (event == nullptr) return false;
                    event->dispatcher->process_event(event->marker);
                    return true;
                }

                /**
     * Runs the enqueued tasks in the specified order, advancing the virtual time as needed until there are no more
     * tasks associated with the dispatchers linked to this scheduler.
     *
     * A breaking change from `TestCoroutineDispatcher.advanceTimeBy` is that it no longer returns the total number of
     * milliseconds by which the execution of this method has advanced the virtual time. If you want to recreate that
     * functionality, query [currentTime] before and after the execution to achieve the same result.
     */
                void advance_until_idle() {
                    advance_until_idle_or([]() {
                        return events_->none([](const TestDispatchEvent<void *> *e) { return e->is_foreground; });
                    });
                }

                /**
     * [condition]: guaranteed to be invoked under the lock.
     */
                void advance_until_idle_or(std::function<bool()> condition) {
                    while (true) {
                        if (!try_run_next_task_unless(condition))
                            return;
                    }
                }

                /**
     * Runs the tasks that are scheduled to execute at this moment of virtual time.
     */
                void run_current() {
                    int64_t time_mark = synchronized(*lock_, [this]() { return current_time_; });
                    while (true) {
                        auto *event = synchronized(*lock_, [&]() -> TestDispatchEvent<void *> * {
                            return events_->remove_first_if([time_mark](const TestDispatchEvent<void *> *e) {
                                return e->time <= time_mark;
                            });
                        });
                        if (event == nullptr) return;
                        event->dispatcher->process_event(event->marker);
                    }
                }

                /**
     * Moves the virtual clock of this dispatcher forward by [the specified amount][delayTimeMillis], running the
     * scheduled tasks in the meantime.
     *
     * Breaking changes from [TestCoroutineDispatcher.advanceTimeBy]:
     * - Intentionally doesn't return a `Long` value, as its use cases are unclear. We may restore it in the future;
     *   please describe your use cases at [the issue tracker](https://github.com/Kotlin/kotlinx.coroutines/issues/).
     *   For now, it's possible to query [currentTime] before and after execution of this method, to the same effect.
     * - It doesn't run the tasks that are scheduled at exactly [currentTime] + [delayTimeMillis]. For example,
     *   advancing the time by one millisecond used to run the tasks at the current millisecond *and* the next
     *   millisecond, but now will stop just before executing any task starting at the next millisecond.
     * - Overflowing the target time used to lead to nothing being done, but will now run the tasks scheduled at up to
     *   (but not including) [Long.MAX_VALUE].
     *
     * @throws IllegalArgumentException if passed a negative [delay][delayTimeMillis].
     */
                // @ExperimentalCoroutinesApi
                void advance_time_by(int64_t delay_time_millis) {
                    advance_time_by(std::chrono::milliseconds(delay_time_millis));
                }

                /**
     * Moves the virtual clock of this dispatcher forward by [the specified amount][delayTime], running the
     * scheduled tasks in the meantime.
     *
     * @throws IllegalArgumentException if passed a negative [delay][delayTime].
     */
                void advance_time_by(Duration delay_time) {
                    if (delay_time.count() < 0) {
                        throw std::invalid_argument(
                            "Can not advance time by a negative delay: " + std::to_string(delay_time.count()));
                    }
                    int64_t starting_time = current_time_;
                    int64_t target_time = add_clamping(starting_time,
                                                       std::chrono::duration_cast<std::chrono::milliseconds>(delay_time)
                                                       .count());
                    while (true) {
                        auto *event = synchronized(*lock_, [&]() -> TestDispatchEvent<void *> * {
                            int64_t time_mark = current_time_;
                            auto *evt = events_->remove_first_if([target_time](const TestDispatchEvent<void *> *e) {
                                return target_time > e->time;
                            });
                            if (evt == nullptr) {
                                current_time_ = target_time;
                                return nullptr;
                            } else if (time_mark > evt->time) {
                                current_time_ahead_of_events();
                            } else {
                                current_time_ = evt->time;
                            }
                            return evt;
                        });
                        if (event == nullptr) return;
                        event->dispatcher->process_event(event->marker);
                    }
                }

                /**
     * Checks that the only tasks remaining in the scheduler are cancelled.
     */
                bool is_idle(bool strict = true) const {
                    return synchronized(*lock_, [&]() {
                        if (strict) return events_->is_empty();
                        return events_->none([](const TestDispatchEvent<void *> *e) { return !e->is_cancelled(); });
                    });
                }

                /**
     * Notifies this scheduler about a dispatch event.
     *
     * [context] is the context in which the task will be dispatched.
     */
                void send_dispatch_event(const CoroutineContext &context) {
                    dispatch_events_->try_send({});
                    if (context[BackgroundWork::Key] != &BackgroundWork::instance)
                        dispatch_events_foreground_->try_send({});
                }

                /**
     * Waits for a notification about a dispatch event.
     */
                // TODO: suspend function - coroutine semantics not implemented
                void receive_dispatch_event() {
                    dispatch_events_->receive();
                }

                /**
     * Consumes the knowledge that a dispatch event happened recently.
     */
                std::shared_ptr<selects::SelectClause1<void> > on_dispatch_event() const {
                    return dispatch_events_->on_receive();
                }

                /**
     * Consumes the knowledge that a foreground work dispatch event happened recently.
     */
                std::shared_ptr<selects::SelectClause1<void> > on_dispatch_event_foreground() const {
                    return dispatch_events_foreground_->on_receive();
                }

                /**
     * Returns the [TimeSource] representation of the virtual time of this scheduler.
     */
                TimeSource::WithComparableMarks *time_source() {
                    // TODO: Kotlin class expression needs C++ lambda or custom class
                    return new TimeSourceImpl([this]() { return current_time_; });
                }
            };

            // Some error-throwing functions for pretty stack traces
            void current_time_ahead_of_events() {
                invalid_scheduler_state();
            }

            void invalid_scheduler_state() {
                throw std::logic_error(
                    "The test scheduler entered an invalid state. Please report this at https://github.com/Kotlin/kotlinx.coroutines/issues.");
            }

            /** [ThreadSafeHeap] node representing a scheduled task, ordered by the planned execution time. */
            template<typename T>
            class TestDispatchEvent : Comparable<TestDispatchEvent<T> >, ThreadSafeHeapNode {
            public:
                // @JvmField
                TestDispatcher *dispatcher;

            private:
                int64_t count_;

            public:
                // @JvmField
                int64_t time;
                // @JvmField
                T marker;
                // @JvmField
                bool is_foreground;
                // TODO: remove once the deprecated API is gone
                // @JvmField
                std::function<bool()> is_cancelled;

                ThreadSafeHeap<TestDispatchEvent<T> > *heap;
                int index;

                TestDispatchEvent(TestDispatcher *disp, int64_t cnt, int64_t tm, T mrk, bool fg,
                                  std::function<bool()> cancelled_fn)
                    : dispatcher(disp), count_(cnt), time(tm), marker(mrk), is_foreground(fg),
                      is_cancelled(cancelled_fn),
                      heap(nullptr), index(0) {
                }

                int compare_to(const TestDispatchEvent<T> &other) const override {
                    if (time < other.time) return -1;
                    if (time > other.time) return 1;
                    if (count_ < other.count_) return -1;
                    if (count_ > other.count_) return 1;
                    return 0;
                }

                std::string to_string() const override {
                    std::string result = "TestDispatchEvent(time=" + std::to_string(time) +
                                         ", dispatcher=" + dispatcher->to_string();
                    if (!is_foreground) result += ", background";
                    result += ")";
                    return result;
                }
            };

            // works with positive `a`, `b`
            int64_t add_clamping(int64_t a, int64_t b) {
                int64_t result = a + b;
                return result >= 0 ? result : LLONG_MAX;
            }

            void check_scheduler_in_context(const TestCoroutineScheduler &scheduler, const CoroutineContext &context) {
                auto *ctx_scheduler = context[TestCoroutineScheduler::KEY];
                if (ctx_scheduler != nullptr) {
                    if (ctx_scheduler != &scheduler) {
                        throw std::logic_error(
                            "Detected use of different schedulers. If you need to use several test coroutine dispatchers, "
                            +
                            std::string("create one `TestCoroutineScheduler` and pass it to each of them.")
                        );
                    }
                }
            }

            /**
 * A coroutine context key denoting that the work is to be executed in the background.
 * @see [TestScope.backgroundScope]
 */
            // TODO: class singleton pattern
            class BackgroundWork : CoroutineContext::Key<BackgroundWork>, CoroutineContext::Element {
            public:
                class Key : CoroutineContext::Key<BackgroundWork> {
                };

                static Key KEY;
                static BackgroundWork instance;

                CoroutineContext::Key<BackgroundWork> *key() const override {
                    return &KEY;
                }

                std::string to_string() const override {
                    return "BackgroundWork";
                }
            };

            BackgroundWork BackgroundWork::instance;
            BackgroundWork::Key BackgroundWork::KEY;

            template<typename T>
            bool none(ThreadSafeHeap<T> *heap, std::function<bool(const T *)> predicate) {
                return heap->find(predicate) == nullptr;
            }
        } // namespace test
    } // namespace coroutines
} // namespace kotlinx
