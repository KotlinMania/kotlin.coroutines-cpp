// Original: kotlinx-coroutines-core/common/test/LaunchLazyTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: Handle lazy launch and Job.start() semantics
// TODO: Map test framework annotations to C++ test framework

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlin.test.*

        class LaunchLazyTest : public TestBase {
        public:
            // TODO: @Test
            void test_launch_and_yield_join() {
                // TODO: runTest {
                expect(1);
                // TODO: const auto job = launch(start = CoroutineStart.LAZY) {
                expect(4);
                // TODO: yield(); // does nothing -- main waits
                expect(5);
                // }
                expect(2);
                // TODO: yield(); // does nothing, was not started yet
                expect(3);
                // TODO: assertTrue(!job.is_active() && !job.is_completed());
                // TODO: job.join();
                // TODO: assertTrue(!job.is_active() && job.is_completed());
                finish(6);
                // TODO: }
            }

            // TODO: @Test
            void test_start() {
                // TODO: runTest {
                expect(1);
                // TODO: const auto job = launch(start = CoroutineStart.LAZY) {
                expect(5);
                // TODO: yield(); // yields back to main
                expect(7);
                // }
                expect(2);
                // TODO: yield(); // does nothing, was not started yet
                expect(3);
                // TODO: assertTrue(!job.is_active() && !job.is_completed());
                // TODO: assertTrue(job.start());
                // TODO: assertTrue(job.is_active() && !job.is_completed());
                // TODO: assertTrue(!job.start()); // start again -- does nothing
                // TODO: assertTrue(job.is_active() && !job.is_completed());
                expect(4);
                // TODO: yield(); // now yield to started coroutine
                expect(6);
                // TODO: assertTrue(job.is_active() && !job.is_completed());
                // TODO: yield(); // yield again
                // TODO: assertTrue(!job.is_active() && job.is_completed()); // it completes this time
                expect(8);
                // TODO: job.join(); // immediately returns
                finish(9);
                // TODO: }
            }

            // TODO: @Test
            void test_invoke_on_completion_and_start() {
                // TODO: runTest {
                expect(1);
                // TODO: const auto job = launch(start = CoroutineStart.LAZY) {
                expect(5);
                // }
                // TODO: yield(); // no started yet!
                expect(2);
                // TODO: job.invoke_on_completion {
                expect(6);
                // }
                expect(3);
                // TODO: job.start();
                expect(4);
                // TODO: yield();
                finish(7);
                // TODO: }
            }
        };
    } // namespace coroutines
} // namespace kotlinx