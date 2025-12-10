#pragma once
#include "kotlinx/coroutines/test/TestScope.hpp"
#include <chrono>
#include <functional>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace test {

using TestResult = void;

/**
 * Runs a test in a [TestScope].
 *
 * This function creates a new [TestScope] with a [TestDispatcher] and executes the given [testBody]
 * within it. It ensures that all coroutines launched in the test scope are completed before returning.
 *
 * The [timeout] parameter specifies the maximum duration the test is allowed to run.
 * If the test exceeds this timeout, it fails (behavior depends on implementation).
 *
 * @param context Additional context elements to be appended to the test scope's context.
 * @param timeout The maximum duration for the test.
 * @param testBody The body of the test to execute.
 */
inline TestResult run_test(
    std::shared_ptr<CoroutineContext> context,
    std::chrono::milliseconds timeout,
    std::function<void(TestScope&)> test_body
) {
    // 1. Create TestDispatcher
    auto scheduler = TestDispatcher::create();
    
    // 2. Create Context (TestDispatcher + context)
    // For now simple aggregation stub or just use scheduler
    // std::shared_ptr<CoroutineContext> fullContext = scheduler + context;
    
    // 3. Create Scope
    TestScope scope(scheduler);
    
    // 4. Run body
    try {
        test_body(scope);
        
        // 5. Advance time / run outstanding tasks
        scheduler->execute_tasks();
        
    } catch (...) {
        // Handle exceptions or rethrow
        throw;
    }
}

/**
 * Overload for run_test with default context and timeout.
 */
inline TestResult run_test(std::function<void(TestScope&)> test_body) {
    return run_test(nullptr, std::chrono::milliseconds(10000), test_body);
}

} // namespace test
} // namespace coroutines
} // namespace kotlinx
