// Original Kotlin package: kotlinx.coroutines.debug.junit4
// Line-by-line C++ transliteration from Kotlin
//
// TODO: JUnit4 TestRule, Statement, Description - Java testing framework types
// TODO: @get:Rule annotation - Kotlin property annotation for JUnit rules
// TODO: @JvmOverloads - Kotlin annotation for generating overloaded constructors
// TODO: companion object - Kotlin static members container
// TODO: TimeUnit - Java time unit, use C++ chrono
// TODO: require() - Kotlin precondition check, use assert or throw
// TODO: DebugProbes - needs implementation

#include <stdexcept>
#include <chrono>

// Forward declarations - JUnit4 types would need C++ testing framework equivalent
// TODO: These are Java JUnit4 types - no direct C++ equivalent

namespace kotlinx {
namespace coroutines {
namespace debug {
namespace junit4 {

// Forward declarations
class CoroutinesTimeoutStatement;

// Coroutines timeout rule for JUnit4 that is applied to all methods in the class.
// This rule is very similar to Timeout rule: it runs tests in a separate thread,
// fails tests after the given timeout and interrupts test thread.
//
// Additionally, this rule installs DebugProbes and dumps all coroutines at the moment of the timeout.
// It may cancel coroutines on timeout if cancelOnTimeout set to `true`.
// enableCoroutineCreationStackTraces controls the corresponding DebugProbes.enableCreationStackTraces property
// and can be optionally enabled if the creation stack traces are necessary.
//
// Example of usage:
// ```
// class HangingTest {
//     @get:Rule
//     val timeout = CoroutinesTimeout.seconds(5)
//
//     @Test
//     fun testThatHangs() = runBlocking {
//          ...
//          delay(Long.MAX_VALUE) // somewhere deep in the stack
//          ...
//     }
// }
// ```
class CoroutinesTimeout /* TODO: : TestRule */ {
public:
    CoroutinesTimeout(
        long test_timeout_ms,
        bool cancel_on_timeout = false,
        bool enable_coroutine_creation_stack_traces = false
    )
        : test_timeout_ms_(test_timeout_ms)
        , cancel_on_timeout_(cancel_on_timeout)
        , enable_coroutine_creation_stack_traces_(enable_coroutine_creation_stack_traces)
    {
        // TODO: require(testTimeoutMs > 0)
        if (test_timeout_ms <= 0) {
            throw std::invalid_argument("Expected positive test timeout, but had " + std::to_string(test_timeout_ms));
        }

        // Install probes in the constructor, so all the coroutines launched from within
        // target test constructor will be captured

        // Do not preserve previous state for unit-test environment
        // TODO: DebugProbes.enableCreationStackTraces = enable_coroutine_creation_stack_traces_;
        // TODO: DebugProbes.install();
    }

    // TODO: @Suppress("UNUSED") // Binary compatibility
    CoroutinesTimeout(long test_timeout_ms, bool cancel_on_timeout = false)
        : CoroutinesTimeout(test_timeout_ms, cancel_on_timeout, true)
    {
    }

    // TODO: companion object -> static methods

    // Creates CoroutinesTimeout rule with the given timeout in seconds.
    // TODO: @JvmOverloads
    static CoroutinesTimeout seconds(
        int seconds,
        bool cancel_on_timeout = false,
        bool enable_coroutine_creation_stack_traces = true
    ) {
        return seconds(static_cast<long>(seconds), cancel_on_timeout, enable_coroutine_creation_stack_traces);
    }

    // Creates CoroutinesTimeout rule with the given timeout in seconds.
    // TODO: @JvmOverloads
    static CoroutinesTimeout seconds(
        long seconds,
        bool cancel_on_timeout = false,
        bool enable_coroutine_creation_stack_traces = true
    ) {
        // TODO: TimeUnit.SECONDS.toMillis(seconds) - use chrono
        long timeout_ms = seconds * 1000; // Overflow is properly handled by TimeUnit
        return CoroutinesTimeout(timeout_ms, cancel_on_timeout, enable_coroutine_creation_stack_traces);
    }

    // @suppress suppress from Dokka
    // TODO: override fun apply(base: Statement, description: Description): Statement
    // TODO: JUnit4 TestRule interface - no C++ equivalent
    // Statement apply(Statement base, Description description) {
    //     return CoroutinesTimeoutStatement(base, description, test_timeout_ms_, cancel_on_timeout_);
    // }

private:
    long test_timeout_ms_;
    bool cancel_on_timeout_;
    bool enable_coroutine_creation_stack_traces_;
};

} // namespace junit4
} // namespace debug
} // namespace coroutines
} // namespace kotlinx
