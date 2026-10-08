# Native WorkerDispatcher source repair — 2026-10-08

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
