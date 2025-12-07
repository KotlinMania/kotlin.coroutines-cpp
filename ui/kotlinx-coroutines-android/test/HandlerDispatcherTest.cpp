// Original: ui/kotlinx-coroutines-android/test/HandlerDispatcherTest.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement Robolectric test runner integration
// TODO: Map @RunWith, @LooperMode, @Config annotations
// TODO: Convert MainDispatcherTestBase inheritance
// TODO: Implement Android Looper and Handler API
// TODO: Implement Robolectric Shadows and ShadowChoreographer
// TODO: Convert suspend functions to C++ coroutine equivalents
// TODO: Implement hang helper function
// TODO: Implement awaitFrame API

namespace kotlinx {
namespace coroutines {
namespace android {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import android.os.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.testing.*
// TODO: import org.junit.Test
// TODO: import org.junit.runner.*
// TODO: import org.robolectric.*
// TODO: import org.robolectric.annotation.*
// TODO: import org.robolectric.shadows.*
// TODO: import java.util.concurrent.*
// TODO: import kotlin.test.*

// TODO: @RunWith(RobolectricTestRunner::class)
// TODO: @LooperMode(LooperMode.Mode.LEGACY)
// TODO: @Config(manifest = Config.NONE, sdk = [28])
class HandlerDispatcherTest : public MainDispatcherTestBase::WithRealTimeDelay {
public:
    // TODO: @Test
    void testDefaultDelayIsNotDelegatedToMain() {
        runTest([this] {
            auto main_looper = Shadows::shadowOf(Looper::getMainLooper());
            main_looper.pause();
            assertFalse(main_looper.scheduler().areAnyRunnable());

            auto job = launch(Dispatchers.Default, CoroutineStart::UNDISPATCHED, [this] {
                expect(1);
                delay(LONG_MAX);
                expectUnreached();
            });
            expect(2);
            assertEquals(0, main_looper.scheduler().size());
            job.cancelAndJoin();
            finish(3);
        });
    }

    // TODO: @Test
    void testWithTimeoutIsDelegatedToMain() {
        runTest([this] {
            auto main_looper = Shadows::shadowOf(Looper::getMainLooper());
            main_looper.pause();
            assertFalse(main_looper.scheduler().areAnyRunnable());
            auto job = launch(Dispatchers.Main, CoroutineStart::UNDISPATCHED, [this] {
                withTimeout(1, [this] {
                    expect(1);
                    hang([this] { expect(3); });
                });
                expectUnreached();
            });
            expect(2);
            assertEquals(1, main_looper.scheduler().size());
            // Schedule cancellation
            main_looper.runToEndOfTasks();
            job.join();
            finish(4);
        });
    }

    // TODO: @Test
    void testDelayDelegatedToMain() {
        runTest([this] {
            auto main_looper = Shadows::shadowOf(Looper::getMainLooper());
            main_looper.pause();
            auto job = launch(Dispatchers.Main, CoroutineStart::UNDISPATCHED, [this] {
                expect(1);
                delay(1);
                expect(3);
            });
            expect(2);
            assertEquals(1, main_looper.scheduler().size());
            // Schedule cancellation
            main_looper.runToEndOfTasks();
            job.join();
            finish(4);
        });
    }

    // TODO: @Test
    void testAwaitFrame() {
        runTest([this] {
            do_test_await_frame();

            reset();

            // Now the second test: we cannot test it separately because we're caching choreographer in HandlerDispatcher
            do_test_await_with_detected_choreographer();
        });
    }

private:
    void do_test_await_frame() {
        ShadowChoreographer::setPostFrameCallbackDelay(100);
        auto main_looper = Shadows::shadowOf(Looper::getMainLooper());
        main_looper.pause();
        launch(Dispatchers.Main, CoroutineStart::UNDISPATCHED, [this] {
            expect(1);
            awaitFrame();
            expect(3);
        });
        expect(2);
        // Run choreographer detection
        main_looper.runOneTask();
        finish(4);
    }

    void do_test_await_with_detected_choreographer() {
        ShadowChoreographer::setPostFrameCallbackDelay(100);
        auto main_looper = Shadows::shadowOf(Looper::getMainLooper());
        launch(Dispatchers.Main, CoroutineStart::UNDISPATCHED, [this] {
            expect(1);
            awaitFrame();
            expect(4);
        });
        // Run choreographer detection
        expect(2);
        main_looper.scheduler().advanceBy(50, TimeUnit::MILLISECONDS);
        expect(3);
        main_looper.scheduler().advanceBy(51, TimeUnit::MILLISECONDS);
        finish(5);
    }

public:
    bool isMainThread() override {
        return Looper::getMainLooper().thread() == std::this_thread::get_id();
    }

    void scheduleOnMainQueue(std::function<void()> block) override {
        Handler(Looper::getMainLooper()).post(block);
    }

    // by default, Robolectric only schedules tasks on the main thread but doesn't run them.
    // This function nudges it to run them, 10 milliseconds of virtual time at a time.
    // TODO: Convert suspend function
    void spinTest(Job& test_body) override {
        auto main_looper = Shadows::shadowOf(Looper::getMainLooper());
        while (test_body.isActive()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            main_looper.idleFor(10, TimeUnit::MILLISECONDS);
        }
    }
};

} // namespace android
} // namespace coroutines
} // namespace kotlinx
