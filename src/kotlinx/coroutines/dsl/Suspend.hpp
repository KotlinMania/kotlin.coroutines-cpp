#pragma once
/**
 * Kotlin-aligned suspend DSL helpers.
 *
 * Two approaches supported:
 *
 * 1. Plugin-based: The compiler plugin recognizes `suspend(expr)` as a suspension
 *    point inside functions marked `[[suspend]]` and generates state machines with
 *    void* labels and computed gotos (Kotlin/Native indirectbr parity).
 *
 * 2. Macro-based: For hand-written suspend functions without the plugin, use
 *    coroutine_begin/coroutine_yield/coroutine_end macros with __LINE__ state.
 *
 * Plugin example:
 * ```cpp
 * [[suspend]]
 * void* foo(std::shared_ptr<Continuation<void*>> completion) {
 *     suspend(bar(completion));
 *     return nullptr;
 * }
 * ```
 *
 * Macro example:
 * ```cpp
 * struct MyCoroutine {
 *     int _label = 0;
 *     void* invoke_suspend(Result<void*> result) {
 *         coroutine_begin(this)
 *         // code before suspend
 *         coroutine_yield(this, yield(completion));
 *         // code after suspend
 *         coroutine_end(this)
 *     }
 * };
 * ```
 */

#include <utility>
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

namespace kotlinx {
namespace coroutines {
namespace dsl {

/**
 * Plugin marker - zero-cost identity function.
 * The plugin transforms calls to suspend() into state machine transitions.
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
 * Computed goto mode (KXS_COMPUTED_GOTO):
 * Uses GCC/Clang labels-as-values extension for Kotlin/Native parity.
 *
 * The _label field is void* storing blockaddress:
 *   - nullptr on first call → jump to start
 *   - &&resume_label on resume → indirectbr to that label
 *
 * This compiles to LLVM indirectbr + blockaddress, matching Kotlin/Native exactly.
 *
 * Requirements:
 *   - Coroutine class must have: void* _label = nullptr;
 *   - Each coroutine_yield must use a unique label (via __LINE__ or __COUNTER__)
 */
#if defined(KXS_COMPUTED_GOTO) && (defined(__GNUC__) || defined(__clang__))

// Helper to create unique label names
#define _KXS_CONCAT(a, b) a##b
#define _KXS_LABEL(prefix, line) _KXS_CONCAT(prefix, line)

#define coroutine_begin(c) \
    if ((c)->_label == nullptr) goto _kxs_start; \
    goto *(c)->_label; \
    _kxs_start:

// Store blockaddress, execute expr, check if suspended, provide resume point
#define coroutine_yield(c, expr) \
    do { \
        (c)->_label = &&_KXS_LABEL(_kxs_resume_, __LINE__); \
        { \
            auto _kxs_tmp = (expr); \
            if (::kotlinx::coroutines::intrinsics::is_coroutine_suspended(_kxs_tmp)) \
                return _kxs_tmp; \
        } \
        _KXS_LABEL(_kxs_resume_, __LINE__):; \
    } while (0)

#define coroutine_end(c) \
    return nullptr;

#else
// Runtime mode: switch/__LINE__ state machine (works without IR transform)

#define coroutine_begin(c) \
    switch ((c)->_label) { \
    case 0:

#define coroutine_yield(c, expr) \
    do { \
        (c)->_label = __LINE__; \
        { \
            auto _tmp = (expr); \
            if (::kotlinx::coroutines::intrinsics::is_coroutine_suspended(_tmp)) \
                return _tmp; \
        } \
        case __LINE__:; \
    } while (0)

#define coroutine_end(c) \
    } \
    return nullptr;

#endif

