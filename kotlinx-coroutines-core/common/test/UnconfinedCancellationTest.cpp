// Original: kotlinx-coroutines-core/common/test/UnconfinedCancellationTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: Handle Unconfined dispatcher cancellation behavior
// TODO: Map test framework annotations to C++ test framework

namespace kotlinx {
namespace coroutines {

class UnconfinedCancellationTest : public TestBase {
public:
    // TODO: @Test
    void test_unconfined_cancellation() {
        // TODO: runTest {
        // TODO: const auto parent = Job();
        // TODO: launch(parent) {
            expect(1);
            // TODO: parent->cancel();
            // TODO: launch(Dispatchers.Unconfined) {
            //     expect_unreached();
            // }
        // }.join();
        finish(2);
        // TODO: }
    }

    // TODO: @Test
    void test_unconfined_cancellation_state() {
        // TODO: runTest {
        // TODO: const auto parent = Job();
        // TODO: launch(parent) {
            expect(1);
            // TODO: parent->cancel();
            // TODO: const auto job = launch(Dispatchers.Unconfined) {
            //     expect_unreached();
            // }
            // TODO: assertTrue(job.is_cancelled());
            // TODO: assertTrue(job.is_completed());
            // TODO: assertFalse(job.is_active());
        // }.join();
        finish(2);
        // TODO: }
    }

    // TODO: @Test
    void test_unconfined_cancellation_lazy() {
        // TODO: runTest {
        // TODO: const auto parent = Job();
        // TODO: launch(parent) {
            expect(1);
            // TODO: const auto job = launch(Dispatchers.Unconfined, start = CoroutineStart.LAZY) {
            //     expect_unreached();
            // }
            // TODO: job.invoke_on_completion { expect(2); }
            // TODO: assertFalse(job.is_completed());
            // TODO: parent->cancel();
            // TODO: job.join();
        // }.join();
        finish(3);
        // TODO: }
    }

    // TODO: @Test
    void test_undispatched_cancellation() {
        // TODO: runTest {
        // TODO: const auto parent = Job();
        // TODO: launch(parent) {
            expect(1);
            // TODO: parent->cancel();
            // TODO: launch(start = CoroutineStart.UNDISPATCHED) {
                expect(2);
                // TODO: yield();
                // expect_unreached();
            // }
        // }.join();
        finish(3);
        // TODO: }
    }

    // TODO: @Test
    void test_cancelled_atomic_unconfined() {
        // TODO: runTest {
        // TODO: const auto parent = Job();
        // TODO: launch(parent) {
            expect(1);
            // TODO: parent->cancel();
            // TODO: launch(Dispatchers.Unconfined, start = CoroutineStart.ATOMIC) {
                expect(2);
                // TODO: yield();
                // expect_unreached();
            // }
        // }.join();
        finish(3);
        // TODO: }
    }

    // TODO: @Test
    void test_cancelled_with_context_unconfined() {
        // TODO: runTest {
        // TODO: const auto parent = Job();
        // TODO: launch(parent) {
            expect(1);
            // TODO: parent->cancel();
            // TODO: with_context(Dispatchers.Unconfined) {
            //     expect_unreached();
            // }
        // }.join();
        finish(2);
        // TODO: }
    }
};

} // namespace coroutines
} // namespace kotlinx
