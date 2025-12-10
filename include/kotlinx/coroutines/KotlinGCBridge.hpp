/**
 * @file KotlinGCBridge.hpp
 * @brief Kotlin Native GC Integration Bridge
 *
 * Provides zero-overhead integration with Kotlin Native's garbage collector
 * for C++ code that may be called from Kotlin Native.
 *
 * ## Architecture
 *
 * This header provides weak-linked declarations to Kotlin Native's thread state
 * management functions. When compiled standalone, these resolve to inline no-op
 * implementations. When linked with Kotlin Native runtime, they resolve to the
 * actual GC coordination functions.
 *
 * ## Thread States
 *
 * Kotlin Native uses a two-state model for threads:
 * - **kRunnable**: Thread can execute Kotlin code and access managed objects
 * - **kNative**: Thread is in native code; GC proceeds without waiting
 *
 * ## Usage
 *
 * ### RAII Guard (Recommended)
 * ```cpp
 * extern "C" void long_operation() {
 *     kotlinx::coroutines::KotlinNativeStateGuard guard;
 *     // Heavy C++ work - GC doesn't wait
 *     do_work();
 *     // Guard destructor switches back to Runnable
 * }
 * ```
 *
 * ### Manual State Control
 * ```cpp
 * extern "C" void mixed_operation() {
 *     Kotlin_mm_switchThreadStateNative();
 *     do_cpp_work();
 *     
 *     Kotlin_mm_switchThreadStateRunnable();
 *     call_kotlin_callback();
 *     
 *     Kotlin_mm_switchThreadStateNative();
 *     do_more_work();
 *     
 *     Kotlin_mm_switchThreadStateRunnable();
 * }
 * ```
 *
 * ### With Safepoint Checks
 * ```cpp
 * extern "C" void interruptible_operation() {
 *     kotlinx::coroutines::KotlinNativeStateGuard guard;
 *     for (int i = 0; i < 1000000; i++) {
 *         do_work();
 *         if (i % 1000 == 0) {
 *             kotlinx::coroutines::check_safepoint();
 *         }
 *     }
 * }
 * ```
 *
 * ## Build Configuration
 *
 * ### Standalone C++ (Default)
 * ```bash
 * cmake -DKOTLIN_NATIVE_RUNTIME_AVAILABLE=0 ..
 * ```
 * Functions are inline stubs (optimized away by compiler).
 *
 * ### With Kotlin Native
 * ```bash
 * cmake -DKOTLIN_NATIVE_RUNTIME_AVAILABLE=1 ..
 * ```
 * Functions are weak-linked; resolved by Kotlin Native runtime at link time.
 *
 * ## Safety Guarantees
 *
 * **In kNative state (after switchThreadStateNative)**:
 * - ✓ Can call C/C++ functions
 * - ✓ Can perform heavy computation
 * - ✓ Can call check_safepoint() periodically
 * - ✗ Cannot access Kotlin objects
 * - ✗ Cannot call Kotlin functions
 * - ✗ Cannot allocate Kotlin objects
 *
 * **In kRunnable state (after switchThreadStateRunnable)**:
 * - ✓ Can access Kotlin objects
 * - ✓ Can call Kotlin functions
 * - ✓ Can allocate Kotlin objects
 * - ⚠ GC may pause thread at safepoints
 *
 * @see docs/KOTLIN_GC_INTEGRATION.md for detailed integration guide
 * @see tests/gc_bridge/ for usage examples
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup KotlinNativeGC Kotlin Native GC Functions
 * @brief Weak-linked declarations to Kotlin Native's GC coordination functions
 * @{
 */

// Forward declarations of Kotlin Native runtime functions.
// These are defined in Kotlin Native's Memory.h (runtime/src/main/cpp/Memory.h).
// When KOTLIN_NATIVE_RUNTIME_AVAILABLE is false, stub implementations are provided below.

#ifndef KOTLIN_NATIVE_RUNTIME_AVAILABLE
#define KOTLIN_NATIVE_RUNTIME_AVAILABLE 0
#endif

#if KOTLIN_NATIVE_RUNTIME_AVAILABLE

/**
 * @brief Switch current thread to Native state
 *
 * Informs the GC that this thread will not access Kotlin-managed objects.
 * The GC can proceed with collections without waiting for this thread.
 *
 * @note Must be paired with Kotlin_mm_switchThreadStateRunnable()
 * @warning Cannot access Kotlin objects while in Native state
 */
extern void Kotlin_mm_switchThreadStateNative() __attribute__((weak));

/**
 * @brief Switch current thread to Runnable state
 *
 * Informs the GC that this thread may access Kotlin-managed objects.
 * The GC will coordinate with this thread at safepoints.
 *
 * @note Must be in Runnable state to interact with Kotlin runtime
 */
extern void Kotlin_mm_switchThreadStateRunnable() __attribute__((weak));

/**
 * @brief Check if GC needs this thread to pause (function prologue)
 *
 * Called automatically by Kotlin compiler at function entry points.
 * Rarely needed in manually-written C++ code.
 */
extern void Kotlin_mm_safePointFunctionPrologue() __attribute__((weak));

/**
 * @brief Check if GC needs this thread to pause (loop body)
 *
 * Should be called periodically in long-running loops to allow GC coordination.
 * This is the primary safepoint check for C++ code.
 *
 * @note Only relevant when thread is in Runnable state
 */
extern void Kotlin_mm_safePointWhileLoopBody() __attribute__((weak));

#else

/**
 * @brief No-op stub: Switch to Native state
 * @note Inlined and optimized away when standalone
 */
inline void Kotlin_mm_switchThreadStateNative() {}

