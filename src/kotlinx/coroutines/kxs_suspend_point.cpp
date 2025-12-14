/**
 * @file kxs_suspend_point.cpp
 * @brief Defines the IR-visible suspend-point marker used by kxs tooling.
 *
 * Kotlin/Native lowering uses explicit suspension points in the IR. In this
 * project, we tag suspension sites with calls to `__kxs_suspend_point(i32)`.
 *
 * The IR transformation tooling (e.g. `cmake/Modules/kxs_transform_ir.cmake`
 * and `src/kotlinx/coroutines/tools/kxs_inject/`) is expected to rewrite or
 * remove these marker calls. In non-transformed builds, this symbol must exist
 * to satisfy the linker; it is intentionally a no-op.
 */

extern "C" void __kxs_suspend_point(int id) {}

