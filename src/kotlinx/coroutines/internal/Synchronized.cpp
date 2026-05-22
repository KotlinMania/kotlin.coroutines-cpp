/**
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/Synchronized.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * The native-side `SynchronizedObject` and `synchronizedImpl` are owned by the common
 * companion (internal/Synchronized.common.cpp), which backs them with
 * std::recursive_mutex and std::lock_guard. This translation unit is the per-platform
 * inventory companion for the file pair; the native actual delegates to the common
 * implementation because the C++ port's mutex primitive is already cross-platform.
 */

#include <mutex>
