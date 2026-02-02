#pragma once
// port-lint: source flow/terminal/Collection.kt
/**
 * @file Collection.hpp
 * @brief Terminal flow operators for collecting to containers: toList, toSet, toCollection
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collection.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include <vector>
#include <set>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

// ============================================================================
// Line 21-26: toCollection operator
// ============================================================================

/**
 * Collects given flow into a destination collection.
 *
 * Transliterated from:
 * public suspend fun <T, C : MutableCollection<in T>> Flow<T>.toCollection(destination: C): C
 *
 * @param flow The flow to collect
 * @param destination The collection to add elements to
 * @return The destination collection with all elements
 */
template <typename T, typename Container>
Container& to_collection(const std::shared_ptr<Flow<T>>& flow, Container& destination) {
    class ToCollectionCollector : public FlowCollector<T> {
    public:
        explicit ToCollectionCollector(Container& dest) : destination_(dest) {}

        void* emit(T value, Continuation<void*>* /*cont*/) override {
            destination_.insert(destination_.end(), std::move(value));
            return nullptr;
        }

    private:
        Container& destination_;
    };

    ToCollectionCollector collector(destination);
    flow->collect(&collector, nullptr);

    return destination;
}

// ============================================================================
// Line 11: toList operator
// ============================================================================

/**
 * Collects given flow into a vector (list).
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.toList(destination: MutableList<T> = ArrayList()): List<T>
 *
 * @param flow The flow to collect
 * @return A vector containing all elements from the flow
 */
template <typename T>
std::vector<T> to_list(const std::shared_ptr<Flow<T>>& flow) {
    std::vector<T> result;
    to_collection(flow, result);
    return result;
}

/**
 * Collects given flow into an existing vector (list).
 *
 * @param flow The flow to collect
 * @param destination The vector to add elements to
 * @return The destination vector with all elements
 */
template <typename T>
std::vector<T>& to_list(const std::shared_ptr<Flow<T>>& flow, std::vector<T>& destination) {
    return to_collection(flow, destination);
}

// ============================================================================
// Line 16: toSet operator
// ============================================================================

/**
 * Collects given flow into a set.
 *
 * Transliterated from:
 * public suspend fun <T> Flow<T>.toSet(destination: MutableSet<T> = LinkedHashSet()): Set<T>
 *
 * @param flow The flow to collect
 * @return A set containing all unique elements from the flow
 */
template <typename T>
std::set<T> to_set(const std::shared_ptr<Flow<T>>& flow) {
    std::set<T> result;

    class ToSetCollector : public FlowCollector<T> {
    public:
        explicit ToSetCollector(std::set<T>& dest) : destination_(dest) {}

        void* emit(T value, Continuation<void*>* /*cont*/) override {
            destination_.insert(std::move(value));
            return nullptr;
        }

    private:
        std::set<T>& destination_;
    };

    ToSetCollector collector(result);
    flow->collect(&collector, nullptr);

    return result;
}

/**
 * Collects given flow into an existing set.
 *
 * @param flow The flow to collect
 * @param destination The set to add elements to
 * @return The destination set with all elements
 */
template <typename T>
std::set<T>& to_set(const std::shared_ptr<Flow<T>>& flow, std::set<T>& destination) {
    class ToSetCollector : public FlowCollector<T> {
    public:
        explicit ToSetCollector(std::set<T>& dest) : destination_(dest) {}

        void* emit(T value, Continuation<void*>* /*cont*/) override {
            destination_.insert(std::move(value));
            return nullptr;
        }

    private:
        std::set<T>& destination_;
    };

    ToSetCollector collector(destination);
    flow->collect(&collector, nullptr);

    return destination;
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
