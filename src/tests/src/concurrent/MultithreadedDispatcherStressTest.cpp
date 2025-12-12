// Original: kotlinx-coroutines-core/concurrent/test/MultithreadedDispatcherStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement atomic operations with std::atomic
// TODO: Implement newFixedThreadPoolContext
// TODO: Implement EmptyCoroutineContext, Runnable

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.atomicfu.*
        // TODO: import kotlin.coroutines.*
        // TODO: import kotlin.test.*

        class MultithreadedDispatcherStressTest {
        private:
            std::atomic<int> shared{0};

        public:
            /**
     * Tests that [newFixedThreadPoolContext] will not drop tasks when closed.
     */
            // @Test
            // TODO: Convert test annotation
            void test_closing_not_dropping_tasks() {
                for (int it = 0; it < 7; ++it) {
                    shared.store(0);
                    int n_threads = it + 1;
                    auto dispatcher = new_fixed_thread_pool_context(n_threads, "testMultiThreadedContext");
                    for (int i = 0; i < 1'000; ++i) {
                        dispatcher.dispatch(EmptyCoroutineContext, Runnable([&]() {
                            shared.fetch_add(1);
                        }));
                    }
                    dispatcher.close();
                    while (shared.load() < 1'000) {
                        // spin.
                        // the test will hang here if the dispatcher drops tasks.
                    }
                }
            }
        };
    } // namespace coroutines
} // namespace kotlinx