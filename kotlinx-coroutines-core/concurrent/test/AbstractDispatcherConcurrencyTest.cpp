// Original: kotlinx-coroutines-core/concurrent/test/AbstractDispatcherConcurrencyTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement runTest, runBlocking, GlobalScope.launch
// TODO: Implement CoroutineDispatcher interface
// TODO: Implement expect/finish test utilities

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlin.test.*

class AbstractDispatcherConcurrencyTest : public TestBase {
public:
    // TODO: Convert abstract property to pure virtual function
    virtual CoroutineDispatcher* dispatcher() = 0;

    // @Test
    // TODO: Convert test annotation
    void test_launch_and_join() {
        // TODO: Implement runTest
        // runTest {
        expect(1);
        int captured_mutable_state = 0;
        auto job = GlobalScope::launch(dispatcher(), [&]() {
            // TODO: suspend function
            ++captured_mutable_state;
            expect(2);
        });
        runBlocking([&]() {
            // TODO: suspend function
            job.join();
        });
        assertEquals(1, captured_mutable_state);
        finish(3);
        // }
    }

    // @Test
    // TODO: Convert test annotation
    void test_dispatcher_has_own_threads() {
        // TODO: Implement runTest
        // runTest {
        auto channel = Channel<int>();
        GlobalScope::launch(dispatcher(), [&]() {
            // TODO: suspend function
            channel.send(42);
        });

        auto result = ChannelResult<int>::failure();
        while (!result.is_success()) {
            result = channel.try_receive();
            // Block the thread, wait
        }
        // Delivery was successful, let's check it
        assertEquals(42, result.get_or_throw());
        // }
    }

    // @Test
    // TODO: Convert test annotation
    void test_delay_in_dispatcher() {
        // TODO: Implement runTest
        // runTest {
        expect(1);
        auto job = GlobalScope::launch(dispatcher(), [&]() {
            // TODO: suspend function
            expect(2);
            delay(100);
            expect(3);
        });
        runBlocking([&]() {
            // TODO: suspend function
            job.join();
        });
        finish(4);
        // }
    }
};

} // namespace coroutines
} // namespace kotlinx
