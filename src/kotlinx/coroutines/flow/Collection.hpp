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
 *
 * Suspend ABI: returns `void*` per the project convention — the boxed destination pointer on
 * completion, or `COROUTINE_SUSPENDED` when the underlying `collect` suspends. Callers unbox
 * with `static_cast<Container*>(result)` once `is_coroutine_suspended(result)` is false.
 */
template <typename T, typename Container>
[[suspend]]
inline void* to_collection(
    Flow<T>* flow,
    Container* destination,
    Continuation<void*>* completion = nullptr) {
    detail::CollectingFlowCollector<T, Container> collector(destination);
    void* collect_result =
        dsl::suspend(flow->collect(&collector, completion));
    if (intrinsics::is_coroutine_suspended(collect_result)) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }
    return static_cast<void*>(destination);
}

template <typename T, typename Container>
[[suspend]]
inline void* to_collection(
    Flow<T>* flow,
    Container* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, Container>(flow, destination, completion.get());
}

template <typename T, typename Container>
[[suspend]]
inline void* to_collection(
    std::shared_ptr<Flow<T>> flow,
    Container* destination,
    Continuation<void*>* completion = nullptr) {
    return to_collection<T, Container>(flow.get(), destination, completion);
}

template <typename T, typename Container>
[[suspend]]
inline void* to_collection(
    std::shared_ptr<Flow<T>> flow,
    Container* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, Container>(flow.get(), destination, completion.get());
}

/**
 * Upstream:
 *   public suspend fun <T> Flow<T>.toList(destination: MutableList<T> = ArrayList()): List<T> =
 *       toCollection(destination)
 */
template <typename T>
[[suspend]]
inline void* to_list(
    Flow<T>* flow,
    std::vector<T>* destination,
    Continuation<void*>* completion = nullptr) {
    return to_collection<T, std::vector<T>>(flow, destination, completion);
}

template <typename T>
[[suspend]]
inline void* to_list(
    Flow<T>* flow,
    std::vector<T>* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, std::vector<T>>(flow, destination, completion.get());
}

template <typename T>
[[suspend]]
inline void* to_list(
    std::shared_ptr<Flow<T>> flow,
    std::vector<T>* destination,
    Continuation<void*>* completion = nullptr) {
    return to_collection<T, std::vector<T>>(flow.get(), destination, completion);
}

template <typename T>
[[suspend]]
inline void* to_list(
    std::shared_ptr<Flow<T>> flow,
    std::vector<T>* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, std::vector<T>>(flow.get(), destination, completion.get());
}

template <typename T>
[[suspend]]
inline void* to_list(
    Flow<T>* flow,
    Continuation<void*>* completion = nullptr) {
    auto* destination = new std::vector<T>();
    try {
        void* res = to_collection<T, std::vector<T>>(flow, destination, completion);
        if (intrinsics::is_coroutine_suspended(res)) {
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
        return res;
    } catch (...) {
        delete destination;
        throw;
    }
}

template <typename T>
[[suspend]]
inline void* to_list(
    Flow<T>* flow,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_list<T>(flow, completion.get());
}

template <typename T>
[[suspend]]
inline void* to_list(
    std::shared_ptr<Flow<T>> flow,
    Continuation<void*>* completion = nullptr) {
    return to_list<T>(flow.get(), completion);
}

template <typename T>
[[suspend]]
inline void* to_list(
    std::shared_ptr<Flow<T>> flow,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_list<T>(flow.get(), completion.get());
}

/**
 * Upstream:
 *   public suspend fun <T> Flow<T>.toSet(destination: MutableSet<T> = LinkedHashSet()): Set<T> =
 *       toCollection(destination)
 */
template <typename T>
[[suspend]]
inline void* to_set(
    Flow<T>* flow,
    std::set<T>* destination,
    Continuation<void*>* completion = nullptr) {
    return to_collection<T, std::set<T>>(flow, destination, completion);
}

template <typename T>
[[suspend]]
inline void* to_set(
    Flow<T>* flow,
    std::set<T>* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, std::set<T>>(flow, destination, completion.get());
}

template <typename T>
[[suspend]]
inline void* to_set(
    std::shared_ptr<Flow<T>> flow,
    std::set<T>* destination,
    Continuation<void*>* completion = nullptr) {
    return to_collection<T, std::set<T>>(flow.get(), destination, completion);
}

template <typename T>
[[suspend]]
inline void* to_set(
    std::shared_ptr<Flow<T>> flow,
    std::set<T>* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, std::set<T>>(flow.get(), destination, completion.get());
}

template <typename T>
[[suspend]]
inline void* to_set(
    Flow<T>* flow,
    Continuation<void*>* completion = nullptr) {
    auto* destination = new std::set<T>();
    try {
        void* res = to_collection<T, std::set<T>>(flow, destination, completion);
        if (intrinsics::is_coroutine_suspended(res)) {
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
        return res;
    } catch (...) {
        delete destination;
        throw;
    }
}

template <typename T>
[[suspend]]
inline void* to_set(
    Flow<T>* flow,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_set<T>(flow, completion.get());
}

template <typename T>
[[suspend]]
inline void* to_set(
    std::shared_ptr<Flow<T>> flow,
    Continuation<void*>* completion = nullptr) {
    return to_set<T>(flow.get(), completion);
}

template <typename T>
[[suspend]]
inline void* to_set(
    std::shared_ptr<Flow<T>> flow,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_set<T>(flow.get(), completion.get());
}

} // namespace kotlinx::coroutines::flow
