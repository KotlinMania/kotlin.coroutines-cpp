# Native WorkerDispatcher source repair — 2026-10-08

## Comparable marks and source interfaces — 2026-10-08

Continuation from `5312f3a0` adds kotlin/Comparable.hpp from the actual Native
Comparable.kt:11-21. It is the source public generic ordering interface, with a
borrowed const-reference argument and virtual destruction for C++ cleanup.
ComparableTimeMark.hpp mirrors TimeSource.kt:197-243: covariant plus/minus,
different-source difference contract, ordering, nullable Any equality and hash.
The two default bodies in ComparableTimeMark.cpp delegate minus to plus(-duration)
and compare_to to the actual virtual difference compared against Duration.ZERO.
TimeSource.hpp declares the source abstract TimeSource and nested
WithComparableMarks interfaces, retaining covariant mark_now returns. Returned
marks follow the existing caller-owned delete/adopt contract. Private algorithms
remain in .cpp; the actual implementation is registered in CMake.

The equality argument forward-declares the existing real kotlin::Any contract;
ordinary C++ marks do not inherit the compiler-owned Any representation. The
generated ValueTimeMark equality/hash/text and its actual Kotlin object boundary
still require translation. No different-source diagnostic or equality result is
fabricated as a production implementation. These are upstream abstract methods,
not placeholder concrete bodies. TimeSource.Monotonic and its value marks remain
absent; the worker's provisional expression still cannot compile against them.

Strict Clang23.1.2 -Wall/-Wextra/-Wpedantic/-Werror and O1 ASan/UBSan compilation
and execution of test_comparable_time_mark exit0, with assertions enabled. The
test uses a controllable source/mark only as test infrastructure. It checks
covariant mark creation through TimeSource, duration subtraction through the
comparable base, ordering through Comparable, and propagation of an actual
different-source exception. Its test-only equality/hash implementations are not
production parity evidence. Neither worker scheduling nor Native interop executes.

Both relevant deep scans exit0. Current time root:14/44 bodies,4/13 types,0.50
body similarity,zero scoring failures. TimeSource pair:7/18 bodies,4/7 types,0.36;
Monotonic, ValueTimeMark and reading type remain missing. Existing AdjustedTimeMark
and primitive-extension body/symbol scope discrepancies remain. The Native Kotlin
runtime reference root against src/kotlin reports0/1234 bodies,1/258 types,zero
scoring failures. Comparable itself has1/1 types and0/0 bodies because it is an
abstract interface; its1.00 body score cannot establish runtime translation.
Receipts are build/source-continuation/time-source-distance/ and
native-ordering-distance/. Source inventories here are bounded working-tree
references, not the complete Kotlin repository or complete compiler implementation.

Continue actual Monotonic/ValueTimeMark operations, the low-level Native timing
source, the generated value-class object boundary and Worker/Future. Full library,
compiler lowering and both standalone/Native MLX paths remain unfinished.

## TimeMark default operations — 2026-10-08

Continuation from `3ac53464` translates TimeSource.kt:128-195,246-250 into
src/kotlin/time/TimeMark.hpp:14 and TimeMark.cpp:10,31-42. The interface exposes
elapsed_now and the four source defaults. The private AdjustedTimeMark subtracts
its adjustment and combines further adjustments against the original mark,
rather than wrapping the previous adjustment. The actual source is registered
in the library; there is no production replacement clock.

NOTE(port) ownership projection: plus/minus return caller-owned heap marks,
which callers must delete or adopt in smart pointers. Raw pointer return types
permit the upstream covariant overrides in the remaining comparable/value-mark
translation. An adjusted mark retains an existing shared owner of its original
receiver. Raw/stack receivers remain borrowed and must outlive their adjustments;
the no-op shared alias does not acquire ownership. Virtual destruction supplies
the C++ cleanup contract. No existing receiver API or ownership was changed.

Strict Clang23.1.2 -Wall/-Wextra/-Wpedantic/-Werror compilation and O1 ASan/UBSan
execution of the registered test_time_mark exit0, with assertions retained in
Release. The controllable test mark is test infrastructure, not a production
time source. Checks cover repeated live elapsed queries, positive/negative/zero
passed predicates, composed plus/minus adjustments, original shared-owner
retention and final destruction, borrowed receiver non-destruction, infinite
adjustments and opposite-infinity rejection. No worker execution is established.

