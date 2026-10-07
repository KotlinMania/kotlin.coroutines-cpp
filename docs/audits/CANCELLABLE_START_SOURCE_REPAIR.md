# Already-created cancellable start repair

Date: 2026-10-07. Read complete Cancellable.kt, Undispatched.kt and the consumed
Native IntrinsicsNative.kt and ContinuationImpl.kt before editing. CoroutineStart
is sixth in the dependency priority report; its actual intrinsic dependencies
must be translated before replacing its existing dispatch shortcut.

`intrinsics/Cancellable.cpp:34` implements the already-created continuation entry
from common Cancellable.kt:33-36. It uses the actual intercepted continuation and
resume_cancellable_with. Native intercepted() returns a plain continuation
unchanged; the old C++ dynamic-cast branch silently skipped it. The new regression
compiled against the preceding committed header exits one at its plain-continuation
resume check. Against the edited implementation it exits zero.

`Cancellable.cpp:10` implements dispatcherFailure's type check: a DispatchException
reports its original cause; all other exceptions report themselves. It resumes
fatal completion with that same exception_ptr and then rethrows it. If the fatal
completion itself throws, that failure escapes, following source evaluation
order. run_safely at :25 executes the block once and forwards its failure through
the concrete helper. The preceding raw helper reported the wrapper exception,
while a second .cpp overload duplicated failure handling without the type check.
Both concrete helpers and the already-created entry now live in .cpp, with
header declarations at `Cancellable.hpp:101,108,169` and source provenance.

The already-created entry retains an existing frame owner without adopting a
borrowed plain continuation. On fatal start failure it releases the actual
frame's cached interception before reporting failure. This is an explicit C++
ownership adaptation to the interception cycles collected by Native GC; it does
not introduce a substitute frame or coroutine state machine. Resource and frame
identity remain unchanged while waiting in the dispatcher. Completion, queued
cancellation, dispatcher-query failure and dispatch failure release those owners.

The library and nine targeted executables build. Nine CTest executables finish
with zero failures: test_cancellable_start, test_channel_consumption,
test_channel_as_flow_smoke, BuildersTest, test_sync, test_suspension_core,
test_continuation_dispatch, test_sharing_suspension and test_collect_reduce_smoke.
The new test exercises plain borrowed continuations, ordinary and wrapped failure
identity, fatal completion failure precedence, queued and immediate execution,
cancel-before-start, dispatcher query/dispatch failures, and real frame/resource
cleanup. Receipts are `build/ir-recovery/cancellable-start-build.log` and
`cancellable-start-tests.log`. The baseline reproduction is recorded in
`cancellable-start-before-build.log` and `cancellable-start-before-tests.log`.

The same test and actual Cancellable.cpp compile directly with Clang and the core
C++ archive under AddressSanitizer and UndefinedBehaviorSanitizer and exit zero,
with no diagnostics. That direct component build invokes no Kotlin compiler and
links no Native runtime library. This does not establish the complete standalone
or Native shared-state-machine MLX application paths. Receipts are
`cancellable-start-sanitizer-build.log` and `cancellable-start-sanitizer-tests.log`.
All ten source provenance ranges across the two edited files resolve to existing
source files with valid bounds. Two old Atomic-start provenance paths were
corrected to the consumed stdlib Continuation.kt; valid provenance does not prove
their bodies are fully translated.

Both full-root deep reports are regenerated. Library totals remain 781/2918
matched body names, 341/560 types, average body similarity 0.26 and 122 scoring
failures. Cancellable remains 5/5 matched names; similarity is 0.14, previously
0.16. These scores and name presence do not establish complete Cancellable parity.

Cold callable start remains demonstrably incomplete. An explicit instantiation
of start_coroutine_cancellable<void*> fails because Continuation has no
shared_from_this member (`cancellable-start-cold-probe.log`, header :126). The
LambdaContinuation/ReceiverLambdaContinuation bodies also construct an abstract
Continuation<void>, bypass the source wrapper frame as their block argument, and
lack Native's label-0/1/completed callable-wrapper algorithm. Native sources
IntrinsicsNative.kt:142-152,177-189,221-263,296-324 contain the needed contracts.
The same mixed header's Atomic/Undispatched callable entries and CoroutineStart's
custom dispatch body remain incomplete. They were not counted as repaired by
the already-created-entry tests or by a successful uninstantiated library build.
