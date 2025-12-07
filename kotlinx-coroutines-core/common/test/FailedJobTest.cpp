// Original: kotlinx-coroutines-core/common/test/FailedJobTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED") // KT-21913
// TODO: Handle suspend functions and Job lifecycle
// TODO: Map test framework annotations to C++ test framework

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*

// see https://github.com/Kotlin/kotlinx.coroutines/issues/585
class FailedJobTest : public TestBase {
public:
    // TODO: @Test
    void test_cancelled_job() {
        // TODO: runTest {
        expect(1);
        // TODO: const auto job = launch {
        //     expect_unreached();
        // }
        expect(2);
        // TODO: job.cancelAndJoin();
        finish(3);
        // TODO: assertTrue(job.isCompleted);
        // TODO: assertTrue(!job.isActive);
        // TODO: assertTrue(job.isCancelled);
        // TODO: }
    }

    // TODO: @Test
    void test_failed_job() {
        // TODO: runTest(
        //     unhandled = listOf({it -> it is TestException })
        // ) {
        expect(1);
        // TODO: const auto job = launch(NonCancellable) {
            expect(3);
            // TODO: throw TestException();
        // }
        expect(2);
        // TODO: job.join();
        finish(4);
        // TODO: assertTrue(job.isCompleted);
        // TODO: assertTrue(!job.isActive);
        // TODO: assertTrue(job.isCancelled);
        // TODO: }
    }

    // TODO: @Test
    void test_failed_child_job() {
        // TODO: runTest(
        //     unhandled = listOf({it -> it is TestException })
        // ) {
        expect(1);
        // TODO: const auto job = launch(NonCancellable) {
            expect(3);
            // TODO: launch {
            //     throw TestException();
            // }
        // }
        expect(2);
        // TODO: job.join();
        finish(4);
        // TODO: assertTrue(job.isCompleted);
        // TODO: assertTrue(!job.isActive);
        // TODO: assertTrue(job.isCancelled);
        // TODO: }
    }
};

} // namespace coroutines
} // namespace kotlinx
