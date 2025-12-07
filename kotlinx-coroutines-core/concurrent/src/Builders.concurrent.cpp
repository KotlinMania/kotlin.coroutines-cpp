// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/concurrent/src/Builders.concurrent.kt
//
// TODO: Implement coroutine suspension semantics (runBlocking blocks the thread)
// TODO: Handle 'expect' keyword - this declares a platform-specific implementation
// TODO: Implement CoroutineContext and EmptyCoroutineContext
// TODO: Implement CoroutineScope receiver syntax for block parameter

namespace kotlinx {
namespace coroutines {

/**
 * Runs a new coroutine and **blocks** the current thread until its completion.
 *
 * It is designed to bridge regular blocking code to libraries that are written in suspending style, to be used in
 * `main` functions and in tests.
 *
 * Calling [runBlocking] from a suspend function is redundant.
 * For example, the following code is incorrect:
 * ```
 * suspend fun loadConfiguration() {
 *     // DO NOT DO THIS:
 *     val data = runBlocking { // <- redundant and blocks the thread, do not do that
 *         fetchConfigurationData() // suspending function
 *     }
 * }
 * ```
 *
 * Here, instead of releasing the thread on which `loadConfiguration` runs if `fetchConfigurationData` suspends, it will
 * block, potentially leading to thread starvation issues.
 */
// TODO: 'expect' means platform-specific implementation required
template<typename T>
T run_blocking(/* TODO: CoroutineContext context = EmptyCoroutineContext, */
               /* TODO: suspend CoroutineScope.() -> T block */);

} // namespace coroutines
} // namespace kotlinx
