// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/CancellableContinuationTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

// @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED") // KT-21913

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.test.*

class CancellableContinuationTest : public TestBase {
public:
    // @Test
    void test_resume_with_exception_and_resume_with_exception() {
        run_test([this]() {
            Continuation<void>* continuation = nullptr;
            auto job = launch([this, &continuation]() {
                try {
                    expect(2);
                    suspend_cancellable_coroutine<void>([&continuation](auto c) {
                        continuation = &c;
                    });
                } catch (const TestException& e) {
                    expect(3);
                }
            });
            expect(1);
            yield();
            continuation->resume_with_exception(TestException());
            yield();
            assert_fails_with<IllegalStateException>([&]() {
                continuation->resume_with_exception(TestException());
            });
            job.join();
            finish(4);
        });
    }

    // @Test
    void test_resume_and_resume_with_exception() {
        run_test([this]() {
            Continuation<void>* continuation = nullptr;
            auto job = launch([this, &continuation]() {
                expect(2);
                suspend_cancellable_coroutine<void>([&continuation](auto c) {
                    continuation = &c;
                });
                expect(3);
            });
            expect(1);
            yield();
            continuation->resume();
            job.join();
            assert_fails_with<IllegalStateException>([&]() {
                continuation->resume_with_exception(TestException());
            });
            finish(4);
        });
    }

    // @Test
    void test_resume_and_resume() {
        run_test([this]() {
            Continuation<void>* continuation = nullptr;
            auto job = launch([this, &continuation]() {
                expect(2);
                suspend_cancellable_coroutine<void>([&continuation](auto c) {
                    continuation = &c;
                });
                expect(3);
            });
            expect(1);
            yield();
            continuation->resume();
            job.join();
            assert_fails_with<IllegalStateException>([&]() { continuation->resume(); });
            finish(4);
        });
    }

    /**
     * Cancelling outer job may, in practise, race with attempt to resume continuation and resumes
     * should be ignored. Here suspended coroutine is cancelled but then resumed with exception.
     */
    // @Test
    void test_cancel_and_resume_with_exception() {
        run_test([this]() {
            Continuation<void>* continuation = nullptr;
            auto job = launch([this, &continuation]() {
                try {
                    expect(2);
                    suspend_cancellable_coroutine<void>([&continuation](auto c) {
                        continuation = &c;
                    });
                } catch (const CancellationException& e) {
                    expect(3);
                }
            });
            expect(1);
            yield();
            job.cancel(); // Cancel job
            yield();
            continuation->resume_with_exception(TestException()); // Should not fail
            finish(4);
        });
    }

    /**
     * Cancelling outer job may, in practise, race with attempt to resume continuation and resumes
     * should be ignored. Here suspended coroutine is cancelled but then resumed with exception.
     */
    // @Test
    void test_cancel_and_resume() {
        run_test([this]() {
            Continuation<void>* continuation = nullptr;
            auto job = launch([this, &continuation]() {
                try {
                    expect(2);
                    suspend_cancellable_coroutine<void>([&continuation](auto c) {
                        continuation = &c;
                    });
                } catch (const CancellationException& e) {
                    expect(3);
                }
            });
            expect(1);
            yield();
            job.cancel(); // Cancel job
            yield();
            continuation->resume(); // Should not fail
            finish(4);
        });
    }

    // @Test
    void test_complete_job_while_suspended() {
        run_test([this]() {
            expect(1);
            auto completable_job = Job();
            auto coroutine_block = []() {
                assert_fails_with<CancellationException>([]() {
                    suspend_cancellable_coroutine<void>([](auto cont) {
                        expect(2);
                        assert_same(completable_job, cont.context[Job()]);
                        completable_job.complete();
                    });
                    expect_unreached();
                });
                expect(3);
            };
            coroutine_block.start_coroutine(Continuation(completable_job, [this](auto it) {
                assert_equals(std::monostate{}, it.get_or_null());
                expect(4);
            }));
            finish(5);
        });
    }
};

} // namespace coroutines
} // namespace kotlinx
