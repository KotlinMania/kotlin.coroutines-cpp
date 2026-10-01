/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/Synchronized.common.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream:
 *   @InternalCoroutinesApi public expect open class SynchronizedObject()
 *   @InternalCoroutinesApi public expect inline fun <T> synchronizedImpl(
 *       lock: SynchronizedObject, block: () -> T): T
 *   @InternalCoroutinesApi public inline fun <T> synchronized(
 *       lock: SynchronizedObject, block: () -> T): T = synchronizedImpl(lock, block)
 *
 * The C++ port resolves both `expect`s: `SynchronizedObject` owns a recursive_mutex, and
 * `synchronized_impl` is a std::lock_guard scope. Kotlin's
 * `contract { callsInPlace(block, EXACTLY_ONCE) }` is an invocation-count hint with no
 * C++ equivalent — std::lock_guard already enforces exactly-once entry/exit dynamically.
 */

#include "kotlinx/coroutines/internal/SynchronizedObject.hpp"

namespace kotlinx::coroutines::internal {

// All declarations and templates for SynchronizedObject are in SynchronizedObject.hpp

} // namespace kotlinx::coroutines::internal
