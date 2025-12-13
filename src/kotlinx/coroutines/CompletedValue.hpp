#pragma once
/**
 * @file CompletedValue.hpp
 * @brief Job completion state wrapper for successful results.
 *
 * Kotlin source reference:
 * - kotlinx-coroutines-core/common/src/JobSupport.kt (stores completion state as `Any?`)
 *
 * In Kotlin, a Job's completion state is `Any?` and can be a regular value (including `Unit` or `null`)
 * or an exceptional wrapper (`CompletedExceptionally`).
 *
 * In C++, JobSupport stores states as `JobState*` and uses `dynamic_cast` to distinguish exceptional
 * vs non-exceptional completion. Therefore, successful completion values must also be represented as
 * `JobState`-derived objects to avoid undefined behavior.
 */

#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include <utility>

namespace kotlinx::coroutines {

template <typename T>
struct CompletedValue final : public JobState {
    explicit CompletedValue(T v) : value(std::move(v)) {}
    T value;
};

} // namespace kotlinx::coroutines

