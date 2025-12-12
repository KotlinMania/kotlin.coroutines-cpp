# kotlinx.coroutines (C++) — Cancellation vs Suspend Notes

This folder contains the C++ transliteration of Kotlin `kotlinx.coroutines`.
The guiding rule is **near line‑by‑line parity with Kotlin** unless a small helper
is required to bridge language/ABI differences.

## Key conclusion: cancellation is library‑level

Kotlin cancellation **does not require compiler/IR support** the way `suspend`
functions do.

- `suspend` needs compiler lowering into a continuation/state machine
  (hence the Clang suspend‑plugin + Continuation ABI in this repo).
- Cancellation is implemented entirely in **library code**:
  atomics, state machines, and `Job` propagation.
- The only “intrinsics” cancellation code uses are the **standard coroutine
  suspend intrinsics** (`suspend_coroutine_unintercepted_or_return`,
  `create_coroutine_unintercepted`, `COROUTINE_SUSPENDED`, etc.).
  These are not cancellation‑specific; they are required because cancellable
  operations are also suspendable.

There are no cancellation‑specific compiler annotations or typed intrinsics
in upstream `kotlinx.coroutines`.

## Kotlin sources that define cancellation

These are the ground‑truth files in `tmp/` that cancellation behavior comes from:

- `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/CancellableContinuation.kt`
  - Public `CancellableContinuation<T>` API.
  - `suspend_cancellable_coroutine{}` entry points and prompt cancellation docs.
- `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt`
  - Atomic state machine: `_state`, `decision_and_index_`, `try_suspend/try_resume`,
    parent handle installation, cancellation handlers, reusable continuations.
- `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt`
  - Dispatch‑time cancellation checks (`job->is_active()` gate).
- `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/internal/DispatchedContinuation.kt`
  - `count_or_element` caching and undispatched resume paths.
- `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/JobSupport.kt`
  - Parent/child propagation and cancellation cause rules.
- `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt`
  - Cancellable coroutine start helpers (still library code).

## C++ counterparts in this repo

Primary cancellation machinery lives here:

- `src/kotlinx/coroutines/CancellableContinuation.hpp`
- `src/kotlinx/coroutines/CancellableContinuationImpl.hpp`
- `src/kotlinx/coroutines/internal/DispatchedTask.hpp`
- `src/kotlinx/coroutines/internal/DispatchedContinuation.hpp`
- `src/kotlinx/coroutines/JobSupport.hpp` / `.cpp`
- Suspend intrinsics/ABI helpers:
  - `src/kotlinx/coroutines/intrinsics/Intrinsics.hpp`
  - `src/kotlinx/coroutines/dsl/Suspend.hpp` (future plugin‑migration surface)

## Porting implications

- **Do not invent new cancellation abstractions.**
  Cancellation logic should be transliterated 1:1 from the Kotlin sources above.
- Any “inventive” work is limited to **suspend lowering helpers**
  (Continuation ABI, suspend DSL/plugin). Cancellation does not need its own IR.
- Preserve Kotlin KDoc in headers/impls, translating to C++/Google style only
  where required.

If you’re changing cancellation behavior in C++, always re‑open the matching
Kotlin file under `tmp/` first and keep run‑rate parity.
