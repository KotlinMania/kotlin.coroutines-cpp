/**
 * @file SelectOld.cpp
 * @brief Legacy select implementations for binary compatibility
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/SelectOld.kt
 *
 * For binary compatibility, we need to maintain the previous select implementations.
 * Thus, we keep SelectBuilderImpl and UnbiasedSelectBuilderImpl and implement the
 * functions marked with @PublishedApi.
 *
 * TODO:
 * - Implement SelectBuilderImpl for legacy compatibility
 * - Implement UnbiasedSelectBuilderImpl
 * - Implement selectOld and selectUnbiasedOld test functions
 */

#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"

namespace kotlinx {
namespace coroutines {
namespace selects {

// TODO: Implement legacy select builders for binary compatibility
// These are kept for test purposes and backward compatibility

} // namespace selects
} // namespace coroutines
} // namespace kotlinx
