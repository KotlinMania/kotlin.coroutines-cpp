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
JobCancellationException.toString() remains absent. The hashCode source repair is recorded below. Equality and
the matching Native class layout were subsequently repaired; see
[NATIVE_JOB_CANCELLATION_EQUALITY.md](NATIVE_JOB_CANCELLATION_EQUALITY.md). Their
dependencies include source Throwable text, structural equality for message/job/
cause, UTF-16 String hashing, and the Job/Any hash contract. These have not been
replaced with C++ what(), std::hash, or an invented pointer registry. The Native
RECOVER_STACK_TRACES=false constant is retained as specified by its source.

## Native JobCancellationException hash source repair

The complete Native Exceptions.kt, common coroutine-context sources and their
C++ counterparts were read for this checkpoint. The Native Any and String
sources, consumed identity primitive and polynomial hash kernel were also read.
Native Exceptions is still fourth in the dependency priority report with six
dependents. Its missing hashCode body is now implemented at
`native/Exceptions.cpp:147`, with the surface at `native/Exceptions.hpp:30`.
The actual expression preserves message, Job and nullable-cause operands,
virtual hash dispatch and Kotlin's wrapping Int arithmetic. Operands evaluate
in source order; a failure in Job hashing prevents cause hashing. The borrowed
Job property is const, matching the source val, and is not adopted as an owner.

CancellationException's inherited Any hash at `native/Exceptions.cpp:72` and the
context base at `context_impl.cpp:13` use the actual low-32-bit Native identity
primitive from Natives.cpp:40-49 on C++ object storage. An initial attempt to
reuse the compiler-object Runtime helper failed at link time because that helper
is not part of the standalone core archive. The final bodies directly project
the same source primitive and have no compiler-object or Kotlin runtime link
dependency. No alternate object identity registry is introduced.

Message hashing at :83 converts the existing UTF-8 C++ message representation
to UTF-16 using Native's actual utfcpp dependency, then computes its polynomial
hash over code units. Native object-header hash caching is unnecessary for this
C++ value representation. The three dependency headers under third_party/utfcpp
are copied byte-for-byte from the checked-in Native runtime dependency, including
all copyright/license notices; ORIGIN.txt identifies their source. They require
no installed Kotlin toolchain. Invalid UTF-8 uses the same replacement conversion
as Native's non-validating UTF-8 input boundary. This C++ text representation does
not expose arbitrary unpaired UTF-16 surrogates as independent String values.

Cause hashing at :97 rethrows the actual exception_ptr to invoke an overridden
CancellationException hash. Other std::exception carriers inherit the source
Throwable identity primitive; null contributes zero. Non-std C++ thrown values
are outside the existing Throwable carrier projection and propagate on hashing,
rather than receiving a fabricated hash. Equality and hashing use the same actual
retained exceptions. Nested JobCancellationException causes hash recursively.

Context hashing requires the real source dependencies. EmptyCoroutineContext's
hash at `context_impl.cpp:170` returns source zero. CombinedContext's hash at :65
returns left.hashCode + element.hashCode, wrapping exactly as Kotlin Int. The
existing pending context implementations are captured with this prerequisite:
get/for_each/minus_key, source plus ordering and interceptor placement, structural
CombinedContext equality and source text. Public context headers expose the
surface; CombinedContext remains concrete/private in .cpp. The earlier header
shortcut that represented empty context as nullptr is removed in the committed
state. Existing unrelated work outside these context prerequisites is preserved.
This does not complete polymorphic keys, serialization or the full stdlib port.

`src/tests/src/suspend/test_channel_consumption.cpp:150` adds source hash
regressions: golden ASCII, Unicode and supplementary-code-point hashes, embedded
NUL, long overflowing inputs and invalid UTF-8 replacement; structurally equal
Jobs and causes; recursive causes; exact exceptions from Job/cause overrides;
source operand order; real borrowed exception identity; empty and combined
context hashes, ordering invariance and key removal. The Job hash default uses
the canonical C++ CoroutineContext subobject identity. This preserves C++ object
identity without interpreting it as a Kotlin GC object.

The full core archive and ten focused executables build. Ten CTests finish with
zero failures in 2.55 seconds. Receipts are
build/ir-recovery/native-exception-hash-focused-{build,tests}.log. The regression
fixture, Native Exceptions.cpp and context_impl.cpp are directly instrumented
with AddressSanitizer and UndefinedBehaviorSanitizer, compiled with the existing
LLVM/Clang plugins and linked with the ordinary C++ core archive. Execution exits
zero with no diagnostics; receipts are native-exception-hash-sanitizer-{build,tests}.log.
An ordinary C++ application also compiles without any Clang plugin flags and runs
with the repaired exceptions as std::unordered_set keys. Equal cancellation
values deduplicate, distinct messages remain distinct, lookups succeed, and the
borrowed Job has no added shared owner. otool lists only libSystem and libc++ as
dynamic dependencies. Receipts are native-exception-hash-standalone-{build,tests}.log;
its source is build/ir-recovery/native_exception_hash_standalone.cpp.

Eighty-seven ranged provenance references across six library files resolve;
no prohibited source markers occur there. These checks do not establish the full
Native/MLX product acceptance paths or whole-library translation completion.

Both complete-root deep scans finish with exit zero. The library report now
records 804/2918 matched body names, 358/560 types, average body similarity 0.26
and 123 scoring failures. Native Exceptions is 3/4 body names (was 2/4), with
body similarity 0.15. Its toString body remains absent and its CancellationException
actual typealias remains an expect/actual inventory limitation. The compiler-root
report still lists CoroutineContextImpl as unmatched, despite the inspected
consumed bodies under the library's kotlinx::coroutines namespace; namespace/file
projection requires further investigation. No score is overridden and no source
provenance is changed to conceal this report. Deep receipts are
native-exception-hash-{library,compiler}-deep.log.
