/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/StateFlow.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.flow
 *
 * StateFlow / MutableStateFlow / MutableStateFlowImpl all live as templates in the
 * matching header (flow/StateFlow.hpp) because every operation is parameterised by the
 * element type. This translation unit is the inventory companion for the file pair —
 * an explicit-template-instantiation site for common element types if a future
 * separately-compiled link unit needs them.
 */

#include "kotlinx/coroutines/flow/StateFlow.hpp"
