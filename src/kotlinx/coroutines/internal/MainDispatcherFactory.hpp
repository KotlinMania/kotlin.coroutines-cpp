#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/MainDispatcherFactory.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 */

#include <string>
#include <vector>

namespace kotlinx::coroutines {

// Forward declaration of the dispatcher this factory produces (defined in
// MainCoroutineDispatcher.hpp).
class MainCoroutineDispatcher;

namespace internal {

/**
 * @suppress Emulating DI for Kotlin object's
 *
 * Upstream annotation: `@InternalCoroutinesApi` — translated as documentation only;
 * there is no C++ annotation equivalent.
 *
 * Upstream:
 *   public interface MainDispatcherFactory {
 *       public val loadPriority: Int
 *       public fun createDispatcher(allFactories: List<MainDispatcherFactory>): MainCoroutineDispatcher
 *       public fun hintOnError(): String? = null
 *   }
 */
class MainDispatcherFactory {
public:
    virtual ~MainDispatcherFactory() = default;

    /** Upstream: public val loadPriority: Int — higher priority wins. */
    virtual int load_priority() const = 0;

    /**
     * Creates the main dispatcher. [all_factories] parameter contains all factories found by
     * service loader. This method is not guaranteed to be idempotent.
     *
     * It is required that this method fails with an exception instead of returning an instance
     * that doesn't work correctly as a [Delay]. The reason for this is that, on the JVM,
     * [DefaultDelay] will use [Dispatchers.Main] for most delays by default if this method
     * returns an instance without throwing.
     */
    virtual MainCoroutineDispatcher* create_dispatcher(
        const std::vector<MainDispatcherFactory*>& all_factories) = 0;

    /**
     * Hint used along with error message when the factory failed to create a dispatcher.
     *
     * Upstream: public fun hintOnError(): String? = null
     * Empty string is used as the null sentinel; callers should treat it as "no hint".
     */
    virtual std::string hint_on_error() const { return std::string(); }
};

} // namespace internal
} // namespace kotlinx::coroutines
