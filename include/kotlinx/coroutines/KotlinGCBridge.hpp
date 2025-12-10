/*
 * Kotlin Native GC Integration Bridge
 * 
 * When our C++ coroutine library is called FROM Kotlin Native code,
 * we can integrate with Kotlin's existing GC by calling these functions.
 * 
 * If the Kotlin Native runtime is NOT present, these are no-ops.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// These are defined in Kotlin Native's runtime (Memory.h)
// If we're called from Kotlin, they're already available
// If we're standalone C++, we provide stub implementations below

#ifndef KOTLIN_NATIVE_RUNTIME_AVAILABLE
#define KOTLIN_NATIVE_RUNTIME_AVAILABLE 0
#endif

#if KOTLIN_NATIVE_RUNTIME_AVAILABLE

// Forward declare Kotlin Native's GC functions
// These are already compiled into the Kotlin Native runtime
extern void Kotlin_mm_switchThreadStateNative() __attribute__((weak));
extern void Kotlin_mm_switchThreadStateRunnable() __attribute__((weak));
extern void Kotlin_mm_safePointFunctionPrologue() __attribute__((weak));
extern void Kotlin_mm_safePointWhileLoopBody() __attribute__((weak));

#else

// Stub implementations when not linked with Kotlin Native
inline void Kotlin_mm_switchThreadStateNative() {}
inline void Kotlin_mm_switchThreadStateRunnable() {}
inline void Kotlin_mm_safePointFunctionPrologue() {}
inline void Kotlin_mm_safePointWhileLoopBody() {}

#endif

#ifdef __cplusplus
}

namespace kotlinx {
namespace coroutines {

/**
 * RAII guard for switching to Native state during long operations.
 * 
 * When called from Kotlin:
 * - Constructor switches to kNative (GC won't wait for this thread)
 * - Destructor switches back to kRunnable
 * 
 * When standalone C++:
 * - No-op (no GC to coordinate with)
 */
class KotlinNativeStateGuard {
public:
    KotlinNativeStateGuard() {
        Kotlin_mm_switchThreadStateNative();
    }
    
    ~KotlinNativeStateGuard() {
        Kotlin_mm_switchThreadStateRunnable();
    }
    
    // Non-copyable, non-movable
    KotlinNativeStateGuard(const KotlinNativeStateGuard&) = delete;
    KotlinNativeStateGuard& operator=(const KotlinNativeStateGuard&) = delete;
};

/**
 * Insert a safepoint check.
 * 
 * Call this in long-running loops to allow GC to run if needed.
 * When called from Kotlin, checks if GC wants to suspend this thread.
 * When standalone C++, this is a no-op.
 */
inline void check_safepoint() {
    Kotlin_mm_safePointWhileLoopBody();
}

/**
 * Check if we're running under Kotlin Native runtime.
 */
inline bool is_kotlin_native_runtime_available() {
#if KOTLIN_NATIVE_RUNTIME_AVAILABLE
    return Kotlin_mm_safePointFunctionPrologue != nullptr;
#else
    return false;
#endif
}

} // namespace coroutines
} // namespace kotlinx

#endif // __cplusplus
