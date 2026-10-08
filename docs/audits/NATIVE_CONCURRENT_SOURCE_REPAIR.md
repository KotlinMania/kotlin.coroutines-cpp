# Warning-visible Native argument binding

The complete Native and common Concurrent sources were reread. Native
Concurrent.kt:11 explicitly ignores expectedSize. Concurrent.hpp:32 preserves
its int argument and empty set construction, omitting only the unused C++ local
binding. This does not add reserve behavior, an invented read or a suppression
attribute. The concrete Concurrent.cpp compiles with -Wall -Wextra -Wpedantic
-Werror, exit zero. Receipt: build/ir-recovery/concurrent-warning-visible-build.log.
A fresh standalone build of the existing test_concurrent_native.cpp together
with Concurrent.cpp and OnDemandAllocatingPool.cpp returns one on the latter's
unused private loop function. The test executable was not produced or run.
Receipt: concurrent-warning-visible-test-build.log in the same directory.
Existing historical full-core results below predate warning suppression removal.

# Native Concurrent source repair

Date: 2026-10-07. Source authority is the complete Native `internal/Concurrent.kt`
and common `internal/Concurrent.common.kt`, read before changing their C++ pair.
The dependency report ranks this group third, with 14 dependents.

`internal/Concurrent.hpp:32` now constructs the empty set exactly as the Native
`HashSet()` expression does. The required expected-size parameter remains, with
no invented default or reserve. The preceding implementation failed the new
contract check on `identity_set(-1)` with `__next_prime overflow`. The Native
source ignores that input, including large positive hints.

`Concurrent.hpp:22` preserves the lock action's deduced return type and accepts
move-only captures and results. The previous `std::function<void()>` conversion
could discard a lambda's result. The concrete void overload is declared at :28
and implemented in `Concurrent.cpp:10`; both implementations release the
recursive lock through exception unwinding.

`Concurrent.hpp:43-73` retains the actual reference wrapper's constructor, field
and four operations, with snake_case names and per-function provenance. Its
load, store, exchange and strong CAS use sequential consistency. This matches
Native `kotlin/concurrent/Atomics.kt`'s AtomicReference and field-intrinsic
contracts, including reference identity and absence of spurious CAS failure.
The preexisting borrowed C++ pointer representation remains borrowed.
`get_value`, `set_value` and `loop` at :77,83,89 mirror common source :31-40.
The loop reloads the field before each action and propagates action failure.

The complete library target and eight focused test executables build. The eight
CTest executables finish with zero failures: `test_concurrent_native`,
`BuildersTest`, `test_sync`, `test_suspension_core`, `test_continuation_dispatch`,
`test_channel_as_flow_smoke`, `test_sharing_suspension` and
`test_collect_reduce_smoke`. The final edited contract test was rebuilt after
the callable-return repair. Receipts are `build/ir-recovery/concurrent-native-build.log`,
`concurrent-native-final-build.log` and `concurrent-native-tests.log`.

A direct Clang C++20 build of the real test and Concurrent.cpp also executes
with AddressSanitizer and UndefinedBehaviorSanitizer and exits zero. That bounded
standalone component build invokes no Kotlin compiler or runtime. Clang's emitted
LLVM in `concurrent-native-order.ll` records `seq_cst` for load/store/exchange
and both CAS orderings; the CAS is strong. These component checks do not establish
the complete standalone/Native coroutine and MLX application paths.

Both full-root deep reports are regenerated. Native Concurrent remains 6/6
matched body names, 2/3 types, with body similarity increasing from 0.31 to 0.36.
`BenignDataRace` remains missing. Its actual Native source alias points to
`kotlin.concurrent.Volatile`; C++ field-annotation translation of that dependency
is absent. The previous comment describing it as a C++ no-op was inaccurate and
is removed. No completed annotation implementation is claimed.

The common expect group separately reports missing declarations despite the
Native actual class being present at `Concurrent.hpp:43`. Its common C++ unit
includes the Native header; the inventory does not expand that include or unify
expect/actual source sets. This measurement limitation remains visible. It does
not close the missing Volatile dependency or remaining source/body criteria.

The full goal remains incomplete: library body-name coverage is 780/2918,
types 341/560, average body similarity 0.26 and 122 scoring failures. The source
repair order and complete project requirements remain in effect.
