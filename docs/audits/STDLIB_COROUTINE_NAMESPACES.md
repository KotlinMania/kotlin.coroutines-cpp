# Consumed coroutine standard-library namespaces

Date: 2026-10-07. Full-tree checkpoint e42543fc preserves the dirty library,
compiler and test work before this mechanical namespace repair. It is a recovery
commit, not a claim that the captured translation is complete.

The corresponding Kotlin Context, ContextImpl, Continuation, Interceptor,
Native ContinuationImpl and Native Intrinsics sources were read before editing.
Intrinsics.kt was missing from the extracted working snapshot; its exact blob was
restored from the existing pinned upstream checkout's HEAD. No revision was
fetched or source contract invented.

| Kotlin package and source | Actual C++ definitions |
|---|---|
| kotlin.coroutines, CoroutineContext.kt | CoroutineContext.hpp:11 |
| kotlin.coroutines, CoroutineContextImpl.kt | context_impl.hpp:9; context_impl.cpp:10 |
| kotlin.coroutines, Continuation.kt | Continuation.hpp:19 |
| kotlin.coroutines, ContinuationInterceptor.kt | ContinuationInterceptor.hpp:9; ContinuationInterceptor.cpp:8 |
| kotlin.coroutines.native.internal, Native ContinuationImpl.kt | ContinuationImpl.hpp:22; ContinuationImpl.cpp:18 |
| kotlin.coroutines.intrinsics, Intrinsics.kt and Native IntrinsicsNative.kt | intrinsics/Intrinsics.hpp:16; intrinsics/IntrinsicsNative.hpp:9; intrinsics/IntrinsicsNative.cpp:6 |
| kotlin.coroutines.native.internal, Native DebugProbes.kt | DebugProbes.hpp:6; ContinuationImpl.cpp:10 |

AbstractCoroutineContextElement now resides in the ContextImpl header, matching
its Kotlin source. Existing concrete algorithms and object layouts are retained.
The C++ owning/borrowed entry ABI helpers are in ContinuationBindings.hpp/.cpp,
separate from Native continuation class definitions. intercepted is in the Native
intrinsics implementation, where its actual Kotlin function is defined.

CoroutineImports.hpp, ContextImports.hpp and ContinuationImports.hpp express
library imports with C++ using declarations. They import the same actual types
and functions; they supply no substitute implementation, class alias, fallback
or alternate state machine. Explicit imports avoid transitive C++ wildcard
ambiguity between standard-library and coroutine-library intrinsics. Coroutine
library Cancellable/Undispatched intrinsics retain kotlinx::coroutines::intrinsics.
Existing Result/Unit C++ carriers remain explicit consumed dependencies; their
broader source translation is not completed by this namespace repair.

Exact qualified consumer references, the frontend continuation-type predicate and
its emitted frame/completion/intrinsic names now resolve the actual standard
namespaces. No global replacement of kotlinx with kotlin was performed. Job and
ConcurrentLinkedList forward declarations no longer invent library-local standard
types; the common probe helper also imports the actual Continuation type.

The core archive and ten focused executables build. All ten CTests finish with
zero failures (5.47 seconds), including typed suspension, dispatcher return,
context keys, channel cleanup, SafeCollector ancestry and owner lifetimes.
Receipts: build/ir-recovery/stdlib-namespace-focused-{build,tests}.log.

An ordinary C++ application compiles without plugin flags, Kotlin build tools or
Native runtime linkage. It statically checks imported/actual type identity and
Native frame inheritance, then executes dispatcher lookup/removal and typed key
lookup with the actual kotlin::coroutines names. Execution exits zero. Dynamic
dependencies are libc++ and libSystem. Source/receipts:
build/ir-recovery/stdlib_namespace_standalone.cpp and
stdlib-namespace-standalone-{build,tests}.log.

Both full-root ast_distance --deep scans are regenerated after final source
changes. The strict compiler inventory now recognizes CoroutineContextImpl
(15/23 bodies, 4/5 types), ContinuationInterceptor (3/3 bodies, 1/1 type),
Continuation (3/9 bodies, 1/1 type), Native ContinuationImpl (8/9 bodies, 4/4
types) and Native Intrinsics (10/18 bodies). Their remaining bodies, serialization
and metadata mismatches remain visible. No tool rule or scoring threshold was
changed to equate distinct namespaces.

The library measurement remains 811/2918 body names, 359/560 types, average body
similarity 0.26 and 123 scoring failures. These counts do not establish faithful
whole-body translation. The repair closes the identified consumed namespace gap;
it does not establish complete source parity or the required real Native/MLX GPU
shared-state-machine acceptance paths. Deep receipts:
stdlib-namespace-{library,compiler}-deep.log.

Two frontend analyzer/collector CTests finish with zero failures (1.10 seconds).
The broader kxs_plugin_handoff CTest fails after 105.25 seconds while compiling
the retained-locals fixture: libc++ variant __make_dispatch cannot deduce its
auto return during Result<void*> copy assignment in the generated-frame parse.
Earlier direct, expression and restricted-receiver fixtures execute, but the
full suite is incomplete. This is not represented as a successful lowering
acceptance path. Receipt: stdlib-namespace-plugin-tests.log. No Result algorithm
or compiler fallback was substituted to suppress this diagnostic.

All 229 ranged provenance references in changed/new source files resolve to
existing Kotlin files with valid line bounds. Receipt:
stdlib-namespace-provenance.log. This validates references, not complete source
behavior.

The same optional<Result<void*>> copy assignment compiles directly with the
frontend plugin enabled; receipt stdlib-namespace-result-copy-build.log and
fixture stdlib_namespace_result_copy.cpp. This bounds the observed failure to
the retained-locals generated-frame compilation, without claiming that its
cause is fixed or that the namespace migration proves complete lowering.
