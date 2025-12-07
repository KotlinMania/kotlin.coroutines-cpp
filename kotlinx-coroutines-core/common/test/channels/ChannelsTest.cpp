// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/ChannelsTest.kt
//
// TODO: Translate file-level annotations (@file:Suppress)
// TODO: Translate imports
// TODO: Translate suspend functions to C++ coroutines
// TODO: Translate test annotations to C++ test framework

// TODO: @file:Suppress("DEPRECATION")

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.math.*
// TODO: import kotlin.test.*

class ChannelsTest : public TestBase {
private:
    // TODO: std::vector<int> testList = {1, 2, 3};

public:
    // TODO: @Test
    void testIterableAsReceiveChannel() /* = runTest */ {
        // TODO: assertEquals(testList, testList.asReceiveChannel().toList());
    }

    // TODO: @Test
    void testCloseWithMultipleSuspendedReceivers() /* = runTest */ {
        // Once the channel is closed, the waiting
        // requests should be cancelled in the order
        // they were suspended in the channel.
        // TODO: Implementation
    }

    // TODO: @Test
    void testCloseWithMultipleSuspendedSenders() /* = runTest */ {
        // Once the channel is closed, the waiting
        // requests should be cancelled in the order
        // they were suspended in the channel.
        // TODO: Implementation
    }

    // TODO: @Test
    void testEmptyList() /* = runTest */ {
        // TODO: assertTrue(std::vector<int>{}.asReceiveChannel().toList().empty());
    }

    // TODO: @Test
    void testToList() /* = runTest */ {
        // TODO: assertEquals(testList, testList.asReceiveChannel().toList());
    }

    // TODO: @Test
    void testToListOnFailedChannel() /* = runTest */ {
        // TODO: auto channel = Channel<int>();
        // TODO: channel.close(TestException());
        // TODO: assertFailsWith<TestException>([&]() {
        //     channel.toList();
        // });
    }

private:
    template<typename E>
    /* ReceiveChannel<E> */ void* asReceiveChannel(/* const std::vector<E>& iterable, CoroutineContext context = Dispatchers::Unconfined */) {
        // TODO: return GlobalScope.produce(context, [&]() {
        //     for (const auto& element : iterable)
        //         send(element);
        // });
        return nullptr;
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
