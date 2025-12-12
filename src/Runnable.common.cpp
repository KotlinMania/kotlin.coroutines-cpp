#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, mechanical syntax mapping)
// Original: kotlinx-coroutines-core/common/src/Runnable.common.kt
//
// TODO:
// - expect fun struct needs platform-specific implementation
// - On JVM this should map to java.lang.Runnable

namespace kotlinx {
    namespace coroutines {
        /**
 * A runnable task for [CoroutineDispatcher.dispatch].
 *
 * It is equivalent to the type `() -> Unit`, but on the JVM, it is represented as a `java.lang.Runnable`,
 * making it easier to wrap the interfaces that expect `java.lang.Runnable` into a [CoroutineDispatcher].
 */
        // TODO: expect fun struct - needs platform-specific implementation (e.g., java.lang.Runnable on JVM)
        class Runnable {
        public:
            /**
     * @suppress
     */
            virtual void run() = 0;

            virtual ~Runnable() = default;
        };
    } // namespace coroutines
} // namespace kotlinx