# Kotlin Intrinsics and Cancellation (Port Notes)

This directory contains C++ equivalents of Kotlin coroutine **intrinsics** and closely-related runtime helpers.
These concepts are foundational for the rest of the port because they define the **suspend ABI**, the **suspension
sentinel**, and how the runtime drives a compiled **state machine** via `Continuation`.

## Does cancellation require compiler / IR support?

**No — cancellation is implemented in library code.**  
Kotlin cancellation is cooperative and is built on top of:

- the coroutine context `Job` hierarchy (library),
- `CancellationException` propagation (library),
- suspend lowering + continuation passing (compiler),
- and one key intrinsic used by many suspend primitives: `suspendCoroutineUninterceptedOrReturn` (compiler intrinsic).

In other words:

- **Suspend functions** require compiler transformation into a state machine (labels/spills/resume).
- **Cancellation** does *not* require its own compiler transformation; it is enforced by coroutine runtime + dispatchers
  around suspension/resumption.

### Kotlin ground-truth references

In upstream Kotlin (`tmp/kotlinx.coroutines`), cancellable suspension is implemented as library code:

- `kotlinx-coroutines-core/common/src/CancellableContinuation.kt`
  - `suspendCancellableCoroutine { ... }` is implemented using `suspendCoroutineUninterceptedOrReturn { uCont -> ... }`.
  - Explains the **prompt cancellation guarantee** and how it requires coordination with `CoroutineDispatcher`.
- `kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt`
  - Implements the state machine and atomic transitions for `CancellableContinuationImpl`.

These files use annotations like `@InternalCoroutinesApi`, `@PublishedApi`, `@OptIn(...)`, etc. as **API/visibility**
markers — they are not compiler “magic” for cancellation.

## C++ equivalents in this repo

### Suspension sentinel and checks

- `kotlinx/coroutines/intrinsics/Intrinsics.hpp`
  - Defines `COROUTINE_SUSPENDED` and `intrinsics::is_coroutine_suspended(void*)`.
  - C++ suspend functions must return either `COROUTINE_SUSPENDED` or a boxed `void*` result (`nullptr` is used for
    Unit/void in many call sites).

### Running the state machine

- `kotlinx/coroutines/ContinuationImpl.hpp`
  - `BaseContinuationImpl::resume_with(...)` runs the `invoke_suspend(...)` loop and checks
    `intrinsics::is_coroutine_suspended(...)`.

### Cancellable suspension helper

The C++ port models Kotlin’s `suspendCancellableCoroutine` pattern via the existing continuation ABI and helper
utilities (e.g. `suspend_cancellable_coroutine<T>(...)` in the coroutine implementation headers).

## How cancellation propagates (conceptually)

1. A coroutine suspends at a suspension point (state machine returns `COROUTINE_SUSPENDED`).
2. Cancellation is signaled by completing/cancelling the `Job` in the coroutine context (library).
3. When a continuation is resumed, dispatchers may check the `Job` before executing the resumed continuation to enforce
   the **prompt cancellation guarantee** (library/runtime behavior).

The compiler is responsible only for suspend lowering and intrinsics; cancellation semantics live in library/runtime
code layered on top of the lowered suspend machinery.

