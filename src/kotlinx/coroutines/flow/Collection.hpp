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
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/LinkedHashSet.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <atomic>
#include <exception>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::flow {

namespace detail {

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collection.kt:21-26
 *
 * Heap-allocated state machine frame for `to_collection`, `to_list`, and `to_set`.
 * Implements both FlowCollector<T> (sink collector) and Continuation<void*> (completion interception),
 * anchoring the destination container and flow instance across suspension points to eliminate stack-UAF.
 */
template <typename T, typename Container>
struct ToCollectionFrame : public FlowCollector<T>,
                           public Continuation<void*>,
                           public std::enable_shared_from_this<ToCollectionFrame<T, Container>> {
    Container* destination = nullptr;
    Continuation<void*>* completion = nullptr;
    std::unique_ptr<Container> owned_container;
    std::shared_ptr<Flow<T>> flow_holder;

    std::recursive_mutex mutex;
    std::exception_ptr failure = nullptr;
    std::atomic<bool> completed{false};
    std::shared_ptr<ToCollectionFrame<T, Container>> self_ref;

    ToCollectionFrame(
        Container* dest,
        Continuation<void*>* comp,
        std::unique_ptr<Container> owned = nullptr,
        std::shared_ptr<Flow<T>> flow = nullptr)
        : destination(dest),
          completion(comp),
          owned_container(std::move(owned)),
          flow_holder(std::move(flow)) {}

    ~ToCollectionFrame() override = default;

    void retain_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!completed.load()) {
            self_ref = this->shared_from_this();
        }
    }

    void release_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        self_ref = nullptr;
        flow_holder = nullptr;
    }

    bool is_completed() const {
        return completed.load();
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return completion ? completion->get_context() : nullptr;
    }

    void resume_with(Result<void*> collect_res) override {
        std::shared_ptr<ToCollectionFrame<T, Container>> guard;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (completed.exchange(true)) return;
            guard = std::move(self_ref);
            self_ref = nullptr;
            flow_holder = nullptr;
        }

        std::exception_ptr fail = nullptr;
        if (collect_res.is_success()) {
            {
                std::lock_guard<std::recursive_mutex> lock(mutex);
                fail = failure;
            }
            if (fail) {
                if (completion) {
                    completion->resume_with(Result<void*>::failure(fail));
                }
            } else {
                if (completion) {
                    void* res = owned_container ? static_cast<void*>(owned_container.release())
                                                : static_cast<void*>(destination);
                    completion->resume_with(Result<void*>::success(res));
                }
            }
        } else {
            if (completion) {
                completion->resume_with(Result<void*>::failure(collect_res.exception_or_null()));
            }
        }
    }

    void* emit(T value, Continuation<void*>* /*continuation*/) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (failure) {
            std::rethrow_exception(failure);
        }
        try {
            destination->insert(destination->end(), std::move(value));
        } catch (...) {
            failure = std::current_exception();
            throw;
        }
        return nullptr;
    }
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
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collection.kt:21-26
 */
template <typename T, typename Container>
[[suspend]]
inline void* to_collection(
    Flow<T>* flow,
    Container* destination,
    Continuation<void*>* completion = nullptr,
    std::unique_ptr<Container> owned_container = nullptr,
    std::shared_ptr<Flow<T>> flow_holder = nullptr) {
    if (!flow) {
        throw std::invalid_argument("flow cannot be null");
    }
    if (!destination) {
        throw std::invalid_argument("destination cannot be null");
    }

    auto frame = std::make_shared<detail::ToCollectionFrame<T, Container>>(
        destination, completion, std::move(owned_container), std::move(flow_holder));
    frame->retain_self();

    void* r = nullptr;
    try {
        r = flow->collect(frame.get(), frame.get());
    } catch (...) {
        frame->release_self();
        throw;
    }

    if (intrinsics::is_coroutine_suspended(r) || frame->is_completed()) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }

    frame->completed.store(true);
    frame->release_self();

    if (frame->failure) {
        std::rethrow_exception(frame->failure);
    }

    if (frame->owned_container) {
        return static_cast<void*>(frame->owned_container.release());
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
    if (!flow) {
        throw std::invalid_argument("flow cannot be null");
    }
    auto* flow_ptr = flow.get();
    return to_collection<T, Container>(flow_ptr, destination, completion, nullptr, std::move(flow));
}

