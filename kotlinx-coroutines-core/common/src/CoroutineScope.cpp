/**
 * @file CoroutineScope.cpp
 * @brief Implementation of CoroutineScope.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/CoroutineScope.hpp`.
 */

#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"

namespace kotlinx {
namespace coroutines {

GlobalScope* GlobalScope::instance() {
    static GlobalScope s_instance;
    return &s_instance;
}

std::shared_ptr<CoroutineContext> GlobalScope::get_coroutine_context() const {
    // Return EmptyCoroutineContext shared ptr
    // Since we don't have EmptyCoroutineContext class defined, return base or new impl
    // We need an EmptyCoroutineContext implementation.
    class EmptyContext : public CoroutineContext {
    public:
         // Key is not part of CoroutineContext, but maybe this was intending to act as Element?
         // In original code it had key(). We keep it to avoid regression if used via cast.
         class Key* key() const { return nullptr; }
    };
    return std::make_shared<EmptyContext>();
}

} // namespace coroutines
} // namespace kotlinx