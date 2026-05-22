#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collection.kt
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
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::flow {

namespace detail {

/**
 * Internal sink collector used by `to_collection` to mirror upstream's
 * `collect { value -> destination.add(value) }` lambda.
 */
template <typename T, typename Container>
class CollectingFlowCollector : public FlowCollector<T> {
public:
    explicit CollectingFlowCollector(Container* destination) : destination_(destination) {}

    void* emit(T value, Continuation<void*>* /*continuation*/) override {
        destination_->insert(destination_->end(), std::move(value));
        return nullptr;
    }

private:
    Container* destination_;
};

} // namespace detail

/**
 * Collects given flow into a [destination].
 *
 * Upstream:
 *   public suspend fun <T, C : MutableCollection<in T>> Flow<T>.toCollection(destination: C): C {
 *       collect { value -> destination.add(value) }
 *       return destination
 *   }
 */
template <typename T, typename Container>
[[suspend]]
inline Container* to_collection(
    Flow<T>* flow,
    Container* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    detail::CollectingFlowCollector<T, Container> collector(destination);
    void* collect_result =
        dsl::suspend(flow->collect(&collector, completion.get()));
    if (intrinsics::is_coroutine_suspended(collect_result)) {
        return reinterpret_cast<Container*>(intrinsics::get_COROUTINE_SUSPENDED());
    }
    return destination;
}

/**
 * Upstream:
 *   public suspend fun <T> Flow<T>.toList(destination: MutableList<T> = ArrayList()): List<T> =
 *       toCollection(destination)
 */
template <typename T>
[[suspend]]
inline std::vector<T>* to_list(
    Flow<T>* flow,
    std::vector<T>* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, std::vector<T>>(flow, destination, std::move(completion));
}

/**
 * Default-destination overload, matching the Kotlin
 * `destination: MutableList<T> = ArrayList()` default parameter value.
 */
template <typename T>
[[suspend]]
inline std::shared_ptr<std::vector<T>> to_list(
    Flow<T>* flow,
    std::shared_ptr<Continuation<void*>> completion) {
    auto destination = std::make_shared<std::vector<T>>();
    to_collection<T, std::vector<T>>(flow, destination.get(), std::move(completion));
    return destination;
}

/**
 * Upstream:
 *   public suspend fun <T> Flow<T>.toSet(destination: MutableSet<T> = LinkedHashSet()): Set<T> =
 *       toCollection(destination)
 */
template <typename T>
[[suspend]]
inline std::set<T>* to_set(
    Flow<T>* flow,
    std::set<T>* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, std::set<T>>(flow, destination, std::move(completion));
}

/**
 * Default-destination overload, matching the Kotlin
 * `destination: MutableSet<T> = LinkedHashSet()` default parameter value.
 */
template <typename T>
[[suspend]]
inline std::shared_ptr<std::set<T>> to_set(
    Flow<T>* flow,
    std::shared_ptr<Continuation<void*>> completion) {
    auto destination = std::make_shared<std::set<T>>();
    to_collection<T, std::set<T>>(flow, destination.get(), std::move(completion));
    return destination;
}

} // namespace kotlinx::coroutines::flow
