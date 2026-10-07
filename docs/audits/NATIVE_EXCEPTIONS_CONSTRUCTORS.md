# Native cancellation constructor repair

Date: 2026-10-07. Read the complete Native and common coroutine Exceptions sources,
the stdlib CancellationException constructors, and Native Throwable before changing
the C++ pair. Native Exceptions remains fourth in the dependency priority report,
with six dependents.

`src/kotlinx/coroutines/Exceptions.hpp:63` now derives CancellationException from
the existing IllegalStateException C++ type, matching the declared upstream base.
The preceding committed header failed a direct base-class relationship check.
This repair does not complete the broader Kotlin Throwable/Exception hierarchy.

Constructors at `native/Exceptions.cpp:11,14,18,22,27,32,36` preserve a null message
separately from an empty string and retain the supplied exception_ptr cause.
The getters at :40 and :43 expose those actual fields. The non-null C++ what()
view maps a null message to empty text; get_message() retains source nullability.
Concrete implementations live in the .cpp, with declaration and implementation
provenance. All 28 referenced source ranges across the two files resolve and have
valid line bounds; this check establishes references, not complete body parity.

The source factory is now named `cancellation_exception` in snake_case, replacing
the invented make-prefix. Its overloads at :47,52,57 preserve nullable inputs and
the original cause. The returned allocation remains owned by the caller. The
preceding committed factory explicitly discarded the cause; the repair also
incorporates the previously pending cause-retention work.

The targeted build finishes successfully and eight CTest executables finish with
zero failures: BuildersTest, test_sync, test_suspension_core,
test_continuation_dispatch, test_channel_as_flow_smoke, test_sharing_suspension,
test_collect_reduce_smoke, and test_timeout. The BuildersTest cancellation case
checks null versus empty messages, default construction, cause identity, copied
exceptions caught through IllegalStateException, and resource retention/release
through copies of the cause. That test file also contains previously pending
source and ownership regression cases included in this checkpoint.

Receipts are `build/ir-recovery/native-exceptions-final-build.log` and
`native-exceptions-tests.log`. A separate direct Clang build of the actual header
and Native implementation executes with AddressSanitizer and
UndefinedBehaviorSanitizer and exits zero. Logs are
`native-exceptions-standalone-build.log` and `native-exceptions-standalone-tests.log`.
This bounded component build requires no Kotlin compiler or Native runtime; it
does not establish the complete standalone/Native shared-state-machine MLX paths.

Both full-root deep scans finish with exit code zero. The library report changes
from 780 to 781 of 2918 matched body names; types remain 341/560, average body
similarity 0.26, and scoring failures 122. Native Exceptions changes from 0/4 to
1/4 matched bodies and similarity 0.00 to 0.11. Its two types are still attributed
to the common header group rather than the Native group, an expect/actual
projection limitation. Names present in an inventory do not establish completed
algorithms. Generated evidence remains under `project-wide/library` and
`project-wide/compiler`.

Incomplete source behavior remains: the cause-only constructor needs Native
Throwable.toString(), including the actual qualified class-name contract.
JobCancellationException.toString() and hashCode() remain absent. Equality and
the matching Native class layout were subsequently repaired; see
[NATIVE_JOB_CANCELLATION_EQUALITY.md](NATIVE_JOB_CANCELLATION_EQUALITY.md). Their
dependencies include source Throwable text, structural equality for message/job/
cause, UTF-16 String hashing, and the Job/Any hash contract. These have not been
replaced with C++ what(), std::hash, or an invented pointer registry. The Native
RECOVER_STACK_TRACES=false constant is retained as specified by its source.
