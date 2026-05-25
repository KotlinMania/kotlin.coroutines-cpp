/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.flow.internal
 *
 * SafeCollector wraps a downstream FlowCollector and enforces the upstream context
 * preservation contract: a flow may only emit from the context it was collected on.
 * The full SafeCollectorBase / SafeCollector implementation lives as templates in the
 * matching header (flow/internal/SafeCollector.hpp); this translation unit is the
 * inventory companion for the file pair.
 */

#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"
