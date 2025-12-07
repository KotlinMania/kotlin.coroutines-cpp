// Original: ui/kotlinx-coroutines-android/android-unit-tests/test/ordered/tests/FirstRobolectricTest.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement Robolectric test runner integration
// TODO: Map @RunWith, @Config, @LooperMode annotations
// TODO: Implement ShadowLooper API
// TODO: Convert Dispatchers.setMain/resetMain to C++ equivalents
// TODO: Convert suspend functions to C++ coroutine equivalents

namespace ordered {
namespace tests {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.test.*
// TODO: import org.junit.Test
// TODO: import org.junit.runner.*
// TODO: import org.robolectric.*
// TODO: import org.robolectric.annotation.*
// TODO: import org.robolectric.shadows.*
// TODO: import kotlin.test.*

// TODO: @RunWith(RobolectricTestRunner::class)
// TODO: @Config(manifest = Config.NONE, sdk = [28])
// TODO: @LooperMode(LooperMode.Mode.LEGACY)
class FirstRobolectricTest {
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

    // TODO: @Test
    void testDelay() {
        TestComponent component;
        auto main_looper = ShadowLooper::getShadowMainLooper();
        main_looper.pause();
        component.launch_delayed();
        main_looper.runToNextTask();
        assertFalse(component.delayed_launch_completed);
        main_looper.runToNextTask();
        assertTrue(component.delayed_launch_completed);
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
