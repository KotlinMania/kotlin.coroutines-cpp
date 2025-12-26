#pragma once
/**
 * @file Suspend.hpp
 * @brief Coroutine state machine macros for Kotlin/Native parity.
 *
 * Provides coroutine_begin/coroutine_yield/coroutine_end macros that implement
 * stackless coroutine state machines matching Kotlin/Native's pattern.
 *
 * Uses computed goto (Clang labels-as-values extension):
 *   - `void* _label` stores `&&label` blockaddresses
 *   - `goto *_label` for computed goto dispatch
 *   - Compiles to LLVM `indirectbr` + `blockaddress` (exact Kotlin/Native parity)
 *
 * Usage example:
 * ```cpp
 * class MyCoroutine : public ContinuationImpl {
 *     void* _label = nullptr;  // blockaddress storage (computed goto)
 *     int my_state;    // spilled variables
 *
 *     void* invoke_suspend(Result<void*> result) override {
 *         coroutine_begin(this)
 *
 *         my_state = 10;
 *         coroutine_yield(this, yield(completion));  // suspend point
 *
 *         my_state = 20;
 *         coroutine_yield(this, delay(100, completion));  // another suspend
 *
 *         coroutine_end(this)
 *     }
 * };
 * ```
 */

#include <utility>
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

// IR-visible marker used by kxs tooling (see cmake/Modules/kxs_transform_ir.cmake).
// The transformer is expected to rewrite/remove these calls.
extern "C" void __kxs_suspend_point(int id);

// Helper to create unique label names.
// Note: __LINE__ must be unique per suspend point; do not put multiple
// coroutine_yield/coroutine_yield_value calls on the same source line.
#ifndef _KXS_CONCAT
#define _KXS_CONCAT(a, b) a##b
#endif
#ifndef _KXS_LABEL
#define _KXS_LABEL(prefix, line) _KXS_CONCAT(prefix, line)
#endif

namespace kotlinx {
namespace coroutines {
namespace dsl {

/**
 * Identity function - used to mark suspension points in code.
 * The CMake IR transform can optionally process these for optimization.
 */
template <typename T>
inline T suspend(T&& value) {
    return std::forward<T>(value);
}

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx

/**
 * Macro-based state machine for hand-written suspend functions.
 *
 * Uses computed goto (labels-as-values) for Kotlin/Native binary parity.
 * Compiles to LLVM `indirectbr` + `blockaddress`.
 *
 * Usage:
 *   coroutine_begin(coroutine_ptr)
 *       // code
 *       coroutine_yield(coroutine_ptr, suspend_call_expr);
 *       // more code
 *   coroutine_end(coroutine_ptr)
 *
 * NOTE:
 * - The `invoke_suspend` parameter name must be `result` (Result<void*>) so
 *   the macros can mirror Kotlin's `getOrThrow(resultArgument)` behavior.
 * - Suspension sites emit `__kxs_suspend_point(i32)` markers for IR tooling.
 */

/**
 * Computed goto implementation (Clang labels-as-values extension).
 *
 * The _label field is void* storing blockaddress:
 *   - nullptr on first call → jump to start
 *   - &&resume_label on resume → indirectbr to that label
 *
 * This compiles to LLVM indirectbr + blockaddress, matching Kotlin/Native exactly.
 * Required for proper Kotlin/Native interop.
 *
 * Requirements:
 *   - Coroutine class must have: void* _label = nullptr;
 *   - Each coroutine_yield must use a unique label (via __LINE__ or __COUNTER__)
 */
#if defined(__clang__)

#define coroutine_begin(c) \
    if ((c)->_label == nullptr) goto _kxs_start; \
    goto *(c)->_label; \
    _kxs_start: \
    /* Kotlin: throwIfNotNull(exceptionOrNull(resultArgument)) */ \
    (void)(result).get_or_throw();

// Store blockaddress, execute expr, check if suspended, provide resume point.
//
// This mirrors Kotlin/Native's suspension point behavior:
// - If the call suspends, return COROUTINE_SUSPENDED.
// - On resume, throw if `result` is exceptional (getOrThrow), then continue.
//
// NOTE: This macro expects the enclosing invoke_suspend signature to use the
// parameter name `result` (Result<void*>), matching Kotlin's invokeSuspend contract.
#define coroutine_yield(c, expr) \
    do { \
        (c)->_label = &&_KXS_LABEL(_kxs_resume_, __LINE__); \
        ::__kxs_suspend_point(__LINE__); \
        { \
            auto _kxs_tmp = (expr); \
            if (::kotlinx::coroutines::intrinsics::is_coroutine_suspended(_kxs_tmp)) \
                return _kxs_tmp; \
        } \
        goto _KXS_LABEL(_kxs_cont_, __LINE__); \
        _KXS_LABEL(_kxs_resume_, __LINE__): \
        (void)(result).get_or_throw(); \
        _KXS_LABEL(_kxs_cont_, __LINE__):; \
    } while (0)

// Suspension point that produces a value.
//
// This mirrors Kotlin/Native's IrSuspensionPoint + continuationBlock pattern:
// - normal path computes `expr` and jumps to continuation
// - resume path computes value from `result.get_or_throw()` and jumps to continuation
//
// The "continuation" is implemented via an explicit local label to avoid
// running resume-only code on the non-suspending fast path.
#define coroutine_yield_value(c, result, expr, out_lvalue) \
    do { \
        (c)->_label = &&_KXS_LABEL(_kxs_resume_, __LINE__); \
        ::__kxs_suspend_point(__LINE__); \
        { \
            auto _kxs_tmp = (expr); \
            if (::kotlinx::coroutines::intrinsics::is_coroutine_suspended(_kxs_tmp)) \
                return _kxs_tmp; \
            (out_lvalue) = _kxs_tmp; \
        } \
        goto _KXS_LABEL(_kxs_cont_, __LINE__); \
        _KXS_LABEL(_kxs_resume_, __LINE__): \
        (out_lvalue) = (result).get_or_throw(); \
        _KXS_LABEL(_kxs_cont_, __LINE__):; \
    } while (0)

#define coroutine_end(c) \
    return nullptr;

#else
#error "kotlinx.coroutines-cpp requires Clang for computed goto support"
#endif
