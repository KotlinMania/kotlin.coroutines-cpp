/**
 * @file Collect.cpp
 * @brief Terminal flow operators: collect, launchIn, collectIndexed, collectLatest, emitAll
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt
 *
 * TODO:
 * - Implement proper launchIn with Job return
 * - Implement collectLatest with cancellation
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <functional>
#include <stdexcept>

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            /**
 * A collector that ignores all values (for collect() with no action).
 */
            template<typename T>
            struct NopCollector : FlowCollector<T> {
                void emit(T) override {
                    // Do nothing
                }
            };

            /**
 * Helper to check for index overflow.
 */
            inline int check_index_overflow(int index) {
                if (index < 0) {
                    throw std::overflow_error("Index overflow has happened");
                }
                return index;
            }

            // Note: Template functions are declared in headers.
            // The implementations here are for documentation and non-template helpers only.
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx