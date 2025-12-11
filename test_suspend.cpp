/**
 * test_suspend.cpp - Test for suspend function state machine implementation
 *
 * This test demonstrates the C++ equivalent of Kotlin's suspend function compilation.
 * Based on NativeSuspendFunctionLowering.kt:
 * - invokeSuspend is the state machine method
 * - label field tracks suspension points
 * - ContinuationImpl is the base class
 *
 * NOTE: IntelliJ IDEA may report false syntax errors in this file due to macro-based
 * state machine generation. The code compiles correctly with g++/clang.
 * See IDE_ERROR_INVESTIGATION.md for details.
 */

// noinspection CppDFAUnreachableCode
// noinspection CppDFAUnusedValue
// noinspection CppNoDiscardExpression

#include "kotlinx/coroutines/SuspendMacros.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include <iostream>
#include <cassert>
#include <memory>

using kotlinx::coroutines::SuspendLambda;
using kotlinx::coroutines::BaseContinuationImpl;
using kotlinx::coroutines::Continuation;
using kotlinx::coroutines::EmptyCoroutineContext;
using kotlinx::coroutines::CoroutineContext;
using kotlinx::coroutines::intrinsics::is_coroutine_suspended;
using kotlinx::coroutines::Result;

// A simple suspend function that returns a value
// Kotlin equivalent:
//   suspend fun getValue(): Int {
//       return 42
//   }
class GetValueSuspendFn : public SuspendLambda<int> {
public:
    using SuspendLambda::SuspendLambda;

    // NOLINT - IDE cannot parse suspend macros
    void* invoke_suspend(Result<void*> result) override {
        SUSPEND_BEGIN(1) // NOLINT

        // No suspension, just return the value
        SUSPEND_RETURN(42);

        SUSPEND_END // NOLINT
    }
};

// A suspend function that calls another suspend function
// Kotlin equivalent:
//   suspend fun example(): Int {
//       val x = getValue()  // suspend call
//       return x + 1
//   }
class ExampleSuspendFn : public SuspendLambda<int> {
    // Spilled local variable (survives across suspension)
    int saved_x = 0;

public:
    using SuspendLambda::SuspendLambda;

    // NOLINT - IDE cannot parse suspend macros
    void* invoke_suspend(Result<void*> result) override {
        void* tmp = nullptr;
        int x;
        std::shared_ptr<GetValueSuspendFn> get_value_fn;

        SUSPEND_BEGIN(2) // NOLINT

        // val x = getValue() - this is a suspend call
        get_value_fn = std::make_shared<GetValueSuspendFn>( // NOLINT
            std::dynamic_pointer_cast<Continuation<void*>>(shared_from_this())
        );
        SUSPEND_CALL(1, get_value_fn->invoke_suspend(Result<void*>::success(nullptr)), tmp) // NOLINT

        // After SUSPEND_CALL:
        // - If no suspension: tmp contains the direct result
        // - If resumed from suspension: result parameter contains the value
        // The macro fall-through means we arrive here in both cases, but the value source differs
        x = static_cast<int>(reinterpret_cast<intptr_t>(tmp));
        saved_x = x;

        // return x + 1
        SUSPEND_RETURN(saved_x + 1);

        SUSPEND_END // NOLINT
    }
};

// Test completion continuation that captures the result
template<typename T>
class TestCompletion : public Continuation<void*> {
public:
    bool completed = false;
    T result_value{};
    std::exception_ptr exception;

    std::shared_ptr<CoroutineContext> get_context() const override {
        return EmptyCoroutineContext::instance();
    }

    void resume_with(Result<void*> result) override {
        completed = true;
        if (result.is_failure()) {
            exception = result.exception_or_null();
        } else {
            result_value = static_cast<T>(reinterpret_cast<intptr_t>(result.get_or_throw()));
        }
    }
};

