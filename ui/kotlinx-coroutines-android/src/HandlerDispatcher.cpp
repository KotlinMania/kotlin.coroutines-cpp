#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin: kotlinx.coroutines.android.HandlerDispatcher
// Original package: kotlinx.coroutines.android
//
// TODO: This is a mechanical C++ transliteration. The following constructs need proper implementation:
// - Android OS API calls (Handler, Looper, Build, Choreographer)
// - AndroidX annotations (@VisibleForTesting, @JvmName, @JvmOverloads, @JvmField)
// - Kotlin sealed classes -> abstract base class with constructor
// - Kotlin class declarations -> singletons (namespace with static members or Meyer's singleton)
// - Kotlin extension functions (Handler.asCoroutineDispatcher, Looper.asHandler, Dispatchers.JavaFx extension)
// - Kotlin suspend functions (awaitFrame, awaitFrameSlowPath) -> TODO: coroutine semantics not implemented
// - MainCoroutineDispatcher, Delay, CoroutineDispatcher interfaces
// - Reflection for Handler.createAsync and constructor lookup
// - Kotlin data types: Runnable, CancellableContinuation, DisposableHandle, CoroutineContext
// - Kotlin nullable types (T*) -> pointers or std::optional
// - Kotlin companion object and top-level functions
// - Kotlin @Deprecated with DeprecationLevel.HIDDEN
// - Kotlin lambda with receiver syntax
// - Volatile annotation -> std::atomic

#include <atomic>
#include <optional>
#include <string>
#include <memory>
#include <functional>
#include <stdexcept>
#include <limits>

// TODO: Remove placeholder imports; map to actual Android/coroutine headers
// #include <android/os/Handler.h>
// #include <android/os/Looper.h>
// #include <android/os/Build.h>
// #include <android/view/Choreographer.h>
// #include <kotlinx/coroutines/MainCoroutineDispatcher.h>
// #include <kotlinx/coroutines/Delay.h>
// #include <kotlinx/coroutines/Dispatchers.h>
// #include <kotlinx/coroutines/CancellableContinuation.h>
// #include <kotlin/coroutines/CoroutineContext.h>

