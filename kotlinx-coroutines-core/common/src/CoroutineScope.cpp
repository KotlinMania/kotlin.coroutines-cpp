/**
 * @file CoroutineScope.cpp
 * @brief Implementation of CoroutineScope.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/CoroutineScope.hpp`.
 */

#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"

#include "kotlinx/coroutines/context_impl.hpp"

namespace kotlinx {
namespace coroutines {

GlobalScope* GlobalScope::instance() {
    static GlobalScope s_instance;
    return &s_instance;
}

std::shared_ptr<CoroutineContext> GlobalScope::get_coroutine_context() const {
    return EmptyCoroutineContext::instance();
}

} // namespace coroutines
} // namespace kotlinx