template <typename T, typename Container>
[[suspend]]
inline void* to_collection(
    std::shared_ptr<Flow<T>> flow,
    Container* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, Container>(std::move(flow), destination, completion.get());
}

/**
 * Upstream:
 *   public suspend fun <T> Flow<T>.toList(destination: MutableList<T> = ArrayList()): List<T> =
 *       toCollection(destination)
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collection.kt:11-12
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
    return to_collection<T, std::vector<T>>(std::move(flow), destination, completion);
}

template <typename T>
[[suspend]]
inline void* to_list(
    std::shared_ptr<Flow<T>> flow,
    std::vector<T>* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, std::vector<T>>(std::move(flow), destination, completion.get());
}

template <typename T>
[[suspend]]
inline void* to_list(
    Flow<T>* flow,
    Continuation<void*>* completion = nullptr) {
    auto owned = std::make_unique<std::vector<T>>();
    auto* dest = owned.get();
    return to_collection<T, std::vector<T>>(flow, dest, completion, std::move(owned));
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
    if (!flow) {
        throw std::invalid_argument("flow cannot be null");
    }
    auto owned = std::make_unique<std::vector<T>>();
    auto* dest = owned.get();
    auto* flow_ptr = flow.get();
    return to_collection<T, std::vector<T>>(flow_ptr, dest, completion, std::move(owned), std::move(flow));
}

template <typename T>
[[suspend]]
inline void* to_list(
    std::shared_ptr<Flow<T>> flow,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_list<T>(std::move(flow), completion.get());
}

/**
 * Upstream:
 *   public suspend fun <T> Flow<T>.toSet(destination: MutableSet<T> = LinkedHashSet()): Set<T> =
 *       toCollection(destination)
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collection.kt:16-17
 */
// Explicit LinkedHashSet destination overloads
template <typename T>
[[suspend]]
inline void* to_set(
    Flow<T>* flow,
    LinkedHashSet<T>* destination,
    Continuation<void*>* completion = nullptr) {
    return to_collection<T, LinkedHashSet<T>>(flow, destination, completion);
}

template <typename T>
[[suspend]]
inline void* to_set(
    Flow<T>* flow,
    LinkedHashSet<T>* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, LinkedHashSet<T>>(flow, destination, completion.get());
}

template <typename T>
[[suspend]]
inline void* to_set(
    std::shared_ptr<Flow<T>> flow,
    LinkedHashSet<T>* destination,
    Continuation<void*>* completion = nullptr) {
    return to_collection<T, LinkedHashSet<T>>(std::move(flow), destination, completion);
}

template <typename T>
[[suspend]]
inline void* to_set(
    std::shared_ptr<Flow<T>> flow,
    LinkedHashSet<T>* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, LinkedHashSet<T>>(std::move(flow), destination, completion.get());
}

// Backward compatibility: explicit std::set destination overloads
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
    return to_collection<T, std::set<T>>(std::move(flow), destination, completion);
}

template <typename T>
[[suspend]]
inline void* to_set(
    std::shared_ptr<Flow<T>> flow,
    std::set<T>* destination,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_collection<T, std::set<T>>(std::move(flow), destination, completion.get());
}

// Default destination overloads (creates and returns LinkedHashSet<T> matching Kotlin upstream)
template <typename T>
[[suspend]]
inline void* to_set(
    Flow<T>* flow,
    Continuation<void*>* completion = nullptr) {
    auto owned = std::make_unique<LinkedHashSet<T>>();
    auto* dest = owned.get();
    return to_collection<T, LinkedHashSet<T>>(flow, dest, completion, std::move(owned));
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
    if (!flow) {
        throw std::invalid_argument("flow cannot be null");
    }
    auto owned = std::make_unique<LinkedHashSet<T>>();
    auto* dest = owned.get();
    auto* flow_ptr = flow.get();
    return to_collection<T, LinkedHashSet<T>>(flow_ptr, dest, completion, std::move(owned), std::move(flow));
}

template <typename T>
[[suspend]]
inline void* to_set(
    std::shared_ptr<Flow<T>> flow,
    std::shared_ptr<Continuation<void*>> completion) {
    return to_set<T>(std::move(flow), completion.get());
}

} // namespace kotlinx::coroutines::flow
