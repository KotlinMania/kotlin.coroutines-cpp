// Transliterated from Kotlin to C++
// Original: ui/kotlinx-coroutines-swing/test/SwingTest.kt
// TODO: Resolve imports and dependencies
// TODO: Implement JUnit test framework equivalents
// TODO: Handle Swing platform initialization
// TODO: Implement MainDispatcherTestBase base class

namespace kotlinx {
namespace coroutines {
namespace swing {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import org.junit.*
// TODO: import org.junit.Test
// TODO: import javax.swing.*
// TODO: import kotlin.test.*

class SwingTest : public MainDispatcherTestBase::WithRealTimeDelay {
public:
    // TODO: @Before annotation
    void setup() {
        ignore_lost_threads("AWT-EventQueue-");
    }

    bool is_main_thread() override {
        return SwingUtilities::is_event_dispatch_thread();
    }

    void schedule_on_main_queue(std::function<void()> block) override {
        SwingUtilities::invoke_later([block]() { block(); });
    }

    // Tests that the Main dispatcher is in fact the JavaFx one.
    // TODO: @Test annotation
    void test_main_is_java_fx() {
        assert_same(Dispatchers::Swing, Dispatchers::Main);
    }
};

} // namespace swing
} // namespace coroutines
} // namespace kotlinx
