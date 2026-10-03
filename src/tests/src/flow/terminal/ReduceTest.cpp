/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/ReduceTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Reduce.hpp"

#include <iostream>
#include <optional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace kotlinx {
namespace coroutines {

using namespace flow;
using namespace testing;

namespace {

class TestLatch {
    std::mutex m_;
    std::condition_variable cv_;
    bool ready_ = false;
public:
    void send() {
        {
            std::lock_guard<std::mutex> lk(m_);
            ready_ = true;
        }
        cv_.notify_all();
    }
    void receive() {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [this] { return ready_; });
    }
};

} // namespace

class ReduceTest : public TestBase {
private:
    std::mutex hang_mutex_;
    std::condition_variable hang_cv_;
    bool hang_cancelled_ = false;

public:
    template <typename OnCancellation>
    void hang(OnCancellation&& on_cancellation) {
        std::unique_lock<std::mutex> lk(hang_mutex_);
        hang_cv_.wait(lk, [this] { return hang_cancelled_; });
        on_cancellation();
    }

    void cancel_hang() {
        {
            std::lock_guard<std::mutex> lk(hang_mutex_);
            hang_cancelled_ = true;
        }
        hang_cv_.notify_all();
    }

    void reset_hang() {
        std::lock_guard<std::mutex> lk(hang_mutex_);
        hang_cancelled_ = false;
    }

    // @Test
    void test_reduce() {
        run_test([](CoroutineScope*) {
            auto f = flow::flow<int>([](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                collector->emit(1, cont);
                collector->emit(2, cont);
                return nullptr;
            });

            void* result = reduce<int>(f, [](int acc, int value) { return acc + value; }, nullptr);
            assert_not_null(result);
            assert_equals(3, *static_cast<int*>(result));
            delete static_cast<int*>(result);
        });
    }

    // @Test
    void test_empty_reduce() {
        run_test([](CoroutineScope*) {
            auto f = empty_flow<int>();
            assert_fails_with<NoSuchElementException>([&]() {
                reduce<int>(f, [](int acc, int value) { return acc + value; }, nullptr);
            });
        });
    }

    // @Test
    void test_nullable_reduce() {
        run_test([](CoroutineScope*) {
            auto f = flow_of<std::optional<int>>({1, std::nullopt, std::nullopt, 2});
            int invocations = 0;
            void* res = reduce<std::optional<int>>(f, [&](std::optional<int>, std::optional<int> value) {
                ++invocations;
                return value;
            }, nullptr);
            assert_not_null(res);
            auto sum = *static_cast<std::optional<int>*>(res);
            delete static_cast<std::optional<int>*>(res);
            assert_true(sum.has_value());
            assert_equals(2, sum.value());
            assert_equals(3, invocations);
        });
    }

    // @Test
    void test_reduce_nulls() {
        run_test([](CoroutineScope*) {
            auto f1 = flow_of<std::optional<int>>({std::nullopt});
            void* res1 = reduce<std::optional<int>>(f1, [](std::optional<int>, std::optional<int> v) { return v; }, nullptr);
            assert_not_null(res1);
            auto val1 = *static_cast<std::optional<int>*>(res1);
            delete static_cast<std::optional<int>*>(res1);
            assert_true(!val1.has_value());

            auto f2 = flow_of<std::optional<int>>({std::nullopt, std::nullopt});
            void* res2 = reduce<std::optional<int>>(f2, [](std::optional<int>, std::optional<int> v) { return v; }, nullptr);
            assert_not_null(res2);
            auto val2 = *static_cast<std::optional<int>*>(res2);
            delete static_cast<std::optional<int>*>(res2);
            assert_true(!val2.has_value());

            auto f3 = flow_of<std::optional<int>>({});
            assert_fails_with<NoSuchElementException>([&]() {
                reduce<std::optional<int>>(f3, [](std::optional<int>, std::optional<int> v) { return v; }, nullptr);
            });
        });
    }

    // @Test
    void test_error_cancels_upstream() {
        run_test([this](CoroutineScope*) {
            reset_hang();
            TestLatch latch;
            auto f = flow::flow<int>([this, &latch](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                expect(2);
                std::thread child([this, &latch]() {
                    latch.send();
                    expect(3);
                    hang([this]() { expect(5); });
                });

                try {
                    collector->emit(1, cont);
                    collector->emit(2, cont);
                } catch (...) {
                    cancel_hang();
                    if (child.joinable()) child.join();
                    throw;
                }
                cancel_hang();
                if (child.joinable()) child.join();
                return nullptr;
            });

            expect(1);
            assert_fails_with<TestException>([&]() {
                reduce<int>(f, [this, &latch](int /*acc*/, int /*value*/) -> int {
                    latch.receive();
                    expect(4);
                    throw TestException();
                }, nullptr);
            });
            finish(6);
        });
    }
};

} // namespace coroutines
} // namespace kotlinx

int main() {
    using namespace kotlinx::coroutines;
    ReduceTest test;
    int failed = 0;

    auto run = [&](const char* name, void (ReduceTest::*method)()) {
        std::cout << "Running " << name << "..." << std::endl;
        try {
            (test.*method)();
            test.reset();
            std::cout << "  PASSED" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "  FAILED: " << e.what() << std::endl;
            failed++;
            test.reset();
        }
    };

    std::cout << "=== ReduceTest ===" << std::endl;
    run("test_reduce", &ReduceTest::test_reduce);
    run("test_empty_reduce", &ReduceTest::test_empty_reduce);
    run("test_nullable_reduce", &ReduceTest::test_nullable_reduce);
    run("test_reduce_nulls", &ReduceTest::test_reduce_nulls);
    run("test_error_cancels_upstream", &ReduceTest::test_error_cancels_upstream);

    std::cout << "=== Results: " << (5 - failed) << "/5 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}