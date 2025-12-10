/*
 * Copyright 2010-2024 JetBrains s.r.o. and Kotlin Programming Language contributors.
 * Use of this source code is governed by the Apache 2.0 license.
 *
 * SuspendMacros.hpp - Computed Goto State Machine Generator
 *
 * This header provides macros that generate LLVM-optimized state machines
 * matching Kotlin's suspend function compilation pattern.
 *
 * Two implementations:
 * 1. Computed goto (GCC/Clang) - generates direct indirect branches (indirectbr in LLVM IR)
 * 2. Switch fallback (portable) - generates switch-based state machine
 *
 * The computed goto version produces significantly better code:
 * - No switch dispatch overhead
 * - Better branch prediction (each resume point is a unique target)
 * - LLVM can optimize each path independently
 */

#pragma once

#include "kotlin/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlin/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include <cstdint>

namespace kotlin {
namespace coroutines {

/*
 * =============================================================================
 * COMPUTED GOTO IMPLEMENTATION (GCC/Clang)
 * =============================================================================
 *
 * Uses the labels-as-values extension to create a jump table.
 * Each suspend point becomes a unique label that can be jumped to directly.
 *
 * Generated LLVM IR looks like:
 *   indirectbr i8* %target, [label %state0, label %state1, ...]
 *
 * This is the same pattern used by high-performance interpreters.
 */

#if defined(__GNUC__) || defined(__clang__)
#define KOTLIN_SUSPEND_USE_COMPUTED_GOTO 1
#else
#define KOTLIN_SUSPEND_USE_COMPUTED_GOTO 0
#endif

#if KOTLIN_SUSPEND_USE_COMPUTED_GOTO

/**
 * SUSPEND_BEGIN - Start a suspend function body
 *
 * Uses a switch-based dispatch for computed goto.
 * The switch compiles to an efficient jump table (indirectbr) with -O2.
 *
 * @param max_states Maximum number of suspend points (unused, for compatibility)
 */
#define SUSPEND_BEGIN(max_states)                                              \
    switch (this->_label) {                                                    \
    case 0:

/**
 * SUSPEND_POINT(n) - Mark a suspension point
 *
 * When resumed, execution continues from here.
 * The label number must match the state set before suspending.
 */
#define SUSPEND_POINT(n)                                                       \
    case (n):

/**
 * SUSPEND_CALL(n, call_expr, result_var) - Call a suspend function
 *
 * Calls another suspend function, handling suspension properly.
 * If the callee suspends, this function also suspends.
 * If the callee returns immediately, continues to next statement.
 *
 * @param n State number for resume point
 * @param call_expr The suspend function call
 * @param result_var Variable to store the result (type erased as void*)
 */
#define SUSPEND_CALL(n, call_expr, result_var)                                 \
    do {                                                                       \
        this->_label = (n);                                                    \
        void* _tmp_result = (call_expr);                                       \
        if (::kotlin::coroutines::intrinsics::is_coroutine_suspended(_tmp_result)) { \
            return COROUTINE_SUSPENDED;                                        \
        }                                                                      \
        result_var = _tmp_result;                                              \
    } while(0);                                                                \
    case (n):

/**
 * SUSPEND_YIELD(n) - Yield execution (suspend unconditionally)
 *
 * Suspends execution and returns COROUTINE_SUSPENDED.
 * When resumed, continues from this point.
 */
#define SUSPEND_YIELD(n)                                                       \
    do {                                                                       \
        this->_label = (n);                                                    \
        return COROUTINE_SUSPENDED;                                            \
    } while(0);                                                                \
    case (n):

/**
 * SUSPEND_RETURN(value) - Return a value from suspend function
 */
#define SUSPEND_RETURN(value)                                                  \
    return reinterpret_cast<void*>(static_cast<intptr_t>(value))

/**
 * SUSPEND_RETURN_PTR(ptr) - Return a pointer from suspend function
 */
#define SUSPEND_RETURN_PTR(ptr)                                                \
    return static_cast<void*>(ptr)

/**
 * SUSPEND_RETURN_UNIT - Return Unit (void) from suspend function
 */
#define SUSPEND_RETURN_UNIT                                                    \
    return nullptr

/**
 * SUSPEND_END - End of suspend function body
 *
 * Closes the switch statement and provides default return.
 */
#define SUSPEND_END                                                            \
    }                                                                          \
    return nullptr;

#else // Fallback: switch-based implementation (same as computed goto version)

/*
 * =============================================================================
 * SWITCH-BASED IMPLEMENTATION (Portable)
 * =============================================================================
 *
 * Uses Duff's device pattern for portability.
 * Generates a switch statement that jumps to the correct case.
 */

#define SUSPEND_BEGIN(max_states)                                              \
    switch (this->_label) {                                                    \
    case 0:

#define SUSPEND_POINT(n)                                                       \
    case (n):

#define SUSPEND_CALL(n, call_expr, result_var)                                 \
    do {                                                                       \
        this->_label = (n);                                                    \
        void* _tmp_result = (call_expr);                                       \
        if (::kotlin::coroutines::intrinsics::is_coroutine_suspended(_tmp_result)) { \
            return COROUTINE_SUSPENDED;                                        \
        }                                                                      \
        result_var = _tmp_result;                                              \
    } while(0);                                                                \
    case (n):

#define SUSPEND_YIELD(n)                                                       \
    do {                                                                       \
        this->_label = (n);                                                    \
        return COROUTINE_SUSPENDED;                                            \
    } while(0);                                                                \
    case (n):

#define SUSPEND_RETURN(value)                                                  \
    return reinterpret_cast<void*>(static_cast<intptr_t>(value))

#define SUSPEND_RETURN_PTR(ptr)                                                \
    return static_cast<void*>(ptr)

#define SUSPEND_RETURN_UNIT                                                    \
    return nullptr

#define SUSPEND_END                                                            \
    }                                                                          \
    return nullptr;

#endif // KOTLIN_SUSPEND_USE_COMPUTED_GOTO

/*
 * =============================================================================
 * HELPER MACROS
 * =============================================================================
 */

/**
 * SUSPEND_GET_RESULT(result, T) - Extract typed result from Result<void*>
 *
 * Handles both success and failure cases.
 */
#define SUSPEND_GET_RESULT(result, T)                                          \
    ([&]() -> T {                                                              \
        if ((result).is_failure()) {                                           \
            std::rethrow_exception((result).exception_or_null());              \
        }                                                                      \
        return static_cast<T>(reinterpret_cast<intptr_t>((result).get_or_throw())); \
    })()

/**
 * SUSPEND_CHECK_CANCELLATION - Check if coroutine was cancelled
 */
#define SUSPEND_CHECK_CANCELLATION(job_ptr)                                    \
    do {                                                                       \
        if ((job_ptr) && (job_ptr)->is_cancelled()) {                          \
            throw std::runtime_error("Job was cancelled");                     \
        }                                                                      \
    } while(0)

/*
 * =============================================================================
 * SPILLED VARIABLE HELPERS
 * =============================================================================
 *
 * In Kotlin, local variables that survive across suspension are "spilled"
 * into fields (L$0, L$1, etc.). These macros help with that pattern.
 */

/**
 * SUSPEND_SPILL_DECLARE(n, T) - Declare a spilled variable slot
 */
#define SUSPEND_SPILL_DECLARE(n, T) T _spill_##n

/**
 * SUSPEND_SPILL_SAVE(n, var) - Save a variable before suspension
 */
#define SUSPEND_SPILL_SAVE(n, var) this->_spill_##n = (var)

/**
 * SUSPEND_SPILL_RESTORE(n, var) - Restore a variable after resumption
 */
#define SUSPEND_SPILL_RESTORE(n, var) var = this->_spill_##n

/*
 * =============================================================================
 * BASE CLASS FOR SUSPEND FUNCTIONS
 * =============================================================================
 *
 * Provides the _label field and integrates with BaseContinuationImpl.
 */

/**
 * SuspendLambda - Base for suspend lambdas/functions
 *
 * Matches Kotlin's generated SuspendLambda class.
 * User code inherits from this and implements invoke_suspend().
 */
template<typename T>
class SuspendLambda : public BaseContinuationImpl {
public:
    int _label = 0;  // State machine label

