// Original: kotlinx-coroutines-core/common/test/JobExtensionsTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: Handle Job extension methods and CoroutineScope
// TODO: Map test framework annotations to C++ test framework

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlin.coroutines.*
        // TODO: import kotlin.test.*

        class JobExtensionsTest : public TestBase {
        private:
            Job *job = new Job();
            CoroutineScope *scope; // TODO: = CoroutineScope(job + CoroutineExceptionHandler { _, _ ->  });

        public:
            // TODO: @Test
            void test_is_active() {
                // TODO: runTest {
                expect(1);
                // TODO: scope->launch(Dispatchers.Unconfined) {
                //     ensure_active();
                //     coroutineContext.ensure_active();
                //     coroutineContext[Job]!!.ensure_active();
                expect(2);
                //     delay(Long.MAX_VALUE);
                // }

                expect(3);
                // TODO: job->ensure_active();
                // TODO: scope->ensure_active();
                // TODO: scope->coroutine_context().ensure_active();
                // TODO: job->cancel_and_join();
                finish(4);
                // TODO: }
            }

            // TODO: @Test
            void test_is_completed() {
                // TODO: runTest {
                expect(1);
                // TODO: scope->launch(Dispatchers.Unconfined) {
                //     ensure_active();
                //     coroutineContext.ensure_active();
                //     coroutineContext[Job]!!.ensure_active();
                expect(2);
                // }

                expect(3);
                // TODO: job->complete();
                // TODO: job->join();
                // TODO: assertFailsWith<JobCancellationException> { job->ensure_active(); }
                // TODO: assertFailsWith<JobCancellationException> { scope->ensure_active(); }
                // TODO: assertFailsWith<JobCancellationException> { scope->coroutine_context().ensure_active(); }
                finish(4);
                // TODO: }
            }

            // TODO: @Test
            void test_is_cancelled() {
                // TODO: runTest {
                expect(1);
                // TODO: scope->launch(Dispatchers.Unconfined) {
                //     ensure_active();
                //     coroutineContext.ensure_active();
                //     coroutineContext[Job]!!.ensure_active();
                expect(2);
                //     throw TestException();
                // }

                expect(3);
                // TODO: check_exception([&]() { job->ensure_active(); });
                // TODO: check_exception([&]() { scope->ensure_active(); });
                // TODO: check_exception([&]() { scope->coroutine_context().ensure_active(); });
                finish(4);
                // TODO: }
            }

            // TODO: @Test
            void test_ensure_active_with_empty_context() {
                // TODO: runTest {
                //     with_empty_context {
                //         ensure_active(); // should not do anything
                //     }
                // }
            }

        private:
            template<typename F>
            void check_exception(F block) {
                // TODO: const auto result = run_catching(block);
                // TODO: const auto exception = result.exception_or_null() ?: fail();
                // TODO: assertIs<JobCancellationException>(exception);
                // TODO: assertIs<TestException>(exception.cause);
            }

        public:
            // TODO: @Test
            void test_job_extension() {
                // TODO: runTest {
                // TODO: assertSame(coroutineContext[Job]!!, coroutineContext.job);
                // TODO: assertSame(NonCancellable, NonCancellable.job);
                // TODO: assertSame(job, job->job);
                // TODO: assertFailsWith<IllegalStateException> { EmptyCoroutineContext.job }
                // TODO: assertFailsWith<IllegalStateException> { Dispatchers.Default.job }
                // TODO: assertFailsWith<IllegalStateException> { (Dispatchers.Default + CoroutineName("")).job }
                // TODO: }
            }
        };
    } // namespace coroutines
} // namespace kotlinx