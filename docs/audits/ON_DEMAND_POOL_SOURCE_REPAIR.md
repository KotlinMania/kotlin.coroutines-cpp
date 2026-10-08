# Current warning-visible pool build

Complete OnDemandAllocatingPool.kt and both C++ files reread. The private unused
loop duplicate in .cpp was removed. Its source inline nonlocal-return call sites
already expand to the actual while loops in allocate/close; those algorithms are
unchanged. No suppression annotation or dummy call is used. Concrete control-bit
and formatting bodies remain in .cpp at lines 17,22,31.

The existing test_concurrent_native.cpp was freshly compiled with Concurrent.cpp
and OnDemandAllocatingPool.cpp using -Wall -Wextra -Wpedantic -Werror, then executed.
Build and execution both return zero. It exercises actual set/reference/lock
operations, pool reservation/publication races, resource identity and cleanup,
concurrent closing and the source creation-failure behavior. Receipts under
build/ir-recovery: concurrent-warning-visible-test-{build,run}.log. This bounded
component executable does not establish current whole-core compilation, the
changed Flow suspension paths or the Native/MLX acceptance scenarios.

The refreshed oracle reports 6/7 source function names with private loop missing,
1/1 types and body similarity 0.08. No duplicate helper is restored merely to
recover name coverage; the inlined call-site loops are the executable source
projection. This is not whole-file syntactic parity. Both full-root scans exit
zero; see FLOW_CHANNELS_SOURCE_REPAIR.md for current complete-root totals.

Historical source and build checkpoints follow.

# On-demand allocating pool source repair

Ground truth: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/concurrent/src/internal/OnDemandAllocatingPool.kt`, read in full before changing its counterpart. Native MultithreadedDispatchers.kt consumes this pool for on-demand Worker creation; that file was also read in full.

The preceding pool template existed only inside concurrent/internal/OnDemandAllocatingPool.cpp. No header or explicit instantiations made it available to consumers. Raw heap slot boxes were exchanged and deleted by close while diagnostic readers could still retain their addresses; destruction without close left owned boxes allocated. Closed diagnostics spelled empty slots as nullptr, unlike Kotlin null.

The instantiable generic binding now lives at `src/kotlinx/coroutines/concurrent/internal/OnDemandAllocatingPool.hpp:39,49,67,86,101,107`. It follows source allocation capacity checks, reservation CAS before creation, atomic publication, closed-bit reservation, ordered spin-until-publication extraction and repeated-close behavior. Atomic retained immutable value boxes supply C++ ownership for Kotlin's nullable reference slots. A shared-owner T keeps the original resource identity; a raw-pointer T stays borrowed. Pool destruction releases its owned slot storage without inventing a close call. Atomic operations use sequential consistency, matching atomicfu's contract.

Concrete control-bit operations, list assembly and the private source loop remain in `OnDemandAllocatingPool.cpp:17,25,30,39`. C++ unsigned control storage preserves Kotlin Int high-bit operations without signed-shift overflow. Source inline nonlocal returns expand to the allocation and close loops; the private helper's own while-true body is also retained. Its inline/unused annotations preserve that source body in a compilation unit whose call sites are already expanded.

The initially introduced detail helper namespace conflicted with the companion's actual Kotlin package. The strict ast_distance rule correctly rejected that layout. Both companions now use kotlinx::coroutines::internal. No matching/scoring rule was weakened, no alias was added and no analyzer code was changed.

The source's creation-exception KDoc (:46) claims no effect, but its actual body (:52-53) increments the reservation before calling create. A thrown creation leaves an unpublished slot; close subsequently spins waiting for it. The regression preserves the executable source body and original exception identity, confirms the retained reservation and avoids calling that unbounded close. No rollback algorithm absent from the pinned source is introduced, and the Kotlin source is not edited to change the oracle. This upstream contradiction remains a concrete source issue, not a completed exception-safety claim.

`src/tests/src/test_concurrent_native.cpp:27` verifies capacity, allocation-index order, repeated close, null diagnostics and publication/closure races. A promise blocks creation after reservation; observing allocate return false establishes that the concurrent closer has set the closed bit before publication. Closing then returns the original resource. Concurrent allocators and two closers collectively extract each slot exactly once. Extracted owners and unclosed slot storage release their resources at the verified endpoints.

The build-only preceding implementation at build/ir-recovery/pool-before reproduces [nullptr][closed] and exits one against the source-derived [null][closed] expectation. Receipts: pool-before-{build,tests}.log. The fixture includes that preceding implementation only in the isolated control; production consumers include the real header.

This is a pool prerequisite repair. The Native Worker dependency, WorkerDispatcher/MultiWorkerDispatcher integration, replacement of the existing substitute fixed pool/DefaultExecutor timers and full generic Any.toString projection remain incomplete. Numeric/string diagnostics and owned resources exercised here do not establish arbitrary Kotlin object formatting. No complete Native dispatcher or standalone/Native MLX acceptance claim is made.

Final verification (2026-10-07): the core plus test_concurrent_native, test_continuation_dispatch, test_select_arguments and test_sync build with the existing Clang frontend and mandatory LLVM plugin. All four CTests finish with zero failures (0.75 seconds). The final Native concurrent/pool fixture compiles and executes with AddressSanitizer/UndefinedBehaviorSanitizer and detect_stack_use_after_return=1, with no diagnostics. Sixteen ranged references across both pool files resolve to valid Kotlin bounds, and neither file contains prohibited source markers. Receipts under build/ir-recovery: pool-final-build.log, pool-tests.log, pool-sanitizer-{build,tests}.log and pool-provenance.log.

Both final full-root ast_distance --deep scans exit zero after all source edits. OnDemandAllocatingPool records 7/7 matched functions, 1/1 matched types and body similarity 0.17. The library totals remain 812/2918 functions, 359/560 types, average body similarity 0.26 and 123 scoring failures. Compiler symbol/body totals are unchanged; target file count includes the new header. Generated reports are committed verbatim. Receipts: pool-{library,compiler}-deep.log. These measurements and focused tests do not establish whole-library completion.

The user required committing before further changes during this repair. Source work was checkpointed in b7c2ef6b, refreshed evidence in 5152b966, restoration of the private loop in 2817f993 and the subsequent compiler annotation repair in 40a8174d. The final evidence/audit commit follows those checkpoints. This commit-before-change requirement applies to subsequent work.
