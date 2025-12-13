#pragma once
/**
 * @file Suspend.hpp
 * @brief Coroutine state machine macros for Kotlin/Native parity.
 *
 * Provides coroutine_begin/coroutine_yield/coroutine_end macros that implement
 * stackless coroutine state machines matching Kotlin/Native's pattern.
 *
 * Two modes:
 * 1. Computed goto (KXS_COMPUTED_GOTO): Uses void* _label with &&label addresses.
 *    Compiles to LLVM indirectbr + blockaddress - exact Kotlin/Native parity.
 *
 * 2. Switch mode (default): Uses int _label with switch/case. Portable fallback.
 *
 * Usage example:
 * ```cpp
 * class MyCoroutine : public ContinuationImpl {
 *     int _label = 0;  // or void* _label = nullptr for computed goto
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
 * Two modes:
 * 1. Computed goto mode (KXS_COMPUTED_GOTO): Uses void* _label with &&label
 *    addresses. Compiles to LLVM indirectbr + blockaddress (Kotlin/Native parity).
 *
 * 2. Switch mode (default): Uses int _label with switch/case. Portable fallback
 *    that works on all compilers including MSVC.
 *
 * Usage:
 *   coroutine_begin(coroutine_ptr)
 *       // code
 *       coroutine_yield(coroutine_ptr, suspend_call_expr);
 *       // more code
 *   coroutine_end(coroutine_ptr)
 *
 * For computed goto mode:
 *   - Define KXS_COMPUTED_GOTO before including this header
 *   - Use void* _label = nullptr in your coroutine class
 *
 * For switch mode (default):
 *   - Use int _label = 0 in your coroutine class
 */

/**
 * Computed goto mode (DEFAULT on GCC/Clang):
 * Uses labels-as-values extension for Kotlin/Native parity.
 *
 * The _label field is void* storing blockaddress:
 *   - nullptr on first call → jump to start
 *   - &&resume_label on resume → indirectbr to that label
 *
 * This compiles to LLVM indirectbr + blockaddress, matching Kotlin/Native exactly.
 * This is REQUIRED for proper Kotlin/Native interop.
 *
 * Requirements:
 *   - Coroutine class must have: void* _label = nullptr;
 *   - Each coroutine_yield must use a unique label (via __LINE__ or __COUNTER__)
 *
 * To force Duff's device fallback on GCC/Clang, define KXS_NO_COMPUTED_GOTO.
 */
#if (defined(__GNUC__) || defined(__clang__)) && !defined(KXS_NO_COMPUTED_GOTO)

#define coroutine_begin(c) \
    if ((c)->_label == nullptr) goto _kxs_start; \
    goto *(c)->_label; \
    _kxs_start:

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
// MSVC fallback: Duff's device state machine (switch/__LINE__)
// This is ONLY for portability to MSVC which lacks computed goto.
// The primary mode is KXS_COMPUTED_GOTO above, which compiles to
// LLVM indirectbr + blockaddress - required for Kotlin/Native interop.

#define coroutine_begin(c) \
    switch ((c)->_label) { \
    case 0:

// __LINE__ creates the unique case label for switch dispatch.
//
// NOTE: This macro expects the enclosing invoke_suspend signature to use the
// parameter name `result` (Result<void*>), matching Kotlin's invokeSuspend contract.
#define coroutine_yield(c, expr) \
    do { \
        (c)->_label = __LINE__; \
        ::__kxs_suspend_point(__LINE__); \
        { \
            auto _tmp = (expr); \
            if (::kotlinx::coroutines::intrinsics::is_coroutine_suspended(_tmp)) \
                return _tmp; \
        } \
        goto _KXS_LABEL(_kxs_cont_, __LINE__); \
        case __LINE__: \
        (void)(result).get_or_throw(); \
        _KXS_LABEL(_kxs_cont_, __LINE__):; \
    } while (0)

// Value-producing suspension point (Duff's device fallback).
#define coroutine_yield_value(c, result, expr, out_lvalue) \
    do { \
        (c)->_label = __LINE__; \
        ::__kxs_suspend_point(__LINE__); \
        { \
            auto _tmp = (expr); \
            if (::kotlinx::coroutines::intrinsics::is_coroutine_suspended(_tmp)) \
                return _tmp; \
            (out_lvalue) = _tmp; \
        } \
        goto _KXS_LABEL(_kxs_cont_, __LINE__); \
        case __LINE__: \
        (out_lvalue) = (result).get_or_throw(); \
        _KXS_LABEL(_kxs_cont_, __LINE__):; \
    } while (0)

#define coroutine_end(c) \
    } \
    return nullptr;

#endif
