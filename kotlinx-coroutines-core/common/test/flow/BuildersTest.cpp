// Original: kotlinx-coroutines-core/common/test/flow/BuildersTest.kt
// TODO: Translate imports to proper C++ includes
// TODO: Implement TestBase base class
// TODO: Implement @Test annotation equivalent (test framework registration)
// TODO: Implement runTest coroutine test runner
// TODO: Implement Flow, asFlow, single, toList APIs
// TODO: Implement suspend lambda support
// TODO: Implement range and array conversion to Flow

#include <vector>
#include <array>
// TODO: #include proper headers

namespace kotlinx {
namespace coroutines {
namespace flow {

class BuildersTest : public TestBase {
public:
    // @Test
    void test_suspend_lambda_as_flow() {
        // TODO: runTest coroutine runner
        run_test([]() -> /* suspend */ void {
            auto lambda = /* suspend */ []() -> int { return 42; };
            assert_equals(42, lambda.as_flow().single());
        });
    }

    // @Test
    void test_range_as_flow() {
        // TODO: runTest coroutine runner
        run_test([]() -> /* suspend */ void {
            // (0..9).toList() → range conversion
            std::vector<int> expected_0_9 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            assert_equals(expected_0_9, range(0, 9).as_flow().to_list());

            std::vector<int> empty_list;
            assert_equals(empty_list, range(0, -1).as_flow().to_list());

            std::vector<long> expected_0_9_long = {0L, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L, 9L};
            assert_equals(expected_0_9_long, range(0L, 9L).as_flow().to_list());

            std::vector<long> empty_list_long;
            assert_equals(empty_list_long, range(0L, -1L).as_flow().to_list());
        });
    }

    // @Test
    void test_array_as_flow() {
        // TODO: runTest coroutine runner
        run_test([]() -> /* suspend */ void {
            // IntArray(10) { it }
            std::array<int, 10> int_array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            std::vector<int> expected_0_9 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            assert_equals(expected_0_9, int_array.as_flow().to_list());

            std::vector<int> empty_int_array;
            assert_equals(std::vector<int>(), empty_int_array.as_flow().to_list());

            // LongArray(10) { it.toLong() }
            std::array<long, 10> long_array = {0L, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L, 9L};
            std::vector<long> expected_0_9_long = {0L, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L, 9L};
            assert_equals(expected_0_9_long, long_array.as_flow().to_list());

            std::vector<long> empty_long_array;
            assert_equals(std::vector<long>(), empty_long_array.as_flow().to_list());
        });
    }

    // @Test
    void test_sequence() {
        // TODO: runTest coroutine runner
        run_test([]() -> /* suspend */ void {
            std::vector<int> expected = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            assert_equals(expected, expected.begin().as_flow().to_list());
            assert_equals(expected, expected.as_iterable().as_flow().to_list());
        });
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
