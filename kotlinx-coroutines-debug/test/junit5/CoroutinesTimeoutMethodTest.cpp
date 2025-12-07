// Original: kotlinx-coroutines-debug/test/junit5/CoroutinesTimeoutMethodTest.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement JUnit5 test framework integration (@Test, @TestMethodOrder, @Order)
// TODO: Map @CoroutinesTimeout annotation to C++ equivalent
// TODO: Convert suspend functions to C++ coroutine equivalents
// TODO: Map runBlocking to C++ blocking coroutine runner
// TODO: Implement MethodOrderer.OrderAnnotation equivalent

namespace kotlinx {
namespace coroutines {
namespace debug {
namespace junit5 {

// TODO: import kotlinx.coroutines.*
// TODO: import org.junit.jupiter.api.*

/**
 * Tests usage of [CoroutinesTimeout] on classes and test methods when only methods are annotated.
 *
 * This test class is not intended to be run manually. Instead, use [CoroutinesTimeoutTest] as the entry point.
 */
// TODO: @TestMethodOrder(MethodOrderer.OrderAnnotation::class)
class CoroutinesTimeoutMethodTest {
public:
    // TODO: @Test
    // TODO: @Order(1)
    void noClassTimeout() {
        runBlocking([&] {
            delay(150);
        });
    }

    // TODO: @CoroutinesTimeout(100)
    // TODO: @Test
    // TODO: @Order(2)
    void usesMethodTimeoutWithNoClassTimeout() {
        runBlocking([&] {
            delay(1000);
        });
    }

    // TODO: @CoroutinesTimeout(1000)
    // TODO: @Test
    // TODO: @Order(3)
    void fitsInMethodTimeout() {
        runBlocking([&] {
            delay(10);
        });
    }
};

} // namespace junit5
} // namespace debug
} // namespace coroutines
} // namespace kotlinx
