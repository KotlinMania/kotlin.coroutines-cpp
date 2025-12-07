// Original: ui/kotlinx-coroutines-android/android-unit-tests/test/ordered/tests/FirstMockedMainTest.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement JUnit @Before, @After, @Test annotations
// TODO: Convert TestBase inheritance
// TODO: Convert Dispatchers.setMain/resetMain to C++ equivalents
// TODO: Implement exception message checking (contains)
// TODO: Convert nullable assertion operator (!!)

namespace ordered {
namespace tests {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.test.*
// TODO: import org.junit.*
// TODO: import org.junit.Test
// TODO: import java.lang.IllegalStateException
// TODO: import kotlin.test.*

class FirstMockedMainTest : public TestBase {
public:
    // TODO: @Before
    void setUp() {
        Dispatchers::setMain(Dispatchers.Unconfined);
    }

    // TODO: @After
    void tearDown() {
        Dispatchers::resetMain();
    }

    // TODO: @Test
    void testComponent() {
        TestComponent component;
        component.launch_something();
        assertTrue(component.launch_completed);
    }

    // TODO: @Test
    void testFailureWhenReset() {
        Dispatchers::resetMain();
        TestComponent component;
        try {
            component.launch_something();
            throw component.caught_exception;
        } catch (const std::invalid_argument& e) {
            assertTrue(std::string(e.what()).find("Dispatchers.setMain") != std::string::npos);
        }
    }
};

} // namespace tests
} // namespace ordered
