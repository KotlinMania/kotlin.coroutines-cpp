#pragma once
/**
 * Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/
 * org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2289-2348
 *
 * Coroutine authoring markers for mandatory Kotlin/Native LLVM injection.
 * The frontend supplies the persistent frame label field and function-local
 * resume blocks. kxs-inject emits label loads/stores and indirectbr dispatch.
 * Immediate/suspended/resumed result paths retain the Continuation ABI.
 * Live values still require retained frame storage until compiler spill lowering
 * supplies it. The marker declarations intentionally have no runtime definition.
 */

#include <utility>
#include <optional>
#include <functional>
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

// IR contracts consumed by kxs-inject before optimization/code generation.
extern "C" void __kxs_coroutine_begin(void** label_field) noexcept;
extern "C" void __kxs_suspend_point(int id, void** label_field, void* resume_address) noexcept;

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
 * Compiler extraction must lower suspension before the LLVM injection stage.
 */
template <typename T>
inline T suspend(T&& value) {
    return std::forward<T>(value);
}

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx

/**
 * Frontend result/resume regions for LLVM-injected suspend functions.
 *
 * Supplies labels-as-values to the injector; no source-level resume dispatch
 * or label-field store is generated here.
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
 * - Entry supplies the exact frame field; sites supply actual resume addresses.
 */

/**
 * LLVM injection authoring surface (Clang labels-as-values extension).
 *
 * The _label field is void* storing blockaddress:
 *   - nullptr on first call → jump to start
 *   - &&resume_label on resume → indirectbr to that label
 *
 * kxs-inject constructs LLVM indirectbr and address stores. Kotlin/Native interop
 * additionally requires compatible frame, result, ownership and GC contracts.
 *
 * Requirements:
 *   - Coroutine class must have: void* _label = nullptr;
 *   - Each coroutine_yield must use a unique label (via __LINE__ or __COUNTER__)
 */
#if defined(__clang__)

#define coroutine_begin(c) \
    ::__kxs_coroutine_begin(&(c)->_label); \
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
        ::__kxs_suspend_point(__LINE__, &(c)->_label, &&_KXS_LABEL(_kxs_resume_, __LINE__)); \
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
        ::__kxs_suspend_point(__LINE__, &(c)->_label, &&_KXS_LABEL(_kxs_resume_, __LINE__)); \
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
