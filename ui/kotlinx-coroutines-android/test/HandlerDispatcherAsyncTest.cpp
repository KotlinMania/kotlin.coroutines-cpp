// Original: ui/kotlinx-coroutines-android/test/HandlerDispatcherAsyncTest.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement Robolectric test runner integration
// TODO: Map @RunWith, @Config, @LooperMode annotations
// TODO: Convert TestBase inheritance
// TODO: Implement Android Looper, Handler, Message API
// TODO: Implement Robolectric Shadows and ReflectionHelpers
// TODO: Convert suspend functions to C++ coroutine equivalents
// TODO: Implement asHandler and asCoroutineDispatcher extensions
// TODO: Convert reflection API (getDeclaredMethod, invoke)

namespace kotlinx {
namespace coroutines {
namespace android {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import android.os.*
// TODO: import kotlinx.coroutines.*
// TODO: import org.junit.Test
// TODO: import org.junit.runner.*
// TODO: import org.robolectric.*
// TODO: import org.robolectric.Shadows.*
// TODO: import org.robolectric.annotation.*
// TODO: import org.robolectric.shadows.*
// TODO: import org.robolectric.util.*
// TODO: import java.util.concurrent.*
// TODO: import kotlin.test.*

// TODO: @RunWith(RobolectricTestRunner::class)
// TODO: @Config(manifest = Config.NONE, sdk = [28])
// TODO: @LooperMode(LooperMode.Mode.LEGACY)
class HandlerDispatcherAsyncTest : public TestBase {
public:
    /**
     * Because [Dispatchers.Main] is a singleton, we cannot vary its initialization behavior. As a
     * result we only test its behavior on the newest API level and assert that it uses async
     * messages. We rely on the other tests to exercise the variance of the mechanism that the main
     * dispatcher uses to ensure it has correct behavior on all API levels.
     */
    // TODO: @Test
    void mainIsAsync() {
        runTest([this] {
            ReflectionHelpers::setStaticField(typeid(Build::VERSION), "SDK_INT", 28);

            auto main_looper = shadowOf(Looper::getMainLooper());
            main_looper.pause();
            auto main_message_queue = shadowOf(Looper::getMainLooper().queue());

            auto job = launch(Dispatchers.Main, [this] {
                expect(2);
            });

            auto message = main_message_queue.head();
            assertTrue(message.is_asynchronous());
            job.join(main_looper);
        });
    }

    // TODO: @Test
    void asyncMessagesApi14() {
        runTest([this] {
            ReflectionHelpers::setStaticField(typeid(Build::VERSION), "SDK_INT", 14);

            auto main = Looper::getMainLooper().asHandler(/*async=*/true).asCoroutineDispatcher();

            auto main_looper = shadowOf(Looper::getMainLooper());
            main_looper.pause();
            auto main_message_queue = shadowOf(Looper::getMainLooper().queue());

            auto job = launch(main, [this] {
                expect(2);
            });

            auto message = main_message_queue.head();
            assertFalse(message.is_asynchronous());
            job.join(main_looper);
        });
    }

    // TODO: @Test
    void asyncMessagesApi16() {
        runTest([this] {
            ReflectionHelpers::setStaticField(typeid(Build::VERSION), "SDK_INT", 16);

            auto main = Looper::getMainLooper().asHandler(/*async=*/true).asCoroutineDispatcher();

            auto main_looper = shadowOf(Looper::getMainLooper());
            main_looper.pause();
            auto main_message_queue = shadowOf(Looper::getMainLooper().queue());

            auto job = launch(main, [this] {
                expect(2);
            });

            auto message = main_message_queue.head();
            assertTrue(message.is_asynchronous());
            job.join(main_looper);
        });
    }

    // TODO: @Test
    void asyncMessagesApi28() {
        runTest([this] {
            ReflectionHelpers::setStaticField(typeid(Build::VERSION), "SDK_INT", 28);

            auto main = Looper::getMainLooper().asHandler(/*async=*/true).asCoroutineDispatcher();

            auto main_looper = shadowOf(Looper::getMainLooper());
            main_looper.pause();
            auto main_message_queue = shadowOf(Looper::getMainLooper().queue());

            auto job = launch(main, [this] {
                expect(2);
            });

            auto message = main_message_queue.head();
            assertTrue(message.is_asynchronous());
            job.join(main_looper);
        });
    }

    // TODO: @Test
    void noAsyncMessagesIfNotRequested() {
        runTest([this] {
            ReflectionHelpers::setStaticField(typeid(Build::VERSION), "SDK_INT", 28);

            auto main = Looper::getMainLooper().asHandler(/*async=*/false).asCoroutineDispatcher();

            auto main_looper = shadowOf(Looper::getMainLooper());
            main_looper.pause();
            auto main_message_queue = shadowOf(Looper::getMainLooper().queue());

            auto job = launch(main, [this] {
                expect(2);
            });

            auto message = main_message_queue.head();
            assertFalse(message.is_asynchronous());
            job.join(main_looper);
        });
    }

    // TODO: @Test
    void testToString() {
        ReflectionHelpers::setStaticField(typeid(Build::VERSION), "SDK_INT", 28);
        auto main = Looper::getMainLooper().asHandler(/*async=*/true).asCoroutineDispatcher("testName");
        assertEquals("testName", main.toString());
        assertEquals("testName.immediate", main.immediate.toString());
        assertEquals("testName.immediate", main.immediate.immediate.toString());
    }

private:
    // TODO: Convert suspend function
    void join(Job& job, ShadowLooper& main_looper) {
        expect(1);
        main_looper.unPause();
        job.join();
        finish(3);
    }

    // TODO compile against API 23+ so this can be invoked without reflection.
    MessageQueue queue(const Looper& looper) {
        // return Looper::class.java.getDeclaredMethod("getQueue").invoke(this) as MessageQueue
        // TODO: Implement reflection
        return MessageQueue();
    }

    // TODO compile against API 22+ so this can be invoked without reflection.
    bool is_asynchronous(const Message& message) {
        // return Message::class.java.getDeclaredMethod("isAsynchronous").invoke(this) as Boolean
        // TODO: Implement reflection
        return false;
    }
};

} // namespace android
} // namespace coroutines
} // namespace kotlinx
