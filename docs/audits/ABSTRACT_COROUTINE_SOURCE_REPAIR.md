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

Both full-root deep scans completed with exit 0 using the existing analyzer
binary; its fresh strict rebuild remains unresolved. Library measurements:
824/2918 functions, 359/560 types, average body cosine 0.26, documentation cosine
0.37, documentation line amount 7232/7437 (97%), 123 scoring failures.
AbstractCoroutine remains 9/9 function names and 1/1 type, but reported body
cosine changed from 0.39 to 0.29 after typed overload and unused binding changes.
This measured regression is retained without weakening the oracle. Full symbol
presence and all-eight exact KDoc text presence do not establish full source
body parity. The line-amount decrease from 7238 to 7232 reflects replacing the
long invented class description with the actual shorter KDoc; quantity and
correspondence must be evaluated separately.

Consumed compiler/stdlib measurements: 592/7657 functions, 174/1727 types,
body cosine 0.36, documentation cosine 0.60, documentation line amount
2090/5654 (37%), 24 scoring failures. Positional deep comparison, word-frequency
cosines, capped per-file documentation amount and exact block text are distinct
evidence. The source-first goal remains active.
