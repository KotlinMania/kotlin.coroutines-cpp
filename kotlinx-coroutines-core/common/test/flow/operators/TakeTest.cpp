// Original file: kotlinx-coroutines-core/common/test/flow/operators/TakeTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations

namespace kotlinx {
namespace coroutines {
namespace flow {

class TakeTest : public TestBase {
public:
    // @Test
    void test_take() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            emit(1);
            emit(2);
        });

        assert_equals(3, flow.take(2).sum());
        assert_equals(3, flow.take(INT_MAX).sum());
        assert_equals(1, flow.take(1).single());
        assert_equals(2, flow.drop(1).take(1).single());
    }

    // @Test
    void test_illegal_argument() {
        assert_fails_with<IllegalArgumentException>([&]() { flow_of(1).take(0); });
        assert_fails_with<IllegalArgumentException>([&]() { flow_of(1).take(-1); });
    }

    // @Test
    void test_take_suspending() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            emit(1);
            yield();
            emit(2);
            yield();
        });

        assert_equals(3, flow.take(2).sum());
        assert_equals(3, flow.take(INT_MAX).sum());
        assert_equals(1, flow.take(1).single());
        assert_equals(2, flow.drop(1).take(1).single());
    }

    // @Test
    void test_empty_flow() /* TODO: = runTest */ {
        auto sum = empty_flow<int>().take(10).sum();
        assert_equals(0, sum);
    }

    // @Test
    void test_non_positive_values() {
        auto flow = flow_of(1);
        assert_fails_with<IllegalArgumentException>([&]() {
            flow.take(-1);
        });

        assert_fails_with<IllegalArgumentException>([&]() {
            flow.take(0);
        });
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
            });
        });

        assert_equals(1, flow.take(1).single());
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
        }).take(2)
            .map<int, int>([](int) {
                throw TestException();
            }).catch_error([](auto) { emit(42); });

        assert_equals(42, flow.single());
        assert_true(cancelled);
    }

    // @Test
    void take_with_retries() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            expect(1);
            emit(1);
            expect(2);
            emit(2);

            while (true) {
                emit(42);
                expect_unreached();
            }

        }).retry(2, [](const std::exception&) {
            expect_unreached();
            return true;
        }).take(2);

        auto sum = flow.sum();
        assert_equals(3, sum);
        finish(3);
    }

    // @Test
    void test_non_idempotent_retry() /* TODO: = runTest */ {
        int count = 0;
        flow([]() /* TODO: suspend */ { while (true) emit(1); })
            .retry([&count](const std::exception&) { return count++ % 2 != 0; })
            .take(1)
            .collect([](int) {
                expect(1);
            });
        finish(2);
    }

    // @Test
    void test_nested_take() /* TODO: = runTest */ {
        auto inner = flow([]() /* TODO: suspend */ {
            emit(1);
            expect_unreached();
        }).take(1);
        auto outer = flow([&inner]() /* TODO: suspend */ {
            while(true) {
                emit_all(inner);
            }
        });
        assert_equals(std::vector<int>{1, 1, 1}, outer.take(3).to_list());
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
