// Original: kotlinx-coroutines-core/common/test/WithTimeoutDurationTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED", "UNREACHABLE_CODE") // KT-21913
// TODO: Handle withTimeout with Duration parameter
// TODO: Map test framework annotations to C++ test framework

namespace kotlinx {
    namespace coroutines {
        class WithTimeoutDurationTest : public TestBase {
        public:
            // TODO: @Test - Tests a case of no timeout and no suspension inside.
            void test_basic_no_suspend() {
                /* expect(1); const auto result = with_timeout(10.seconds) { expect(2); "OK"; } assertEquals("OK", result); finish(3); */
            }

            // TODO: @Test - Tests a case of no timeout and one suspension inside.
            void test_basic_suspend() {
                /* expect(1); const auto result = with_timeout(10.seconds) { expect(2); yield(); expect(3); "OK"; } assertEquals("OK", result); finish(4); */
            }

            // TODO: @Test - Tests proper dispatching of withTimeout blocks
            void test_dispatch() {
                /* expect(1); launch { expect(4); yield(); expect(7); } expect(2); const auto result = with_timeout(1.seconds) { expect(3); yield(); expect(5); "OK"; } assertEquals("OK", result); expect(6); yield(); finish(8); */
            }

            // TODO: @Test - Tests that a 100% CPU-consuming loop will react on timeout if it has yields.
            void test_yield_blocking_with_timeout() {
                /* runTest(expected = { it is CancellationException }) { with_timeout(100.milliseconds) { while (true) { yield(); } } } */
            }

            // TODO: @Test - Tests that withTimeout waits for children coroutines to complete.
            void test_with_timeout_child_wait() {
                /* expect(1); with_timeout(100.milliseconds) { expect(2); launch { expect(4); } expect(3); } finish(5); */
            }

            // TODO: @Test
            void test_bad_class() {
                /* const auto bad = BadClass(); const auto result = with_timeout(100.milliseconds) { bad; } assertSame(bad, result); */
            }

            // TODO: @Test
            void test_exception_on_timeout() {
                /* expect(1); try { with_timeout(100.milliseconds) { expect(2); delay(1000.milliseconds); expectUnreached(); "OK"; } } catch (const CancellationException& e) { assertEquals("Timed out waiting for 100 ms", e.message); finish(3); } */
            }

            // TODO: @Test
            void test_suppress_exception_with_result() {
                /* runTest(expected = { it is CancellationException }) { expect(1); with_timeout(100.milliseconds) { expect(2); try { delay(1000.milliseconds); } catch (...) { finish(3); } "OK"; } expectUnreached(); } */
            }

            // TODO: @Test
            void test_suppress_exception_with_another_exception() {
                /* expect(1); try { with_timeout(100.milliseconds) { expect(2); try { delay(1000.milliseconds); } catch (...) { expect(3); throw TestException(); } expectUnreached(); "OK"; } expectUnreached(); } catch (const TestException& e) { finish(4); } */
            }

            // TODO: @Test
            void test_negative_timeout() {
                /* expect(1); try { with_timeout(-1.milliseconds) { expectUnreached(); "OK"; } } catch (const TimeoutCancellationException& e) { assertEquals("Timed out immediately", e.message); finish(2); } */
            }

            // TODO: @Test
            void test_exception_from_within_timeout() {
                /* expect(1); try { expect(2); with_timeout(1.seconds) { expect(3); throw TestException(); } expectUnreached(); } catch (const TestException& e) { finish(4); } */
            }

            // TODO: @Test
            void test_incomplete_with_timeout_state() {
                /* Job* timeout_job = nullptr; const auto handle = with_timeout(Duration::INFINITE) { timeout_job = &coroutineContext[Job]!!; timeout_job->invoke_on_completion([]{  }); } handle.dispose(); timeout_job->join(); assertTrue(timeout_job->is_completed()); assertFalse(timeout_job->is_active()); assertFalse(timeout_job->is_cancelled()); */
            }

        private:
            struct BadClass {
                // TODO: bool operator==(const BadClass&) const { error("Should not be called"); }
                // TODO: size_t hash() const { error("Should not be called"); }
                // TODO: std::string to_string() const { error("Should not be called"); }
            };
        };
    } // namespace coroutines
} // namespace kotlinx