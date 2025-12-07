// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/CoroutineContext.kt
//
// TODO: actual/expect keyword not directly translatable
// TODO: object keyword - translate to singleton or namespace with static members
// TODO: Continuation interface from Kotlin coroutines
// TODO: suspend function semantics

namespace kotlinx {
namespace coroutines {

// TODO: Remove imports, fully qualify or add includes:
// import kotlinx.coroutines.internal.*
// import kotlin.coroutines.*

// TODO: internal actual object -> singleton pattern
class DefaultExecutor : public CoroutineDispatcher, public Delay {
private:
    WorkerDispatcher delegate;

    // Private constructor for singleton
    DefaultExecutor() : delegate("DefaultExecutor") {}

public:
    static DefaultExecutor& instance() {
        static DefaultExecutor instance;
        return instance;
    }

    void dispatch(CoroutineContext context, Runnable block) override {
        delegate.dispatch(context, block);
    }

    void schedule_resume_after_delay(long time_millis, CancellableContinuation<void>& continuation) override {
        delegate.schedule_resume_after_delay(time_millis, continuation);
    }

    DisposableHandle invoke_on_timeout(long time_millis, Runnable block, CoroutineContext context) override {
        return delegate.invoke_on_timeout(time_millis, block, context);
    }

    void enqueue(Runnable task) {
        delegate.dispatch(EmptyCoroutineContext, task);
    }
};

// TODO: expect function - platform-specific declaration
CoroutineDispatcher* create_default_dispatcher();

// TODO: @PublishedApi internal actual
Delay& kDefaultDelay = DefaultExecutor::instance();

CoroutineContext CoroutineScope::new_coroutine_context(CoroutineContext context) {
    auto combined = this->coroutine_context + context;
    if (combined != Dispatchers::kDefault && combined[ContinuationInterceptor] == nullptr) {
        return combined + Dispatchers::kDefault;
    }
    return combined;
}

CoroutineContext CoroutineContext::new_coroutine_context(CoroutineContext added_context) {
    return *this + added_context;
}

// No debugging facilities on native
template<typename T>
inline T with_coroutine_context(CoroutineContext context, void* count_or_element, std::function<T()> block) {
    return block();
}

template<typename T>
inline T with_continuation_context(Continuation<void>& continuation, void* count_or_element, std::function<T()> block) {
    return block();
}

std::string to_debug_string(Continuation<void>& continuation) {
    // TODO: toString equivalent
    return "";
}

std::string* coroutine_name(CoroutineContext& context) {
    return nullptr; // not supported on native
}

// TODO: internal actual class
template<typename T>
class UndispatchedCoroutine : public ScopeCoroutine<T> {
private:
    Continuation<T>& u_cont;

public:
    UndispatchedCoroutine(CoroutineContext context, Continuation<T>& u_cont)
        : ScopeCoroutine<T>(context, u_cont)
        , u_cont(u_cont)
    {
    }

    void after_resume(void* state) override {
        u_cont.resume_with(recover_result(state, u_cont));
    }
};

} // namespace coroutines
} // namespace kotlinx
