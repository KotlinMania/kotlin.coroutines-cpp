#pragma once

namespace kotlinx {
namespace coroutines {

/**
 * Defines start options for coroutines builders.
 */
enum class CoroutineStart {
    DEFAULT,
    LAZY,
    ATOMIC,
    UNDISPATCHED
};

inline bool is_lazy(CoroutineStart start) {
    return start == CoroutineStart::LAZY;
}

/*
 * TODO: STUB - CoroutineStart.invoke() dispatch logic not implemented
 *
 * Kotlin source: CoroutineStart.invoke() in CoroutineStart.kt
 *
 * What's missing:
 * - DEFAULT: Should dispatch via block.startCoroutineCancellable(receiver, completion)
 * - LAZY: Should not start immediately, just create the coroutine
 * - ATOMIC: Should start atomically without checking cancellation first
 * - UNDISPATCHED: Should run immediately in current thread without dispatching
 *
 * Current behavior: Does nothing - coroutines don't actually start via this path
 * Correct behavior: Dispatch or execute the coroutine block based on start mode
 *
 * Dependencies:
 * - startCoroutineCancellable() for DEFAULT mode
 * - Dispatcher integration for proper thread handling
 * - CancellableContinuation for cancellation checks
 *
 * Note: AbstractCoroutine.start() has its own implementation that works around this.
 */
template <typename Block, typename Receiver, typename Completion>
void invoke(CoroutineStart start, Block&& block, Receiver&& receiver, Completion&& completion) {
    (void)start;
    (void)block;
    (void)receiver;
    (void)completion;
    // Stub - see TODO block above
}

} // namespace coroutines
} // namespace kotlinx
