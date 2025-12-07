// Original: kotlinx-coroutines-core/concurrent/test/JobStructuredJoinStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement suspendCancellableCoroutine, suspendCancellableCoroutineReusable
// TODO: Implement runTest, runBlocking, launch
// TODO: Implement assertFailsWith, TestException
// TODO: Implement function type parameters

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.test.*

/**
 * Test a race between job failure and join.
 *
 * See [#1123](https://github.com/Kotlin/kotlinx.coroutines/issues/1123).
 */
class JobStructuredJoinStressTest : public TestBase {
private:
    int n_repeats = 10'000 * stress_test_multiplier;

public:
    // @Test
    // TODO: Convert test annotation
    void test_stress_regular_join() {
        runTest([&]() {
            // TODO: suspend function
            stress([](Job& job) {
                // TODO: suspend function
                job.join();
            });
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_stress_suspend_cancellable() {
        runTest([&]() {
            // TODO: suspend function
            stress([](Job& job) {
                // TODO: suspend function
                suspend_cancellable_coroutine([&job](auto& cont) {
                    job.invoke_on_completion([&cont]() {
                        cont.resume(Unit);
                    });
                });
            });
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_stress_suspend_cancellable_reusable() {
        runTest([&]() {
            // TODO: suspend function
            stress([](Job& job) {
                // TODO: suspend function
                suspend_cancellable_coroutine_reusable([&job](auto& cont) {
                    job.invoke_on_completion([&cont]() {
                        cont.resume(Unit);
                    });
                });
            });
        });
    }

private:
    template<typename JoinFunc>
    void stress(JoinFunc join) {
        expect(1);
        for (int index = 0; index < n_repeats; ++index) {
            assert_fails_with<TestException>([&]() {
                runBlocking([&]() {
                    // TODO: suspend function
                    // launch in background
                    auto job = launch(Dispatchers::Default, [&]() {
                        // TODO: suspend function
                        throw TestException("OK"); // crash
                    });
                    try {
                        join(job);
                        error("Should not complete successfully");
                    } catch (const CancellationException& e) {
                        // must always crash with cancellation exception
                        expect(2 + index);
                    } catch (const std::exception& e) {
                        error("Unexpected exception");
                    }
                });
            });
        }
        finish(2 + n_repeats);
    }
};

} // namespace coroutines
} // namespace kotlinx