namespace kotlinx {
namespace coroutines {
namespace android {

// TODO: Forward declarations for unmapped types
class MainCoroutineDispatcher;
class MainDispatcherFactory;
class Delay;
class CoroutineContext;
class Handler;
class Looper;
class Choreographer;
class DisposableHandle;
class NonDisposableHandle;
class CancellableContinuation;

// TODO: Placeholder for Runnable
using Runnable = std::function<void()>;

/**
 * Dispatches execution onto Android [Handler].
 *
 * This class provides type-safety and a point for future extensions.
 */
// sealed class HandlerDispatcher
class HandlerDispatcher : MainCoroutineDispatcher, Delay {
public:
    /**
     * Returns dispatcher that executes coroutines immediately when it is already in the right context
     * (current looper is the same as this handler's looper) without an additional [re-dispatch][CoroutineDispatcher.dispatch].
     * This dispatcher does not use [Handler.post] when current looper is the same as looper of the handler.
     *
     * Immediate dispatcher is safe from stack overflows and in case of nested invocations forms event-loop similar to [Dispatchers.Unconfined].
     * The event loop is an advanced topic and its implications can be found in [Dispatchers.Unconfined] documentation.
     *
     * Example of usage:
     * ```
     * auto  update_ui_element(std::string text) {
     *     /*
     *      * If it is known that updateUiElement can be invoked both from the Main thread and from other threads,
     *      * `immediate` dispatcher is used as a performance optimization to avoid unnecessary dispatch.
     *      *
     *      * In that case, when `updateUiElement` is invoked from the Main thread, `uiElement.text` will be
     *      * invoked immediately without any dispatching, otherwise, the `Dispatchers.Main` dispatch cycle via
     *      * `Handler.post` will be triggered.
     *      */
     *     withContext(Dispatchers.Main.immediate) {
     *         uiElement.text = text
     *     }
     *     // Do context-independent logic such as logging
     * }
     * ```
     */
    // abstract override HandlerDispatcher immediate
    virtual HandlerDispatcher& get_immediate() = 0;

protected:
    // Sealed class -> protected constructor
    HandlerDispatcher() = default;
};

// class AndroidDispatcherFactory
class AndroidDispatcherFactory : MainDispatcherFactory {
public:
    // virtual auto create_dispatcher(allFactories: List<MainDispatcherFactory>)
    MainCoroutineDispatcher* create_dispatcher(const std::vector<MainDispatcherFactory*>& all_factories) override {
        // auto mainLooper = Looper.getMainLooper() ?: throw IllegalStateException("The main looper is not available")
        Looper* main_looper = nullptr; // TODO: Looper::get_main_looper()
        if (main_looper == nullptr) {
            throw std::runtime_error("The main looper is not available");
        }
        // return HandlerContext(mainLooper.asHandler(async = true))
        Handler* handler = as_handler(*main_looper, true);
        return new HandlerContext(handler);
    }

    // virtual auto hint_on_error(): std::string
    std::string hint_on_error() override {
        return "For tests Dispatchers.setMain from kotlinx-coroutines-test module can be used";
    }

    // override Int loadPriority
    int get_load_priority() override {
        return std::numeric_limits<int>::max() / 2;
    }

private:
    // TODO: Declare as_handler here or as free function
    Handler* as_handler(Looper& looper, bool async);
};

/**
 * Represents an arbitrary [Handler] as an implementation of [CoroutineDispatcher]
 * with an optional [name] for nicer debugging
 *
 * ## Rejected execution
 *
 * If the underlying handler is closed and its message-scheduling methods start to return `false` on
 * an attempt to submit a continuation task to the resulting dispatcher,
 * then the [Job] of the affected task is [cancelled][Job.cancel] and the task is submitted to the
 * [Dispatchers.IO], so that the affected coroutine can cleanup its resources and promptly complete.
 */
// @JvmName("from") // this is for a nice Java API, see issue #255
// @JvmOverloads
// auto Handler__dot__asCoroutineDispatcher(std::string* name = nullptr): HandlerDispatcher
// TODO: Extension function -> free function taking Handler as first parameter
HandlerDispatcher* as_coroutine_dispatcher(Handler& handler, const std::optional<std::string>& name = std::nullopt);

// const auto MAX_DELAY = Long.MAX_VALUE / 2
constexpr int64_t kMaxDelay = std::numeric_limits<int64_t>::max() / 2; // cannot delay for too long on Android

// @VisibleForTesting
// auto Looper__dot__asHandler(async: Boolean): Handler
// TODO: Extension function -> free function
Handler* as_handler(Looper& looper, bool async) {
    // Async support was added in API 16.
    // if (!async || Build.VERSION.SDK_INT < 16)
    int sdk_version = 0; // TODO: Build::VERSION::SDK_INT
    if (!async || sdk_version < 16) {
        // return Handler(this)
        return nullptr; // TODO: new Handler(looper)
    }

    if (sdk_version >= 28) {
        // TODO compile against API 28 so this can be invoked without reflection.
        // auto factoryMethod = Handler::class.java.getDeclaredMethod("createAsync", Looper::class.java)
        // return factoryMethod.invoke(nullptr, this) as Handler
        // TODO: Reflection - Handler::createAsync(Looper)
        return nullptr;
    }

    // Constructor<Handler> constructor
    // TODO: Reflection - Handler constructor lookup
    try {
        // constructor = Handler::class.java.getDeclaredConstructor(Looper::class.java,
        //     Handler.Callback::class.java, Boolean::class.javaPrimitiveType)
        // TODO: Find constructor with signature (Looper, Handler.Callback, boolean)
    } catch (...) { // (ignored: NoSuchMethodException)
        // Hidden constructor absent. Fall back to non-async constructor.
        // return Handler(this)
        return nullptr; // TODO: new Handler(looper)
    }
    // return constructor.newInstance(this, nullptr, true)
    return nullptr; // TODO: Invoke constructor with (looper, nullptr, true)
}

// @JvmField
// @Deprecated("Use Dispatchers.Main instead", level = DeprecationLevel.HIDDEN)
// HandlerDispatcher* Main
// TODO: This is a top-level property; consider namespace-level static or singleton
// runCatching { HandlerContext(Looper.getMainLooper().asHandler(async = true)) }.getOrNull()

/**
 * Implements [CoroutineDispatcher] on top of an arbitrary Android [Handler].
 */
// class HandlerContext
class HandlerContext : HandlerDispatcher, Delay {
private:
    Handler* handler;
    std::optional<std::string> name;
    bool invoke_immediately;

    // constructor
    HandlerContext(Handler* handler_, const std::optional<std::string>& name_, bool invoke_immediately_)
        : handler(handler_), name(name_), invoke_immediately(invoke_immediately_)
    {
    }

public:
    /**
     * Creates [CoroutineDispatcher] for the given Android [handler].
     *
     * @param handler a handler.
     * @param name an optional name for debugging.
     */
    // constructor
    HandlerContext(Handler* handler_, const std::optional<std::string>& name_ = std::nullopt)
        : HandlerContext(handler_, name_, false)
    {
    }

    // override HandlerContext immediate
    HandlerContext& get_immediate() override {
        if (invoke_immediately) {
            return *this;
        } else {
            static HandlerContext immediate_ctx(handler, name, true);
            return immediate_ctx;
        }
    }

    // virtual auto is_dispatch_needed(context: CoroutineContext): Boolean
    bool is_dispatch_needed(CoroutineContext& context) override {
        // return !invokeImmediately || Looper.myLooper() != handler.looper
        // TODO: Looper::my_looper() != handler->get_looper()
        return !invoke_immediately; // TODO: Add looper check
    }

    // virtual auto dispatch(CoroutineContext context, block: Runnable)
    void dispatch(CoroutineContext& context, Runnable block) override {
        // if (!handler.post(block))
        bool posted = false; // TODO: handler->post(block)
        if (!posted) {
            cancel_on_rejection(context, block);
        }
    }

    // virtual auto schedule_resume_after_delay(Long timeMillis, continuation: CancellableContinuation<Unit>)
    void schedule_resume_after_delay(int64_t time_millis, CancellableContinuation<void>& continuation) override {
        // auto block = Runnable {
        //     with(continuation) { resumeUndispatched(Unit) }
        // }
        Runnable block = [&continuation]() {
            // TODO: with(continuation) { resumeUndispatched(Unit) }
        };
        int64_t coerced_delay = std::min(time_millis, kMaxDelay);
        // if (handler.postDelayed(block, timeMillis.coerceAtMost(MAX_DELAY)))
        bool posted = false; // TODO: handler->post_delayed(block, coerced_delay)
        if (posted) {
            // continuation.invokeOnCancellation { handler.removeCallbacks(block) }
            // TODO: continuation.invoke_on_cancellation([this, block]() { handler->remove_callbacks(block); })
        } else {
            // cancelOnRejection(continuation.context, block)
            // TODO: cancel_on_rejection(continuation.get_context(), block)
        }
    }

    // virtual auto invoke_on_timeout(Long timeMillis, Runnable block, context: CoroutineContext): DisposableHandle
    DisposableHandle* invoke_on_timeout(int64_t time_millis, Runnable block, CoroutineContext& context) override {
        int64_t coerced_delay = std::min(time_millis, kMaxDelay);
        // if (handler.postDelayed(block, timeMillis.coerceAtMost(MAX_DELAY)))
        bool posted = false; // TODO: handler->post_delayed(block, coerced_delay)
        if (posted) {
            // return DisposableHandle { handler.removeCallbacks(block) }
            // TODO: return new DisposableHandle with cleanup lambda
            return nullptr;
        }
        cancel_on_rejection(context, block);
        return nullptr; // TODO: NonDisposableHandle
    }

private:
    // auto cancel_on_rejection(CoroutineContext context, block: Runnable)
    void cancel_on_rejection(CoroutineContext& context, Runnable block) {
        // context.cancel(CancellationException("The task was rejected, the handler underlying the dispatcher '${tostd::string()}' was closed"))
        // TODO: context.cancel() with message
        // Dispatchers.IO.dispatch(context, block)
        // TODO: Dispatchers::IO.dispatch(context, block)
    }

public:
    // virtual auto to_string(): std::string
    std::string to_string() override {
        // return tostd::stringInternalImpl() ?: run {
        //     auto str = name ?: handler.tostd::string()
        //     if (invokeImmediately) "$str.immediate" else str
        // }
        // TODO: to_string_internal_impl()
        std::string str = name.value_or("Handler"); // TODO: handler->to_string()
        if (invoke_immediately) {
            return str + ".immediate";
        } else {
            return str;
        }
    }

    // virtual auto equals(other: Any*): Boolean
    bool operator==(const HandlerContext& other) const {
        // return other is HandlerContext && other.handler === handler && other.invokeImmediately == invokeImmediately
        return other.handler == handler && other.invoke_immediately == invoke_immediately;
    }

    // inlining `Boolean.hashCode()` for Android compatibility, as requested by Animal Sniffer
    // virtual auto hash_code(): Int
    size_t hash_code() const {
        // return System.identityHashCode(handler) xor if (invokeImmediately) 1231 else 1237
        // TODO: Use std::hash or identity hash
        size_t handler_hash = reinterpret_cast<size_t>(handler);
        return handler_hash ^ (invoke_immediately ? 1231 : 1237);
    }
};

// @Volatile
// Choreographer* choreographer = nullptr
std::atomic<Choreographer*> choreographer{nullptr};

/**
 * Awaits the next animation frame and returns frame time in nanoseconds.
 */
// auto  await_frame(): Long
// TODO: suspend function -> coroutine semantics not implemented
int64_t await_frame() {
    // fast path when choreographer is already known
    // auto choreographer = choreographer
    Choreographer* choreo = choreographer.load();
    if (choreo != nullptr) {
        // return suspendCancellableCoroutine { cont ->
        //     postFrameCallback(choreographer, cont)
        // }
        // TODO: suspend_cancellable_coroutine
        return 0;
    } else {
        return await_frame_slow_path();
    }
}

// auto  await_frame_slow_path(): Long
// TODO: suspend function -> coroutine semantics not implemented
int64_t await_frame_slow_path() {
    // suspendCancellableCoroutine { cont ->
    //     if (Looper.myLooper() === Looper.getMainLooper()) {
    //         updateChoreographerAndPostFrameCallback(cont)
    //     } else {
    //         Dispatchers.Main.dispatch(cont.context, Runnable {
    //             updateChoreographerAndPostFrameCallback(cont)
    //         })
    //     }
    // }
    // TODO: suspend_cancellable_coroutine, Looper checks, dispatch
    return 0;
}

// auto update_choreographer_and_post_frame_callback(cont: CancellableContinuation<Long>)
template<typename T>
void update_choreographer_and_post_frame_callback(CancellableContinuation<T>& cont) {
    // auto choreographer = choreographer ?: Choreographer.getInstance()!!.also { choreographer = it }
    Choreographer* choreo = choreographer.load();
    if (choreo == nullptr) {
        // TODO: choreo = Choreographer::get_instance()
        choreographer.store(choreo);
    }
    post_frame_callback(choreo, cont);
}

// auto post_frame_callback(Choreographer choreographer, cont: CancellableContinuation<Long>)
template<typename T>
void post_frame_callback(Choreographer* choreo, CancellableContinuation<T>& cont) {
    // choreographer.postFrameCallback { nanos ->
    //     with(cont) { Dispatchers.Main.resumeUndispatched(nanos) }
    // }
    // TODO: choreo->post_frame_callback with lambda
}

// Extension function implementation
HandlerDispatcher* as_coroutine_dispatcher(Handler& handler, const std::optional<std::string>& name) {
    return new HandlerContext(&handler, name);
}

} // namespace android
} // namespace coroutines
} // namespace kotlinx
