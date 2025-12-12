#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/MainDispatcherFactory.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: @InternalCoroutinesApi annotation - translate to comment
// TODO: List type needs std::vector or similar
// TODO: MainCoroutineDispatcher, Delay need C++ equivalents

#include <vector>
#include <string>

namespace kotlinx {
    namespace coroutines {
        namespace {
            // Forward declarations
            class MainCoroutineDispatcher;
            class Delay;

            /**
 * @suppress Emulating DI for Kotlin object's
 */
            // TODO: @InternalCoroutinesApi annotation
            class MainDispatcherFactory {
            public:
                virtual ~MainDispatcherFactory() = default;

                int load_priority; // higher priority wins

                /**
     * Creates the main dispatcher. [allFactories] parameter contains all factories found by service loader.
     * This method is not guaranteed to be idempotent.
     *
     * It is required that this method fails with an exception instead of returning an instance that doesn't work
     * correctly as a [Delay].
     * The reason for this is that, on the JVM, [DefaultDelay] will use [Dispatchers.Main] for most delays by default
     * if this method returns an instance without throwing.
     */
                virtual MainCoroutineDispatcher *create_dispatcher(
                    const std::vector<MainDispatcherFactory *> &all_factories) = 0;

                /**
     * Hint used along with error message when the factory failed to create a dispatcher.
     */
                virtual std::string hint_on_error() { return ""; }
            };
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx