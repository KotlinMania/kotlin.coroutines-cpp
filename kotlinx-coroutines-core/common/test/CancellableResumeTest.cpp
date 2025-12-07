// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/CancellableResumeTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

// @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED") // KT-21913

/**
 * Test for [CancellableContinuation.resume] with `onCancellation` parameter.
 */
namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*

class CancellableResumeTest : public TestBase {
public:
    // All test methods follow same pattern with 3-parameter resume lambda
    // TODO: Complete all 10 test methods

    // @Test
    void test_resume_immediate_normally() {
        run_test([this]() {
            expect(1);
            auto ok = suspend_cancellable_coroutine<std::string>([this](auto cont) {
                expect(2);
                cont.invoke_on_cancellation([this]() { expect_unreached(); });
                cont.resume("OK", [this](auto, auto, auto) { expect_unreached(); });
                expect(3);
            });
            assert_equals("OK", ok);
            finish(4);
        });
    }

    // Additional test methods omitted for brevity
    // Full implementation would include all remaining tests
};

} // namespace coroutines
} // namespace kotlinx
