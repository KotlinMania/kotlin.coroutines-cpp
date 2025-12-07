// Original: kotlinx-coroutines-debug/test/junit5/CoroutinesTimeoutExtensionTest.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement JUnit5 test framework integration (@Test, @RegisterExtension)
// TODO: Convert nested classes to C++ nested classes or separate classes
// TODO: Convert suspend functions to C++ coroutine equivalents
// TODO: Map runBlocking to C++ blocking coroutine runner
// TODO: Convert GlobalScope.launch to C++ equivalent
// TODO: Implement CoroutinesTimeoutExtension

namespace kotlinx {
namespace coroutines {
namespace debug {
namespace junit5 {

// TODO: import kotlinx.coroutines.*
// TODO: import org.junit.jupiter.api.Test
// TODO: import org.junit.jupiter.api.extension.*
// TODO: import org.junit.jupiter.api.parallel.*

class CoroutinesTimeoutExtensionTest {
public:
    /**
     * Tests that disabling coroutine creation stacktraces in [CoroutinesTimeoutExtension] does lead to them not being
     * created.
     *
     * Adapted from [CoroutinesTimeoutDisabledTracesTest], an identical test for the JUnit4 rule.
     *
     * This test class is not intended to be run manually. Instead, use [CoroutinesTimeoutTest] as the entry point.
     */
    class DisabledStackTracesTest {
    public:
        // TODO: @JvmField
        // TODO: @RegisterExtension
        CoroutinesTimeoutExtension timeout = CoroutinesTimeoutExtension(500, true, false);

    private:
        Job job = GlobalScope.launch(Dispatchers.Unconfined) { hangForever(); };

        // TODO: Convert suspend function
        void hangForever() {
            suspendCancellableCoroutine<void>([] {  });
            expectUnreached();
        }

    public:
        // TODO: @Test
        void hangingTest() {
            runBlocking<void>([&] {
                waitForHangJob();
                expectUnreached();
            });
        }

    private:
        // TODO: Convert suspend function
        void waitForHangJob() {
            job.join();
            expectUnreached();
        }
    };

    /**
     * Tests that [CoroutinesTimeoutExtension] is installed eagerly and detects the coroutines that were launched before
     * any test events start happening.
     *
     * Adapted from [CoroutinesTimeoutEagerTest], an identical test for the JUnit4 rule.
     *
     * This test class is not intended to be run manually. Instead, use [CoroutinesTimeoutTest] as the entry point.
     */
    class EagerTest {
    public:
        // TODO: @JvmField
        // TODO: @RegisterExtension
        CoroutinesTimeoutExtension timeout = CoroutinesTimeoutExtension(500);

    private:
        Job job = GlobalScope.launch(Dispatchers.Unconfined) { hangForever(); };

        // TODO: Convert suspend function
        void hangForever() {
            suspendCancellableCoroutine<void>([] {  });
            expectUnreached();
        }

    public:
        // TODO: @Test
        void hangingTest() {
            runBlocking<void>([&] {
                waitForHangJob();
                expectUnreached();
            });
        }

    private:
        // TODO: Convert suspend function
        void waitForHangJob() {
            job.join();
            expectUnreached();
        }
    };

    /**
     * Tests that [CoroutinesTimeoutExtension] performs sensibly in some simple scenarios.
     *
     * Adapted from [CoroutinesTimeoutTest], an identical test for the JUnit4 rule.
     *
     * This test class is not intended to be run manually. Instead, use [CoroutinesTimeoutTest] as the entry point.
     */
    class SimpleTest {
    public:
        // TODO: @JvmField
        // TODO: @RegisterExtension
        CoroutinesTimeoutExtension timeout = CoroutinesTimeoutExtension(1000, false, true);

        // TODO: @Test
        void hangingTest() {
            runBlocking<void>([&] {
                suspendForever();
                expectUnreached();
            });
        }

    private:
        // TODO: Convert suspend function
        void suspendForever() {
            delay(LONG_MAX);
            expectUnreached();
        }

    public:
        // TODO: @Test
        void throwingTest() {
            runBlocking<void>([] {
                throw RuntimeException();
            });
        }

        // TODO: @Test
        void successfulTest() {
            runBlocking([&] {
                auto job = launch([&] {
                    yield();
                });

                job.join();
            });
        }
    };
};

void expectUnreached() {
    throw std::runtime_error("Should not be reached");
}

} // namespace junit5
} // namespace debug
} // namespace coroutines
} // namespace kotlinx