The refreshed time-root deep scan exits0 and reports12/44 bodies,1/13 types,
0.46 body similarity and zero scoring failures. TimeSource/TimeMark pairing is
5/18 bodies,1/7 types,0.27 similarity. AdjustedTimeMark::plus is PRESENT in the
symbol inventory but missing in body matching. Conversely the Kotlin base
TimeMark functions appear unscoped in the symbol inventory and are listed missing
against the scoped C++ definitions; the actual definitions are at .cpp:31-42.
These scope inconsistencies do not certify completion. Prior arithmetic counts
below remain the preceding checkpoint, not the current whole-time score.

TimeSource, WithComparableMarks, ComparableTimeMark, ValueTimeMark and Native
MonotonicTimeSource still require translation. WorkerDispatcher's provisional
mark_now/operator+ expression still awaits that actual value API. Worker/Future,
runtime dependencies, complete compiler lowering and both MLX paths remain open.

## Saturated time arithmetic dependency — 2026-10-08

Continuation from `39965bab` translates all eight source functions in
libraries/stdlib/src/kotlin/time/longSaturatedMath.kt:12-80 into
src/kotlin/time/LongSaturatedMath.hpp:11 and LongSaturatedMath.cpp:23-84.
The header holds the four externally consumed declarations; private arithmetic
helpers remain in the implementation. This is the actual dependency of Native
MonotonicTimeSource's elapsedFrom, differenceBetween and adjustReading operations.
It does not supply TimeSource, TimeMark, Worker or Future yet.

Addition preserves saturated readings, rejects opposite infinities, splits an
oversized duration into halves and saturates finite integer overflow. Differences
preserve the source distinction between elapsed time from a saturated origin and
equal saturated origins. Overflowing submillisecond differences are reconstructed
from millisecond quotients and remainders. Two NOTE(port) helpers use unsigned bits
to preserve Kotlin Long wrapping without C++ signed-overflow undefined behavior.
The unit conversion uses the existing Duration conversion and negative infinity
uses its actual unary negation. No replacement clock or state machine is added.

src/CMakeLists.txt registers the actual source in the library. The new registered
test_long_saturated_math retains assertions in Release. Strict Clang23.1.2
compilation with -Wall/-Wextra/-Wpedantic/-Werror and ASan/UBSan passes at O0 and O1;
both executables exit0. Cases cover endpoint saturation, opposite infinities,
half-duration addition yielding a finite result, origin equality, signed integer
overflow and nanosecond/microsecond/millisecond/second/day units. This verifies
the arithmetic dependency, not worker scheduling or complete coroutine execution.

Three omitted sparse-checkout references were materialized byte-for-byte from
the pinned tmp/kotlin fee29910 Git blobs: longSaturatedMath.kt, TimeSource.kt and
TimeSources.kt. Their bytes were compared with git show HEAD; upstream sources
were not edited. The broader time-root scan consequently covers only those three
materialized files; Duration.kt is not present in that reference root.

All three required relevant deep scans finish with exit0. The time root reports
7/44 bodies,0/13 types,0.64 body similarity and zero scoring failures. The selected
longSaturatedMath pair reports7/8 bodies. Its deep symbol inventory nevertheless
records isSaturated PRESENT at LongSaturatedMath.cpp:84; body pairing lists
Long::isSaturated missing because the primitive extension receiver became a free
function. The generated evidence also contains unsupported Kotlin infix/if
emission. These limitations remain visible; neither score is whole-time parity.
Receipts are in build/source-continuation/time-source-distance/.

The refreshed compiler-reference scan is286/7207 bodies,132/1630 types,0.28
similarity,10 scoring failures. The frontend scan is14/7207 bodies,6/1630 types,
0.04 similarity,zero scoring failures. The source inventory grows from695 to698
files because the three real references were restored, without new compiler
matches. Receipts are in compiler-source-distance/ and frontend-source-distance/
under build/source-continuation/. Full library/compiler transliteration, actual
worker runtime closure and both complete docking-ring/MLX paths remain unfinished.

