#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include <functional>
#include <memory>
#include <utility>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * Returns flow where all subsequent repetitions of the same value are filtered out.
 */
template<typename T>
std::shared_ptr<Flow<T>> distinctUntilChanged(std::shared_ptr<Flow<T>> flow) {
    return distinctUntilChangedBy<T, T>(flow, [](T it) { return it; });
}

/**
 * Returns flow where all subsequent repetitions of the same value are filtered out, 
 * when compared with each other via the provided [areEquivalent] function.
 */
template<typename T, typename Fn>
std::shared_ptr<Flow<T>> distinctUntilChanged(std::shared_ptr<Flow<T>> flow, Fn are_equivalent) {
    auto key_selector = [](T it) { return it; };
    return distinctUntilChangedBy<T, T>(flow, key_selector, are_equivalent);
}

/**
 * Returns flow where all subsequent repetitions of the same key are filtered out.
 */
template<typename T, typename K, typename KeySelector, typename AreEquivalent = std::function<bool(K, K)>>
std::shared_ptr<Flow<T>> distinctUntilChangedBy(
    std::shared_ptr<Flow<T>> src, 
    KeySelector key_selector, 
    AreEquivalent are_equivalent = [](K a, K b) { return a == b; }
) {
    return flow<T>([src, key_selector, are_equivalent](FlowCollector<T>* collector) {
        bool has_key = false;
        K previous_key;
        
        src->collect([&](T value) {
            K key = key_selector(value);
            if (!has_key || !are_equivalent(previous_key, key)) {
                has_key = true;
                previous_key = key;
                collector->emit(value);
            }
        });
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
