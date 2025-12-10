#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include <functional>
#include <vector>

namespace kotlinx {
namespace coroutines {
namespace flow {

// Implementation of filter operator
template <typename T>
class FilteringFlow : public Flow<T> {
    Flow<T>* upstream_;
    std::function<bool(T)> predicate_;

public:
    FilteringFlow(Flow<T>* upstream, std::function<bool(T)> predicate) 
        : upstream_(upstream), predicate_(predicate) {}

    void collect(FlowCollector<T>* collector) override {
        struct FilteringCollector : public FlowCollector<T> {
            FlowCollector<T>* downstream;
            std::function<bool(T)>& predicate;

            FilteringCollector(FlowCollector<T>* d, std::function<bool(T)>& p) 
                : downstream(d), predicate(p) {}

            void emit(T value) override {
                if (predicate(value)) {
                    downstream->emit(value);
                }
            }
        };

        FilteringCollector filtering_collector(collector, predicate_);
        upstream_->collect(&filtering_collector);
    }
};

template <typename T>
Flow<T>* filter(Flow<T>* flow, std::function<bool(T)> predicate) {
    return new FilteringFlow<T>(flow, predicate);
}

// Implementation of map operator
template <typename T, typename R>
class TransformingFlow : public Flow<R> {
    Flow<T>* upstream_;
    std::function<R(T)> transform_;

public:
    TransformingFlow(Flow<T>* upstream, std::function<R(T)> transform) 
        : upstream_(upstream), transform_(transform) {}

    void collect(FlowCollector<R>* collector) override {
        struct TransformingCollector : public FlowCollector<T> {
            FlowCollector<R>* downstream;
            std::function<R(T)>& transform;

            TransformingCollector(FlowCollector<R>* d, std::function<R(T)>& t) 
                : downstream(d), transform(t) {}

            void emit(T value) override {
                downstream->emit(transform(value));
            }
        };

        TransformingCollector transforming_collector(collector, transform_);
        upstream_->collect(&transforming_collector);
    }
};

template <typename T, typename R>
Flow<R>* map(Flow<T>* flow, std::function<R(T)> transform) {
    return new TransformingFlow<T, R>(flow, transform);
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
