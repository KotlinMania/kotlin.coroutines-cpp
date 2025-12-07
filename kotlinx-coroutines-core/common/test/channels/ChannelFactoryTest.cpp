// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/ChannelFactoryTest.kt
//
// TODO: Translate imports
// TODO: Translate test annotations to C++ test framework

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.test.*

class ChannelFactoryTest : public TestBase {
public:
    // TODO: @Test
    void testRendezvousChannel() {
        // TODO: assertIs<BufferedChannel<int>*>(Channel<int>());
        // TODO: assertIs<BufferedChannel<int>*>(Channel<int>(0));
    }

    // TODO: @Test
    void testUnlimitedChannel() {
        // TODO: assertIs<BufferedChannel<int>*>(Channel<int>(Channel::UNLIMITED));
        // TODO: assertIs<BufferedChannel<int>*>(Channel<int>(Channel::UNLIMITED, BufferOverflow::DROP_OLDEST));
        // TODO: assertIs<BufferedChannel<int>*>(Channel<int>(Channel::UNLIMITED, BufferOverflow::DROP_LATEST));
    }

    // TODO: @Test
    void testConflatedChannel() {
        // TODO: assertIs<ConflatedBufferedChannel<int>*>(Channel<int>(Channel::CONFLATED));
        // TODO: assertIs<ConflatedBufferedChannel<int>*>(Channel<int>(1, BufferOverflow::DROP_OLDEST));
    }

    // TODO: @Test
    void testBufferedChannel() {
        // TODO: assertIs<BufferedChannel<int>*>(Channel<int>(1));
        // TODO: assertIs<ConflatedBufferedChannel<int>*>(Channel<int>(1, BufferOverflow::DROP_LATEST));
        // TODO: assertIs<BufferedChannel<int>*>(Channel<int>(10));
    }

    // TODO: @Test
    void testInvalidCapacityNotSupported() {
        // TODO: assertFailsWith<IllegalArgumentException>([]() { Channel<int>(-3); });
    }

    // TODO: @Test
    void testUnsupportedBufferOverflow() {
        // TODO: assertFailsWith<IllegalArgumentException>([]() { Channel<int>(Channel::CONFLATED, BufferOverflow::DROP_OLDEST); });
        // TODO: assertFailsWith<IllegalArgumentException>([]() { Channel<int>(Channel::CONFLATED, BufferOverflow::DROP_LATEST); });
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
