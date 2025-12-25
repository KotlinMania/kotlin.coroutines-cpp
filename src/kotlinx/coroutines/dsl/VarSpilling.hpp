#pragma once
/**
 * @file VarSpilling.hpp
 * @brief DSL for coroutine variable spilling (save/restore state).
 *
 * Kotlin source: kotlin-native/.../lower/CoroutinesVarSpillingLowering.kt
 *
 * In Kotlin/Native, variables that are live across suspend points must be
 * saved to the coroutine struct before suspension and restored after resumption.
 * This is handled automatically by the compiler's liveness analysis.
 *
 * In C++, we provide macros to manually specify which variables to spill.
 * The kxs-inject tool can optionally automate this via LLVM liveness analysis.
 */

namespace kotlinx {
namespace coroutines {
namespace dsl {

/**
 * Marker functions for IR tooling.
 *
 * These are no-ops that serve as markers for the kxs-inject tool to identify
 * save/restore points. The tool can then insert actual spill code based on
 * liveness analysis.
 */
extern "C" {
    // Marker: save all live variables to coroutine struct
    inline void __kxs_save_state() {}

    // Marker: restore all live variables from coroutine struct
    inline void __kxs_restore_state() {}
}

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx

/**
 * Manual variable spilling macros.
 *
 * Use these when you need explicit control over which variables are saved/restored.
 * The coroutine struct must have fields for each spilled variable.
 *
 * Usage:
 * ```cpp
 * class MyCoroutine : public ContinuationImpl {
 *     void* _label = nullptr;
 *     int _spilled_x;      // Field for spilled variable x
 *     std::string _spilled_s;  // Field for spilled variable s
 *
 *     void* invoke_suspend(Result<void*> result) {
 *         int x;
 *         std::string s;
 *
 *         coroutine_begin(this)
 *
 *         x = 10;
 *         s = "hello";
 *
 *         // Save before suspend
 *         KXS_SAVE_VAR(this, x, _spilled_x);
 *         KXS_SAVE_VAR(this, s, _spilled_s);
 *
 *         coroutine_yield(this, some_suspend_call(completion));
 *
 *         // Restore after resume
 *         KXS_RESTORE_VAR(this, x, _spilled_x);
 *         KXS_RESTORE_VAR(this, s, _spilled_s);
 *
 *         // x and s are now valid again
 *
 *         coroutine_end(this)
 *     }
 * };
 * ```
 */

// Save a single variable to a coroutine field
#define KXS_SAVE_VAR(coro, var, field) \
    do { (coro)->field = (var); } while(0)

// Restore a single variable from a coroutine field
#define KXS_RESTORE_VAR(coro, var, field) \
    do { (var) = (coro)->field; } while(0)

// Save multiple variables (up to 8)
#define KXS_SAVE_VARS_1(c, v1, f1) KXS_SAVE_VAR(c, v1, f1)
#define KXS_SAVE_VARS_2(c, v1, f1, v2, f2) KXS_SAVE_VAR(c, v1, f1); KXS_SAVE_VAR(c, v2, f2)
#define KXS_SAVE_VARS_3(c, v1, f1, v2, f2, v3, f3) KXS_SAVE_VARS_2(c, v1, f1, v2, f2); KXS_SAVE_VAR(c, v3, f3)
#define KXS_SAVE_VARS_4(c, v1, f1, v2, f2, v3, f3, v4, f4) KXS_SAVE_VARS_3(c, v1, f1, v2, f2, v3, f3); KXS_SAVE_VAR(c, v4, f4)

// Restore multiple variables (up to 8)
#define KXS_RESTORE_VARS_1(c, v1, f1) KXS_RESTORE_VAR(c, v1, f1)
#define KXS_RESTORE_VARS_2(c, v1, f1, v2, f2) KXS_RESTORE_VAR(c, v1, f1); KXS_RESTORE_VAR(c, v2, f2)
#define KXS_RESTORE_VARS_3(c, v1, f1, v2, f2, v3, f3) KXS_RESTORE_VARS_2(c, v1, f1, v2, f2); KXS_RESTORE_VAR(c, v3, f3)
#define KXS_RESTORE_VARS_4(c, v1, f1, v2, f2, v3, f3, v4, f4) KXS_RESTORE_VARS_3(c, v1, f1, v2, f2, v3, f3); KXS_RESTORE_VAR(c, v4, f4)

/**
 * Automatic spilling markers for IR tooling.
 *
 * These emit marker calls that kxs-inject can use to insert automatic
 * spill/restore code based on liveness analysis.
 *
 * Usage:
 * ```cpp
 * coroutine_yield(this, {
 *     KXS_AUTO_SAVE();  // Marker: save live vars here
 *     auto result = some_suspend_call(completion);
 *     KXS_AUTO_RESTORE();  // Marker: restore live vars here
 *     return result;
 * });
 * ```
 */
#define KXS_AUTO_SAVE() ::kotlinx::coroutines::dsl::__kxs_save_state()
#define KXS_AUTO_RESTORE() ::kotlinx::coroutines::dsl::__kxs_restore_state()

/**
 * Combined yield with automatic spilling.
 *
 * This is a convenience macro that combines coroutine_yield with automatic
 * save/restore markers for kxs-inject processing.
 */
#define coroutine_yield_spill(c, expr) \
    do { \
        KXS_AUTO_SAVE(); \
        coroutine_yield(c, expr); \
        KXS_AUTO_RESTORE(); \
    } while(0)
