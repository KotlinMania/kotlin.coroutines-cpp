#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, mechanical syntax mapping)
// Original: kotlinx-coroutines-core/common/src/Unconfined.kt
//
// TODO:
// - class declaration needs singleton pattern
// - CoroutineContext.Key infrastructure
// - YieldContext integration with coroutine context
// - @JvmField annotation (affects JVM bytecode generation only)

#include <stdexcept>
#include <string>

namespace kotlinx {
    namespace coroutines {
        class CoroutineContext;
        class CoroutineDispatcher;
        class Runnable;
        class AbstractCoroutineContextElement;

        /**
 * A coroutine dispatcher that is not confined to any specific thread.
 */
        // object
        class Unconfined /* : CoroutineDispatcher() */ {
        private:
            Unconfined() = default; // Private constructor for singleton

        public:
            static Unconfined &instance();

            CoroutineDispatcher &limited_parallelism(int parallelism, const std::string *name);

            bool is_dispatch_needed(const CoroutineContext &context) { return false; }

            void dispatch(const CoroutineContext &context, Runnable &block);

            std::string to_string() const { return "Dispatchers.Unconfined"; }
        };

        /**
 * Used to detect calls to [Unconfined.dispatch] from [yield] function.
 */
        // @PublishedApi
        // internal
        class YieldContext /* : AbstractCoroutineContextElement(Key) */ {
        public:
            // companion object Key : CoroutineContext.Key<YieldContext>
            struct Key {
                // TODO: Implement CoroutineContext.Key interface
            };

            // @JvmField
            bool dispatcher_was_unconfined = false;

            YieldContext() = default;
        };
    } // namespace coroutines
} // namespace kotlinx