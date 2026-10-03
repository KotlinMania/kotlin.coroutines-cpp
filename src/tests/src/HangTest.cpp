/**
 * @file HangTest.cpp
 * @brief Unit tests for suspending hang() test helper in TestBase.hpp
 *
 * Transliterated from: test-utils/common/src/TestBase.common.kt:241-247
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <atomic>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace testing {

class HangTest : public TestBase {
public:
    void test_hang_in_coroutine_scope() {
        run_test([this](CoroutineScope* scope) {
            expect(1);
            auto job = launch(scope, nullptr, CoroutineStart::UNDISPATCHED, [this](CoroutineScope*) {
                expect(2);
                hang([this]() {
                    expect(4);
                });
            });

            expect(3);
            job->cancel(nullptr);
            finish(5);
        });
    }

    void test_hang_with_cause_callback() {
        run_test([this](CoroutineScope* scope) {
            expect(1);
            auto custom_cause = std::make_exception_ptr(TestCancellationException("cancelled with custom cause"));
            auto job = launch(scope, nullptr, CoroutineStart::UNDISPATCHED, [this, custom_cause](CoroutineScope*) {
                expect(2);
                hang([this, custom_cause](std::exception_ptr cause) {
                    expect(4);
                    assert_not_null(cause);
                });
            });

            expect(3);
            job->cancel(custom_cause);
            finish(5);
        });
    }

    void test_hang_raw_continuation() {
        class MockContinuation : public Continuation<void*> {
        public:
            std::shared_ptr<CoroutineContext> context_ = EmptyCoroutineContext::instance();
            std::atomic<bool> resumed_{false};
            Result<void*> resumed_result_{nullptr};

            std::shared_ptr<CoroutineContext> get_context() const override {
                return context_;
            }

            void resume_with(Result<void*> result) override {
                resumed_.store(true);
                resumed_result_ = result;
            }
        };

        MockContinuation mock_cont;
        std::atomic<int> cancel_call_count{0};

        void* res = hang([&cancel_call_count]() {
            cancel_call_count.fetch_add(1);
        }, &mock_cont);

        assert_true(intrinsics::is_coroutine_suspended(res), "hang must return COROUTINE_SUSPENDED");
        assert_equals(0, cancel_call_count.load(), "Callback should not be called synchronously before cancellation");
    }

    void test_hang_shared_continuation() {
        class MockContinuation : public Continuation<void*> {
        public:
            std::shared_ptr<CoroutineContext> context_ = EmptyCoroutineContext::instance();

            std::shared_ptr<CoroutineContext> get_context() const override {
                return context_;
            }

            void resume_with(Result<void*> result) override {
            }
        };

        auto mock_cont = std::make_shared<MockContinuation>();
        std::atomic<int> cancel_call_count{0};

        void* res = hang([&cancel_call_count]() {
            cancel_call_count.fetch_add(1);
        }, mock_cont);

        assert_true(intrinsics::is_coroutine_suspended(res), "hang must return COROUTINE_SUSPENDED");
        assert_equals(0, cancel_call_count.load(), "Callback should not be called synchronously before cancellation");
    }

    void test_hang_outside_coroutine_throws() {
        bool threw = false;
        try {
            hang([]() {});
        } catch (const std::logic_error& e) {
            threw = true;
        }
        assert_true(threw, "hang() outside coroutine context must throw std::logic_error");
    }

    void test_hang_at_most_once() {
        run_test([this](CoroutineScope* scope) {
            std::atomic<int> count{0};
            auto job = launch(scope, nullptr, CoroutineStart::UNDISPATCHED, [this, &count](CoroutineScope*) {
                hang([&count]() {
                    count.fetch_add(1);
                });
            });

            job->cancel(nullptr);
            job->cancel(nullptr);

            assert_equals(1, count.load(), "hang on_cancellation must be called exactly once");
        });
    }

    void test_hang_no_arg_overload() {
        run_test([this](CoroutineScope* scope) {
            expect(1);
            auto job = launch(scope, nullptr, CoroutineStart::UNDISPATCHED, [this](CoroutineScope*) {
                expect(2);
                hang();
            });

            expect(3);
            job->cancel(nullptr);
            finish(4);
        });
    }
};

} // namespace testing
} // namespace coroutines
} // namespace kotlinx

int main() {
    kotlinx::coroutines::testing::HangTest test;
    test.test_hang_in_coroutine_scope();
    test.reset();

    test.test_hang_with_cause_callback();
    test.reset();

    test.test_hang_raw_continuation();
    test.reset();

    test.test_hang_shared_continuation();
    test.reset();

    test.test_hang_outside_coroutine_throws();
    test.reset();

    test.test_hang_at_most_once();
    test.reset();

    test.test_hang_no_arg_overload();
    test.reset();

    return 0;
}
