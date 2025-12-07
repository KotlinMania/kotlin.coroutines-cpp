#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include <chrono>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace test {

using TestResult = void;
class TestScope;

TestResult run_test(
    CoroutineContext context,
    std::chrono::milliseconds timeout,
    std::function<void(TestScope&)> test_body
);

} // namespace test
} // namespace coroutines
} // namespace kotlinx
