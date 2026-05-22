#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Distinct.kt
 *
 * Kotlin file header (translated):
 *   @file:JvmMultifileClass
 *   @file:JvmName("FlowKt")
 *   package kotlinx.coroutines.flow
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/SharedFlow.hpp" // for StateFlow detection
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <any>
#include <functional>
#include <memory>
#include <utility>

namespace kotlinx::coroutines::flow {

namespace internal_distinct {

/**
 * Marker singleton used as the "no previous value yet" sentinel. Mirrors upstream's
 * `private val NULL: Any` symbol used inside `DistinctFlowImpl.collect`.
 */
struct NullSentinel {};
inline const NullSentinel NULL_VALUE{};

/**
 * Upstream:
 *   private val defaultKeySelector: (Any?) -> Any? = { it }
 */
template <typename T>
inline std::any default_key_selector(T value) {
    return std::any(std::move(value));
}

/**
 * Upstream:
 *   private val defaultAreEquivalent: (Any?, Any?) -> Boolean = { old, new -> old == new }
 */
template <typename T>
inline bool default_are_equivalent(const T& old_value, const T& new_value) {
    return old_value == new_value;
}

/**
 * Upstream:
 *   private class DistinctFlowImpl<T>(
 *       private val upstream: Flow<T>,
 *       @JvmField val keySelector: (T) -> Any?,
 *       @JvmField val areEquivalent: (old: Any?, new: Any?) -> Boolean
 *   ) : Flow<T> { override suspend fun collect(collector: FlowCollector<T>) { ... } }
 */
template <typename T>
class DistinctFlowImpl : public Flow<T> {
public:
    using KeySelector = std::function<std::any(T)>;
    using AreEquivalent = std::function<bool(const std::any&, const std::any&)>;

    DistinctFlowImpl(
        std::shared_ptr<Flow<T>> upstream,
        KeySelector key_selector,
        AreEquivalent are_equivalent)
        : upstream_(std::move(upstream)),
          key_selector_(std::move(key_selector)),
          are_equivalent_(std::move(are_equivalent)) {}

    const KeySelector& key_selector() const { return key_selector_; }
    const AreEquivalent& are_equivalent() const { return are_equivalent_; }

    [[suspend]]
    void* collect(
        FlowCollector<T>* collector,
        std::shared_ptr<Continuation<void*>> completion) override {
        bool has_key = false;
        std::any previous_key;
        // Upstream: upstream.collect { value ->
        //               val key = keySelector(value)
        //               if (previousKey === NULL || !areEquivalent(previousKey, key)) { ... }
        //           }
        auto inner = std::make_shared<DistinctInnerCollector>(
            collector, key_selector_, are_equivalent_, &has_key, &previous_key);
        return upstream_->collect(inner.get(), completion.get());
    }

private:
    class DistinctInnerCollector : public FlowCollector<T> {
    public:
        DistinctInnerCollector(
            FlowCollector<T>* downstream,
            KeySelector key_selector,
            AreEquivalent are_equivalent,
            bool* has_key,
            std::any* previous_key)
            : downstream_(downstream),
              key_selector_(std::move(key_selector)),
              are_equivalent_(std::move(are_equivalent)),
              has_key_(has_key),
              previous_key_(previous_key) {}

        void* emit(T value, Continuation<void*>* cont) override {
            auto key = key_selector_(value);
            if (!*has_key_ || !are_equivalent_(*previous_key_, key)) {
                *has_key_ = true;
                *previous_key_ = key;
                return downstream_->emit(std::move(value), cont);
            }
            return nullptr;
        }

    private:
        FlowCollector<T>* downstream_;
        KeySelector key_selector_;
        AreEquivalent are_equivalent_;
        bool* has_key_;
        std::any* previous_key_;
    };

