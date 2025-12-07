#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/concurrent/src/Dispatchers.kt
//
// TODO: Implement CoroutineDispatcher class
// TODO: Handle 'expect val' - this declares a platform-specific property
// TODO: Implement extension property syntax (Dispatchers.IO)
// TODO: Map @Suppress annotation to appropriate C++ pragma or comment

namespace kotlinx {
namespace coroutines {

/**
 * The [CoroutineDispatcher] that is designed for offloading blocking IO tasks to a shared pool of threads.
 * Additional threads in this pool are created on demand.
 * Default IO pool size is `64`; on JVM it can be configured using JVM-specific mechanisms,
 * please refer to `Dispatchers.IO` documentation on JVM platform.
 *
 * ### Elasticity for limited parallelism
 *
 * `Dispatchers.IO` has a unique property of elasticity: its views
 * obtained with [CoroutineDispatcher.limitedParallelism] are
 * not restricted by the `Dispatchers.IO` parallelism. Conceptually, there is
 * a dispatcher backed by an unlimited pool of threads, and both `Dispatchers.IO`
 * and views of `Dispatchers.IO` are actually views of that dispatcher. In practice
 * this means that, despite not abiding by `Dispatchers.IO`'s parallelism
 * restrictions, its views share threads and resources with it.
 *
 * In the following example
 * ```
 * // 100 threads for MySQL connection
 * auto myMysqlDbDispatcher = Dispatchers.IO.limitedParallelism(100)
 * // 60 threads for MongoDB connection
 * auto myMongoDbDispatcher = Dispatchers.IO.limitedParallelism(60)
 * ```
 * the system may have up to `64 + 100 + 60` threads dedicated to blocking tasks during peak loads,
 * but during its steady state there is only a small number of threads shared
 * among `Dispatchers.IO`, `myMysqlDbDispatcher` and `myMongoDbDispatcher`
 *
 * It is recommended to replace manually created thread-backed executors with `Dispatchers.IO.limitedParallelism` instead:
 * ```
 * // Requires manual closing, allocates resources for all threads
 * auto databasePoolDispatcher = newFixedThreadPoolContext(128)
 *
 * // Provides the same number of threads as a resource but shares and caches them internally
 * auto databasePoolDispatcher = Dispatchers.IO.limitedParallelism(128)
 * ```
 */
// @Suppress("EXTENSION_SHADOWED_BY_MEMBER")
// TODO: 'expect val' means platform-specific implementation required
// TODO: This is an extension property on Dispatchers class - implement as namespace member or static method
// CoroutineDispatcher* io; // declared in Dispatchers namespace/class

} // namespace coroutines
} // namespace kotlinx
