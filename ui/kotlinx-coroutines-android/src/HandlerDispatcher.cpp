// Transliterated from Kotlin: kotlinx.coroutines.android.HandlerDispatcher
// Original package: kotlinx.coroutines.android
//
// TODO: This is a mechanical C++ transliteration. The following constructs need proper implementation:
// - Android OS API calls (Handler, Looper, Build, Choreographer)
// - AndroidX annotations (@VisibleForTesting, @JvmName, @JvmOverloads, @JvmField)
// - Kotlin sealed classes -> abstract base class with private constructor
// - Kotlin object declarations -> singletons (namespace with static members or Meyer's singleton)
// - Kotlin extension functions (Handler.asCoroutineDispatcher, Looper.asHandler, Dispatchers.JavaFx extension)
// - Kotlin suspend functions (awaitFrame, awaitFrameSlowPath) -> TODO: coroutine semantics not implemented
// - MainCoroutineDispatcher, Delay, CoroutineDispatcher interfaces
// - Reflection for Handler.createAsync and constructor lookup
// - Kotlin data types: Runnable, CancellableContinuation, DisposableHandle, CoroutineContext
// - Kotlin nullable types (T?) -> pointers or std::optional
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
// public sealed class HandlerDispatcher
class HandlerDispatcher : public MainCoroutineDispatcher, public Delay {
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
     * suspend fun updateUiElement(val text: String) {
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
    // public abstract override val immediate: HandlerDispatcher
    virtual HandlerDispatcher& get_immediate() = 0;

protected:
    // Sealed class -> protected constructor
    HandlerDispatcher() = default;
};

// internal class AndroidDispatcherFactory
class AndroidDispatcherFactory : public MainDispatcherFactory {
public:
    // override fun createDispatcher(allFactories: List<MainDispatcherFactory>)
    MainCoroutineDispatcher* create_dispatcher(const std::vector<MainDispatcherFactory*>& all_factories) override {
        // val mainLooper = Looper.getMainLooper() ?: throw IllegalStateException("The main looper is not available")
        Looper* main_looper = nullptr; // TODO: Looper::get_main_looper()
        if (main_looper == nullptr) {
            throw std::runtime_error("The main looper is not available");
        }
        // return HandlerContext(mainLooper.asHandler(async = true))
        Handler* handler = as_handler(*main_looper, true);
        return new HandlerContext(handler);
    }

    // override fun hintOnError(): String
    std::string hint_on_error() override {
        return "For tests Dispatchers.setMain from kotlinx-coroutines-test module can be used";
    }

    // override val loadPriority: Int
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
// public fun Handler.asCoroutineDispatcher(name: String? = null): HandlerDispatcher
// TODO: Extension function -> free function taking Handler as first parameter
HandlerDispatcher* as_coroutine_dispatcher(Handler& handler, const std::optional<std::string>& name = std::nullopt);

// private const val MAX_DELAY = Long.MAX_VALUE / 2
constexpr int64_t kMaxDelay = std::numeric_limits<int64_t>::max() / 2; // cannot delay for too long on Android

// @VisibleForTesting
// internal fun Looper.asHandler(async: Boolean): Handler
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
        // val factoryMethod = Handler::class.java.getDeclaredMethod("createAsync", Looper::class.java)
        // return factoryMethod.invoke(null, this) as Handler
        // TODO: Reflection - Handler::createAsync(Looper)
        return nullptr;
    }

    // val constructor: Constructor<Handler>
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
    // return constructor.newInstance(this, null, true)
    return nullptr; // TODO: Invoke constructor with (looper, nullptr, true)
}

// @JvmField
// @Deprecated("Use Dispatchers.Main instead", level = DeprecationLevel.HIDDEN)
// internal val Main: HandlerDispatcher?
// TODO: This is a top-level property; consider namespace-level static or singleton
// runCatching { HandlerContext(Looper.getMainLooper().asHandler(async = true)) }.getOrNull()

