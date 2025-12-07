// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/Scopes.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: CoroutineContext, Continuation need C++ equivalents
// TODO: AbstractCoroutine, CoroutineStackFrame need implementation
// TODO: @JvmField annotation - JVM-specific, translate to comment
// TODO: suspend functions and coroutine semantics not implemented

#include <string>

namespace kotlinx {
namespace coroutines {
namespace internal {

// Forward declarations
class CoroutineContext;
template<typename T> class Continuation;
template<typename T> class AbstractCoroutine;
class CoroutineStackFrame;
class CoroutineScope;

/**
 * This is a coroutine instance that is created by [coroutineScope] builder.
 */
template<typename T>
class ScopeCoroutine : public AbstractCoroutine<T>, public CoroutineStackFrame {
public:
    Continuation<T>* u_cont; // unintercepted continuation

    ScopeCoroutine(CoroutineContext* context, Continuation<T>* u_cont)
        : AbstractCoroutine<T>(context, true, true), u_cont(u_cont) {}

    CoroutineStackFrame* get_caller_frame() override {
        // TODO: u_cont as? CoroutineStackFrame
        return nullptr;
    }

    void* get_stack_trace_element() override { return nullptr; }

    bool is_scoped_coroutine() override { return true; }

    void after_completion(void* state) override {
        // Resume in a cancellable way by default when resuming from another context
        // TODO: u_cont.intercepted().resumeCancellableWith(recoverResult(state, u_cont))
    }

    /**
     * Invoked when a scoped coorutine was completed in an undispatched manner directly
     * at the place of its start because it never suspended.
     */
    virtual void after_completion_undispatched() {
        // Empty default implementation
    }

    void after_resume(void* state) override {
        // Resume direct because scope is already in the correct context
        // TODO: u_cont.resumeWith(recoverResult(state, u_cont))
    }
};

class ContextScope : public CoroutineScope {
private:
    CoroutineContext* coroutine_context_;

public:
    explicit ContextScope(CoroutineContext* context) : coroutine_context_(context) {}

    CoroutineContext* get_coroutine_context() override { return coroutine_context_; }

    // CoroutineScope is used intentionally for user-friendly representation
    std::string to_string() {
        // TODO: "CoroutineScope(coroutineContext=$coroutineContext)"
        return "CoroutineScope(coroutineContext=...)";
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
