# Native context inline source repair — 2026-10-07

Source: `kotlinx-coroutines-core/native/src/CoroutineContext.kt:42-44` and
`common/src/CoroutineContext.common.kt:24-26`, both read in full.

`src/kotlinx/coroutines/common/CoroutineContextUtils.hpp:27,39` now translates the
Native inline block directly with a deduced callable instead of forcing a
`std::function` conversion. This preserves move-only captures at an ordinary C++
boundary. Context/cache parameters retain their types and source names in
comments; the actual Native source does not read them. No dummy reads, warning
controls or additional frames were introduced. Native debugging and common
cache comments are retained, with canonical file and function provenance.

The consumed stdlib `Continuation.kt` was also read in full.
`src/kotlinx/coroutines/Continuation.hpp:31,39,44,52,67,95` retains six source
KDoc blocks verbatim: interface, context, resumeWith, resume,
resumeWithException and continuation factory. Normalized exact-block comparison
finds 6/6 restored blocks present, but only 6/13 total source KDoc blocks in this
header. The source also describes APIs implemented elsewhere or still missing;
text presence here does not establish declaration attachment or whole-file
translation. Receipt: `build/ir-recovery/continuation-kdoc-check.json`.

Verification:

- Fresh isolated executable compilation with Clang, C++20 and
  `-Wall -Wextra -Wpedantic -Werror`: exit 0. Execution: exit 0.
  Checks move-only capture/result ownership, exactly one block invocation and
  original exception propagation. Source and build/run logs are under
  `build/ir-recovery/native-context-inline-check*`.
- Actual `test_cancellable_start.cpp` strict syntax check using the existing
  Clang plugin: exit 1 with eight dependency unused-parameter errors, down from
  twelve. None comes from the context inline wrappers. Log:
  `build/ir-recovery/native-context-inline-strict.log`.
- Both full-root `ast_distance --deep` scans: exit 0, using the previously built
  analyzer. Its fresh strict rebuild remains unresolved. These scans do not
  verify runtime coroutine lowering or GPU interoperability.

Library: 824/2918 functions, 359/560 types, body cosine 0.26; documentation cosine
0.37 and capped per-file documentation line amount 7238/7437 (97%); 123 scoring
failures. Consumed compiler/stdlib report: 592/7657 functions, 174/1727 types,
body cosine 0.36; documentation cosine 0.60 and line amount 2090/5654 (37%);
24 scoring failures. Word-frequency documentation cosine, documentation amount,
exact KDoc text presence, and positional deep correspondence are different
measurements; none alone proves meaningful comment parity.

The existing RTTI/null `to_debug_string` fallback remains a genuine mismatch to
Native `Continuation<*>.toDebugString() = toString()`. The current continuation
interface has no shared object to_string contract. This repair does not claim
the Native context file complete. Eight strict start-fixture dependency
warnings, broad source gaps, current plugin generation failures and both
required MLX GPU executable acceptance paths remain unresolved. No suppressed
warnings or historical executable evidence are treated as fresh verification.
