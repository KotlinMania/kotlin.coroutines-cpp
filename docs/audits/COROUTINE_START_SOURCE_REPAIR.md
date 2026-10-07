# CoroutineStart source selection repair

Date: 2026-10-07. Read complete common CoroutineStart.kt and the existing C++
header/source, actual start intrinsics and consumed callable/context helpers.
CoroutineStart is sixth in the dependency-priority report. Its consumed Native
wrappers were translated in the preceding commit.

`CoroutineStart.cpp:7` mirrors source CoroutineStart.kt:356-362: DEFAULT calls
start_coroutine_cancellable, ATOMIC calls start_coroutine, UNDISPATCHED calls
start_coroutine_undispatched and LAZY does nothing. `CoroutineStart.cpp:24`
implements is_lazy from source :370. The public enum retains its source KDoc and
is declared in the header with provenance. Concrete selection lives in .cpp.

The old generic invoke directly queried dispatchers, enqueued a custom runnable,
checked Jobs and delivered completion itself. It bypassed ordinary continuation
interceptors and caught exceptions from successful completion, attempting an extra
failure delivery. Those substitute algorithms are removed.

`CoroutineStart.hpp:387` binds public generic C++ callables and receivers into
the existing erased suspend ABI. It invokes the actual Native wrapper argument,
returns the actual erased result or boxes an ordinary C++ result, and propagates
exceptions to the source intrinsic. The receiving typed adapter unboxes and
deletes those boxes. Moved callable/receiver owners remain retained; noncopyable
lvalues stay explicitly borrowed. The existing legacy current-continuation binding
uses that actual wrapper. `CoroutineStart.hpp:425` delegates the public receiver
API to the concrete selection, without moving/capturing a LAZY body. The existing
compatibility extension class forwards to the same public entry, including typed
continuation arguments. This generic binding is an explicit C++ ABI adaptation;
it does not implement its own dispatcher or coroutine algorithm.

The expanded test_cancellable_start exercises all four start strategies through
the generic entry. A plain custom ContinuationInterceptor sees interception,
resumption and release for DEFAULT/ATOMIC; UNDISPATCHED enters immediately and
LAZY performs no work. The body receives an actual Native ContinuationImpl frame.
Move-only callable captures survive queued execution and cancellation, then
release. Noncopyable lvalue receiver identity remains borrowed. Typed receiver
compatibility resumes after suspension. Dispatcher failure reports the original
cause, and a throwing Undispatched completion is invoked only once.

The real custom-interceptor regression compiles against both the preceding header
and the new implementation. The preceding program exits one; the new one exits
zero. `build/ir-recovery/coroutine-start-probe-results.json` records both results.
Nine focused executables finish with zero failures in 2.05 seconds, recorded in
`coroutine-start-final-tests.log`, with builds in `coroutine-start-regression-build.log`.
The actual CoroutineStart, start intrinsic and continuation sources compile and
run directly with Clang AddressSanitizer/UndefinedBehaviorSanitizer and the core
archive, exit zero and no diagnostics (`coroutine-start-sanitizer-build.log`,
`coroutine-start-sanitizer-tests.log`). This component command invokes no Kotlin
compiler and links no Native runtime. It does not establish either complete
required standalone/Native MLX application path.

All new source ranges resolve to valid upstream bounds; the two changed library
files contain no prohibited markers. Broader lazy builder, result and compiler
lowering contracts remain incomplete. The full source-first goal remains active.

Both final full-root deep scans exit zero and refresh the inventories, measured
criteria and dependency priorities after the library and tool changes. Library
function names are 783/2918, types 341/560, average body similarity 0.26 and
scoring failures 122. CoroutineStart records 1/1 function names, 1/1 types and
0.13 body similarity. Its method/getter presence is corrected by the reproduced
[enum receiver report repair](AST_DISTANCE_ENUM_RECEIVER_REPAIR.md), rather than
by changing the actual source API to satisfy a false owner-matching report.
Public binding differences remain visible in body/parameter evidence; name
presence and focused tests do not establish whole-library completion. The
separate consumed Native namespace-projection findings remain visible.
