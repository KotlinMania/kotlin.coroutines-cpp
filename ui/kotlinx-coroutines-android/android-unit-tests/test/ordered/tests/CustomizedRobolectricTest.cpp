// Original: ui/kotlinx-coroutines-android/android-unit-tests/test/ordered/tests/CustomizedRobolectricTest.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement Robolectric test runner integration
// TODO: Map @Config, @RunWith, @LooperMode annotations
// TODO: Convert TestBase inheritance
// TODO: Implement ShadowLooper API
// TODO: Convert Dispatchers.setMain/resetMain to C++ equivalents
// TODO: Implement GlobalScope.launch with exception handler

namespace ordered {
namespace tests {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.test.*
// TODO: import org.junit.Test
// TODO: import org.junit.runner.*
// TODO: import org.robolectric.*
// TODO: import org.robolectric.annotation.*
// TODO: import org.robolectric.shadows.*
// TODO: import kotlin.test.*

class InitMainDispatcherBeforeRobolectricTestRunner : public RobolectricTestRunner {
public:
    explicit InitMainDispatcherBeforeRobolectricTestRunner(const std::type_info& test_class)
        : RobolectricTestRunner(test_class) {
        // TODO: Convert kotlin.runCatching
        try {
            // touch Main, watch it burn
            GlobalScope.launch(Dispatchers.Main + CoroutineExceptionHandler([](auto, auto) { })) { };
        } catch (...) {
            // Silently catch exceptions
        }
    }
};

// TODO: @Config(manifest = Config.NONE, sdk = [28])
// TODO: @RunWith(InitMainDispatcherBeforeRobolectricTestRunner::class)
// TODO: @LooperMode(LooperMode.Mode.LEGACY)
class CustomizedRobolectricTest : public TestBase {
public:
    // TODO: @Test
    void testComponent() {
        // Note that main is not set at all
        TestComponent component;
        check_component(component);
    }

    // TODO: @Test
    void testComponentAfterReset() {
        // Note that main is not set at all
        TestComponent component;
        Dispatchers::setMain(Dispatchers.Unconfined);
        Dispatchers::resetMain();
        check_component(component);
    }

private:
    void check_component(TestComponent& component) {
        auto main_looper = ShadowLooper::getShadowMainLooper();
        main_looper.pause();
        component.launch_something();
        assertFalse(component.launch_completed);
        main_looper.unPause();
        assertTrue(component.launch_completed);
    }
};

} // namespace tests
} // namespace ordered
