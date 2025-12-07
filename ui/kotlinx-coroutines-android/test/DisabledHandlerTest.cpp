// Original: ui/kotlinx-coroutines-android/test/DisabledHandlerTest.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement Robolectric test runner integration
// TODO: Map @RunWith, @Config, @LooperMode annotations
// TODO: Convert TestBase inheritance
// TODO: Implement Android Handler and Message API
// TODO: Convert object expression to C++ class/lambda
// TODO: Convert suspend functions to C++ coroutine equivalents
// TODO: Implement asCoroutineDispatcher for Handler

namespace kotlinx {
namespace coroutines {
namespace android {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import android.os.*
// TODO: import kotlinx.coroutines.*
// TODO: import org.junit.*
// TODO: import org.junit.runner.*
// TODO: import org.robolectric.*
// TODO: import org.robolectric.annotation.*

// TODO: @RunWith(RobolectricTestRunner::class)
// TODO: @Config(manifest = Config.NONE, sdk = [28])
// TODO: @LooperMode(LooperMode.Mode.LEGACY)
class DisabledHandlerTest : public TestBase {
private:
    bool delegate_to_super_ = false;

    // TODO: Convert object expression to C++ class
    class DisabledHandler : public Handler {
    private:
        DisabledHandlerTest& parent_;

    public:
        explicit DisabledHandler(DisabledHandlerTest& parent) : parent_(parent) {}

        bool sendMessageAtTime(Message* msg, long uptime_millis) override {
            if (parent_.delegate_to_super_) {
                return Handler::sendMessageAtTime(msg, uptime_millis);
            }
            return false;
        }
    };

    CoroutineDispatcher disabled_dispatcher_ = DisabledHandler(*this).asCoroutineDispatcher();

public:
    // TODO: @Test
    void testRunBlocking() {
        expect(1);
        try {
            runBlocking(disabled_dispatcher_, [] {
                expectUnreached();
            });
            expectUnreached();
        } catch (const CancellationException& e) {
            finish(2);
        }
    }

    // TODO: @Test
    void testInvokeOnCancellation() {
        runTest([this] {
            auto job = launch(disabled_dispatcher_, CoroutineStart::LAZY, [] {
                expectUnreached();
            });
            job.invokeOnCompletion([](auto it) {
                if (it != nullptr) expect(2);
            });
            yield();
            expect(1);
            job.join();
            finish(3);
        });
    }

    // TODO: @Test
    void testWithTimeout() {
        runTest([this] {
            delegate_to_super_ = true;
            try {
                withContext(disabled_dispatcher_, [this] {
                    expect(1);
                    delegate_to_super_ = false;
                    delay(LONG_MAX - 1);
                    expectUnreached();
                });
                expectUnreached();
            } catch (const CancellationException& e) {
                finish(2);
            }
        });
    }
};

} // namespace android
} // namespace coroutines
} // namespace kotlinx
