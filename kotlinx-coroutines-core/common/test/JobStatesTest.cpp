// Original: kotlinx-coroutines-core/common/test/JobStatesTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED") // KT-21913
// TODO: Handle Job state transitions and lifecycle
// TODO: Map test framework annotations to C++ test framework

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*

/**
 * Tests that the transitions to the state of the Job correspond to documentation in the
 * table that is presented in the Job documentation.
 */
class JobStatesTest : public TestBase {
public:
    // TODO: @Test
    void test_normal_completion() {
        // TODO: runTest {
        expect(1);
        // TODO: const auto parent = coroutineContext.job;
        // TODO: const auto job = launch(start = CoroutineStart.LAZY) {
            expect(2);
            // launches child
            // TODO: launch {
                expect(4);
            // }
            // completes normally
        // }
        // New job
        // TODO: assertFalse(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // TODO: assertSame(parent, job.parent());
        // New -> Active
        // TODO: job.start();
        // TODO: assertTrue(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // TODO: assertSame(parent, job.parent());
        // Active -> Completing
        // TODO: yield(); // scheduled & starts child
        expect(3);
        // TODO: assertTrue(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // TODO: assertSame(parent, job.parent());
        // Completing -> Completed
        // TODO: yield();
        finish(5);
        // TODO: assertFalse(job.is_active());
        // TODO: assertTrue(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // TODO: assertNull(job.parent());
        // TODO: }
    }

    // TODO: @Test
    void test_completing_failed() {
        // TODO: runTest(unhandled = listOf({ it -> it is TestException })) {
        expect(1);
        // TODO: const auto job = launch(NonCancellable, start = CoroutineStart.LAZY) {
            expect(2);
            // launches child
            // TODO: launch {
                expect(4);
                // TODO: throw TestException();
            // }
            // completes normally
        // }
        // New job
        // TODO: assertFalse(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // New -> Active
        // TODO: job.start();
        // TODO: assertTrue(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // Active -> Completing
        // TODO: yield(); // scheduled & starts child
        expect(3);
        // TODO: assertTrue(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // Completing -> Cancelled
        // TODO: yield();
        finish(5);
        // TODO: assertFalse(job.is_active());
        // TODO: assertTrue(job.is_completed());
        // TODO: assertTrue(job.is_cancelled());
        // TODO: }
    }

    // TODO: @Test
    void test_failed() {
        // TODO: runTest(unhandled = listOf({ it -> it is TestException })) {
        expect(1);
        // TODO: const auto job = launch(NonCancellable, start = CoroutineStart.LAZY) {
            expect(2);
            // launches child
            // TODO: launch(start = CoroutineStart.ATOMIC) {
                expect(4);
            // }
            // failing
            // TODO: throw TestException();
        // }
        // New job
        // TODO: assertFalse(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // New -> Active
        // TODO: job.start();
        // TODO: assertTrue(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // Active -> Cancelling
        // TODO: yield(); // scheduled & starts child
        expect(3);
        // TODO: assertFalse(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertTrue(job.is_cancelled());
        // Cancelling -> Cancelled
        // TODO: yield();
        finish(5);
        // TODO: assertFalse(job.is_active());
        // TODO: assertTrue(job.is_completed());
        // TODO: assertTrue(job.is_cancelled());
        // TODO: }
    }

    // TODO: @Test
    void test_cancelling() {
        // TODO: runTest {
        expect(1);
        // TODO: const auto job = launch(NonCancellable, start = CoroutineStart.LAZY) {
            expect(2);
            // launches child
            // TODO: launch(start = CoroutineStart.ATOMIC) {
                expect(4);
            // }
            // completes normally
        // }
        // New job
        // TODO: assertFalse(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // New -> Active
        // TODO: job.start();
        // TODO: assertTrue(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // Active -> Completing
        // TODO: yield(); // scheduled & starts child
        expect(3);
        // TODO: assertTrue(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertFalse(job.is_cancelled());
        // Completing -> Cancelling
        // TODO: job.cancel();
        // TODO: assertFalse(job.is_active());
        // TODO: assertFalse(job.is_completed());
        // TODO: assertTrue(job.is_cancelled());
        // Cancelling -> Cancelled
        // TODO: yield();
        finish(5);
        // TODO: assertFalse(job.is_active());
        // TODO: assertTrue(job.is_completed());
        // TODO: assertTrue(job.is_cancelled());
        // TODO: }
    }
};

} // namespace coroutines
} // namespace kotlinx
