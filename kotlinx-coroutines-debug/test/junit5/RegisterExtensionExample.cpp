// Original: kotlinx-coroutines-debug/test/junit5/RegisterExtensionExample.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement JUnit5 test framework integration (@Test, @RegisterExtension)
// TODO: Convert suspend functions to C++ coroutine equivalents
// TODO: Map runBlocking to C++ blocking coroutine runner
// TODO: Implement CoroutinesTimeoutExtension.seconds factory method

namespace kotlinx {
namespace coroutines {
namespace debug {
namespace junit5 {

// TODO: import kotlinx.coroutines.*
// TODO: import org.junit.jupiter.api.*
// TODO: import org.junit.jupiter.api.extension.*

class RegisterExtensionExample {
public:
    // TODO: @JvmField
    // TODO: @RegisterExtension
    CoroutinesTimeoutExtension timeout = CoroutinesTimeoutExtension::seconds(5);

    // TODO: @Test
    void testThatHangs() {
        runBlocking([&] {
            delay(LONG_MAX); // somewhere deep in the stack
        });
    }
};

} // namespace junit5
} // namespace debug
} // namespace coroutines
} // namespace kotlinx
