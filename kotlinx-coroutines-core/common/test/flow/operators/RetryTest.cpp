// Original file: kotlinx-coroutines-core/common/test/flow/operators/RetryTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations

namespace kotlinx {
namespace coroutines {
namespace flow {

class RetryTest : public TestBase {
public:
    // @Test
    void test_retry_when() /* TODO: = runTest */ {
        expect(1);
        auto flow = flow([]() /* TODO: suspend */ {
            emit(1);
            throw TestException();
        });
        auto sum = flow.retry_when([](const std::exception& cause, int64_t attempt) {
            assert_is<TestException>(cause);
            expect(2 + static_cast<int>(attempt));
            return attempt < 3;
        }).catch_error([](const std::exception& cause) {
            expect(6);
            assert_is<TestException>(cause);
        }).sum();
        assert_equals(4, sum);
        finish(7);
    }

    // @Test
    void test_retry() /* TODO: = runTest */ {
        int counter = 0;
        auto flow = flow([&counter]() /* TODO: suspend */ {
            emit(1);
            if (++counter < 4) throw TestException();
        });

        assert_equals(4, flow.retry(4).sum());
        counter = 0;
        assert_fails_with<TestException>(flow);
        counter = 0;
        assert_fails_with<TestException>(flow.retry(2));
    }

    // @Test
    void test_retry_predicate() /* TODO: = runTest */ {
        int counter = 0;
        auto flow = flow([&counter]() /* TODO: suspend */ {
            emit(1);
            if (++counter == 1) throw TestException();
        });

        assert_equals(2, flow.retry(1, [](const std::exception& it) {
            return dynamic_cast<const TestException*>(&it) != nullptr;
        }).sum());
        counter = 0;
        assert_fails_with<TestException>(flow.retry(1, [](const std::exception& it) {
            return dynamic_cast<const TestException*>(&it) == nullptr;
        }));
    }

    // @Test
    void test_retry_exception_from_downstream() /* TODO: = runTest */ {
        int executed = 0;
        auto flow = flow([]() /* TODO: suspend */ {
            emit(1);
        }).retry(42).map([&executed](int) {
            ++executed;
            throw TestException();
        });

        assert_fails_with<TestException>(flow);
        assert_equals(1, executed);
    }

    // @Test
    void test_with_timeout_retried() /* TODO: = runTest */ {
        int state = 0;
        auto flow = flow([&state]() /* TODO: suspend */ {
            if (state++ == 0) {
                expect(1);
                with_timeout(1, [&]() /* TODO: suspend */ {
                    hang([]() { expect(2); });
                });
                expect_unreached();
            }
            expect(3);
            emit(1);
        }).retry(1);

        assert_equals(1, flow.single());
        finish(4);
    }

    // @Test
    void test_cancellation_from_upstream_is_not_retried() /* TODO: = runTest */ {
        auto flow = flow<int>([]() /* TODO: suspend */ {
            hang([]() {});
        }).retry();

        auto job = launch([&]() /* TODO: suspend */ {
            expect(1);
            flow.collect([](int) {});
        });

        yield();
        expect(2);
        job.cancel_and_join();
        finish(3);
    }

    // @Test
    void test_upstream_exception_concurrent_with_downstream() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            try {
                expect(1);
                emit(1);
            } finally {
                expect(3);
                throw TestException();
            }
        }).retry([](const std::exception&) { expect_unreached(); return true; }).on_each([](int) {
            expect(2);
            throw TestException2();
        });

        assert_fails_with<TestException>(flow);
        finish(4);
    }

    // @Test
    void test_upstream_exception_concurrent_with_downstream_cancellation() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            try {
                expect(1);
                emit(1);
            } finally {
                expect(3);
                throw TestException();
            }
        }).retry([](const std::exception&) { expect_unreached(); return true; }).on_each([](int) {
            expect(2);
            throw CancellationException("");
        });

        assert_fails_with<TestException>(flow);
        finish(4);
    }

    // @Test
    void test_upstream_cancellation_is_ignored_when_downstream_fails() /* TODO: = runTest */ {
        auto flow = flow([]() /* TODO: suspend */ {
            try {
                expect(1);
                emit(1);
            } finally {
                expect(3);
                throw CancellationException("");
            }
        }).retry([](const std::exception&) { expect_unreached(); return true; }).on_each([](int) {
            expect(2);
            throw TestException("");
        });

        assert_fails_with<TestException>(flow);
        finish(4);
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
