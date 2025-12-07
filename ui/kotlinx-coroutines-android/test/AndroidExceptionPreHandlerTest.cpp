// Original: ui/kotlinx-coroutines-android/test/AndroidExceptionPreHandlerTest.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement Robolectric test runner integration
// TODO: Map @RunWith, @Config, @LooperMode annotations
// TODO: Convert TestBase inheritance
// TODO: Implement Thread.getDefaultUncaughtExceptionHandler API
// TODO: Convert suspend functions to C++ coroutine equivalents
// TODO: Implement assertIs template function

namespace kotlinx {
namespace coroutines {
namespace android {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import org.junit.Test
// TODO: import org.junit.runner.*
// TODO: import org.robolectric.*
// TODO: import org.robolectric.annotation.*
// TODO: import kotlin.test.*

// TODO: @RunWith(RobolectricTestRunner::class)
// TODO: @Config(manifest = Config.NONE, sdk = [27])
// TODO: @LooperMode(LooperMode.Mode.LEGACY)
class AndroidExceptionPreHandlerTest : public TestBase {
public:
    // TODO: @Test
    void testUnhandledException() {
        runTest([this] {
            auto previous_handler = std::get_terminate();
            try {
                std::set_terminate([this] {
                    expect(3);
                    // TODO: assertIs<TestException>(e)
                });
                expect(1);
                GlobalScope.launch(Dispatchers.Main, [this] {
                    expect(2);
                    throw TestException();
                }).join();
                finish(4);
            } catch (...) {
                std::set_terminate(previous_handler);
                throw;
            }
            std::set_terminate(previous_handler);
        });
    }
};

} // namespace android
} // namespace coroutines
} // namespace kotlinx
