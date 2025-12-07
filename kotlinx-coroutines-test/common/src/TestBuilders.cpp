/**
 * @file TestBuilders.cpp
 * @brief Implementation of TestBuilders.
 *
 * NOTE: The detailed API documentation, KDocs, and declarations are located
 * in the companion header file: `include/kotlinx/coroutines/test/TestBuilders.hpp`.
 */

#include "kotlinx/coroutines/test/TestBuilders.hpp"
#include <stdexcept>

namespace kotlinx {
namespace coroutines {
namespace test {

class TestScope {
public:
    // Stub
};

TestResult run_test(
    CoroutineContext context,
    std::chrono::milliseconds timeout,
    std::function<void(TestScope&)> test_body
) {
    // Stub implementation
    TestScope scope;
    if (test_body) {
        test_body(scope);
    }
}

} // namespace test
} // namespace coroutines
} // namespace kotlinx
