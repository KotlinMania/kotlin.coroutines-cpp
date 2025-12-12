/**
 * @file Builders.cpp
 * @brief Implementation of Flow Builders.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt
 *
 * Kotlin imports:
 * - kotlinx.coroutines.*
 * - kotlinx.coroutines.channels.*
 * - kotlinx.coroutines.channels.Channel.Factory.BUFFERED
 * - kotlinx.coroutines.flow.internal.*
 * - kotlinx.coroutines.flow.internal.unsafeFlow as flow
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/flow/FlowBuilders.hpp`.
 */

#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"

namespace kotlinx::coroutines::flow {

// Template implementation is in the header.

} // namespace kotlinx::coroutines::flow