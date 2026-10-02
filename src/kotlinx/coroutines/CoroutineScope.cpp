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
#include "kotlinx/coroutines/internal/Scopes.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"

namespace kotlinx {
    namespace coroutines {
        GlobalScope *GlobalScope::instance() {
            static GlobalScope s_instance;
            return &s_instance;
        }

        std::shared_ptr<CoroutineContext> GlobalScope::get_coroutine_context() const {
            return EmptyCoroutineContext::instance();
        }

        std::shared_ptr<CoroutineScope> create_coroutine_scope(std::shared_ptr<CoroutineContext> context) {
            if (!context) {
                context = EmptyCoroutineContext::instance();
            }
            if (context->get(Job::type_key) != nullptr) {
                return std::make_shared<internal::ContextScope>(context);
            }
            auto job_ctx = std::dynamic_pointer_cast<CoroutineContext>(make_job());
            return std::make_shared<internal::ContextScope>(context->operator+(job_ctx));
        }
    } // namespace coroutines
} // namespace kotlinx