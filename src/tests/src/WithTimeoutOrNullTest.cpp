// Original: kotlinx-coroutines-core/common/test/WithTimeoutOrNullTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED") // KT-21913
// TODO: Handle withTimeoutOrNull with Long parameter (milliseconds)
// TODO: Map test framework annotations to C++ test framework

#include <optional>
#include <cstdint>

namespace kotlinx {
    namespace coroutines {
        class WithTimeoutOrNullTest : public TestBase {
        public:
            // TODO: @Test - Tests a case of no timeout and no suspension inside.
            void test_basic_no_suspend() {
                /* expect(1); const auto result = with_timeout_or_null(10000) { expect(2); "OK"; } assertEquals("OK", result); finish(3); */
            }

            // TODO: @Test - Tests a case of no timeout and one suspension inside.
            void test_basic_suspend() {
                /* expect(1); const auto result = with_timeout_or_null(10000) { expect(2); yield(); expect(3); "OK"; } assertEquals("OK", result); finish(4); */
            }

            // TODO: @Test - Tests property dispatching of withTimeoutOrNull blocks
            void test_dispatch() {
                /* expect(1); launch { expect(4); yield(); expect(7); } expect(2); const auto result = with_timeout_or_null(1000) { expect(3); yield(); expect(5); "OK"; } assertEquals("OK", result); expect(6); yield(); finish(8); */
            }

            // TODO: @Test - Tests that a 100% CPU-consuming loop will react on timeout if it has yields.
            void test_yield_blocking_with_timeout() {
                /* expect(1); const auto result = with_timeout_or_null(100) { while (true) { yield(); } } assertNull(result); finish(2); */
            }

            // TODO: @Test
            void test_small_timeout() {
                /* const auto channel = Channel<int>(1); const auto value = with_timeout_or_null(1) { channel.receive(); } assertNull(value); */
            }

            // TODO: @Test
            void test_throw_exception() {
                /* runTest(expected = {it is AssertionError}) { with_timeout_or_null<void>(INT64_MAX) { throw std::runtime_error(); } } */
            }

            // TODO: @Test
            void test_inner_timeout() {
                /* runTest(expected = { it is CancellationException }) { with_timeout_or_null(1000) { with_timeout(10) { while (true) { yield(); } } expectUnreached(); } expectUnreached(); } */
            }

            // TODO: @Test
            void test_nested_timeout() {
                /* runTest(expected = { it is TimeoutCancellationException }) { with_timeout_or_null(INT64_MAX) { with_timeout(10) { delay(INT64_MAX); 1; } } expectUnreached(); } */
            }

            // TODO: @Test
            void test_outer_timeout() {
                /* if (is_java_and_windows) return; int counter = 0; const auto result = with_timeout_or_null(320) { while (true) { const auto inner = with_timeout_or_null(150) { while (true) { yield(); } } assertNull(inner); counter++; } } assertNull(result); check(counter >= 1 && counter <= 2); */
            }

            // TODO: @Test
            void test_bad_class() {
                /* const auto bad = BadClass(); const auto result = with_timeout_or_null(100) { bad; } assertSame(bad, result); */
            }

            // TODO: @Test
            void test_null_on_timeout() {
                /* expect(1); const auto result = with_timeout_or_null(100) { expect(2); delay(1000); expectUnreached(); "OK"; } assertNull(result); finish(3); */
            }

            // TODO: @Test
            void test_suppress_exception_with_result() {
                /* expect(1); const auto result = with_timeout_or_null(100) { expect(2); try { delay(1000); } catch (...) { expect(3); } "OK"; } assertNull(result); finish(4); */
            }

            // TODO: @Test
            void test_suppress_exception_with_another_exception() {
                /* expect(1); try { with_timeout_or_null(100) { expect(2); try { delay(1000); } catch (...) { expect(3); throw TestException(); } expectUnreached(); "OK"; } expectUnreached(); } catch (const TestException& e) { finish(4); } */
            }

            // TODO: @Test
            void test_negative_timeout() {
                /* expect(1); std::optional<void*> result = with_timeout_or_null(-1) { expectUnreached(); } assertNull(result); result = with_timeout_or_null(0) { expectUnreached(); } assertNull(result); finish(2); */
            }

            // TODO: @Test
            void test_exception_from_within_timeout() {
                /* expect(1); try { expect(2); with_timeout_or_null(1000) { expect(3); throw TestException(); } expectUnreached(); } catch (const TestException& e) { finish(4); } */
            }
        };
    } // namespace coroutines
} // namespace kotlinx