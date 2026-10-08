# Native WorkerDispatcher source repair — 2026-10-08

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
