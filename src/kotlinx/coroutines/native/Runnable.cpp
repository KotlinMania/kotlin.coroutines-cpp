// port-lint: source Runnable.common.kt
/**
 * @file Runnable.cpp
 * @brief A runnable task for CoroutineDispatcher.dispatch
 *
 * Equivalent to the type `() -> Unit`.
 *
 * Transliterated from: kotlinx-coroutines-core/native/src/Runnable.kt
 *
 * TODO(port): actual fun struct - SAM conversion in Kotlin
 * TODO(port): @Deprecated annotation with custom parameters
 * TODO(port): inline function with crossinline
 */

#include <functional>

namespace kotlinx {
    namespace coroutines {
        /**
 * A runnable task for CoroutineDispatcher.dispatch.
 *
 * Equivalent to the type `() -> Unit`.
 */
        // TODO(port): actual fun struct - use std::function or function pointer
        class Runnable {
        public:
            /**
 * @suppress
 */
            virtual void run() = 0;

            virtual ~Runnable() = default;
        };

        // TODO(port): @Deprecated annotation - Preserved for binary compatibility,
        // see https://github.com/Kotlin/kotlinx.coroutines/issues/4309
        // TODO(port): inline function with crossinline
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