/**
 * Implements [CoroutineDispatcher] on top of an arbitrary Android [Handler].
 */
// internal class HandlerContext
class HandlerContext : public HandlerDispatcher, public Delay {
private:
    Handler* handler;
    std::optional<std::string> name;
    bool invoke_immediately;

    // private constructor
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

    // override val immediate: HandlerContext
    HandlerContext& get_immediate() override {
        if (invoke_immediately) {
            return *this;
        } else {
            static HandlerContext immediate_ctx(handler, name, true);
            return immediate_ctx;
        }
    }

    // override fun isDispatchNeeded(context: CoroutineContext): Boolean
    bool is_dispatch_needed(CoroutineContext& context) override {
        // return !invokeImmediately || Looper.myLooper() != handler.looper
        // TODO: Looper::my_looper() != handler->get_looper()
        return !invoke_immediately; // TODO: Add looper check
    }

    // override fun dispatch(context: CoroutineContext, block: Runnable)
    void dispatch(CoroutineContext& context, Runnable block) override {
        // if (!handler.post(block))
        bool posted = false; // TODO: handler->post(block)
        if (!posted) {
            cancel_on_rejection(context, block);
        }
    }

    // override fun scheduleResumeAfterDelay(timeMillis: Long, continuation: CancellableContinuation<Unit>)
    void schedule_resume_after_delay(int64_t time_millis, CancellableContinuation<void>& continuation) override {
        // val block = Runnable {
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

    // override fun invokeOnTimeout(timeMillis: Long, block: Runnable, context: CoroutineContext): DisposableHandle
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
    // private fun cancelOnRejection(context: CoroutineContext, block: Runnable)
    void cancel_on_rejection(CoroutineContext& context, Runnable block) {
        // context.cancel(CancellationException("The task was rejected, the handler underlying the dispatcher '${toString()}' was closed"))
        // TODO: context.cancel() with message
        // Dispatchers.IO.dispatch(context, block)
        // TODO: Dispatchers::IO.dispatch(context, block)
    }

public:
    // override fun toString(): String
    std::string to_string() override {
        // return toStringInternalImpl() ?: run {
        //     val str = name ?: handler.toString()
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

    // override fun equals(other: Any?): Boolean
    bool operator==(const HandlerContext& other) const {
        // return other is HandlerContext && other.handler === handler && other.invokeImmediately == invokeImmediately
        return other.handler == handler && other.invoke_immediately == invoke_immediately;
    }

    // inlining `Boolean.hashCode()` for Android compatibility, as requested by Animal Sniffer
    // override fun hashCode(): Int
    size_t hash_code() const {
        // return System.identityHashCode(handler) xor if (invokeImmediately) 1231 else 1237
        // TODO: Use std::hash or identity hash
        size_t handler_hash = reinterpret_cast<size_t>(handler);
        return handler_hash ^ (invoke_immediately ? 1231 : 1237);
    }
};

// @Volatile
// private var choreographer: Choreographer? = null
std::atomic<Choreographer*> choreographer{nullptr};

/**
 * Awaits the next animation frame and returns frame time in nanoseconds.
 */
// public suspend fun awaitFrame(): Long
// TODO: suspend function -> coroutine semantics not implemented
int64_t await_frame() {
    // fast path when choreographer is already known
    // val choreographer = choreographer
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

// private suspend fun awaitFrameSlowPath(): Long
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

// private fun updateChoreographerAndPostFrameCallback(cont: CancellableContinuation<Long>)
template<typename T>
void update_choreographer_and_post_frame_callback(CancellableContinuation<T>& cont) {
    // val choreographer = choreographer ?: Choreographer.getInstance()!!.also { choreographer = it }
    Choreographer* choreo = choreographer.load();
    if (choreo == nullptr) {
        // TODO: choreo = Choreographer::get_instance()
        choreographer.store(choreo);
    }
    post_frame_callback(choreo, cont);
}

// private fun postFrameCallback(choreographer: Choreographer, cont: CancellableContinuation<Long>)
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
