/**
 * Transliterated from: integration/kotlinx-coroutines-play-services/src/Tasks.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.tasks
 *   imports: com.google.android.gms.tasks.*, kotlinx.coroutines.*,
 *            java.lang.Runnable, java.util.concurrent.Executor, kotlin.coroutines.*
 *
 * The Google Play Services `Task<T>` is an Android-only JVM-bound type; this C++ port
 * sketches the conversion API but cannot link against a real Play Services artifact. The
 * file exists so the Kotlin source-to-C++ inventory is complete; usable bindings would
 * require routing through the Android JNI layer.
 */

namespace kotlinx {
    namespace coroutines {
        namespace tasks {
            /**
 * Converts this deferred to the instance of [Task].
 * If deferred is cancelled then resulting task will be cancelled as well.
 */
            template<typename T>
            Task<T> as_task(Deferred<T> &deferred) {
                auto cancellation = CancellationTokenSource();
                auto source = TaskCompletionSource<T>(cancellation.token());

                deferred.invoke_on_completion([&](const Throwable *it) {
                    // callback@
                    if (auto *ce = dynamic_cast<const CancellationException *>(it)) {
                        cancellation.cancel();
                        return; // return@callback
                    }

                    auto *t = deferred.get_completion_exception_or_null();
                    if (t == nullptr) {
                        source.set_result(deferred.get_completed());
                    } else {
                        if (auto *exception = dynamic_cast<Exception *>(t)) {
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
            Deferred<T> as_deferred(Task<T> &task) {
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
            Deferred<T> as_deferred(Task<T> &task, CancellationTokenSource &cancellation_token_source) {
                return as_deferred_impl(task, &cancellation_token_source);
            }

            template<typename T>
            Deferred<T> as_deferred_impl(Task<T> &task, CancellationTokenSource *cancellation_token_source) {
                auto deferred = CompletableDeferred<T>();
                if (task.is_complete()) {
                    auto *e = task.exception();
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
                    task.add_on_complete_listener(DIRECT_EXECUTOR, [&deferred](Task<T> &it) {
                        auto *e = it.exception();
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
                // Upstream wraps the Deferred in a private read-only proxy to prevent
                // casts to CompletableDeferred and manual completion. The C++ port
                // currently returns the Deferred directly; an equivalent wrapping layer
                // would require porting the InternalForInheritanceCoroutinesApi shape.
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
            T await(Task<T> &task) {
                // Direct forward — await_impl owns the suspension drive against the
                // upstream Play Services Task completion listener.
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
            T await(Task<T> &task, CancellationTokenSource &cancellation_token_source) {
                return await_impl(task, &cancellation_token_source);
            }

            template<typename T>
            T await_impl(Task<T> &task, CancellationTokenSource *cancellation_token_source) {
                // Upstream uses suspendCancellableCoroutine to bridge the
                // OnCompleteListener callback into a continuation; the C++ port's fast
                // path returns synchronously when the Task is already complete, and the
                // slow path registers a listener that resumes the suspended continuation.
                // fast path
                if (task.is_complete()) {
                    auto *e = task.exception();
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

                return suspend_cancellable_coroutine([&](CancellableContinuation<T> &cont) {
                    // Run the callback directly to avoid unnecessarily scheduling on the main thread.
                    task.add_on_complete_listener(DIRECT_EXECUTOR, [&cont](Task<T> &it) {
                        auto *e = it.exception();
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
                static DirectExecutor &instance() {
                    static DirectExecutor inst;
                    return inst;
                }

                void execute(Runnable &r) {
                    r.run();
                }
            };

            const DirectExecutor &DIRECT_EXECUTOR = DirectExecutor::instance();
        } // namespace tasks
    } // namespace coroutines
} // namespace kotlinx

// Real integration with Google Play Services Tasks would require the JNI bridge plus
// real bindings for: Task / Deferred, CancellationTokenSource, TaskCompletionSource,
// CompletableDeferred, suspendCancellableCoroutine, invokeOnCompletion, the template
// surface, RuntimeExecutionException, the DirectExecutor singleton, and the upstream
// addOnCompleteListener callbacks. None of those are platform-available to a pure C++
// port — this file is the inventory placeholder.
