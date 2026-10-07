// port-lint: source kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.common.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.common.kt
 *                 and kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt
 */

#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include <limits>
#include <string>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.common.kt:92-97
std::shared_ptr<Job> transitive_coroutine_parent(
    std::shared_ptr<Job> current_job,
    const std::shared_ptr<Job>& collect_job
) {
    auto cur = current_job;
    while (cur != nullptr) {
        if (cur.get() == collect_job.get()) {
            return cur;
        }
        if (!dynamic_cast<kotlinx::coroutines::internal::ScopeCoroutineBase*>(cur.get())) {
            return cur;
        }
        cur = cur->get_parent();
    }
    return nullptr;
}

// Transliterated from: kotlinx-coroutines-core/native/src/flow/internal/SafeCollector.kt:7-14
SafeCollectorBase::SafeCollectorBase(std::shared_ptr<CoroutineContext> collect_context)
    : collect_context_(std::move(collect_context)),
      collect_context_size_(0),
      last_emission_context_(nullptr) {
    collect_context_size_ = collect_context_->fold<int>(0, [](int count, std::shared_ptr<CoroutineContext::Element>) {
        return count + 1;
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/SafeCollector.common.kt:22-90
void SafeCollectorBase::check_context(const CoroutineContext& current_context) {
    const int INT_MIN_VALUE = std::numeric_limits<int>::min();

    int result = current_context.fold<int>(0, [&](int count, std::shared_ptr<CoroutineContext::Element> element) -> int {
        auto* key = element->key();
        auto collect_element = collect_context_->get(key);

        if (key != Job::type_key) {
            if (element.get() != collect_element.get()) {
                return INT_MIN_VALUE;
            }
            return count + 1;
        }

        auto collect_job = std::dynamic_pointer_cast<Job>(collect_element);
        auto emission_job = std::dynamic_pointer_cast<Job>(element);
        auto emission_parent_job = transitive_coroutine_parent(emission_job, collect_job);

        if (emission_parent_job.get() != collect_job.get()) {
            std::string parent_str = emission_parent_job ? emission_parent_job->to_string() : "null";
            std::string collect_str = collect_job ? collect_job->to_string() : "null";
            throw IllegalStateException(
                "Flow invariant is violated:\n"
                "\t\tEmission from another coroutine is detected.\n"
                "\t\tChild of " + parent_str + ", expected child of " + collect_str + ".\n"
                "\t\tFlowCollector is not thread-safe and concurrent emissions are prohibited.\n"
                "\t\tTo mitigate this restriction please use 'channelFlow' builder instead of 'flow'"
            );
        }

        if (collect_job == nullptr) {
            return count;
        } else {
            return count + 1;
        }
    });

    if (result != collect_context_size_) {
        std::string collect_str = collect_context_ ? collect_context_->to_string() : "null";
        std::string current_str = current_context.to_string();
        throw IllegalStateException(
            "Flow invariant is violated:\n"
            "\t\tFlow was collected in " + collect_str + ",\n"
            "\t\tbut emission happened in " + current_str + ".\n"
            "\t\tPlease refer to 'flow' documentation or use 'flowOn' instead"
        );
    }
}

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
