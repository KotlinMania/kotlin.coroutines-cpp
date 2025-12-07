// Original file: kotlinx-coroutines-core/common/test/flow/operators/TakeWhileTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations

namespace kotlinx {
namespace coroutines {
namespace flow {

class TakeWhileTest : public TestBase {
public:
    // @Test
    void test_take_while() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            emit(1);
            emit(2);
        });

        assert_equals(3, flow.take_while([](int) { return true; }).sum());
        assert_equals(1, flow.take_while([](int it) { return it < 2; }).single());
        assert_equals(2, flow.drop(1).take_while([](int it) { return it < 3; }).single());
        assert_null(flow.drop(1).take_while([](int it) { return it < 2; }).single_or_null());
    }

    // @Test
    void test_empty_flow() /* TODO: = runTest */ {
        assert_equals(0, empty_flow<int>().take_while([](int) { return true; }).sum());
        assert_equals(0, empty_flow<int>().take_while([](int) { return false; }).sum());
    }

    // @Test
    void test_cancel_upstream() /* TODO: = runTest */ {
        bool cancelled = false;
        auto flow = flow([&cancelled]() /* TODO: suspend */ {
            coroutine_scope([&cancelled]() /* TODO: suspend */ {
                launch(CoroutineStart::ATOMIC, [&cancelled]() /* TODO: suspend */ {
                    hang([&cancelled]() { cancelled = true; });
                });

                emit(1);
                emit(2);
            });
        });

        assert_equals(1, flow.take_while([](int it) { return it < 2; }).single());
        assert_true(cancelled);
    }

    // @Test
    void test_error_cancels_upstream() /* TODO: = runTest */ {
        bool cancelled = false;
        auto flow = flow([&cancelled]() /* TODO: suspend */ {
            coroutine_scope([&cancelled]() /* TODO: suspend */ {
                launch(CoroutineStart::ATOMIC, [&cancelled]() /* TODO: suspend */ {
                    hang([&cancelled]() { cancelled = true; });
                });
                emit(1);
            });
        }).take_while([](int) {
            throw TestException();
        });

        assert_fails_with<TestException>(flow);
        assert_true(cancelled);
        assert_equals(42, flow.catch_error([](auto) { emit(42); }).single());
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
