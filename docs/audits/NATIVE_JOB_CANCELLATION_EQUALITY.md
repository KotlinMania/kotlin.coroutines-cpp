# Native JobCancellationException equality repair

Current source locations: native/Exceptions.cpp:87,92,95 hold the constructor, borrowed Job getter and equality body; the private cause adapter is at :68. Inherited cancellation identity methods now reside in src/kotlin/coroutines/cancellation/CancellationException.cpp:46,52 under the actual stdlib namespace. [NATIVE_CANCELLATION_NAMESPACE.md](NATIVE_CANCELLATION_NAMESPACE.md) records current verification and the complete-build failure; the receipts and locations below are historical.

Date: 2026-10-07. Native Exceptions remains fourth in the dependency-impact
priority report, with six dependents. Read the complete common and Native
coroutine Exceptions sources and the consumed Native Any, Throwable and stdlib
CancellationException sources before changing their C++ counterpart.

The common header now declares the expected JobCancellationException at
`src/kotlinx/coroutines/Exceptions.hpp:100` and selects the matching Native header.
`native/Exceptions.hpp:18` defines the actual final class. Its constructor and
borrowed Job getter are declarations at :21 and :24, with concrete implementations
at `native/Exceptions.cpp:92` and :97. This removes the old inline common definition
and places the Native source class in its actual file pair.

Equality at `native/Exceptions.cpp:100` follows Native Exceptions.kt:27-29:
self identity, actual class check, message equality, the other Job's equality,
and the other cause's equality. Short-circuit order and receiver direction are
preserved. Shared Job or cause operands still invoke virtual equality; only the
source's explicit exception self check bypasses those operations.

CancellationException inherits Native Any's default identity equality through
`Exceptions.hpp:94` and `native/Exceptions.cpp:63`. The private exception-pointer
adapter at :73 rethrows both retained carriers and invokes CancellationException
virtual equality on the actual thrown receiver. Other C++ exception carriers use
identity, matching source Throwable's inherited default. A virtual equality
exception propagates without being replaced or swallowed. This is a projection
onto the existing std::exception carrier, not a complete Kotlin Throwable/Any
hierarchy. Jobs remain borrowed and retain no additional ownership.

The existing `src/tests/src/suspend/test_channel_consumption.cpp:62` executable
now checks identity, type, message, null/distinct/shared causes, borrowed Jobs,
virtual Job equality, cause receiver direction, shared-operand dispatch,
short-circuiting, nested cancellation causes and exact equality-failure identity.
The focused build completes with exit zero. Nine CTest executables finish with
zero failures: BuildersTest, test_sync, test_suspension_core,
test_continuation_dispatch, test_channel_as_flow_smoke, test_sharing_suspension,
test_collect_reduce_smoke, test_cancellable_start and test_channel_consumption.
Logs are `build/ir-recovery/native-exception-equality-regression-build.log` and
`native-exception-equality-focused-tests.log`.

A direct Clang build of this actual executable and Native implementation with
AddressSanitizer and UndefinedBehaviorSanitizer exits zero without diagnostics;
receipts are `native-exception-equality-sanitizer-build.log` and
`native-exception-equality-sanitizer-tests.log` under the same build directory.
These component checks do not establish the complete standalone/Native MLX paths.

Both complete-root deep scans finish with exit zero. Library measurements are
773/2918 body names, 344/560 types, average body similarity 0.26 and 122 scoring
failures. Native Exceptions now records 2/4 body names and 1/2 types, similarity
0.14. Missing toString and hashCode remain correctly reported. Scan receipts are
`native-exception-equality-library-deep.log` and
`native-exception-equality-compiler-deep.log`; generated inventories and reports
are under `docs/audits/project-wide/library` and `compiler`.

Remaining real gaps include JobCancellationException.toString/hashCode,
cause-only CancellationException construction, and the Native actual typealias
projection. These depend on actual Throwable class text, Kotlin String hashing
and Job/Any hashing. This repair introduces no substitute text/hash algorithm.
Forty ranged provenance references across the three library files resolve to
existing sources with valid bounds. No prohibited source comment markers occur
in those files. These checks validate references, not full source parity.
The private channel diagnostic gap also remains unchanged. No AST measurement
code was changed for this source repair.
