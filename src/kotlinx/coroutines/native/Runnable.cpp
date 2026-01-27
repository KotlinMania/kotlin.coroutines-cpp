#include <functional>
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/Runnable.kt
//
// TODO: actual fun struct - SAM conversion in Kotlin
// TODO: @Deprecated annotation with custom parameters
// TODO: inline function with crossinline

namespace kotlinx {
    namespace coroutines {
        /**
 * A runnable task for [CoroutineDispatcher.dispatch].
 *
 * Equivalent to the type `() -> Unit`.
 */
        // TODO: actual fun struct - use std::function or function pointer
        class Runnable {
        public:
            /**
     * @suppress
     */
            virtual void run() = 0;

            virtual ~Runnable() = default;
        };

        // TODO: @Deprecated annotation
        // "Preserved for binary compatibility, see https://github.com/Kotlin/kotlinx.coroutines/issues/4309"
        // level = DeprecationLevel.HIDDEN
        // TODO: inline function with crossinline
        template<typename F>
        Runnable *make_runnable(F block) {
            class RunnableImpl : public Runnable {
            private:
                F block;

            public:
                RunnableImpl(F block) : block(block) {
                }

                void run() override {
                    block();
                }
            };
            return new RunnableImpl(block);
        }
    } // namespace coroutines
} // namespace kotlinx