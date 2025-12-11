#pragma once
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

/**
 * Base class for SafeCollector containing non-generic context validation logic.
 * Moved to .cpp file to reduce template bloat.
 */
class SafeCollectorBase {
public:
    SafeCollectorBase(CoroutineContext collectContext);
    virtual ~SafeCollectorBase() = default;

protected:
    void check_context(const CoroutineContext& currentContext);
    
    CoroutineContext collect_context_;
    int collect_context_size_;
};

/**
 * SafeCollector that ensures flow invariants.
 */
template <typename T>
class SafeCollector : public FlowCollector<T>, public SafeCollectorBase {
public:
    SafeCollector(FlowCollector<T>* downstream, CoroutineContext collectContext)
        : SafeCollectorBase(collectContext), downstream_(downstream) {}

    virtual void emit(T value) override {
        // Get current context (simplified: in C++ we often pass context explicitly or use thread local)
        // Ideally we grab it from `currentCoroutineContext()`.
        // For now, let's assume we can get it or we are called from a suspending function that has it.
        // BUT emit is not suspending in our current C++ interface (void emit).
        // This is a major issue identified in the audit: emit SHOULD be suspending/awaitable.
        
        // Assuming we fix suspension later, we still valid context here.
        // auto current = currentCoroutineContext();
        // check_context(current); 
        
        // Since we don't have global context access easily without arguments, 
        // and our audit showed emit is void, we stub the actual check call site
        // but Provide the logic in Base.
        
        downstream_->emit(value);
    }

private:
    FlowCollector<T>* downstream_;
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
