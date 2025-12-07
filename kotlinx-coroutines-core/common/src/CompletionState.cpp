// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/CompletionState.kt
//
// TODO: Result<T> type needs custom implementation
// TODO: getOrElse extension needs implementation
// TODO: recoverStackTrace needs implementation

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.atomicfu.* - use std::atomic
// TODO: import kotlinx.coroutines.internal.* - use includes
// TODO: import kotlin.coroutines.* - use custom types
// TODO: import kotlin.jvm.* - JVM-specific, ignore

// TODO: internal fun - internal visibility
template<typename T>
void* toState(Result<T> result) {
    // TODO: getOrElse { CompletedExceptionally(it) }
    if (result.isSuccess()) {
        return result.getValue();
    } else {
        return new CompletedExceptionally(result.getException());
    }
}

// TODO: internal fun - internal visibility
template<typename T>
void* toState(Result<T> result, CancellableContinuation<void>* caller) {
    // TODO: getOrElse { CompletedExceptionally(recoverStackTrace(it, caller)) }
    if (result.isSuccess()) {
        return result.getValue();
    } else {
        return new CompletedExceptionally(recoverStackTrace(result.getException(), caller));
    }
}

// TODO: @Suppress("RESULT_CLASS_IN_RETURN_TYPE", "UNCHECKED_CAST") - no C++ equivalent
// TODO: internal fun - internal visibility
template<typename T>
Result<T> recoverResult(void* state, Continuation<T>* u_cont) {
    if (auto* completed_exceptionally = dynamic_cast<CompletedExceptionally*>(state)) {
        return Result<T>::failure(recoverStackTrace(completed_exceptionally->cause, u_cont));
    } else {
        // TODO: state as T - cast
        return Result<T>::success(static_cast<T>(state));
    }
}

/**
 * Class for an internal state of a job that was cancelled (completed exceptionally).
 *
 * @param cause the exceptional completion cause. It's either original exceptional cause
 *        or artificial [CancellationException] if no cause was provided
 */
// TODO: internal open class
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

    bool makeHandled() {
        bool expected = false;
        return _handled.compare_exchange_strong(expected, true);
    }

    std::string toString() const {
        // TODO: classSimpleName - type name extraction
        return "CompletedExceptionally[" + std::string(/* cause->toString() */) + "]";
    }
};

/**
 * A specific subclass of [CompletedExceptionally] for cancelled [AbstractContinuation].
 *
 * @param continuation the continuation that was cancelled.
 * @param cause the exceptional completion cause. If `cause` is null, then a [CancellationException]
 *        if created on first access to [exception] property.
 */
// TODO: internal class
class CancelledContinuation : public CompletedExceptionally {
private:
    std::atomic<bool> _resumed;

public:
    CancelledContinuation(
        Continuation<void>* continuation,
        Throwable* cause,
        bool handled
    ) : CompletedExceptionally(
            cause != nullptr ? cause : new CancellationException("Continuation " + /* continuation->toString() */ std::string() + " was cancelled normally"),
            handled
        ),
        _resumed(false) {}

    bool makeResumed() {
        bool expected = false;
        return _resumed.compare_exchange_strong(expected, true);
    }
};

} // namespace coroutines
} // namespace kotlinx
