#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include <iostream>
#include <cassert>
#include <atomic>
#include <thread>
#include <chrono>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::intrinsics;

// Mock continuation for testing
// suspend_cancellable_coroutine follows Kotlin/Native ABI and expects Continuation<void*>.
class MockContinuation : public Continuation<void*> {
public:
    int result_value = 0;
    std::exception_ptr exception_ptr;
    bool resumed = false;
    
    void resume_with(Result<void*> result) override {
        if (result.is_success()) {
            void* raw = result.get_or_throw();
            if (raw) {
                auto* p = static_cast<int*>(raw);
                result_value = *p;
                delete p;
            } else {
                result_value = 0;
            }
        } else {
            exception_ptr = result.exception_or_null();
        }
        resumed = true;
    }
    
    std::shared_ptr<CoroutineContext> get_context() const override {
        return nullptr; // Empty context for testing
    }
};

// Continuation that carries a specific context and records resume events.
class ContextContinuation : public Continuation<void*> {
public:
    std::shared_ptr<CoroutineContext> ctx;
    bool resumed = false;
    void* raw_value = nullptr;
    std::exception_ptr exception_ptr;

    explicit ContextContinuation(std::shared_ptr<CoroutineContext> c) : ctx(std::move(c)) {}

    std::shared_ptr<CoroutineContext> get_context() const override {
        return ctx;
    }

    void resume_with(Result<void*> result) override {
        resumed = true;
        if (result.is_success()) {
            raw_value = result.get_or_throw();
        } else {
            exception_ptr = result.exception_or_null();
        }
    }
};

// Fake dispatcher that implements Delay and records schedule calls.
class FakeDelayDispatcher : public CoroutineDispatcher, public Delay {
public:
    std::atomic<long long> scheduled_ms{0};
    std::atomic<int> schedule_calls{0};

    void dispatch(const CoroutineContext& /*context*/, std::shared_ptr<Runnable> /*block*/) const override {
        // no-op for tests
    }

    void schedule_resume_after_delay(long long time_millis, CancellableContinuation<void>& continuation) override {
        scheduled_ms.store(time_millis, std::memory_order_relaxed);
        schedule_calls.fetch_add(1, std::memory_order_relaxed);
        continuation.resume();
    }

    std::shared_ptr<DisposableHandle> invoke_on_timeout(
        long long /*time_millis*/,
        std::shared_ptr<Runnable> /*block*/,
        const CoroutineContext& /*context*/) override {
        return std::make_shared<NoOpDisposableHandle>();
    }
};

void test_suspension_infrastructure() {
    std::cout << "Testing suspension infrastructure..." << std::endl;
    
    // Test 1: COROUTINE_SUSPENDED marker
    void* suspended = get_COROUTINE_SUSPENDED();
    std::cout << "COROUTINE_SUSPENDED marker address: " << suspended << std::endl;
    
    // Test 2: is_coroutine_suspended function
    bool is_suspended = is_coroutine_suspended(suspended);
    assert(is_suspended == true);
    std::cout << "✓ is_coroutine_suspended correctly identifies suspended marker" << std::endl;
    
    // Test 3: Non-suspended values
    int some_value = 42;
    bool not_suspended = is_coroutine_suspended(&some_value);
    assert(not_suspended == false);
    std::cout << "✓ is_coroutine_suspended correctly identifies non-suspended values" << std::endl;
    
    // Test 4: Null pointer
    bool null_not_suspended = is_coroutine_suspended(nullptr);
    assert(null_not_suspended == false);
    std::cout << "✓ is_coroutine_suspended correctly identifies nullptr" << std::endl;
    
    std::cout << "All suspension infrastructure tests passed!" << std::endl;
}

void test_cancellable_continuation_suspension() {
    std::cout << "\nTesting CancellableContinuation suspension..." << std::endl;
    
    auto mock_continuation = std::make_shared<MockContinuation>();
    
    // Test synchronous completion (no suspension)
    void* result = suspend_cancellable_coroutine<int>(
        [](CancellableContinuation<int>& cont) {
            // Immediately resume with a value
            cont.resume(42, nullptr);
        },
        mock_continuation.get()
    );
    
    // Should not be suspended since we resumed immediately.
    // In Kotlin semantics, the value is returned directly and the outer continuation
    // is not resumed in this fast path.
    assert(result != COROUTINE_SUSPENDED);
    auto* direct_ptr = static_cast<int*>(result);
    assert(direct_ptr && *direct_ptr == 42);
    delete direct_ptr;
    assert(mock_continuation->resumed == false);
    std::cout << "✓ Synchronous completion returns directly" << std::endl;
    
    // Test asynchronous completion (suspension)
    mock_continuation->resumed = false;
    mock_continuation->result_value = 0;
    
    std::shared_ptr<CancellableContinuation<int>> captured_cont = nullptr;
    result = suspend_cancellable_coroutine<int>(
        [&captured_cont](CancellableContinuation<int>& cont) {
            // Store continuation for later resumption
            captured_cont = std::static_pointer_cast<CancellableContinuation<int>>(
                static_cast<CancellableContinuationImpl<int>*>(&cont)->shared_from_this()
            );
            // Don't resume immediately - should suspend
        },
        mock_continuation.get()
    );
    
    // Should be suspended
    assert(result == COROUTINE_SUSPENDED);
    assert(mock_continuation->resumed == false);
    std::cout << "✓ Asynchronous suspension works correctly" << std::endl;
    
    // Resume after delay
    std::thread([captured_cont, mock_continuation]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        captured_cont->resume(123, nullptr);
    }).detach();
    
    // Wait for completion
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    assert(mock_continuation->resumed == true);
    assert(mock_continuation->result_value == 123);
    std::cout << "✓ Asynchronous resumption works correctly" << std::endl;
}

