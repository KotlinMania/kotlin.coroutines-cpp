#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace flow {

#include <functional>
#include <memory>
//#include "kotlinx/coroutines/flow/internal/NullSurrogate.hpp" // Not found, using local sentinel

namespace kotlinx {
namespace coroutines {
namespace flow {

namespace internal {
    // Local flow builders until central ones are established
    template<typename T>
    struct FlowCollectorImpl : public FlowCollector<T> {
        std::function<void(T)> emit_impl;
        FlowCollectorImpl(std::function<void(T)> e) : emit_impl(e) {}
        void emit(T value) override { emit_impl(value); }
    };

    template<typename T>
    struct FlowImpl : public Flow<T> {
        std::function<void(FlowCollector<T>*)> collect_impl;
        FlowImpl(std::function<void(FlowCollector<T>*)> c) : collect_impl(c) {}
        void collect(FlowCollector<T>* collector) override { collect_impl(collector); }
    };
    
    static const struct NullType {} NULL_SENTINEL;
    static const void* NULL_KEY = &NULL_SENTINEL; 
}

using namespace internal;

/**
 * Returns flow where all subsequent repetitions of the same value are filtered out.
 */
template<typename T>
Flow<T>* distinctUntilChanged(Flow<T>* flow) {
    return distinctUntilChangedBy<T, T>(flow, [](T it) { return it; });
}

/**
 * Returns flow where all subsequent repetitions of the same value are filtered out, 
 * when compared with each other via the provided [areEquivalent] function.
 */
template<typename T, typename Fn>
Flow<T>* distinctUntilChanged(Flow<T>* flow, Fn are_equivalent) {
    auto key_selector = [](T it) { return it; };
    return distinctUntilChangedBy<T, T>(flow, key_selector, are_equivalent);
}

/**
 * Returns flow where all subsequent repetitions of the same key are filtered out.
 */
template<typename T, typename K, typename KeySelector, typename AreEquivalent = std::function<bool(K, K)>>
Flow<T>* distinctUntilChangedBy(
    Flow<T>* flow, 
    KeySelector key_selector, 
    AreEquivalent are_equivalent = [](K a, K b) { return a == b; }
) {
    return new FlowImpl<T>([flow, key_selector, are_equivalent](FlowCollector<T>* collector) {
        // Shared state for previous key. Using void* logic or K type.
        // Kotlin uses Any? which maps to explicit K type here.
        // We need 'k is set' flag or sentinel. 
        auto previous_key = std::make_shared<std::pair<bool, K>>(false, K{});
        
        flow->collect(new FlowCollectorImpl<T>([collector, key_selector, are_equivalent, previous_key](T value) {
            K key = key_selector(value);
            if (!previous_key->first || !are_equivalent(previous_key->second, key)) {
                previous_key->first = true;
                previous_key->second = key;
                collector->emit(value);
            }
        }));
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
