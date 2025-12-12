/**
 * @file AbstractCoroutine.cpp
 * @brief Implementation of AbstractCoroutine.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/AbstractCoroutine.hpp`.
 *
 * Use this file for explicit template instantiations or non-template helper functions.
 */

#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/Unit.hpp"

namespace kotlinx {
    namespace coroutines {
        // AbstractCoroutine is a template class and its implementation has been moved 
        // to include/kotlinx/coroutines/AbstractCoroutine.hpp to support template instantiation.
        //
        // Original Kotlin file: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt
        // Use this file for explicit template instantiations or non-template helper functions.

        // Explicit instantiation for common types to ensure compilation
        // template class AbstractCoroutine<void>; // void instantiation is not supported, use Unit
        template class AbstractCoroutine<Unit>;
    } // namespace coroutines
} // namespace kotlinx