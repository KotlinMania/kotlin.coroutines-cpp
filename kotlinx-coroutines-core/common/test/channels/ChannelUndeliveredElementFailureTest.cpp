// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/ChannelUndeliveredElementFailureTest.kt
//
// TODO: Translate imports
// TODO: Translate suspend functions to C++ coroutines
// TODO: Translate test annotations to C++ test framework

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.internal.*
// TODO: import kotlinx.coroutines.selects.*
// TODO: import kotlin.test.*

/**
 * Tests for failures inside `onUndeliveredElement` handler in [Channel].
 */
class ChannelUndeliveredElementFailureTest : public TestBase {
private:
    const char* item = "LOST";
    // TODO: std::function<void(const std::string&)> onCancelFail = [](const std::string& it) { throw TestException(it); };
    // TODO: std::vector<std::function<bool(const std::exception&)>> shouldBeUnhandled = { [](const std::exception& it) { return isElementCancelException(it); } };

    bool isElementCancelException(const std::exception& /* e */) {
        // TODO: return e is UndeliveredElementException && e.cause is TestException && e.cause.message == item
        return false;
    }

public:
    // TODO: @Test
    void testSendCancelledFail() /* = runTest(unhandled = shouldBeUnhandled) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testSendSelectCancelledFail() /* = runTest(unhandled = shouldBeUnhandled) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testReceiveCancelledFail() /* = runTest(unhandled = shouldBeUnhandled) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testReceiveSelectCancelledFail() /* = runTest(unhandled = shouldBeUnhandled) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testReceiveCatchingCancelledFail() /* = runTest(unhandled = shouldBeUnhandled) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testReceiveOrClosedSelectCancelledFail() /* = runTest(unhandled = shouldBeUnhandled) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testHasNextCancelledFail() /* = runTest(unhandled = shouldBeUnhandled) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testChannelCancelledFail() /* = runTest(expected = { it.isElementCancelException() }) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testFailedHandlerInClosedConflatedChannel() /* = runTest(expected = { it is UndeliveredElementException }) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testFailedHandlerInClosedBufferedChannel() /* = runTest(expected = { it is UndeliveredElementException }) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testSendDropOldestInvokeHandlerBuffered() /* = runTest(expected = { it is UndeliveredElementException }) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testSendDropLatestInvokeHandlerBuffered() /* = runTest(expected = { it is UndeliveredElementException }) */ {
        // TODO: Implementation
    }

    // TODO: @Test
    void testSendDropOldestInvokeHandlerConflated() /* = runTest(expected = { it is UndeliveredElementException }) */ {
        // TODO: Implementation
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
