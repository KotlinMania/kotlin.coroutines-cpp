// Original: kotlinx-coroutines-core/common/test/flow/VirtualTime.kt
// TODO: Translate imports to proper C++ includes
// TODO: Implement CoroutineDispatcher, Delay interfaces
// TODO: Implement CoroutineScope, CoroutineContext
// TODO: Implement DisposableHandle, Runnable
// TODO: Implement CancellableContinuation
// TODO: Implement ContinuationInterceptor
// TODO: Implement launch, withContext
// TODO: Implement ThreadLocalEventLoop
// TODO: Implement JvmField annotation equivalent

#include <vector>
#include <algorithm>
#include <memory>
#include <cstdint>
#include <limits>
// TODO: #include proper headers

namespace kotlinx {
    namespace coroutines {
        class VirtualTimeDispatcher : public CoroutineDispatcher, public Delay {
        private:
            CoroutineDispatcher *original_dispatcher_;
            std::vector<TimedTask *> heap_; // TODO use MPP heap/ordered set implementation (commonize ThreadSafeHeap)
            int64_t current_time_;

            class TimedTask : public DisposableHandle, public Runnable {
            private:
                Runnable *runnable_;
                VirtualTimeDispatcher *parent_;

            public:
                int64_t deadline; // @JvmField equivalent

                TimedTask(Runnable *r, int64_t dl, VirtualTimeDispatcher *p)
                    : runnable_(r), deadline(dl), parent_(p) {
                }

                void dispose() override {
                    auto &heap = parent_->heap_;
                    heap.erase(std::remove(heap.begin(), heap.end(), this), heap.end());
                }

                void run() override {
                    runnable_->run();
                }
            };

        public:
            VirtualTimeDispatcher(CoroutineScope &enclosing_scope)
                : current_time_(0) {
                original_dispatcher_ = dynamic_cast<CoroutineDispatcher *>(
                    &enclosing_scope.coroutine_context()[ContinuationInterceptor]
                );

                /*
         * Launch "event-loop-owning" task on start of the virtual time event loop.
         * It ensures the progress of the enclosing event-loop and polls the timed queue
         * when the enclosing event loop is empty, emulating virtual time.
         */
                enclosing_scope.launch([this]() -> /* suspend */ void {
                    while (true) {
                        auto *event_loop = ThreadLocalEventLoop::current_or_null();
                        if (!event_loop) {
                            throw std::runtime_error(
                                "Event loop is missing, virtual time source works only as part of event loop");
                        }
                        int64_t delay_nanos = event_loop->process_next_event();

                        if (delay_nanos <= 0) continue;

                        if (delay_nanos > 0 && delay_nanos != std::numeric_limits<int64_t>::max()) {
                            if (uses_shared_event_loop) {
                                int64_t target_time = current_time_ + delay_nanos;
                                while (current_time_ < target_time) {
                                    auto min_it = std::min_element(heap_.begin(), heap_.end(),
                                                                   [](TimedTask *a, TimedTask *b) {
                                                                       return a->deadline < b->deadline;
                                                                   });

                                    if (min_it == heap_.end()) break;
                                    TimedTask *next_task = *min_it;

                                    if (next_task->deadline > target_time) break;

                                    heap_.erase(min_it);
                                    current_time_ = next_task->deadline;
                                    next_task->run();
                                }
                                current_time_ = std::max(current_time_, target_time);
                            } else {
                                throw std::runtime_error("Unexpected external delay: " + std::to_string(delay_nanos));
                            }
                        }

                        auto min_it = std::min_element(heap_.begin(), heap_.end(),
                                                       [](TimedTask *a, TimedTask *b) {
                                                           return a->deadline < b->deadline;
                                                       });

                        if (min_it == heap_.end()) return;
                        TimedTask *next_task = *min_it;

                        heap_.erase(min_it);
                        current_time_ = next_task->deadline;
                        next_task->run();
                    }
                });
            }

            int64_t current_time() const {
                return current_time_;
            }

            void dispatch(CoroutineContext &context, Runnable &block) override {
                original_dispatcher_->dispatch(context, block);
            }

            bool is_dispatch_needed(CoroutineContext &context) override {
                return original_dispatcher_->is_dispatch_needed(context);
            }

            DisposableHandle *
            invoke_on_timeout(int64_t time_millis, Runnable &block, CoroutineContext &context) override {
                TimedTask *task = new TimedTask(&block, deadline(time_millis), this);
                heap_.push_back(task);
                return task;
            }

            void schedule_resume_after_delay(int64_t time_millis,
                                             CancellableContinuation<void> &continuation) override {
                auto *block = new /* Runnable */ auto([&continuation]() {
                    continuation.resume_undispatched(/* Unit */ {});
                });
                TimedTask *task = new TimedTask(block, deadline(time_millis), this);
                heap_.push_back(task);
                continuation.invoke_on_cancellation([task](auto) { task->dispose(); });
            }

        private:
            int64_t deadline(int64_t time_millis) {
                return (time_millis == std::numeric_limits<int64_t>::max())
                           ? std::numeric_limits<int64_t>::max()
                           : current_time_ + time_millis;
            }
        };

        /**
 * Runs a test (TestBase::runTest) with a virtual time source.
 * This runner has the following constraints:
 * 1) It works only in the event-loop environment and it is relying on it.
 *    None of the coroutines should be launched in any dispatcher different from a current
 * 2) Regular tasks always dominate delayed ones. It means that
 *    `launch { while(true) yield() }` will block the progress of the delayed tasks
 * 3) TestBase::finish should always be invoked.
 *    Given all the constraints into account, it is easy to mess up with a test and actually
 *    return from withVirtualTime before the test is executed completely.
 *    To decrease the probability of such error, additional `finish` constraint is added.
 */
        template<typename Func>
        void with_virtual_time(TestBase &test_base, Func &&block) {
            test_base.run_test([block = std::forward<Func>(block)]() -> /* suspend */ void {
                with_context(Dispatchers::Unconfined, [&]() -> /* suspend */ void {
                    // Create a platform-independent event loop
                    VirtualTimeDispatcher dispatcher(*this);
                    with_context(dispatcher, [&]() -> /* suspend */ void {
                        block();
                    });
                    check_finish_call(/* allowNotUsingExpect = */ false);
                });
            });
        }
    } // namespace coroutines
} // namespace kotlinx