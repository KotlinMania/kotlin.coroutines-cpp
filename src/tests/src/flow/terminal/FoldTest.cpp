/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/terminal/FoldTest.kt
 */

#include "kotlinx/coroutines/testing/TestBase.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Reduce.hpp"

#include <iostream>
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

class FoldTest : public TestBase {
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
    void test_fold() {
        run_test([](CoroutineScope*) {
            auto f = flow::flow<int>([](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
                collector->emit(1, cont);
                collector->emit(2, cont);
                return nullptr;
            });

            void* result = fold<int, int>(f, 3, [](int acc, int value) { return acc + value; }, nullptr);
            assert_not_null(result);
            assert_equals(6, *static_cast<int*>(result));
            delete static_cast<int*>(result);
        });
    }

    // @Test
    void test_empty_fold() {
        run_test([](CoroutineScope*) {
            auto f = empty_flow<int>();
            void* result = fold<int, int>(f, 42, [](int acc, int value) { return acc + value; }, nullptr);
            assert_not_null(result);
            assert_equals(42, *static_cast<int*>(result));
            delete static_cast<int*>(result);
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
                fold<int, int>(f, 42, [this, &latch](int /*acc*/, int /*value*/) -> int {
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
    FoldTest test;
    int failed = 0;

    auto run = [&](const char* name, void (FoldTest::*method)()) {
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

    std::cout << "=== FoldTest ===" << std::endl;
    run("test_fold", &FoldTest::test_fold);
    run("test_empty_fold", &FoldTest::test_empty_fold);
    run("test_error_cancels_upstream", &FoldTest::test_error_cancels_upstream);

    std::cout << "=== Results: " << (3 - failed) << "/3 passed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}