// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/BroadcastChannelFactoryTest.kt
//
// TODO: Translate imports
// TODO: Translate test annotations to C++ test framework
// TODO: Handle @Suppress annotations

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*

// TODO: @Suppress("DEPRECATION_ERROR")
class BroadcastChannelFactoryTest : public TestBase {
public:

    // TODO: @Test
    void testRendezvousChannelNotSupported() {
        // TODO: assertFailsWith<IllegalArgumentException>([]() { BroadcastChannel<int>(0); });
    }

    // TODO: @Test
    void testUnlimitedChannelNotSupported() {
        // TODO: assertFailsWith<IllegalArgumentException>([]() { BroadcastChannel<int>(Channel::UNLIMITED); });
    }

    // TODO: @Test
    void testConflatedBroadcastChannel() {
        // TODO: assertTrue(/* BroadcastChannel<int>(Channel::CONFLATED) is ConflatedBroadcastChannel */);
    }

    // TODO: @Test
    void testBufferedBroadcastChannel() {
        // TODO: assertTrue(/* BroadcastChannel<int>(1) is BroadcastChannelImpl */);
        // TODO: assertTrue(/* BroadcastChannel<int>(10) is BroadcastChannelImpl */);
    }

    // TODO: @Test
    void testInvalidCapacityNotSupported() {
        // TODO: assertFailsWith<IllegalArgumentException>([]() { BroadcastChannel<int>(-3); });
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
