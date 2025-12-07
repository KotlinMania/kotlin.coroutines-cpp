// Original file: kotlinx-coroutines-test/common/test/Helpers.kt
// TODO: Remove or convert import statements
// TODO: Convert expect/actual mechanism for testResultChain
// TODO: Handle TestResult type and Result<Unit> type
// TODO: Convert lambda/function parameter syntax

namespace kotlinx {
namespace coroutines {
namespace test {

/**
 * Runs [test], and then invokes [block], passing to it the lambda that functionally behaves
 * the same way [test] does.
 */
TestResult test_result_map(
    std::function<void(std::function<void()>)> block,
    std::function<TestResult()> test
) {
    return test_result_chain(
        test,
        [&](Result<void> result) {
            block([&]() { result.get_or_throw(); });
            return create_test_result([]() {});
        }
    );
}

/**
 * Chains together [block] and [after], passing the result of [block] to [after].
 */
// TODO: expect function - needs platform-specific implementation
TestResult test_result_chain(
    std::function<TestResult()> block,
    std::function<TestResult(Result<void>)> after
);

TestResult test_result_chain(
    std::vector<std::function<TestResult(Result<void>)>> chained,
    Result<void> initial_result = Result<void>::success()
) {
    if (chained.empty()) {
        return create_test_result([&]() {
            initial_result.get_or_throw();
        });
    } else {
        return test_result_chain(
            [&]() {
                return chained[0](initial_result);
            },
            [&](Result<void> it) {
                std::vector<std::function<TestResult(Result<void>)>> rest(
                    chained.begin() + 1,
                    chained.end()
                );
                return test_result_chain(rest, it);
            }
        );
    }
}

} // namespace test
} // namespace coroutines
} // namespace kotlinx
