#include <string>
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/Await.kt
//
// TODO: suspend functions not directly supported - need custom coroutine implementation
// TODO: varargs needs C++ variadic templates or std::initializer_list
// TODO: atomicfu library needs C++ atomic equivalent (std::atomic)

#include "kotlinx/coroutines/core_fwd.hpp"
#include <vector>
#include <initializer_list>
#include <atomic>

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.atomicfu.* - use std::atomic
// TODO: import kotlin.coroutines.* - use custom coroutine types

/**
 * Awaits for completion of given deferred values without blocking a thread and resumes normally with the list of values
 * when all deferred computations are complete or resumes with the first thrown exception if any of computations
 * complete exceptionally including cancellation.
 *
 * This function is **not** equivalent to `deferreds.map { it.await() }` which fails only when it sequentially
 * gets to wait for the failing deferred, while this `awaitAll` fails immediately as soon as any of the deferreds fail.
 *
 * This suspending function is cancellable: if the [Job] of the current coroutine is cancelled while this
 * suspending function is waiting, this function immediately resumes with [CancellationException].
 * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
 * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
 */
// TODO: suspend fun - not directly supported, mark as requiring coroutine support
// TODO: vararg deferreds - use template variadic or std::initializer_list
template<typename T>
std::vector<T> await_all(std::initializer_list<Deferred<T>*> deferreds) {
    if (deferreds.size() == 0) {
        return std::vector<T>();
    } else {
        // TODO: AwaitAll(deferreds).await() - need to implement
        return AwaitAll<T>(deferreds).await();
    }
}

/**
 * Awaits for completion of given deferred values without blocking a thread and resumes normally with the list of values
 * when all deferred computations are complete or resumes with the first thrown exception if any of computations
 * complete exceptionally including cancellation.
 *
 * This function is **not** equivalent to `this.map { it.await() }` which fails only when it sequentially
 * gets to wait for the failing deferred, while this `awaitAll` fails immediately as soon as any of the deferreds fail.
 *
 * This suspending function is cancellable: if the [Job] of the current coroutine is cancelled while this
 * suspending function is waiting, this function immediately resumes with [CancellationException].
 * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
 * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
 */
// TODO: Extension function - free function or method
// TODO: Collection<Deferred<T>>.awaitAll() - extension on Collection
template<typename T>
std::vector<T> await_all(const std::vector<Deferred<T>*>& collection) {
    if (collection.empty()) {
        return std::vector<T>();
    } else {
        // TODO: toTypedArray() - already a vector
        return AwaitAll<T>(collection).await();
    }
}

/**
 * Suspends current coroutine until all given jobs are complete.
 * This method is semantically equivalent to joining all given jobs one by one with `jobs.forEach { it.join() }`.
 *
 * This suspending function is cancellable: if the [Job] of the current coroutine is cancelled while this
 * suspending function is waiting, this function immediately resumes with [CancellationException].
 * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
 * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
 */
// TODO: suspend fun - coroutine support needed
// TODO: vararg jobs - use variadic template
void join_all(std::initializer_list<Job*> jobs) {
    for (auto* job : jobs) {
        job->join();
    }
}

/**
 * Suspends current coroutine until all given jobs are complete.
 * This method is semantically equivalent to joining all given jobs one by one with `forEach { it.join() }`.
 *
 * This suspending function is cancellable: if the [Job] of the current coroutine is cancelled while this
 * suspending function is waiting, this function immediately resumes with [CancellationException].
 * There is a **prompt cancellation guarantee**: even if this function is ready to return the result, but was cancelled
 * while suspended, [CancellationException] will be thrown. See [suspendCancellableCoroutine] for low-level details.
 */
// TODO: Extension on Collection - free function
void join_all(const std::vector<Job*>& collection) {
    for (auto* job : collection) {
        job->join();
    }
}

// TODO: class - make nested class or anonymous namespace
template<typename T>
class AwaitAll {
private:
    std::vector<Deferred<T>*> deferreds;
    std::atomic<int> not_completed_count;

public:
    AwaitAll(const std::vector<Deferred<T>*>& deferreds_param)
        : deferreds(deferreds_param), not_completed_count(deferreds_param.size()) {}

    // TODO: suspend fun await - coroutine support needed
    std::vector<T> await() {
        // TODO: suspendCancellableCoroutine - implement continuation suspension
        // return suspendCancellableCoroutine<std::vector<T>>([&](CancellableContinuation<std::vector<T>>* cont) {
            // Intricate dance here
            // Step 1: Create nodes and install them as completion handlers, they may fire!
            std::vector<AwaitAllNode*> nodes(deferreds.size());
            for (size_t i = 0; i < deferreds.size(); ++i) {
                auto* deferred = deferreds[i];
                deferred->start(); // To properly await lazily started deferreds
                auto* node = new AwaitAllNode(cont);
                node->handle = deferred->invokeOnCompletion(node);
                nodes[i] = node;
            }
            auto* disposer = new DisposeHandlersOnCancel(nodes);
            // Step 2: Set disposer to each node
            for (auto* node : nodes) {
                node->disposer = disposer;
            }
            // Here we know that if any code the nodes complete, it will dispose the rest
            // Step 3: Now we can check if continuation is complete
            if (cont->isCompleted()) {
                // it is already complete while handlers were being installed -- dispose them all
                disposer->disposeAll();
            } else {
                cont->invokeOnCancellation(disposer);
            }
        // });
        // TODO: Placeholder return
        return std::vector<T>();
    }

private:
    // TODO: inner class - nested class with access to outer
    class DisposeHandlersOnCancel : CancelHandler {
    private:
        std::vector<AwaitAllNode*> nodes;

    public:
        DisposeHandlersOnCancel(const std::vector<AwaitAllNode*>& nodes_param) : nodes(nodes_param) {}

        void dispose_all() {
            for (auto* node : nodes) {
                node->handle->dispose();
            }
        }

        void invoke(Throwable* cause) override {
            dispose_all();
        }

        std::string toString() const override {
            return "DisposeHandlersOnCancel[" + /* nodes */ "]";
        }
    };

    // TODO: inner class - nested class
    class AwaitAllNode : JobNode {
    private:
        CancellableContinuation<std::vector<T>>* continuation;

    public:
        DisposableHandle* handle;

    private:
        std::atomic<DisposeHandlersOnCancel*> _disposer;

    public:
        AwaitAllNode(CancellableContinuation<std::vector<T>>* cont) : continuation(cont), _disposer(nullptr) {}

        DisposeHandlersOnCancel* get_disposer() const {
            return _disposer.load();
        }

        void set_disposer(DisposeHandlersOnCancel* value) {
            _disposer.store(value);
        }

        bool get_on_cancelling() const override {
            return false;
        }

        void invoke(Throwable* cause) override {
            if (cause != nullptr) {
                auto* token = continuation->tryResumeWithException(cause);
                if (token != nullptr) {
                    continuation->completeResume(token);
                    // volatile read of disposer AFTER continuation is complete
                    // and if disposer was already set (all handlers where already installed, then dispose them all)
                    auto* disposer_val = get_disposer();
                    if (disposer_val != nullptr) {
                        disposer_val->dispose_all();
                    }
                }
            } else if (not_completed_count.fetch_sub(1) - 1 == 0) {
                std::vector<T> results;
                for (auto* deferred : deferreds) {
                    results.push_back(deferred->getCompleted());
                }
                continuation->resume(results);
                // Note that all deferreds are complete here, so we don't need to dispose their nodes
            }
        }
    };
};

} // namespace coroutines
} // namespace kotlinx
