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
        explicit FlowCollectorImpl(std::function<void(T)> e) : emit_impl(std::move(e)) {}

        void* emit(T value, Continuation<void*>* /*continuation*/) override {
            emit_impl(std::move(value));
            return nullptr;
        }
    };

    template<typename T>
    struct FlowImpl : public Flow<T> {
        std::function<void*(FlowCollector<T>*, Continuation<void*>*)> collect_impl;
        explicit FlowImpl(std::function<void*(FlowCollector<T>*, Continuation<void*>*)> c) : collect_impl(std::move(c)) {}

        void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override {
            return collect_impl(collector, continuation);
        }
    };

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
