// Original: kotlinx-coroutines-core/common/test/DispatchedContinuationTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: Handle suspend functions and coroutine machinery
// TODO: Implement Continuation type
// TODO: Map test framework annotations to C++ test framework

#include <string>


    namespace kotlinx::coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlin.coroutines.*
        // TODO: import kotlin.test.*

        /**
 * When using suspendCoroutine from the standard library the continuation must be dispatched atomically,
 * without checking for cancellation at any point in time.
 */
        class DispatchedContinuationTest : public TestBase {
        private:
            DispatchedContinuationTest::Continuation<std::string> *cont{};

        public:
            // TODO: @Test
            static void test_cancel_then_resume() {
                // TODO: runTest {
                expect(1);
                // TODO: launch(start = CoroutineStart.UNDISPATCHED) {
                expect(2);
                // TODO: coroutineContext[Job]!!.cancel()
                // a regular suspendCoroutine will still suspend despite the fact that coroutine was cancelled
                // TODO: const auto value = suspendCoroutine<std::string>([&](auto it) {
                expect(3);
                cont = it;
                // TODO: });
                expect(6);
                // TODO: assertEquals("OK", value);
                // TODO: }
                expect(4);
                // TODO: cont.resume("OK");
                expect(5);
                // TODO: yield(); // to the launched job
                finish(7);
                // TODO: }
            }

            // TODO: @Test
            void test_cancel_then_resume_unconfined() {
                // TODO: runTest {
                expect(1);
                // TODO: launch(Dispatchers.Unconfined) {
                expect(2);
                // TODO: coroutineContext[Job]!!.cancel()
                // a regular suspendCoroutine will still suspend despite the fact that coroutine was cancelled
                // TODO: const auto value = suspendCoroutine<std::string>([&](auto it) {
                expect(3);
                cont = it;
                // TODO: });
                expect(5);
                // TODO: assertEquals("OK", value);
                // TODO: }
                expect(4);
                // TODO: cont.resume("OK"); // immediately resumes -- because unconfined
                finish(6);
                // TODO: }
            }

            // TODO: @Test
            void test_resume_then_cancel() {
                // TODO: runTest {
                expect(1);
                // TODO: const auto job = launch(start = CoroutineStart.UNDISPATCHED) {
                expect(2);
                // TODO: const auto value = suspendCoroutine<std::string>([&](auto it) {
                expect(3);
                cont = it;
                // TODO: });
                expect(7);
                // TODO: assertEquals("OK", value);
                // TODO: }
                expect(4);
                // TODO: cont.resume("OK");
                expect(5);
                // now cancel the job, which the coroutine is waiting to be dispatched
                // TODO: job.cancel();
                expect(6);
                // TODO: yield(); // to the launched job
                finish(8);
                // TODO: }
            }
        };
    } // namespace kotlinx::coroutines
