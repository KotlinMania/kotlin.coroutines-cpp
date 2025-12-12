// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/LocalAtomics.common.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: expect class needs platform-specific implementation
// TODO: std::atomic can be used as implementation

#include <atomic>

namespace kotlinx {
    namespace coroutines {
        namespace internal {
            /*
 * These are atomics that are used as local variables
 * where atomicfu doesn't support its tranformations.
 *
 * Have `Local` prefix to avoid AFU clashes during star-imports
 *
 * TODO: remove after https://youtrack.jetbrains.com/issue/KT-62423/
 */
            // TODO: expect class - needs platform-specific implementation
            class LocalAtomicInt {
            private:
                std::atomic<int> value_;

            public:
                explicit LocalAtomicInt(int value) : value_(value) {
                }

                int get() { return value_.load(); }
                void set(int value) { value_.store(value); }
                int decrement_and_get() { return value_.fetch_sub(1) - 1; }
            };
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx