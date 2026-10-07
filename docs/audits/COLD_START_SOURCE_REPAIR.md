# Cold callable start source repair

Receipt for commit `3e5c6158`. The generic CoroutineStart shortcut described below
is subsequently replaced in [the strategy audit](COROUTINE_START_SOURCE_REPAIR.md).

Date: 2026-10-07. Complete Cancellable.kt, Undispatched.kt and the consumed
Native IntrinsicsNative.kt, ContinuationImpl.kt and DebugProbes.kt were read.
This closes a consumed dependency of the high-fanout CoroutineStart priority.

`intrinsics/IntrinsicsNative.cpp:9,41` translates the two anonymous created
continuations from Native IntrinsicsNative.kt:228-262. The context identity chooses
RestrictedContinuationImpl or ContinuationImpl. Label zero advances to one before
checking the initial result and invoking the body with the actual wrapper; label
one advances to two and returns the resumed result; further invocation reports
completion error. Creation at :192 wraps the callable with the actual Native
start-unintercepted path. Prototype overloads at :203,208 invoke the real generated
frame's create methods instead of fabricating a callable frame.

The two simple wrappers at :73,123 return result.get_or_throw, following Native
IntrinsicsNative.kt:315-324. The callable receives one of these actual frames.
C++ ownership additions retain its callable, typed ABI argument and suspended
frame until termination. Each invocation owns its own typed argument, including
when the same callable starts multiple independent computations. An unstarted
wrapper has no self-root; inline completion before a suspended return does not
reroot the completed frame. Termination releases roots and interception. This
translates the Native objects and adds explicit ownership where Native GC keeps
them alive; it does not substitute another coroutine algorithm.

`Cancellable.cpp:52` uses source run_safely around creation, interception and
resume_cancellable_with. The already-created entry at :47 uses the same concrete
resume helper. Dispatcher failure unwraps DispatchException.cause, reports it,
and rethrows it. `Cancellable.cpp:58` implements stdlib Atomic start: create,
intercept, resume Unit, propagating start-machinery failures without an extra
completion delivery. `Undispatched.cpp:8` probes, invokes Native context helper,
starts unintercepted, handles body failure, then delivers immediate success outside
the body catch. Returned value identity is retained. A throwing completion is
not redelivered by the Undispatched body catch.

`Cancellable.hpp:23` exposes the concrete entries and typed/raw/receiver adapters.
Raw completion pointers remain borrowed. Shared overloads retain explicitly owned
completions; only real BaseContinuationImpl owners are recovered from raw frame
pointers. ResultBoxCompletion in Continuation.hpp maps C++ void to erased Unit.
The receiving typed adapter owns unboxing and deletion. BaseContinuationImpl's
source create methods now return erased Continuation<void*> instead of the
incompatible Continuation<void>; their source unsupported-operation errors live
in ContinuationImpl.cpp. The preexisting continuation source-loop/projection
changes required by these actual wrappers are included with this repair.

The expanded test_cancellable_start verifies cold creation, context class
selection, source label transitions, failed initial result, generated prototype
creation, borrowed stack completion, receiver and immediate typed values,
Default cancellation versus Atomic/Undispatched body entry, dispatcher cause
identity, resumed typed success/failure, owned completion/capture/frame cleanup,
inline resume, and independent concurrent invocations resumed in reverse order.
The old template-instantiation reproduction now compiles with zero diagnostics.
All 61 provenance ranges in the five intrinsic files resolve to source files
with valid bounds. No prohibited markers occur in the eight affected library
files. Those checks do not establish whole-file semantic parity.

At that commit, CoroutineStart's generic C++ invoke still had a custom dispatcher
body; Native callback-continuation helper and
receiver-plus-parameter surface are not covered by this repair. Kotlin compiler
fallback lowering, complete stdlib exception hierarchy, move-only typed Result
support, and the required complete standalone/Native shared-state-machine MLX
paths remain incomplete. The source-first goal remains active.

Verification receipts: `build/ir-recovery/cold-start-final-build.log` records the
library and nine focused target builds. `cold-start-final-tests.log` records nine
executables with zero failures in 2.27 seconds. `cold-start-sanitizer-build.log`
and `cold-start-sanitizer-tests.log` record direct Clang AddressSanitizer and
UndefinedBehaviorSanitizer compilation/execution of the actual intrinsic and
continuation sources with the core archive: exit zero, no diagnostics. The direct
component command invokes no Kotlin compiler or linked Native runtime.
`cold-start-instantiation-probe.log` is empty after the previously failing probe
compiles with exit zero. These receipts establish focused paths, not complete
Native ABI compatibility or either required complete GPU application path.

Both full-root deep scans exit zero and refresh the inventories, criteria and
priorities. Library totals are 782/2918 matched function names, 341/560 types,
0.26 average body similarity and 122 scoring failures. Cancellable has 5/5 names
with 0.15 similarity; Undispatched has 1/6 with 0.02. The compiler report still
lists Native IntrinsicsNative as missing at its Kotlin package identity while the
new dependency lives under the established kotlinx C++ namespace. This namespace
projection discrepancy remains visible; the source implementation and regression
receipts do not justify silently counting all 18 Native functions as translated.
