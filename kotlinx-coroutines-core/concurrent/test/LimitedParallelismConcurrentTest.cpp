// Original: kotlinx-coroutines-core/concurrent/test/LimitedParallelismConcurrentTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement atomic operations with std::atomic
// TODO: Implement limitedParallelism, newFixedThreadPoolContext
// TODO: Implement coroutineScope, launch, async
// TODO: Implement inline crossinline functions

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.atomicfu.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.exceptions.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.test.*

class LimitedParallelismConcurrentTest : public TestBase {
private:
    static constexpr int target_parallelism = 4;
    static constexpr int iterations = 100'000;
    std::atomic<int> parallelism{0};

    void check_parallelism() {
        int value = parallelism.fetch_add(1) + 1;
        random_wait();
        assertTrue(value <= target_parallelism);
        parallelism.fetch_sub(1);
    }

public:
    // @Test
    // TODO: Convert test annotation
    void test_limited_executor() {
        runTest([&]() {
            // TODO: suspend function
            auto executor = new_fixed_thread_pool_context(target_parallelism, "test");
            auto view = executor.limited_parallelism(target_parallelism);
            do_stress([&]() {
                // TODO: suspend function
                for (int i = 0; i < iterations; ++i) {
                    launch(view, [&]() {
                        // TODO: suspend function
                        check_parallelism();
                    });
                }
            });
            executor.close();
        });
    }

private:
    template<typename Block>
    void do_stress(Block block) {
        // TODO: suspend inline function
        for (int i = 0; i < stress_test_multiplier; ++i) {
            coroutine_scope([&]() {
                // TODO: suspend function
                block();
            });
        }
    }

public:
    // @Test
    // TODO: Convert test annotation
    void test_task_fairness() {
        runTest([&]() {
            // TODO: suspend function
            auto executor = new_single_thread_context("test");
            auto view = executor.limited_parallelism(1);
            auto view2 = executor.limited_parallelism(1);
            auto j1 = launch(view, [&]() {
                // TODO: suspend function
                while (true) {
                    yield();
                }
            });
            auto j2 = launch(view2, [&]() {
                // TODO: suspend function
                j1.cancel();
            });
            join_all(j1, j2);
            executor.close();
        });
    }

    /**
     * Tests that, when no tasks are present, the limited dispatcher does not dispatch any tasks.
     * This is important for the case when a dispatcher is closeable and the [CoroutineDispatcher.limitedParallelism]
     * machinery could trigger a dispatch after the dispatcher is closed.
     */
    // @Test
    // TODO: Convert test annotation
    void test_not_doing_dispatches_when_no_tasks_are_present() {
        runTest([&]() {
            // TODO: suspend function
            class NaggingDispatcher : public CoroutineDispatcher {
            private:
                std::atomic<bool> closed_{false};

            public:
                void dispatch(CoroutineContext context, Runnable block) override {
                    if (closed_.load())
                        fail("Dispatcher was closed, but still dispatched a task");
                    Dispatchers::Default.dispatch(context, block);
                }

                void close() {
                    closed_.store(true);
                }
            };

            for (int i = 0; i < stress_test_multiplier * 500'000; ++i) {
                NaggingDispatcher dispatcher;
                auto view = dispatcher.limited_parallelism(1);
                auto deferred = CompletableDeferred<void>();
                auto job = launch(view, [&]() {
                    // TODO: suspend function
                    deferred.await();
                });
                launch(Dispatchers::Default, [&]() {
                    // TODO: suspend function
                    deferred.complete(Unit);
                });
                job.join();
                dispatcher.close();
            }
        });
    }
};

} // namespace coroutines
} // namespace kotlinx
