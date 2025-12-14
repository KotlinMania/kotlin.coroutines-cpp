/**
 * @file test_timeout.cpp
 * @brief Tests for Timeout primitives using strict suspend ABI.
 */

#include "kotlinx/coroutines/Timeout.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include <iostream>
#include <cassert>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::intrinsics;

// Simple completion continuation
class TestCompletion : public Continuation<void*> {
public:
    bool completed = false;
    void* result_value = nullptr;
    std::exception_ptr exception;

    std::shared_ptr<CoroutineContext> get_context() const override {
        return EmptyCoroutineContext::instance();
    }

    void resume_with(Result<void*> result) override {
        completed = true;
        if (result.is_success()) {
            result_value = result.get_or_throw();
        } else {
            exception = result.exception_or_null();
        }
    }
};

void test_timeout_no_suspend() {
    std::cout << "test_timeout_no_suspend... ";
    
    auto completion = std::make_shared<TestCompletion>();
    
    // Test block that returns immediate result
    auto block = [](CoroutineScope& scope) -> void* {
        // Return boxed integer 42
        static int val = 42;
        return &val;
    };
    
    // Call with_timeout
    // We expect it to return the result immediately because block doesn't suspend
    void* result = with_timeout<void*>(1000, block, completion);
    
    assert(!is_coroutine_suspended(result));
    assert(*(int*)result == 42);
    
    std::cout << "PASSED" << std::endl;
}

void test_timeout_throws_exception() {
    std::cout << "test_timeout_throws_exception... ";
    
    auto completion = std::make_shared<TestCompletion>();
    
    auto block = [](CoroutineScope& scope) -> void* {
        throw std::runtime_error("Test Error");
    };
    
    bool caught = false;
    try {
        with_timeout<void*>(1000, block, completion);
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    
    assert(caught);
    
    std::cout << "PASSED" << std::endl;
}

// TODO: Test suspension inside with_timeout (requires async machine)

int main() {
    std::cout << "=== test_timeout ===" << std::endl;
    
    test_timeout_no_suspend();
    test_timeout_throws_exception();
    
    std::cout << "=== All tests passed ===" << std::endl;
    return 0;
}