    std::shared_ptr<Flow<T>> upstream_;
    KeySelector key_selector_;
    AreEquivalent are_equivalent_;
};

/**
 * Upstream:
 *   private fun <T> Flow<T>.distinctUntilChangedBy(
 *       keySelector: (T) -> Any?,
 *       areEquivalent: (old: Any?, new: Any?) -> Boolean
 *   ): Flow<T> = when {
 *       this is DistinctFlowImpl<*> && this.keySelector === keySelector && this.areEquivalent === areEquivalent -> this
 *       else -> DistinctFlowImpl(this, keySelector, areEquivalent)
 *   }
 */
template <typename T>
inline std::shared_ptr<Flow<T>> distinct_until_changed_by_impl(
    std::shared_ptr<Flow<T>> upstream,
    typename DistinctFlowImpl<T>::KeySelector key_selector,
    typename DistinctFlowImpl<T>::AreEquivalent are_equivalent) {
    return std::make_shared<DistinctFlowImpl<T>>(
        std::move(upstream), std::move(key_selector), std::move(are_equivalent));
}

} // namespace internal_distinct

/**
 * Returns flow where all subsequent repetitions of the same value are filtered out.
 *
 * Upstream:
 *   public fun <T> Flow<T>.distinctUntilChanged(): Flow<T> = when (this) {
 *       is StateFlow<*> -> this
 *       else -> distinctUntilChangedBy(defaultKeySelector, defaultAreEquivalent)
 *   }
 */
template <typename T>
inline std::shared_ptr<Flow<T>> distinct_until_changed(std::shared_ptr<Flow<T>> flow) {
    if (std::dynamic_pointer_cast<StateFlow<T>>(flow)) {
        return flow; // state flows are always distinct
    }
    return internal_distinct::distinct_until_changed_by_impl<T>(
        std::move(flow),
        [](T value) { return internal_distinct::default_key_selector<T>(std::move(value)); },
        [](const std::any& a, const std::any& b) {
            const auto* lhs = std::any_cast<T>(&a);
            const auto* rhs = std::any_cast<T>(&b);
            return lhs && rhs && internal_distinct::default_are_equivalent<T>(*lhs, *rhs);
        });
}

/**
 * Returns flow where all subsequent repetitions of the same value are filtered out, when
 * compared with each other via the provided [are_equivalent] function.
 *
 * Upstream:
 *   public fun <T> Flow<T>.distinctUntilChanged(areEquivalent: (old: T, new: T) -> Boolean): Flow<T> =
 *       distinctUntilChangedBy(defaultKeySelector, areEquivalent as (Any?, Any?) -> Boolean)
 */
template <typename T>
inline std::shared_ptr<Flow<T>> distinct_until_changed(
    std::shared_ptr<Flow<T>> flow,
    std::function<bool(const T&, const T&)> are_equivalent) {
    return internal_distinct::distinct_until_changed_by_impl<T>(
        std::move(flow),
        [](T value) { return internal_distinct::default_key_selector<T>(std::move(value)); },
        [are_equivalent = std::move(are_equivalent)](
            const std::any& a, const std::any& b) {
            const auto* lhs = std::any_cast<T>(&a);
            const auto* rhs = std::any_cast<T>(&b);
            return lhs && rhs && are_equivalent(*lhs, *rhs);
        });
}

/**
 * Returns flow where all subsequent repetitions of the same key are filtered out, where the
 * key is extracted with [key_selector] function.
 *
 * Upstream:
 *   public fun <T, K> Flow<T>.distinctUntilChangedBy(keySelector: (T) -> K): Flow<T> =
 *       distinctUntilChangedBy(keySelector, defaultAreEquivalent)
 */
template <typename T, typename K>
inline std::shared_ptr<Flow<T>> distinct_until_changed_by(
    std::shared_ptr<Flow<T>> flow,
    std::function<K(T)> key_selector) {
    return internal_distinct::distinct_until_changed_by_impl<T>(
        std::move(flow),
        [key_selector = std::move(key_selector)](T value) {
            return std::any(key_selector(std::move(value)));
        },
        [](const std::any& a, const std::any& b) {
            const auto* lhs = std::any_cast<K>(&a);
            const auto* rhs = std::any_cast<K>(&b);
            return lhs && rhs && *lhs == *rhs;
        });
}

} // namespace kotlinx::coroutines::flow
