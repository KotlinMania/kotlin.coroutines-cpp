/**
 * @file WhileSelect.cpp
 * @brief Looping select expression implementation
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/WhileSelect.kt
 *
 * Loops while select expression returns true.
 *
 * Note: This is an experimental api. It may be replaced with a higher-performance DSL for selection from loops.
 *
 * TODO:
 * - Implement while_select using the select infrastructure
 * - Integrate with coroutine infrastructure for proper suspension
 */

#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace selects {
            // TODO: Implement while_select
            // Template function that loops while select<bool> returns true
        } // namespace selects
    } // namespace coroutines
} // namespace kotlinx