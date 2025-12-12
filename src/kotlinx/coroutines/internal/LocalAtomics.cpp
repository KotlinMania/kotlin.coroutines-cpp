#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/internal/LocalAtomics.kt
//
// TODO: actual keyword - platform-specific implementation
// TODO: kotlinx.atomicfu atomic primitives

namespace kotlinx {
    namespace coroutines {
        namespace internal {
            // TODO: Remove imports, fully qualify or add includes:
            // import kotlinx.atomicfu.*

            // TODO: actual class
            class LocalAtomicInt {
            private:
                std::atomic<int> i_ref;

            public:
                LocalAtomicInt(int value) : i_ref(value) {
                }

                void set(int value) {
                    i_ref.store(value);
                }

                int get() {
                    return i_ref.load();
                }

                int decrement_and_get() {
                    return i_ref.fetch_sub(1) - 1;
                }
            };
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx