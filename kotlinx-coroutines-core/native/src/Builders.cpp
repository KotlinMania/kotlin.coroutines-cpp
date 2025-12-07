#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/Builders.kt
//
// TODO: File-level annotations not translated: @file:OptIn, @file:Suppress
// TODO: Contracts API not available in C++
// TODO: Coroutine suspend/resume semantics not implemented
// TODO: Worker API from Kotlin/Native needs equivalent implementation
// TODO: ThreadLocal annotation needs equivalent mechanism
// TODO: ObsoleteWorkersApi handling

namespace kotlinx {
namespace coroutines {

// TODO: Remove imports, fully qualify or add includes:
// import kotlinx.cinterop.*
// import kotlin.contracts.*
// import kotlin.coroutines.*
// import kotlin.native.concurrent.*

/**
 * Runs a new coroutine and **blocks** the current thread _interruptibly_ until its completion.
 *
 * It is designed to bridge regular blocking code to libraries that are written in suspending style, to be used in
 * `main` functions and in tests.
 *
 * Calling [runBlocking] from a suspend function is redundant.
 * For example, the following code is incorrect:
 * ```
 * auto load_configuration() {
 *     // DO NOT DO THIS:
 *     auto data = runBlocking { // <- redundant and blocks the thread, do not do that
 *         fetchConfigurationData() // suspending function
 *     }
 * ```
 *
 * Here, instead of releasing the thread on which `loadConfiguration` runs if `fetchConfigurationData` suspends, it will
 * block, potentially leading to thread starvation issues.
 *
 * The default [CoroutineDispatcher] for this builder is an implementation of event loop that processes continuations
 * in this blocked thread until the completion of this coroutine.
 * See [CoroutineDispatcher] for the other implementations that are provided by `kotlinx.coroutines`.
 *
 * When [CoroutineDispatcher] is explicitly specified in the [context], then the new coroutine runs in the context of
 * the specified dispatcher while the current thread is blocked. If the specified dispatcher is an event loop of another `runBlocking`,
 * then this invocation uses the outer event loop.
 *
 * If this blocked thread is interrupted (see [Thread.interrupt]), then the coroutine job is cancelled and
 * this `runBlocking` invocation throws [InterruptedException].
 *
 * See [newCoroutineContext][CoroutineScope.newCoroutineContext] for a description of debugging facilities that are available
 * for a newly created coroutine.
 *
 * @param context the context of the coroutine. The default value is an event loop on the current thread.
 * @param block the coroutine code.
 */
template<typename T>
T run_blocking(CoroutineContext context, /* TODO: suspend function type */ auto block) {
    // TODO: contract { callsInPlace(block, InvocationKind.EXACTLY_ONCE) }

    auto context_interceptor = context[ContinuationInterceptor];
    EventLoop* event_loop;
    CoroutineContext new_context;

    if (context_interceptor == nullptr) {
        // create or use event loop if no dispatcher is specified
        event_loop = ThreadLocalEventLoop::event_loop;
        new_context = GlobalScope::new_coroutine_context(context + event_loop);
    } else {
        // See if context's interceptor is an event loop that we shall use (to support TestContext)
        // or take an existing thread-local event loop if present to avoid blocking it (but don't create one)
        event_loop = dynamic_cast<EventLoop*>(context_interceptor);
        if (event_loop != nullptr && event_loop->should_be_processed_from_context()) {
            // use it
        } else {
            event_loop = ThreadLocalEventLoop::current_or_null();
        }
        new_context = GlobalScope::new_coroutine_context(context);
    }

    auto coroutine = BlockingCoroutine<T>(new_context, event_loop);
    bool completed = false;
    ThreadLocalKeepAlive::add_check([&completed]() { return !completed; });

    try {
        coroutine.start(CoroutineStart::kDefault, coroutine, block);
        return coroutine.join_blocking();
    } catch (...) {
        completed = true;
        throw;
    }
}

// TODO: @ThreadLocal annotation equivalent needed
namespace /* ThreadLocal */ {
    class ThreadLocalKeepAlive {
    private:
        /** If any of these checks passes, this means this [Worker] is still used. */
        static std::vector<std::function<bool()>> checks;

        /** Whether the worker currently tries to keep itself alive. */
        static bool keep_alive_loop_active;

        /** Adds another stopgap that must be passed before the [Worker] can be terminated. */
        static void add_check(std::function<bool()> termination_forbidden) {
            checks.push_back(termination_forbidden);
            if (!keep_alive_loop_active) keep_alive();
        }

        /**
         * Send a ping to the worker to prevent it from terminating while this coroutine is running,
         * ensuring that continuations don't get dropped and forgotten.
         */
        static void keep_alive() {
            // only keep the checks that still forbid the termination
            std::vector<std::function<bool()>> filtered_checks;
            for (auto& check : checks) {
                if (check()) {
                    filtered_checks.push_back(check);
                }
            }
            checks = std::move(filtered_checks);

            // if there are no checks left, we no longer keep the worker alive, it can be terminated
            keep_alive_loop_active = !checks.empty();

            if (keep_alive_loop_active) {
                // TODO: Worker.current.executeAfter equivalent
                // Worker.current.executeAfter(afterMicroseconds = 100_000) {
                //     keepAlive()
                // }
            }
        }

        friend T run_blocking<T>(CoroutineContext, auto);
    };
}

template<typename T>
class BlockingCoroutine : AbstractCoroutine<T> {
private:
    EventLoop* event_loop;
    // TODO: Worker equivalent
    void* join_worker; // = Worker.current

public:
    BlockingCoroutine(CoroutineContext parent_context, EventLoop* event_loop)
        : AbstractCoroutine<T>(parent_context, true, true)
        , event_loop(event_loop)
        , join_worker(nullptr) // TODO: Worker.current
    {
    }

    bool is_scoped_coroutine() const override { return true; }

    void after_completion(void* state) override {
        // wake up blocked thread
        // TODO: Worker comparison
        // if (joinWorker != Worker.current) {
        //     // Unpark waiting worker
        //     joinWorker.executeAfter(0L, {}) // send an empty task to unpark the waiting event loop
        // }
    }

    T join_blocking() {
        try {
            if (event_loop != nullptr) {
                event_loop->increment_use_count();
            }

            while (true) {
                long park_nanos;
                // Workaround for bug in BE optimizer that cannot eliminate boxing here
                if (event_loop != nullptr) {
                    park_nanos = event_loop->process_next_event();
                } else {
                    park_nanos = LONG_MAX;
                }
                // note: processNextEvent may lose unpark flag, so check if completed before parking
                if (this->is_completed()) break;
                // TODO: joinWorker.park equivalent
                // joinWorker.park(parkNanos / 1000L, true)
            }
        } catch (...) {
            if (event_loop != nullptr) {
                event_loop->decrement_use_count();
            }
            throw;
        }

        // now return result
        auto state = this->state.unbox_state();
        auto* completed_exceptionally = dynamic_cast<CompletedExceptionally*>(state);
        if (completed_exceptionally != nullptr) {
            throw completed_exceptionally->cause;
        }
        return static_cast<T>(state);
    }
};

} // namespace coroutines
} // namespace kotlinx
