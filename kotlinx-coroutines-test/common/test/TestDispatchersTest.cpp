// Original file: kotlinx-coroutines-test/common/test/TestDispatchersTest.kt
// TODO: Remove or convert import statements
// TODO: Convert @Test, @BeforeTest, @AfterTest annotations to appropriate test framework
// TODO: Convert suspend functions and coroutine builders
// TODO: Handle OrderedExecutionTestBase inheritance
// TODO: Convert inner class syntax

namespace kotlinx {
namespace coroutines {
namespace test {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.test.internal.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.test.*

class TestDispatchersTest : public OrderedExecutionTestBase {
public:
    // TODO: @BeforeTest
    void set_up() {
        Dispatchers::set_main(StandardTestDispatcher());
    }

    // TODO: @AfterTest
    void tear_down() {
        Dispatchers::reset_main();
    }

    /** Tests that asynchronous execution of tests does not happen concurrently with [AfterTest]. */
    // TODO: @Test
    void test_main_mocking() {
        run_test([&]() {
            auto main_at_start = TestMainDispatcher::current_test_dispatcher;
            assert(main_at_start != nullptr);
            with_context(Dispatchers::main(), [&]() {
                delay(10);
            });
            with_context(Dispatchers::default_dispatcher(), [&]() {
                delay(10);
            });
            with_context(Dispatchers::main(), [&]() {
                delay(10);
            });
            // TODO: assertSame
            assert(main_at_start == TestMainDispatcher::current_test_dispatcher);
        });
    }

    /** Tests that the mocked [Dispatchers.Main] correctly forwards [Delay] methods. */
    // TODO: @Test
    void test_mocked_main_implements_delay() {
        run_test([&]() {
            auto main = Dispatchers::main();
            with_context(main, [&]() {
                delay(10);
            });
            with_context(Dispatchers::default_dispatcher(), [&]() {
                delay(10);
            });
            with_context(main, [&]() {
                delay(10);
            });
        });
    }

    /** Tests that [Dispatchers.setMain] fails when called with [Dispatchers.Main]. */
    // TODO: @Test
    void test_self_set() {
        // TODO: assertFailsWith<IllegalArgumentException>
        try {
            Dispatchers::set_main(Dispatchers::main());
            assert(false && "should have thrown");
        } catch (const std::invalid_argument& e) {
            // expected
        }
    }

    // TODO: @Test
    void test_immediate_dispatcher() {
        run_test([&]() {
            Dispatchers::set_main(new ImmediateDispatcher());
            expect(1);
            with_context(Dispatchers::main(), [&]() {
                expect(3);
            });

            Dispatchers::set_main(new RegularDispatcher());
            with_context(Dispatchers::main(), [&]() {
                expect(6);
            });

            finish(7);
        });
    }

private:
    class ImmediateDispatcher : public CoroutineDispatcher {
    private:
        TestDispatchersTest* outer_;
    public:
        ImmediateDispatcher(TestDispatchersTest* outer) : outer_(outer) {}

        bool is_dispatch_needed(const CoroutineContext& context) override {
            outer_->expect(2);
            return false;
        }

        void dispatch(const CoroutineContext& context, Runnable block) override {
            throw std::runtime_error("Shouldn't be reached");
        }
    };

    class RegularDispatcher : public CoroutineDispatcher {
    private:
        TestDispatchersTest* outer_;
    public:
        RegularDispatcher(TestDispatchersTest* outer) : outer_(outer) {}

        bool is_dispatch_needed(const CoroutineContext& context) override {
            outer_->expect(4);
            return true;
        }

        void dispatch(const CoroutineContext& context, Runnable block) override {
            outer_->expect(5);
            block.run();
        }
    };
};

} // namespace test
} // namespace coroutines
} // namespace kotlinx