void test_exception_fast_path() {
    std::cout << "\nTesting exception fast path..." << std::endl;

    auto outer = std::make_shared<MockContinuation>();
    bool threw = false;

    try {
        (void)suspend_cancellable_coroutine<int>(
            [](CancellableContinuation<int>& cont) {
                cont.resume_with_exception(
                    std::make_exception_ptr(std::runtime_error("boom")));
            },
            outer.get()
        );
    } catch (const std::runtime_error&) {
        threw = true;
    }

    assert(threw);
    assert(outer->resumed == false);
    std::cout << "✓ Fast exception propagates without resuming outer" << std::endl;
}

void test_prompt_cancellation_fast_path() {
    std::cout << "\nTesting prompt cancellation guarantee..." << std::endl;

    auto job = make_job();
    auto ctx = EmptyCoroutineContext::instance()->operator+(
        std::static_pointer_cast<CoroutineContext>(job));
    auto outer = std::make_shared<ContextContinuation>(ctx);

    bool threw = false;
    try {
        (void)suspend_cancellable_coroutine<int>(
            [job](CancellableContinuation<int>& cont) {
                cont.resume(7, nullptr);
                job->cancel(std::make_exception_ptr(CancellationException("cancelled")));
            },
            outer.get()
        );
    } catch (const CancellationException&) {
        threw = true;
    }

    assert(threw);
    assert(outer->resumed == false);
    std::cout << "✓ Cancellation throws even with ready result" << std::endl;
}

void test_delay_integration() {
    std::cout << "\nTesting Delay integration via free delay()..." << std::endl;

    auto dispatcher = std::make_shared<FakeDelayDispatcher>();
    auto ctx = EmptyCoroutineContext::instance()->operator+(
        std::static_pointer_cast<CoroutineContext>(dispatcher));
    auto outer = std::make_shared<ContextContinuation>(ctx);

    void* result = delay(25, outer.get());

    // delay should schedule via dispatcher and either suspend or complete fast.
    assert(dispatcher->schedule_calls.load() == 1);
    assert(dispatcher->scheduled_ms.load() == 25);

    if (result == COROUTINE_SUSPENDED) {
        for (int spin = 0; spin < 50 && !outer->resumed; ++spin) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        assert(outer->resumed);
    } else {
        assert(result == nullptr);
        assert(outer->resumed == false);
    }

    std::cout << "✓ delay routed to Delay dispatcher" << std::endl;
}

void test_void_fast_path() {
    std::cout << "\nTesting void suspend_cancellable_coroutine fast path..." << std::endl;

    auto outer = std::make_shared<MockContinuation>();
    void* result = suspend_cancellable_coroutine<void>(
        [](CancellableContinuation<void>& cont) {
            cont.resume(nullptr);
        },
        outer.get()
    );

    assert(result == nullptr);
    assert(outer->resumed == false);
    std::cout << "✓ Void fast path returns nullptr without outer resume" << std::endl;
}

void stress_decision_races() {
    std::cout << "\nStress testing resume/suspend races..." << std::endl;

    const int iterations = 500;
    for (int i = 0; i < iterations; ++i) {
        auto outer = std::make_shared<MockContinuation>();
        std::shared_ptr<CancellableContinuation<int>> captured_cont = nullptr;

        void* result = suspend_cancellable_coroutine<int>(
            [&captured_cont, i](CancellableContinuation<int>& cont) {
                captured_cont = std::static_pointer_cast<CancellableContinuation<int>>(
                    static_cast<CancellableContinuationImpl<int>*>(&cont)->shared_from_this()
                );

                if (i % 2 == 0) {
                    // Resume immediately (resume-before-suspend)
                    captured_cont->resume(i, nullptr);
                } else {
                    // Resume later (suspend-before-resume)
                    auto local = captured_cont;
                    std::thread([local, i]() {
                        std::this_thread::sleep_for(std::chrono::microseconds(50));
                        local->resume(i, nullptr);
                    }).detach();
                }
            },
            outer.get()
        );

        if (result == COROUTINE_SUSPENDED) {
            for (int spin = 0; spin < 100 && !outer->resumed; ++spin) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            assert(outer->resumed);
            assert(outer->result_value == i);
        } else {
            auto* direct_ptr = static_cast<int*>(result);
            assert(direct_ptr && *direct_ptr == i);
            delete direct_ptr;
            assert(outer->resumed == false);
        }
    }

    std::cout << "✓ Race stress passed" << std::endl;
}

void test_proper_suspension_marker_usage() {
    std::cout << "\nTesting proper suspension marker usage..." << std::endl;
    
    // Test that the marker is consistent
    void* marker1 = get_COROUTINE_SUSPENDED();
    void* marker2 = get_COROUTINE_SUSPENDED();
    assert(marker1 == marker2);
    std::cout << "✓ COROUTINE_SUSPENDED marker is consistent" << std::endl;
    
    // Test that the marker is not null
    assert(marker1 != nullptr);
    std::cout << "✓ COROUTINE_SUSPENDED marker is not null" << std::endl;
    
    // Test that the marker is a valid pointer
    assert(is_coroutine_suspended(marker1));
    std::cout << "✓ COROUTINE_SUSPENDED marker passes validation" << std::endl;
}

int main() {
    try {
        test_suspension_infrastructure();
        test_cancellable_continuation_suspension();
        test_exception_fast_path();
        test_prompt_cancellation_fast_path();
        test_delay_integration();
        test_void_fast_path();
        stress_decision_races();
        test_proper_suspension_marker_usage();
        
        std::cout << "\n🎉 All suspension tests passed!" << std::endl;
        std::cout << "The suspension infrastructure is working correctly." << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