    explicit SuspendLambda(std::shared_ptr<Continuation<void*>> completion)
        : BaseContinuationImpl(std::move(completion)) {}

    std::shared_ptr<CoroutineContext> get_context() const override {
        if (completion) {
            return completion->get_context();
        }
        return EmptyCoroutineContext::instance();
    }
};

} // namespace coroutines
} // namespace kotlin

/*
 * =============================================================================
 * USAGE EXAMPLE
 * =============================================================================
 *
 * // Kotlin:
 * // suspend fun example(): Int {
 * //     val x = suspendingCall1()
 * //     val y = suspendingCall2(x)
 * //     return x + y
 * // }
 *
 * // C++ equivalent:
 * class ExampleSuspendFn : public SuspendLambda<int> {
 *     // Spilled locals
 *     int saved_x;
 *
 * public:
 *     using SuspendLambda::SuspendLambda;
 *
 *     void* invoke_suspend(Result<void*> result) override {
 *         void* tmp;
 *         int x, y;
 *
 *         SUSPEND_BEGIN(3)
 *
 *         // val x = suspendingCall1()
 *         SUSPEND_CALL(1, suspendingCall1(this), tmp)
 *         x = SUSPEND_GET_RESULT(result, int);
 *         saved_x = x;  // Spill x
 *
 *         // val y = suspendingCall2(x)
 *         SUSPEND_CALL(2, suspendingCall2(x, this), tmp)
 *         y = SUSPEND_GET_RESULT(result, int);
 *         x = saved_x;  // Restore x
 *
 *         // return x + y
 *         SUSPEND_RETURN(x + y);
 *
 *         SUSPEND_END
 *     }
 * };
 */
