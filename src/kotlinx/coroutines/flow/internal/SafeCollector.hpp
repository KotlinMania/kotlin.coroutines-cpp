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
    SafeCollectorBase(std::shared_ptr<CoroutineContext> collectContext);
    virtual ~SafeCollectorBase() = default;

protected:
    void check_context(const CoroutineContext& currentContext);
    
    std::shared_ptr<CoroutineContext> collect_context_;
    int collect_context_size_;
};

/**
 * SafeCollector that ensures flow invariants and context preservation.
 *
 * This wrapper collector ensures that emissions happen in the correct context
 * and provides exception transparency guarantees. It wraps a downstream collector
 * and validates context before forwarding emissions.
 *
 * @note **CURRENT LIMITATION**: Context validation is not fully implemented yet.
 */
template <typename T>
class SafeCollector : public FlowCollector<T>, public SafeCollectorBase {
public:
    /**
     * Creates a SafeCollector wrapping the given downstream collector.
     *
     * @param downstream The collector to wrap and protect
     * @param collectContext The context in which collection started
     */
    SafeCollector(FlowCollector<T>* downstream, std::shared_ptr<CoroutineContext> collectContext)
        : SafeCollectorBase(collectContext), downstream_(downstream) {}

    /**
     * Emits a value after validating the execution context.
     *
     * This method ensures that the emission happens in the same context
     * as the original collect() call, preserving flow invariants.
     *
     * @param value The value to emit
     *
     * @note **BROKEN SEMANTICS**: This method should be suspending and return
     *       void* with a Continuation parameter. The current void return breaks
     *       backpressure guarantees.
     *
     * @note **MISSING VALIDATION**: Context validation is not implemented due
     *       to lack of currentCoroutineContext() access. The check_context()
     *       call is stubbed out.
     *
     * @throws IllegalStateException if context validation fails (not implemented)
     */
    void* emit(T value, Continuation<void*>* continuation) override {
        // TODO: Implement proper context validation
        // auto current = currentCoroutineContext();
        // check_context(current); 

        return downstream_->emit(std::move(value), continuation);
    }

private:
    FlowCollector<T>* downstream_;
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
