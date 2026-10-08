# AbstractCoroutine source repair — 2026-10-07

The complete common `AbstractCoroutine.kt` and C++ header were read before editing.

The source start API at Kotlin lines 133-135 now has a typed receiver/block entry
at `src/kotlinx/coroutines/AbstractCoroutine.hpp:233`. It forwards to the actual
CoroutineStart typed entry with the original `Continuation<T>` completion. Existing
ordinary C++ and erased owning ABI bindings at :242 and :249 remain available.
Parent initialization retains the existing C++ construction adaptation; no
coroutine state machine or new frame was introduced.

The source's genuinely empty completion hooks at Kotlin :70 and :83 keep their
parameter types and names as comments at header :129 and :144. No fake reads or
warning suppressions were introduced. Source final completion contracts are now
explicit on resume_with (:164), on_completion_internal (:189), and
handle_on_completion_exception (:198).

All eight source KDoc blocks are retained with exact normalized text, including
cancellation timing, hook parameters, conceptual resume and start strategy
contracts. The C++ header has twelve documentation blocks including provenance
and existing C++ boundary notes. Receipt:
`build/ir-recovery/abstract-coroutine-kdoc-check.json`. Text presence does not
prove all declaration attachment, source logic or runtime lowering.

Fresh strict checks with C++20, -Wall -Wextra -Wpedantic -Werror and the existing
Clang suspend plugin:

- Actual `src/tests/src/suspend/test_cancellable_start.cpp` syntax: exit 1.
- Explicit AbstractCoroutine<int> typed-start instantiation covering all four
  strategy values: exit 1.
- Both expose the same five dependency unused-parameter errors in JobSupport,
  DispatchedTask and CancellableContinuationImpl. The three AbstractCoroutine
  hook errors are resolved. Logs are
  `build/ir-recovery/abstract-coroutine-typed-strict.log` and
  `build/ir-recovery/abstract-coroutine-typed-instantiation.log`.

There is no fresh successful coroutine-start executable result for this edit.
Historical queued receiver/capture lifetime tests do not verify the new typed
entry. Deferred parent setup, context construction, erased result projection,
actual class-name diagnostics, full JobSupport algorithms, compiler generation
and both MLX GPU acceptance paths remain incomplete. In particular,
cancellation_exception_message still hardcodes AbstractCoroutine instead of the
source classSimpleName contract. Native JobCancellationException.to_string and
Continuation.to_debug_string also remain genuine dependency-impact source gaps.
