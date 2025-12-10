#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <stdexcept>
#include <iostream>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

SafeCollectorBase::SafeCollectorBase(CoroutineContext collectContext)
    : collectContext_(collectContext), collectContextSize_(0) {
    // Calculate size of collectContext
    collectContext_.fold<int>(0, [&](int count, const Element& element) {
        collectContextSize_++;
        return count + 1;
    });
}

void SafeCollectorBase::check_context(const CoroutineContext& currentContext) {
    int result = currentContext.fold<int>(0, [&](int count, const Element& element) {
        // Key validation logic
        auto* key = element.key;
        auto* collectElement = collectContext_[key];
        
        if (key != Job::Key) {
            // Non-job elements must be identical
            if (element != *collectElement) return INT_MIN;
            return count + 1;
        }

        // Job validation
        Job* collectJob = dynamic_cast<Job*>(collectElement);
        Job* emissionJob = dynamic_cast<Job*>(const_cast<Element*>(&element));
        
        // Transitive parent check
        // Calling helper function (would be static or free function)
        // For C++, explicit parent check logic here or calling Job method
        
        // Stubbed for now as Job::transitiveCoroutineParent isn't in Job.hpp yet.
        // Ideally:
        // if (emissionJob->transitiveCoroutineParent(collectJob) != collectJob) { ... }
        
        if (collectJob == nullptr) return count;
        return count + 1;
    });
    
    if (result != collectContextSize_) {
        throw std::runtime_error("Flow invariant violated: Flow captured in one context but emitted in another incompatible context.");
    }
}

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