/**
 * @brief No-op stub: Switch to Runnable state
 * @note Inlined and optimized away when standalone
 */
inline void Kotlin_mm_switchThreadStateRunnable() {}

/**
 * @brief No-op stub: Function prologue safepoint
 * @note Inlined and optimized away when standalone
 */
inline void Kotlin_mm_safePointFunctionPrologue() {}

/**
 * @brief No-op stub: Loop body safepoint
 * @note Inlined and optimized away when standalone
 */
inline void Kotlin_mm_safePointWhileLoopBody() {}

#endif

/** @} */

#ifdef __cplusplus
}

namespace kotlinx {
namespace coroutines {

/**
 * @class KotlinNativeStateGuard
 * @brief RAII guard for managing Kotlin Native thread state transitions
 *
 * Automatically switches the current thread to Native state (kNative) on construction
 * and back to Runnable state (kRunnable) on destruction.
 *
 * ## Behavior
 *
 * **When called from Kotlin Native**:
 * - Constructor calls `Kotlin_mm_switchThreadStateNative()`
 * - Destructor calls `Kotlin_mm_switchThreadStateRunnable()`
 * - Thread is in kNative state for the guard's lifetime
 * - GC does not wait for this thread during the guarded scope
 *
 * **When standalone C++**:
 * - All operations are no-ops (inlined and optimized away)
 * - Zero runtime overhead
 *
 * ## Thread Safety
 *
 * Each thread maintains its own state. Guards are thread-local and do not
 * synchronize between threads.
 *
 * ## Usage Example
 *
 * ```cpp
 * extern "C" void mlx_inference(int64_t model_ptr) {
 *     // Switch to Native state
 *     kotlinx::coroutines::KotlinNativeStateGuard guard;
 *     
 *     // Heavy computation - GC doesn't wait
 *     auto* model = reinterpret_cast<mlx::core::nn::Module*>(model_ptr);
 *     auto result = model->forward(input);
 *     
 *     // Guard destructor switches back to Runnable
 *     return result;
 * }
 * ```
 *
 * ## Nesting
 *
 * Guards can be nested. Each guard independently manages state transitions:
 * ```cpp
 * {
 *     KotlinNativeStateGuard outer;  // kRunnable → kNative
 *     do_work();
 *     {
 *         // Already in kNative; redundant but safe
 *         KotlinNativeStateGuard inner;
 *         do_more_work();
 *     }  // Switches to kRunnable (temporarily)
 *     // Outer guard restores kNative
 * }  // Finally back to kRunnable
 * ```
 *
 * @warning While guard is active (thread in kNative state):
 *          - Cannot access Kotlin objects
 *          - Cannot call Kotlin functions
 *          - Cannot allocate Kotlin-managed memory
 *
 * @see Kotlin_mm_switchThreadStateNative()
 * @see Kotlin_mm_switchThreadStateRunnable()
 */
class KotlinNativeStateGuard {
public:
    /**
     * @brief Construct guard and switch to Native state
     *
     * Calls `Kotlin_mm_switchThreadStateNative()` to inform the GC
     * that this thread will not access Kotlin-managed objects.
     */
    KotlinNativeStateGuard() {
        Kotlin_mm_switchThreadStateNative();
    }
    
    /**
     * @brief Destroy guard and switch back to Runnable state
     *
     * Calls `Kotlin_mm_switchThreadStateRunnable()` to inform the GC
     * that this thread may again access Kotlin-managed objects.
     */
    ~KotlinNativeStateGuard() {
        Kotlin_mm_switchThreadStateRunnable();
    }
    
    // Non-copyable, non-movable (RAII resource)
    KotlinNativeStateGuard(const KotlinNativeStateGuard&) = delete;
    KotlinNativeStateGuard& operator=(const KotlinNativeStateGuard&) = delete;
};

/**
 * @brief Insert a GC safepoint check
 *
 * Allows the GC to pause this thread if a collection is pending.
 * Should be called periodically in long-running loops when the thread
 * is in Runnable state.
 *
 * ## When to Use
 *
 * - In loops that may run for extended periods (>1ms)
 * - When thread is in kRunnable state
 * - To reduce GC pause latency
 *
 * ## When NOT to Use
 *
 * - When thread is in kNative state (has no effect)
 * - In tight inner loops (adds overhead)
 * - When already calling Kotlin functions (implicit safepoints)
 *
 * ## Example
 *
 * ```cpp
 * // Processing that occasionally calls Kotlin
 * for (int i = 0; i < 1000000; i++) {
 *     process_item(i);
 *     
 *     if (i % 1000 == 0) {
 *         kotlinx::coroutines::check_safepoint();
 *     }
 * }
 * ```
 *
 * @note When standalone (no Kotlin runtime), this is a no-op
 */
inline void check_safepoint() {
    Kotlin_mm_safePointWhileLoopBody();
}

/**
 * @brief Check if Kotlin Native runtime is available
 *
 * Determines whether this code is running with Kotlin Native runtime
 * or standalone C++.
 *
 * ## Return Value
 *
 * - `true`: Kotlin Native runtime is present; GC functions are active
 * - `false`: Standalone C++; GC functions are no-ops
 *
 * ## Implementation
 *
 * When `KOTLIN_NATIVE_RUNTIME_AVAILABLE` is 1, checks if weak-linked
 * functions are non-null. When 0, always returns false.
 *
 * ## Example
 *
 * ```cpp
 * if (kotlinx::coroutines::is_kotlin_native_runtime_available()) {
 *     std::cout << "Running with Kotlin Native GC integration\n";
 * } else {
 *     std::cout << "Running standalone C++\n";
 * }
 * ```
 *
 * @return true if Kotlin Native runtime is available
 * @return false if running standalone
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
