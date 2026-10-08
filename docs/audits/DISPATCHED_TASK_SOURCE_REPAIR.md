# DispatchedTask source repair — 2026-10-07

The full Kotlin `common/src/internal/DispatchedTask.kt`, C++ DispatchedTask pair
and common/DispatchedTaskDispatch.hpp were read before editing. The full
DispatchedContinuation Kotlin/C++ pair was also read before changing its
execute_unconfined and inherited run paths.

Translated source behavior:

- `common/DispatchedTaskDispatch.hpp:48`: run uses the checked dispatched-delegate
  cast, rather than silently delivering through an arbitrary continuation. A
  wrong-type Job-key element is rejected. Original resumed exceptions continue
  to dominate cancellation. The actual Native recover_stack_trace entry is used.
- `common/DispatchedTaskDispatch.hpp:103`: the missing source
  run_unconfined_event_loop extension now implements source block execution,
  queue draining, fatal exception reporting and finally use-count cleanup.
  Cleanup also executes if fatal reporting throws. Its template is required by
  the source inline block and typed task; it is not an invented substitute API.
- `common/DispatchedTaskDispatch.hpp:129,145,170`: unconfined resume uses the actual
  event loop instead of an invented missing-loop fallback. Dispatch uses the
  required dispatcher/context objects. Undispatched resume performs the source
  checked cast and calls resume_undispatched_with directly.
- `internal/DispatchedContinuation.hpp:354`: execute_unconfined now calls the
  actual source extension rather than duplicating a loop whose exceptional
  reporting could skip cleanup. It accepts the inline callable directly.
- `internal/DispatchedTask.hpp:107,133`: the genuinely empty source default hook
  retains unused argument names as comments, and run is final. The redundant
  derived run override is removed. get_exceptional_result is open/virtual.
- `internal/DispatchedTask.cpp:9,19`: concrete DispatchException construction and
  C++ exception text transport moved out of the template header. Actual non-null
  context objects supply their virtual to_string; RTTI names are removed. Existing
  null pointer boundary text remains a documented C++ deviation, used by callers.

No warning suppression, dummy argument reads or alternate coroutine frames were
introduced. Existing task shared ownership is retained; a borrowed object is not
adopted. Genuine source mandatory object contracts are required on dispatch paths.

Comments and provenance:

All 10/10 source DispatchedTask KDoc blocks match after whitespace normalization,
including legacy constant stability, result projection, fatal machinery errors
and dispatcher exception handling. The header has thirteen documentation blocks
including provenance and C++ projection notes. Source inline comments explaining
exception precedence, cancellation, queue draining and dispatch branches are
retained. DispatchedContinuation's executeUnconfined KDoc is also restored.
23 source provenance ranges refer to existing source files within valid bounds.
Receipt: `build/ir-recovery/dispatched-task-source-comments-check.json`. These
checks establish text presence and valid ranges, not whole-file runtime fidelity.

Fresh strict verification uses Clang C++20 and
`-Wall -Wextra -Wpedantic -Werror`:

- Concrete DispatchedTask.cpp syntax compilation: exit 0. Receipt:
  `build/ir-recovery/dispatched-task-concrete-strict.log`.
- Actual coroutine-start fixture syntax using the existing Clang plugin: exit 1
  on three unused dependency parameters in JobSupport and
  CancellableContinuationImpl, reduced from five. Receipt:
  `build/ir-recovery/dispatched-task-source-strict.log`.
- Both full-root ast_distance --deep scans: exit 0 using the existing analyzer.
  Fresh strict analyzer/compiler-module builds remain unresolved. There is no
  fresh successful executable result for these dispatch algorithms.

Measured library evidence: 825/2918 functions (previously 824), 359/560 types,
body cosine 0.26, documentation cosine 0.38 (previously 0.37), documentation line
amount 7276/7437 (98%), 123 scoring failures. DispatchedTask is now 10/10
functions and 2/2 source types, body cosine 0.25 (previously 9/10 and 0.19).
These measurements do not establish full body correspondence.

Consumed compiler/stdlib evidence is unchanged: 592/7657 functions, 174/1727
types, body cosine 0.36, documentation cosine 0.60, documentation line amount
2090/5654 (37%), 24 scoring failures. Exact KDoc text, documentation word-frequency
cosine, capped per-file quantity and positional deep comparison are distinct.

Remaining gaps include the fatal diagnostic's hardcoded task name instead of
source object text, the existing typed Result representation and complete
successful-result override projection, actual class-name/debug-text contracts,
ThreadLocalEventLoop source/lifetime mismatches, wider DispatchedContinuation
reuse/state correspondence, strict dependency warnings, compiler generation,
and both MLX GPU acceptance paths. The report also still lists the
DispatchedContinuation source pair as unmatched; source text here is not used
to override that oracle finding. No subsystem is declared complete.
