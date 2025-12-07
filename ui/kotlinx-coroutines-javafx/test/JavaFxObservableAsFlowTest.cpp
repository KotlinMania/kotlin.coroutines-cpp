// Transliterated from Kotlin to C++
// Original: ui/kotlinx-coroutines-javafx/test/JavaFxObservableAsFlowTest.kt
// TODO: Resolve imports and dependencies
// TODO: Implement JUnit test framework equivalents
// TODO: Handle JavaFX SimpleIntegerProperty and Flow integration
// TODO: Implement TestBase base class
// TODO: Handle suspend functions and coroutines

namespace kotlinx {
namespace coroutines {
namespace javafx {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import javafx.beans.property.SimpleIntegerProperty
// TODO: import kotlinx.coroutines.testing.TestBase
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.flow.*
// TODO: import org.junit.Before
// TODO: import org.junit.Test
// TODO: import kotlin.test.*

class JavaFxObservableAsFlowTest : public TestBase {
public:
    // TODO: @Before annotation
    void setup() {
        ignore_lost_threads("JavaFX Application Thread", "Thread-", "QuantumRenderer-", "InvokeLaterDispatcher");
    }

    // TODO: @Test annotation
    // TODO: suspend function - runTest
    void test_flow_order() {
        // TODO: suspend function implementation
        if (!init_platform()) {
            std::cout << "Skipping JavaFxTest in headless environment" << std::endl;
            return; // ignore test in headless environments
        }

        auto integer_property = SimpleIntegerProperty(0);
        auto n = 1000;
        auto flow = integer_property.as_flow().take_while([n](auto j) { return j != n; });
        // TODO: use(pool) pattern with newSingleThreadContext
        new_single_thread_context("setter").use([&](auto pool) {
            launch(pool, [&]() {
                for (int i = 1; i <= n; i++) {
                    launch(Dispatchers::JavaFx, [&, i]() {
                        integer_property.set(i);
                    });
                }
            });
            int i = -1;
            flow.collect([&](auto j) {
                assert_true(i < static_cast<int>(j), "Elements are neither repeated nor shuffled");
                i = j;
            });
        });
    }

    // TODO: @Test annotation
    // TODO: suspend function - runTest
    void test_conflation() {
        // TODO: suspend function implementation
        if (!init_platform()) {
            std::cout << "Skipping JavaFxTest in headless environment" << std::endl;
            return; // ignore test in headless environments
        }

        with_context(Dispatchers::JavaFx, [&]() {
            const int kEndMarker = -1;
            auto integer_property = SimpleIntegerProperty(0);
            auto flow = integer_property.as_flow().take_while([kEndMarker](auto j) { return j != kEndMarker; });
            launch([&]() {
                yield(); // to subscribe to [integerProperty]
                yield(); // send 0
                integer_property.set(1);
                expect(3);
                yield(); // send 1
                expect(5);
                integer_property.set(2);
                for (int i = -100; i <= -2; i++) {
                    integer_property.set(i); // should be skipped due to conflation
                }
                integer_property.set(3);
                expect(6);
                yield(); // send 2 and 3
                integer_property.set(-1);
            });
            expect(1);
            flow.collect([&](auto i) {
                switch (i) {
                    case 0: expect(2); break;
                    case 1: expect(4); break;
                    case 2: expect(7); break;
                    case 3: expect(8); break;
                    default: fail("i is " + std::to_string(i));
                }
            });
            finish(9);
        });
    }

    // TODO: @Test annotation
    // TODO: suspend function - runTest
    void test_intermediate_crash() {
        // TODO: suspend function implementation
        if (!init_platform()) {
            std::cout << "Skipping JavaFxTest in headless environment" << std::endl;
            return; // ignore test in headless environments
        }

        auto property = SimpleIntegerProperty(0);

        assert_fails_with<TestException>([&]() {
            property.as_flow().on_each([&](auto it) {
                yield();
                throw TestException();
            }).collect();
        });
    }
};

} // namespace javafx
} // namespace coroutines
} // namespace kotlinx
