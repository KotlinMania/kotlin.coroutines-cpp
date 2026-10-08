# Native time and worker contracts

## Source and implementation

`src/kotlin/time/` translates common TimeSource/TimeMark contracts and Native
MonotonicTimeSource and longSaturatedMath. `src/kotlin/system/Timing.hpp/.cpp`
contains Native clock getters and inline measurements. WorkerDispatcher is in
`src/kotlinx/coroutines/native/MultithreadedDispatchers.hpp/.cpp`.
Ground truth is the matching common stdlib, Native runtime and coroutine source
under `tmp/kotlin` and `tmp/kotlinx.coroutines`.

## Time arithmetic and ownership

Saturated arithmetic preserves upstream overflow, infinity and unit-conversion
branches. Defined unsigned operations reproduce wrapping Long subtraction.
Monotonic readings are relative to a retained source zero. Elapsed time,
adjustment and mark difference use the corresponding saturated operations.

TimeMark defaults compose adjusted marks. Comparable marks derive ordering from
their source difference. Mark-returning pointers have a caller-owned delete/adopt
policy. An existing shared mark owner can be retained by an adjusted mark;
a borrowed mark remains borrowed and must outlive its use.

ValueTimeMark stores an immutable reading. Its nested public name aliases the same
concrete type so C++ can validate covariant returns. Its hash combines the Long's
upper and lower halves; its text formats the stored reading. These operations do
not supply the generated boxed-value equality contract.

## Worker scheduling

WorkerDispatcher dispatches through the actual Worker execute-after API.
DisposableBlock atomically releases its runnable on disposal, leaving the queued
shell. Delay scheduling checks disposal and the target moment, queues 100 ms
quanta while needed, then queues the remaining nonnegative microsecond delay.
Cancellation disposes the scheduled handle. Close requests worker termination
and waits for its result. DefaultExecutor delegates to WorkerDispatcher.

The scheduled continuation retains an existing shared owner when available.
Other interface receivers remain borrowed. Moving the continuation lease into
its single invocation avoids retaining it through a completed cancellation handle.

## Unfinished source contracts

ValueTimeMark declares `equals(const kotlin::Any*)` without a definition. Its
actual boxed-object/type-check boundary is absent, and the current inheritance
does not provide that Any relationship. The value-mark consumer cannot link.

Actual Worker/Future dependencies, MultiWorkerDispatcher and factory translation
remain incomplete. The scheduling draft does not establish running worker,
timing, cancellation or resource cleanup behavior. Native clock exports and
compiler metadata remain distinct from standalone C++ clock implementations.

These gaps do not make timing tests the next translation task. Library source
priority comes from the current Kanban assignment and complete ast_distance
inventories; runtime acceptance follows the connected implementation's readiness.
