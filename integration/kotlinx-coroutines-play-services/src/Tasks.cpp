// Transliterated from: integration/kotlinx-coroutines-play-services/src/Tasks.kt

// @file:Suppress("RedundantVisibilityModifier")

// TODO: #include equivalent
// import com.google.android.gms.tasks.*
// import kotlinx.coroutines.*
// import java.lang.Runnable
// import java.util.concurrent.Executor
// import kotlin.coroutines.*

namespace kotlinx {
namespace coroutines {
namespace tasks {

/**
 * Converts this deferred to the instance of [Task].
 * If deferred is cancelled then resulting task will be cancelled as well.
 */
template<typename T>
Task<T> as_task(Deferred<T>& deferred) {
    auto cancellation = CancellationTokenSource();
    auto source = TaskCompletionSource<T>(cancellation.token());

    deferred.invoke_on_completion([&](const Throwable* it) {
        // callback@
        if (auto* ce = dynamic_cast<const CancellationException*>(it)) {
            cancellation.cancel();
            return; // return@callback
        }

        auto* t = deferred.get_completion_exception_or_null();
        if (t == nullptr) {
            source.set_result(deferred.get_completed());
        } else {
            if (auto* exception = dynamic_cast<Exception*>(t)) {
                source.set_exception(*exception);
            } else {
                source.set_exception(RuntimeExecutionException(*t));
            }
        }
    });

    return source.task();
}

/**
 * Converts this task to an instance of [Deferred].
 * If task is cancelled then resulting deferred will be cancelled as well.
 * However, the opposite is not true: if the deferred is cancelled, the [Task] will not be cancelled.
 * For bi-directional cancellation, an overload that accepts [CancellationTokenSource] can be used.
 */
template<typename T>
Deferred<T> as_deferred(Task<T>& task) {
    return as_deferred_impl(task, nullptr);
}

/**
 * Converts this task to an instance of [Deferred] with a [CancellationTokenSource] to control cancellation.
 * The cancellation of this function is bi-directional:
 * - If the given task is cancelled, the resulting deferred will be cancelled.
 * - If the resulting deferred is cancelled, the provided [cancellationTokenSource] will be cancelled.
 *
 * Providing a [CancellationTokenSource] that is unrelated to the receiving [Task] is not supported and
 * leads to an unspecified behaviour.
 */
// @ExperimentalCoroutinesApi // Since 1.5.1, tentatively until 1.6.0
template<typename T>
Deferred<T> as_deferred(Task<T>& task, CancellationTokenSource& cancellation_token_source) {
    return as_deferred_impl(task, &cancellation_token_source);
}

template<typename T>
Deferred<T> as_deferred_impl(Task<T>& task, CancellationTokenSource* cancellation_token_source) {
    auto deferred = CompletableDeferred<T>();
    if (task.is_complete()) {
        auto* e = task.exception();
        if (e == nullptr) {
            if (task.is_canceled()) {
                deferred.cancel();
            } else {
                // @Suppress("UNCHECKED_CAST")
                deferred.complete(task.result());
            }
        } else {
            deferred.complete_exceptionally(*e);
        }
    } else {
        // Run the callback directly to avoid unnecessarily scheduling on the main thread.
        task.add_on_complete_listener(kDirectExecutor, [&deferred](Task<T>& it) {
            auto* e = it.exception();
            if (e == nullptr) {
                // @Suppress("UNCHECKED_CAST")
                if (it.is_canceled()) {
                    deferred.cancel();
                } else {
                    deferred.complete(it.result());
                }
            } else {
                deferred.complete_exceptionally(*e);
            }
        });
    }

    if (cancellation_token_source != nullptr) {
        deferred.invoke_on_completion([cancellation_token_source]() {
            cancellation_token_source->cancel();
        });
    }
    // Prevent casting to CompletableDeferred and manual completion.
    // @OptIn(InternalForInheritanceCoroutinesApi::class)
    // TODO: return wrapped deferred
    return deferred;
}

/**
 * Awaits the completion of the task without blocking a thread.
 *
 * This suspending function is cancellable.
 * If the [Job] of the current coroutine is cancelled while this suspending function is waiting, this function
 * stops waiting for the completion stage and immediately resumes with [CancellationException].
 *
 * For bi-directional cancellation, an overload that accepts [CancellationTokenSource] can be used.
 */
template<typename T>
T await(Task<T>& task) {
    // TODO: implement coroutine suspension
    return await_impl(task, nullptr);
}

/**
 * Awaits the completion of the task that is linked to the given [CancellationTokenSource] to control cancellation.
 *
 * This suspending function is cancellable and cancellation is bi-directional:
 * - If the [Job] of the current coroutine is cancelled while this suspending function is waiting, this function
 * cancels the [cancellationTokenSource] and throws a [CancellationException].
 * - If the task is cancelled, then this function will throw a [CancellationException].
 *
 * Providing a [CancellationTokenSource] that is unrelated to the receiving [Task] is not supported and
 * leads to an unspecified behaviour.
 */
// @ExperimentalCoroutinesApi // Since 1.5.1, tentatively until 1.6.0
template<typename T>
T await(Task<T>& task, CancellationTokenSource& cancellation_token_source) {
    // TODO: implement coroutine suspension
    return await_impl(task, &cancellation_token_source);
}

template<typename T>
T await_impl(Task<T>& task, CancellationTokenSource* cancellation_token_source) {
    // TODO: implement coroutine suspension
    // fast path
    if (task.is_complete()) {
        auto* e = task.exception();
        if (e == nullptr) {
            if (task.is_canceled()) {
                throw CancellationException("Task " + task.to_string() + " was cancelled normally.");
            } else {
                // @Suppress("UNCHECKED_CAST")
                return task.result();
            }
        } else {
            throw *e;
        }
    }

    return suspend_cancellable_coroutine([&](CancellableContinuation<T>& cont) {
        // Run the callback directly to avoid unnecessarily scheduling on the main thread.
        task.add_on_complete_listener(kDirectExecutor, [&cont](Task<T>& it) {
            auto* e = it.exception();
            if (e == nullptr) {
                // @Suppress("UNCHECKED_CAST")
                if (it.is_canceled()) {
                    cont.cancel();
                } else {
                    cont.resume(it.result());
                }
            } else {
                cont.resume_with_exception(*e);
            }
        });

        if (cancellation_token_source != nullptr) {
            cont.invoke_on_cancellation([cancellation_token_source]() {
                cancellation_token_source->cancel();
            });
        }
    });
}

/**
 * An [Executor] that just directly executes the [Runnable].
 */
// object DirectExecutor : Executor
class DirectExecutor {
public:
    static DirectExecutor& instance() {
        static DirectExecutor inst;
        return inst;
    }

    void execute(Runnable& r) {
        r.run();
    }
};

const DirectExecutor& kDirectExecutor = DirectExecutor::instance();

} // namespace tasks
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement Task/Deferred integration
// 2. Implement CancellationTokenSource
// 3. Implement TaskCompletionSource
// 4. Implement CompletableDeferred
// 5. Implement suspendCancellableCoroutine
// 6. Implement invokeOnCompletion
// 7. Handle template type parameters
// 8. Implement RuntimeExecutionException
// 9. Handle object singleton pattern for DirectExecutor
// 10. Implement addOnCompleteListener callbacks
