// Source contracts: kotlin-native/runtime/src/main/kotlin/kotlin/Numbers.kt:12-265
#include "kotlin/Numbers.hpp"
#include "kotlin/collections/HashMapFunctions.hpp"
#include <array>
#include <bit>
#include <cassert>
#include <limits>
using namespace kotlin;

int main() {
    using namespace kotlin::collections::hash_map::detail;
    assert(compute_hash_size(-1) == 2);
    assert(compute_hash_size(0) == 2);
    assert(compute_hash_size(1) == 2);
    assert(compute_hash_size(8) == 16);
    assert(compute_shift(16) == 28);
    assert(compute_shift(2) == 31);
    assert(compute_shift(0) == 33);
    assert(compute_hash_size(std::numeric_limits<std::int32_t>::max()) == 1073741824);
    assert(count_leading_zero_bits(std::int32_t{0}) == 32);
    assert(count_trailing_zero_bits(std::int32_t{0}) == 32);
    assert(count_leading_zero_bits(std::int64_t{0}) == 64);
    assert(count_trailing_zero_bits(std::int64_t{0}) == 64);
    assert(count_one_bits(std::int32_t{-1}) == 32);
    assert(count_one_bits(std::int64_t{-1}) == 64);
    assert(take_highest_one_bit(std::int32_t{0}) == 0);
    assert(take_lowest_one_bit(std::int64_t{0}) == 0);
    for (unsigned position = 0; position < 64; ++position) {
        const auto wide = std::bit_cast<std::int64_t>(std::uint64_t{1} << position);
        assert(count_one_bits(wide) == 1);
        assert(count_leading_zero_bits(wide) == 63 - static_cast<int>(position));
        assert(count_trailing_zero_bits(wide) == static_cast<int>(position));
        assert(take_highest_one_bit(wide) == wide);
        assert(take_lowest_one_bit(wide) == wide);
        if (position < 32) {
            const auto narrow = std::bit_cast<std::int32_t>(std::uint32_t{1} << position);
            assert(count_one_bits(narrow) == 1);
            assert(count_leading_zero_bits(narrow) == 31 - static_cast<int>(position));
            assert(count_trailing_zero_bits(narrow) == static_cast<int>(position));
            assert(take_highest_one_bit(narrow) == narrow);
            assert(take_lowest_one_bit(narrow) == narrow);
        }
    }
    // Mixed patterns, negative counts and extreme counts exercise shift masking.
    const std::array<std::int32_t, 11> counts = {0, 1, -1, 31, 32, -32, 63, 64, -64,
        std::numeric_limits<std::int32_t>::min(), std::numeric_limits<std::int32_t>::max()};
    for (const auto count : counts) {
        const auto narrow = std::bit_cast<std::int32_t>(std::uint32_t{0x87654321});
        const auto wide = std::bit_cast<std::int64_t>(std::uint64_t{0x8765432101234567});
        assert(rotate_right(rotate_left(narrow, count), count) == narrow);
        assert(rotate_left(rotate_right(wide, count), count) == wide);
    }
    assert(rotate_left(std::int32_t{1}, -1) == std::numeric_limits<std::int32_t>::min());
    assert(rotate_right(std::int64_t{1}, 1) == std::numeric_limits<std::int64_t>::min());
    assert(take_highest_one_bit(std::int32_t{-1}) == std::numeric_limits<std::int32_t>::min());
    assert(take_lowest_one_bit(std::numeric_limits<std::int64_t>::min()) == std::numeric_limits<std::int64_t>::min());
    const auto float_nan1 = float_companion::from_bits(std::int32_t{0x7fc00001});
    const auto float_nan2 = float_companion::from_bits(std::bit_cast<std::int32_t>(std::uint32_t{0xffc12345}));
    assert(is_nan(float_nan1) && is_nan(float_nan2));
    assert(!is_finite(float_nan1) && !is_infinite(float_nan1));
    assert(to_raw_bits(float_nan1) != to_raw_bits(float_nan2));
    assert(to_bits(float_nan1) == to_bits(float_nan2));
    const auto double_nan1 = double_companion::from_bits(std::int64_t{0x7ff8000000000001});
    const auto double_nan2 = double_companion::from_bits(std::bit_cast<std::int64_t>(std::uint64_t{0xfff8123456789012}));
    assert(is_nan(double_nan1) && is_nan(double_nan2));
    assert(to_raw_bits(double_nan1) != to_raw_bits(double_nan2));
    assert(to_bits(double_nan1) == to_bits(double_nan2));
    const auto negative_zero = std::numeric_limits<std::int64_t>::min();
    assert(to_raw_bits(double_companion::from_bits(negative_zero)) == negative_zero);
    assert(to_bits(double_companion::from_bits(negative_zero)) == negative_zero);
    assert(is_infinite(std::numeric_limits<double>::infinity()));
    assert(is_infinite(-std::numeric_limits<float>::infinity()));
    assert(!is_finite(std::numeric_limits<double>::infinity()));
    assert(is_finite(std::numeric_limits<float>::denorm_min()));
    assert(is_finite(std::numeric_limits<double>::max()));
}
