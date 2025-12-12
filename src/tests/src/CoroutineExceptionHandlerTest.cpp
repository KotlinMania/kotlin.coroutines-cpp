// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/CoroutineExceptionHandlerTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlin.test.*

        class CoroutineExceptionHandlerTest : public TestBase {
        public:
            // Parent Job() does not handle exception --> handler is invoked on child crash
            // @Test
            void test_job() {
                run_test([this]() {
                    expect(1);
                    std::exception *coroutine_exception = nullptr;
                    auto handler = CoroutineExceptionHandler([&coroutine_exception, this](auto, auto ex) {
                        coroutine_exception = ex;
                        expect(3);
                    });
                    auto parent = Job();
                    auto job = launch(handler + parent, [this]() {
                        throw TestException();
                    });
                    expect(2);
                    job.join();
                    finish(4);
                    assert_is<TestException>(coroutine_exception);
                    assert_true(parent.is_cancelled());
                });
            }

            // Parent CompletableDeferred() "handles" exception --> handler is NOT invoked on child crash
            // @Test
            void test_completable_deferred() {
                run_test([this]() {
                    expect(1);
                    auto handler = CoroutineExceptionHandler([this](auto, auto) {
                        expect_unreached();
                    });
                    auto parent = CompletableDeferred<void>();
                    auto job = launch(handler + parent, [this]() {
                        throw TestException();
                    });
                    expect(2);
                    job.join();
                    finish(3);
                    assert_true(parent.is_cancelled());
                    assert_is<TestException>(parent.get_completion_exception_or_null());
                });
            }
        };
    } // namespace coroutines
} // namespace kotlinx