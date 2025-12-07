// Original file: kotlinx-coroutines-core/common/test/flow/operators/OnEmptyTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations

namespace kotlinx {
namespace coroutines {
namespace flow {

class OnEmptyTest : public TestBase {
public:
    // @Test
    void test_on_empty_invoked() /* TODO: = runTest */ {
        auto flow = empty_flow<int>().on_empty([]() /* TODO: suspend */ { emit(1); });
        assert_equals(1, flow.single());
    }

    // @Test
    void test_on_empty_not_invoked() /* TODO: = runTest */ {
        auto flow = flow_of(1).on_empty([]() /* TODO: suspend */ { emit(2); });
        assert_equals(1, flow.single());
    }

    // @Test
    void test_on_empty_not_invoked_on_error() /* TODO: = runTest */ {
        auto flow = flow<int>([]() /* TODO: suspend */ {
            throw TestException();
        }).on_empty([]() { expect_unreached(); });
        assert_fails_with<TestException>(flow);
    }

    // @Test
    void test_on_empty_not_invoked_on_cancellation() /* TODO: = runTest */ {
        auto flow = flow<int>([]() /* TODO: suspend */ {
            expect(2);
            hang([]() { expect(4); });
        }).on_empty([]() { expect_unreached(); });

        expect(1);
        auto job = flow.on_each([](int) { expect_unreached(); }).launch_in(*this);
        yield();
        expect(3);
        job.cancel_and_join();
        finish(5);
    }

    // @Test
    void test_on_empty_cancellation() /* TODO: = runTest */ {
        auto flow = empty_flow<int>().on_empty([]() /* TODO: suspend */ {
            expect(2);
            hang([]() { expect(4); });
            emit(1);
        });
        expect(1);
        auto job = flow.on_each([](int) { expect_unreached(); }).launch_in(*this);
        yield();
        expect(3);
        job.cancel_and_join();
        finish(5);
    }

    // @Test
    void test_transparency_violation() /* TODO: = runTest */ {
        auto flow = empty_flow<int>().on_empty([]() /* TODO: suspend */ {
            expect(2);
            coroutine_scope([&]() /* TODO: suspend */ {
                launch([&]() /* TODO: suspend */ {
                    try {
                        emit(1);
                    } catch (const IllegalStateException& e) {
                        expect(3);
                    }
                });
            });
        });
        expect(1);
        assert_null(flow.single_or_null());
        finish(4);
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
