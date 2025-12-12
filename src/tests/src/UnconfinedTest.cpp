// Original: kotlinx-coroutines-core/common/test/UnconfinedTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: Handle Unconfined dispatcher execution order
// TODO: Map test framework annotations to C++ test framework

namespace kotlinx {
    namespace coroutines {
        class UnconfinedTest : public TestBase {
        public:
            // TODO: @Test
            void test_order() {
                // TODO: runTest {
                expect(1);
                // TODO: launch(Dispatchers.Unconfined) {
                expect(2);
                // TODO: launch {
                expect(4);
                // TODO: launch {
                expect(6);
                // }
                // TODO: launch {
                expect(7);
                // }
                expect(5);
                // }
                expect(3);
                // }
                finish(8);
                // TODO: }
            }

            // TODO: @Test
            void test_block_throws() {
                // TODO: runTest {
                expect(1);
                // TODO: try {
                //     with_context(Dispatchers.Unconfined) {
                expect(2);
                //         with_context(Dispatchers.Unconfined + CoroutineName("a")) {
                expect(3);
                //         }
                expect(4);
                //         launch(start = CoroutineStart.ATOMIC) {
                expect(5);
                //         }
                //         throw TestException();
                //     }
                // } catch (const TestException& e) {
                finish(6);
                // }
                // TODO: }
            }

            // TODO: @Test
            void test_enter_multiple_times() {
                // TODO: runTest {
                // TODO: launch(Unconfined) {
                expect(1);
                // }
                // TODO: launch(Unconfined) {
                expect(2);
                // }
                // TODO: launch(Unconfined) {
                expect(3);
                // }
                finish(4);
                // TODO: }
            }

            // TODO: @Test
            void test_yield() {
                // TODO: runTest {
                expect(1);
                // TODO: launch(Dispatchers.Unconfined) {
                expect(2);
                // TODO: yield();
                // TODO: launch {
                expect(4);
                // }
                expect(3);
                // TODO: yield();
                expect(5);
                // }.join();
                finish(6);
                // TODO: }
            }

            // TODO: @Test
            void test_cancellation_with_yields() {
                // TODO: runTest {
                expect(1);
                // TODO: GlobalScope.launch(Dispatchers.Unconfined) {
                //     const auto job = coroutineContext[Job]!!;
                expect(2);
                //     yield();
                //     GlobalScope.launch(Dispatchers.Unconfined) {
                expect(4);
                //         job.cancel();
                expect(5);
                //     }
                expect(3);
                //     try {
                //         yield();
                //     } finally {
                expect(6);
                //     }
                // }
                finish(7);
                // TODO: }
            }
        };
    } // namespace coroutines
} // namespace kotlinx