Continuation from `c5c168f3` translates the selected WorkerDispatcher class from
tmp/kotlinx.coroutines/kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:20-76.
The former native/MultithreadedDispatchers.cpp contained only an empty namespace
and a claim that the common thread pool supplied the Native implementation.
That thread pool does not implement the upstream WorkerDispatcher algorithm.

## Source changes

- native/MultithreadedDispatchers.hpp:13 declares the source class, with its
  overrides and an opaque concrete private field. Implementation and helpers
  live in native/MultithreadedDispatchers.cpp.
- The implementation creates the actual Worker dependency, queues dispatch with
  execute_after(0), schedules resume_undispatched, registers disposable cancellation,
  and joins request_termination().result() in close.
- DisposableBlock at .cpp:16 holds the original runnable in an atomic shared
  handle. Disposal clears that holder; queued operations retain only the shell.
  run_after_delay at :34 preserves the source monotonic target,100ms quantum,
  disposal check, microsecond conversion and nonnegative final wait.
- schedule at :108 uses the source Monotonic ValueTimeMark through the TimeMark
  interface. Duration's existing compare_to and to_long(MICROSECONDS) express the
  source operator comparison and inWholeMicroseconds; no chrono timer replacement
  is introduced.
- native/CoroutineContext.cpp:40 now constructs WorkerDispatcher("DefaultExecutor")
  and delegates dispatch, delay, timeout and enqueue exactly as CoroutineContext.kt:6-24.
  Detached threads, sleep_for, non-source synchronous timeout execution and
  immediate resume of unrecognized continuation implementations are removed.
- Delay.hpp no longer includes private CancellableContinuationImpl or Intrinsics
  definitions that its declaration-only surface does not use. Delay.cpp retains
  its explicit implementation includes.

## Ownership adaptation and limits

bind_continuation at .cpp:53 preserves an existing CancellableContinuationImpl
shared owner when available; other raw interface receivers remain borrowed.
The scheduled once-only resume operation moves that lease into its invocation
before resuming, so the completed callback cannot retain a shared-owner cycle
through its cancellation handle. Cancellation retains and disposes the actual
handle. These C++ adaptations are marked NOTE(port); no borrowed receiver is made
owned and no missing-owner receiver is resumed early. These lifetime paths have
not executed because the production dependencies below are absent.

Worker/Future contracts were read from the actual pinned tmp/kotlin Native runtime
sources. TimeSource.kt was read with git show at the pinned fee29910 checkout,
because that tracked source is omitted from the sparse working tree. This does
not add a time-source implementation or satisfy the missing dependency.

## Verification and remaining translation

Strict Clang23.1.2 syntax checking with -Wall/-Wextra/-Wpedantic/-Werror passes
the actual new dispatcher header and rewritten native/CoroutineContext.cpp.
The actual WorkerDispatcher implementation fails before dependency/body closure:
kotlin/native/concurrent/Worker.hpp is missing, and the included existing
CancellableContinuationImpl.hpp reports unused taken_state and cause parameters.
TimeSource/TimeMark and Future are also absent from production C++ sources.
No warnings are suppressed, no replacement dependencies are provided, and no
fresh dispatcher runtime, retained fixture or complete Native/MLX result is claimed.

Continue translating Worker.kt, Future.kt and their actual runtime consumers,
then the pinned monotonic time-source dependencies. Preserve standalone C++
independence and the explicit actual Native boundary. The larger MultiWorkerDispatcher
at MultithreadedDispatchers.kt:78-188 and the Native fixed-pool factory still need
translation; the existing common ExecutorCoroutineDispatcherImpl is not their source
implementation. The selected class body is a source draft, not a completed file port.

The required library-root deep scan measures670/2918 function bodies,181/560 types,
0.24 body similarity and11 scoring failures. Native MultithreadedDispatchers is
5/19 bodies,2/3 types at0.09 similarity. Its local DisposableBlock methods and
runAfterDelay remain listed as missing because the C++ private helper scope differs;
the actual bodies exist but this pairing does not certify their parameter/algorithm
parity. Native CoroutineContext remains paired to UndispatchedCoroutine at9/10
bodies,2/2 types,0.39; that pairing does not independently certify this DefaultExecutor
body. Keep both measurement limitations visible and do not infer completion from counts.
