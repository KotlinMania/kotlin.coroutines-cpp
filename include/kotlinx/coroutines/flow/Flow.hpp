#pragma once
#include "kotlinx/coroutines/flow/FlowCollector.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * An asynchronous data stream that sequentially emits values and completes normally or with an exception.
 */
template <typename T>
struct Flow {
    virtual ~Flow() = default;
    virtual void collect(FlowCollector<T>* collector) = 0;
};

/**
 * Base class for stateful implementations of `Flow`.
 */
template<typename T>
class AbstractFlow : public Flow<T> {
public:
    void collect(FlowCollector<T>* collector) override {
        // TODO: SafeCollector implementation to ensure context preservation
        collect_safely(collector);
    }

    virtual void collect_safely(FlowCollector<T>* collector) = 0;
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
