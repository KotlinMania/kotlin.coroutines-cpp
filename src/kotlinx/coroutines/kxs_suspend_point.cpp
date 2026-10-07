/**
 * @file kxs_suspend_point.cpp
 * @brief Reserves compiler markers for mandatory LLVM coroutine injection.
 *
 * Kotlin/Native lowering uses explicit suspension points in the IR. In this
 * project, the frontend supplies the frame label field and actual function-local
 * block addresses to kxs-inject. These symbols have no runtime implementation.
 *
 * kxs-inject must replace every marker with Kotlin/Native dispatch and stores.
 * An untransformed definition must fail to link instead of silently running
 * without coroutine lowering.
 */

