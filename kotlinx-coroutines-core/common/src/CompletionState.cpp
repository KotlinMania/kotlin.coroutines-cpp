// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/CompletionState.kt
//
// TODO: Result<T> type needs custom implementation
// TODO: getOrElse extension needs implementation
// TODO: recoverStackTrace needs implementation

#include "kotlinx/coroutines/core_fwd.hpp"
#include <atomic>
#include <string>

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.atomicfu.* - use std::atomic
// TODO: import kotlinx.coroutines.internal.* - use includes
// TODO: import kotlin.coroutines.* - use custom types
// TODO: import kotlin.jvm.* - JVM-specific, ignore

// TODO: fun - visibility
template<typename T>
void* to_state(Result<T> result) {
    // TODO: getOrElse { CompletedExceptionally(it) }
    if (result.isSuccess()) {
        return result.getValue();
    } else {
        return new CompletedExceptionally(result.getException());
    }
}

// TODO: fun - visibility
template<typename T>
void* to_state(Result<T> result, CancellableContinuation<void>* caller) {
    // TODO: getOrElse { CompletedExceptionally(recoverStackTrace(it, caller)) }
    if (result.isSuccess()) {
        return result.getValue();
    } else {
        return new CompletedExceptionally(recover_stack_trace(result.getException(), caller));
    }
}

// TODO: @Suppress("RESULT_CLASS_IN_RETURN_TYPE", "UNCHECKED_CAST") - no C++ equivalent
// TODO: fun - visibility
template<typename T>
Result<T> recover_result(void* state, Continuation<T>* u_cont) {
    if (auto* completed_exceptionally = dynamic_cast<CompletedExceptionally*>(state)) {
        return Result<T>::failure(recover_stack_trace(completed_exceptionally->cause, u_cont));
    } else {
        // TODO: state as T - cast
        return Result<T>::success(static_cast<T>(state));
    }
}

/**
 * Class for an state of a job that was cancelled (completed exceptionally).
 *
 * @param cause the exceptional completion cause. It's either original exceptional cause
 *        or artificial [CancellationException] if no cause was provided
 */
// TODO: open class
class CompletedExceptionally {
private:
    std::atomic<bool> _handled;

public:
    // TODO: @JvmField - JVM-specific, no C++ equivalent
    Throwable* cause;

    CompletedExceptionally(Throwable* cause_param, bool handled = false)
        : cause(cause_param), _handled(handled) {}

    bool get_handled() const {
        return _handled.load();
    }

    bool make_handled() {
        bool expected = false;
        return _handled.compare_exchange_strong(expected, true);
    }

    std::string to_string() const {
        // TODO: classSimpleName - type name extraction
        return "CompletedExceptionally[" + std::string(/* cause->tostd::string() */) + "]";
    }
};

/**
 * A specific subclass of [CompletedExceptionally] for cancelled [AbstractContinuation].
 *
 * @param continuation the continuation that was cancelled.
 * @param cause the exceptional completion cause. If `cause` is nullptr, then a [CancellationException]
 *        if created on first access to [exception] property.
 */
// TODO: class
class CancelledContinuation : CompletedExceptionally {
private:
    std::atomic<bool> _resumed;

public:
    CancelledContinuation(
        Continuation<void>* continuation,
        Throwable* cause,
        bool handled
    ) : CompletedExceptionally(
            cause != nullptr ? cause : new CancellationException("Continuation " + /* continuation->tostd::string() */ std::string() + " was cancelled normally"),
            handled
        ),
        _resumed(false) {}

    bool make_resumed() {
        bool expected = false;
        return _resumed.compare_exchange_strong(expected, true);
    }
};

} // namespace coroutines
} // namespace kotlinx
