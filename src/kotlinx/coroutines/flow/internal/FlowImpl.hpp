#pragma once

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

    template<typename T>
    struct FlowCollectorImpl : public FlowCollector<T> {
        std::function<void(T)> emit_impl;
        FlowCollectorImpl(std::function<void(T)> e) : emit_impl(e) {}
        void emit(T value) override {
            emit_impl(value);
        }
    };

    template<typename T>
    struct FlowImpl : public Flow<T> {
        std::function<void(FlowCollector<T>*)> collect_impl;
        FlowImpl(std::function<void(FlowCollector<T>*)> c) : collect_impl(c) {}
        void collect(FlowCollector<T>* collector) override {
            collect_impl(collector);
        }
    };

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
