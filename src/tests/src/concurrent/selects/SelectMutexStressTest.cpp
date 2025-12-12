// Original: kotlinx-coroutines-core/concurrent/test/selects/SelectMutexStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement Mutex, MutexImpl, select

namespace kotlinx {
    namespace coroutines {
        namespace selects {
            // TODO: import kotlinx.coroutines.testing.*
            // TODO: import kotlinx.coroutines.*
            // TODO: import kotlinx.coroutines.sync.*
            // TODO: import kotlin.test.*

            class SelectMutexStressTest : public TestBase {
            public:
                // @Test
                // TODO: Convert test annotation
                void test_select_cancelled_resource_release() {
                    runTest([&]() {
                        // TODO: suspend function
                        int n = 1'000 * stress_test_multiplier;
                        auto *mutex = new MutexImpl(true); // locked
                        expect(1);
                        for (int i = 0; i < n; ++i) {
                            auto job = launch(coroutine_context(), [&, i]() {
                                // TODO: suspend function
                                expect(i + 2);
                                select<void>([&](auto &builder) {
                                    builder.on_lock(*mutex, [&]() {
                                        expectUnreached(); // never able to lock
                                    });
                                });
                            });
                            yield(); // to the launched job, so that it suspends
                            job.cancel(); // cancel the job and select
                            yield(); // so it can cleanup after itself
                        }
                        assertTrue(mutex->is_locked());
                        finish(n + 2);
                    });
                }
            };
        } // namespace selects
    } // namespace coroutines
} // namespace kotlinx