void test_simple_suspend_function() {
    std::cout << "Test 1: Simple suspend function..." << std::endl;

    auto completion = std::make_shared<TestCompletion<int>>();
    auto coroutine = std::make_shared<GetValueSuspendFn>(
        std::static_pointer_cast<Continuation<void*>>(completion)
    );

    // Start the coroutine
    void* result = coroutine->invoke_suspend(Result<void*>::success(nullptr));

    // Should complete immediately (no actual suspension)
    assert(!is_coroutine_suspended(result));
    assert(static_cast<int>(reinterpret_cast<intptr_t>(result)) == 42);

    std::cout << "  Result: " << static_cast<int>(reinterpret_cast<intptr_t>(result)) << std::endl;
    std::cout << "  PASSED" << std::endl;
}

void test_chained_suspend_function() {
    std::cout << "Test 2: Chained suspend function..." << std::endl;

    auto completion = std::make_shared<TestCompletion<int>>();
    auto coroutine = std::make_shared<ExampleSuspendFn>(
        std::static_pointer_cast<Continuation<void*>>(completion)
    );

    // Start the coroutine - it calls getValue() which doesn't actually suspend
    void* result = coroutine->invoke_suspend(Result<void*>::success(nullptr));

    // The inner call returns immediately, so we should get 42 + 1 = 43
    if (!is_coroutine_suspended(result)) {
        int final_result = static_cast<int>(reinterpret_cast<intptr_t>(result));
        std::cout << "  Result: " << final_result << std::endl;
        assert(final_result == 43);
    } else {
        std::cout << "  Suspended (unexpected for this test)" << std::endl;
        assert(false);
    }

    std::cout << "  PASSED" << std::endl;
}

// A suspend function that actually suspends
// Kotlin equivalent:
//   suspend fun suspendAndReturn(): Int {
//       delay(100) // actually suspends
//       return 99
//   }
class SuspendAndReturnFn : public SuspendLambda<int> {
    std::shared_ptr<Continuation<void*>> saved_continuation;

public:
    using SuspendLambda::SuspendLambda;

    // Simulate external resume (like delay() completing)
    void external_resume(int value) {
        if (completion) {
            // Resume the state machine with the result
            this->resume_with(Result<void*>::success(reinterpret_cast<void*>(static_cast<intptr_t>(value))));
        }
    }

    // NOLINT - IDE cannot parse suspend macros
    void* invoke_suspend(Result<void*> result) override {
        SUSPEND_BEGIN(2) // NOLINT

        // State 0: Initial - simulate suspending
        this->_label = 1;
        return COROUTINE_SUSPENDED;  // Actually suspend

        SUSPEND_POINT(1) // NOLINT
        // State 1: Resumed after suspension
        // result contains the value we were resumed with
        if (result.is_failure()) {
            std::rethrow_exception(result.exception_or_null());
        }

        // Return the value we got + 1
        {
            int resumed_value = static_cast<int>(reinterpret_cast<intptr_t>(result.get_or_throw()));
            SUSPEND_RETURN(resumed_value + 1);
        }

        SUSPEND_END // NOLINT
    }
};

void test_actual_suspension() {
    std::cout << "Test 3: Actual suspension and resume..." << std::endl;

    auto completion = std::make_shared<TestCompletion<int>>();
    auto coroutine = std::make_shared<SuspendAndReturnFn>(
        std::static_pointer_cast<Continuation<void*>>(completion)
    );

    // Start the coroutine - should suspend
    void* result = coroutine->invoke_suspend(Result<void*>::success(nullptr));
    assert(is_coroutine_suspended(result));
    std::cout << "  Coroutine suspended (as expected)" << std::endl;

    // Simulate external event resuming the coroutine with value 50
    coroutine->external_resume(50);

    // Check completion
    assert(completion->completed);
    assert(completion->exception == nullptr);
    std::cout << "  Resumed with result: " << completion->result_value << std::endl;
    assert(completion->result_value == 51);  // 50 + 1

    std::cout << "  PASSED" << std::endl;
}

int main() {
    std::cout << "=== Suspend Function Tests ===" << std::endl;
    std::cout << std::endl;

    test_simple_suspend_function();
    test_chained_suspend_function();
    test_actual_suspension();

    std::cout << std::endl;
    std::cout << "=== All tests passed ===" << std::endl;
    return 0;
}
