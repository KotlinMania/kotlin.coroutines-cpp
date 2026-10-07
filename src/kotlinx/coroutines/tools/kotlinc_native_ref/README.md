# Kotlin/Native IR Reference (for C++ DSL plugin)

This directory contains compiled coroutine LLVM helpers translated from the
Kotlin/Native compiler snapshot under `tmp/kotlin/`, plus a corpus-generation tool.

The runtime target is Native machine-code execution with real MLX/GPU interoperability.
Bare metal does not introduce a separate freestanding or no-OS target. Native runtime and actual stdlib
sources define its behavior. Shared compiler Java sources describe compiler classes;
executing those classes on a JVM is not target-runtime acceptance. The compiler may
run on the development host; its output must preserve Native runtime/frame contracts.

Current consumed collection conversion: shared copy loops and Native allocation/
forwarding bodies now compile in KotlinxCompilerObjects, with source-matched
.cpp/.hpp files. This supplies the consumed internal Any? instantiation. Ten
Native/C++ allocation/empty-array observations agree; actual collection copying
remains unexecuted until the genuine ancestors/ArrayList are available. Five main
and three Native-OFF tests record zero failures; ordinary C++ retains 42/43/82.
Six files retain 29 checked pinned ranges. Both full-root deep scans cover
compiler 672/library 354, 587 paired units/763 physical files. Shared conversion
2/54 functions, Native Arrays 6/10; normalized-logic/span zeros and generated
errors remain provisional. Return these dependencies to actual list/IR consumers,
then real descriptors/binding and Native scopes. Both complete MLX paths remain
unfinished. Evidence: build/ir-recovery/ir-collection-conversion/source-split/
and docs/audits/DOCKING_RING_HANDOFF.md.

Earlier compiler-owned class binding checkpoint: real Any/KClassImpl C++ objects execute
canonical identity across two translation units/generic specializations, names,
subtypes, equality/hash and text. Temporary plugin diagnostics are removed.
KotlinxCompilerObjects supplies the same translated bodies to production compiler
targets and the object test. Native-only rooted name transport remains separate.
The Native-OFF compiler/library/plugins/application build invokes no Kotlin tools;
36 selected target build records and ten binary/shared-library dependency lists
exclude Kotlin runtime dependencies. The two executables link only libc++/libSystem.
Five main tests and three Native-OFF tests execute with zero failures; ordinary
C++ executes 42/43/82. The real Native name regression retains 72 observations.
Eleven files retain 71 checked provenance ranges. Both full-root deep reports
cover compiler 672/library 354, 586 paired units/759 files. Required provisional
score/logic/span zeros/generated errors remain for Any/KClassImpl/TypeInfoNames/
StringNumberConversions; KClassImpl remains 4/11 functions, 1/2 types, body 0.10.
This is the consumed C++ ABI binding, not the complete Native RTTI generator,
Native object/GC/frame layout or constant-constructor lowering. Return to actual
Any ancestry/list/IR consumers and Native scopes/buildStateMachine. Both complete
MLX demos remain open. Evidence: build/ir-recovery/ir-class-binding-integration/
and the current handoff/dependency ledger.

Earlier consumed Native class checkpoint: genuine KClass/TypeInfoHolder and
marker interfaces, consumed KClassImpl methods and checked metadata/name getters
are translated. Three new nongeneric bodies and AnyToString.cpp compile; 25 actual
type checks/four real consumers compile. Pointer hashing agrees in 21 observations
on actual Native 2.4.10 metadata before/after GC plus null. Actual compiler-object
metadata/instance binding and constant-constructor lowering remain unfinished;
no C++ Any/KClass instance or new production integration is claimed. Nine C++
files retain 40 checked ranges/seven source KDocs across seven exact pinned sources.
Both full-root scans cover compiler 672/library 354, 579 paired units/752 files.
KClassImpl raw functions/types 4/11 and 1/2, body similarity 0.10, reported-missing
NativePtr and all five affected provisional deep logic/span/score zeros/generated
errors remain required findings. Return through actual metadata to Any/list/IR
consumers and Native scopes/buildStateMachine. Both complete MLX demos stay open.
Evidence: build/ir-recovery/ir-kclass-contract/ and the dated dependency ledger.

Earlier Native Any prerequisite checkpoint: the complete consumed public class
and three method bodies are written. Identity bodies and Long formatting compile;
393 C++ sanitizer digit/error observations execute without Kotlin runtime libraries.
Any text compilation still stops at actual KClassImpl.hpp and real metadata/link
closure. No Any instance or new IR inheritance/production integration is claimed.
These compiler classes do not impose a Kotlin base or storage format on ordinary
C++ application classes. Both full-root scans cover compiler 671/library 354,
571 paired units/743 physical files. Raw Any functions/types are 3/3 and 1/1,
body similarity 0.53; StringNumberConversions remains 0/17 and Char missing.
Affected deep score/logic/span zeros and generated errors remain provisional.
Return through real metadata/list ancestors/ArrayList to IR binding and Native
scopes/buildStateMachine. Neither complete MLX GPU demonstration is established.
Evidence: build/ir-recovery/ir-any-contract/ and the dated dependency ledger.

Earlier Native ArrayList prerequisite checkpoint: six complete consumed predicate
removal bodies and the actual RandomAccess marker are translated. Five real-interface
consumers compile, including actual IR declaration elements; no backing list/iterator
or IR instance was fabricated. Generic Native range-copy/array-return bodies execute
on compiler-owned storage: 23 observations agree with exact pinned source bodies
running Native 2.4.10 and strict C++ sanitizer execution. Native copying uses the
installed stdlib intrinsic. C++ reference/uninitialized-slot assertions do not
establish actual Native heap/GC or shared-frame compatibility. Four headers retain
26 checked ranges/15 KDocs across four pinned sources. Full-root inventories are
compiler 670/library 354, 567 paired units/738 physical files. Raw MutableCollections
functions 6/33, Native Arrays 3/10, generated arrays 9/240, RandomAccess 1/1 types,
provisional logic/span zeros and generated errors remain required. The actual
copy consumer still stops at missing ArrayList.hpp; finish its actual ancestors/
backing/constructor/set and return to IR binding/Native scopes. No C++ list removal
execution, new production integration or complete MLX GPU demonstration is claimed.
Receipts: build/ir-recovery/ir-array-list-ancestors/ and the dated dependency ledger.

Earlier actual IR class list-rewrite checkpoint: complete consumed Native
MutableList and both compiler rewrite source algorithms are written. Strict Clang
compiles the actual in-place adapter, index helpers and IrClassChildren source;
28 genuine type checks/three consumers compile. Copy-on-change still requires
the actual Native ArrayList.hpp; its written body is not compilation/execution
acceptance and the forward declaration supplies no implementation. No real nodes,
backing lists or visitors were fabricated. New bodies are not production linked
and no traversal/copy/binding runtime acceptance is established. Seven changed
C++ files retain 47 checked ranges/nine KDoc blocks across ten pinned sources.
Full-root scans cover compiler 668/library 354, 565 paired units/736 physical files.
Raw transform functions 1/9, Native Collections 1/11, List 1/2 types/forced zero,
missing shared Collections, Native ArrayList 91 missing functions/three types and
the transform-header namespace rejection remain visible. Provisional logic/span
zeros and generated errors remain required. Return genuine ArrayList constructor/
set/backing closure to copy-on-change, then empty lists/descriptors and actual
parameter/variable owner binding/Native scopes. Neither MLX GPU demo is complete.
Receipts: build/ir-recovery/ir-class-list-rewrites/ and the dated dependency ledger.

Earlier actual IR parameter/default-body checkpoint: actual type/value parameters,
their complete concrete constructors and leaf symbols, TypeParameterDescriptor
and default-expression bodies are translated. Six interface/visitor bodies and
33 actual-type checks/four consumer functions compile. Actual constructors still
need empty_list and concrete symbols still need IR-based descriptors; no actual
parameter instance was constructed/bound or newly linked into production. The
class-child consumer now stops at actual MutableList.hpp, beyond its old missing
parameter headers. Return through those consumed transforms and actual descriptor/
signature/rendering to class traversal, real binding and Native suspension scopes.
22 files retain 267 pinned ranges and eight complete comment blocks, including
isHidden nested examples/question with only its prohibited label changed. Five
existing compiler targets build and four CTest checks have zero failures.
Native-OFF ordinary_cpp rebuilds/executes 42/43/captured-object 82 with OS libraries
only and empty Kotlin tool configuration; this is the existing scalar example.
Full-root scans cover compiler 664/library 354, 560 paired units/730 physical files.
Raw parameter functions 1/2 and 1/4, bodies 1/1 and 1/4, symbols 6/23 types and
implementations 3/17, TypeSystemContext 9/32 and 0/74 functions remain measured.
Forced property-only scores, provisional logic/span zeros, generated errors and
missing container findings remain required. Neither complete MLX GPU demo is
established. Receipts: build/ir-recovery/ir-parameter-nodes/ and the dated ledger.

Earlier actual IR class checkpoint: the complete consumed IrClass and five
inherited source interfaces, typed classifier/class symbols and ClassDescriptor
contracts are written. IrType includes the actual classifier; classifier and
ClassKind bodies compile in three production compiler targets. Class child
traversal remains uncompiled at missing IrTypeParameter.hpp, with actual receiver,
mutable-list/transform and base-renderer dependencies still required. No actual
class, descriptor or bound symbol instance was fabricated or executed. Four strict
body compiles, 26 real-type checks/two compiled call sites, 142 exact ranges and
18 KDoc blocks are recorded. All 48 class-kind property observations agree with
unchanged pinned Native source and C++ sanitizer execution. Four CTest checks
have zero failures. Native-OFF ordinary_cpp still builds/runs with OS libraries
only and no Kotlin tools/runtime in its build graph. Full-root scans cover compiler
659/library 354, 549 paired units/709 physical files. IrClass raw functions 0/3,
ClassKind forced zero, two reported-missing container files, TypeSystemContext
8/32 types and 0/74 functions, provisional logic zeros and source errors remain
required. Return to actual class children/transforms, metadata/Any/attributes and
real variable/binding/Native scopes. Neither MLX GPU demo is complete. Evidence:
build/ir-recovery/ir-class-identity/ and the dated dependency-ledger checkpoint.

Earlier Native class-name checkpoint: TypeInfoNames source getters use actual
Native metadata, strong runtime entries and real roots. Seventy-two observations
agree with unchanged pinned source on twelve Kotlin-generated classes before/after
GC. Nineteen ranges/three KDoc blocks and two optional CMake checks are verified.
Native-OFF ordinary_cpp rebuilds/runs with OS libraries only; this unit is absent
from its build graph. A broader all-target build stops at unfinished delay/yield
authoring in JobTest/AsyncTest. Actual C++ class metadata, Any ancestry and the
variable/binding/Native-scope consumer chain remain unfinished. Full-root scans
cover compiler 652/library 354, 534 paired units/690 physical files. Raw source
1/1 types and 0/0 body functions, seven extra target bodies, forced function zero
and provisional logic/span zeros are retained. Evidence:
build/ir-recovery/ir-native-type-names/. Both complete MLX GPU demos remain required.

Earlier actual type checkpoint: explicit IrType source contracts/default bodies,
seven complete consumed type markers and TypeRefMarker are written. Variables and
expressions now include the actual type header; type/variance bodies compile in
all three compiler targets. Twenty actual-type checks and eight new-node body
recompiles succeed. Thirty-five scalar enum observations match unchanged pinned
Native source (full Variance and exact complete nullability enum fragment) and
strict address/undefined sanitizer output. Three affected checks have zero failures.
63 ranges/four KDoc blocks retain exact pin provenance; C++ execution loads only
libc++/libSystem. No concrete IR types/nodes were fabricated or constructed.
A real self-equality compile probe exposes missing implicit Any ancestry. Actual
Any/class metadata, sealed metadata/concrete equality, collections, empty lists,
IR-based descriptors/rendering and variable/scope integration remain unfinished.
Full-root deep inventories: compiler 652/library 354, 532 paired C++ units/687
physical files. Raw type/variance function gaps, 25 missing type-system types/74
unported functions and all four provisional logic/span zeros remain required.
Evidence: build/ir-recovery/ir-type-contracts/. Return to actual variables/owners
and Native suspension scopes/buildStateMachine. Both complete MLX GPU demos remain
required; primitive enum execution does not establish them.

Earlier real-node source checkpoint: ten generated value-access/suspension classes
have eighteen C++ files and complete source constructor/property/traversal bodies.
Eight strict body compiles, twenty-seven actual-type checks and four interface
consumer bodies compile; 177 ranges/six KDoc blocks resolve to the exact pin.
Actual IrValueSymbol and IrVariable identities and normal/resumed child order are
retained. No actual nodes were instantiated. Compiler link probes still need the
real IrDeclarationBase parent/typeinfo bodies and their untranslated IR renderer;
these files remain outside production targets, and the stopping condition is unmet.
Actual object/attribute/empty-list/descriptor/binding/scope closure remains required.
Both full-root deep scans inventory compiler 649/library 354 against 527 paired
C++ units/680 physical files. Ten visitor operations remain unmatched, constructor/
property groups have forced function-score zeros and all ten groups retain
provisional normalized-logic/span zeros. No criteria or findings are suppressed.
Evidence: build/ir-recovery/ir-value-suspension-nodes/. Return actual nodes to the
recorded Native variable/suspension scopes and buildStateMachine. Neither required
standalone C++/MLX nor direct Native/C++ shared-state-machine/MLX demo is complete.

Earlier build integration checkpoint: root CMake now applies the existing frontend
and mandatory LLVM stage together to the core/tests. The earlier unavailable-target
warning came from checking before frontend registration; actual requested packages
are now required. A separate Release CMake application with Native interop/compiler
tests disabled builds both plugins and the full C++ library, runs the existing
ordinary-C++ lambda fixture (42/43/captured-object 82), and loads only libc++ and
libSystem. Two missing defining Clang includes were corrected without changing
translated algorithms or optimization. Four pipeline checks and three final focused
checks have zero failures. Full-root deep reports still retain 1/2 RestrictSuspension
and 1/47 IrTypeUtils matches, missing APIs and provisional logic zeros. The existing
Native callback/StableRef fixture does not prove direct shared-state-machine or MLX
acceptance. Return to actual object/attribute/variable identity and Native scopes.
Evidence: build/ir-recovery/frontend-cmake-repair/.

Earlier property/identity checkpoint: the four consumed Native property/type
interfaces compile, with eighteen actual-type checks and five abstract consumer
bodies. Fourteen ranges/nine KDoc blocks retain exact pinned provenance. Real
properties/classifier/projection objects remain unfinished. Native identity hash
now has its exact source operation in a .cpp body: ordinary C++ Name/string objects
need no Kotlin runtime, and ten comparisons on actual rooted Native objects agree
with both Kotlin's intrinsic and runtime before/after GC. Four affected CTest
checks have zero failures. Source receipts are in
build/ir-recovery/ir-property-contracts/ and ir-native-identity-hash/.
Both full-root --deep scans cover compiler 645/library 354 source files against
517 paired C++ units and 662 physical target files. All new groups retain
provisional logic/span zeros; unused APIs and represented property gaps remain
visible. Actual Any/class metadata/WeakReference and executable attribute/variable
construction still precede Native scopes. This closes a consumed hash operation,
not shared coroutine states or either required MLX demonstration. The early
frontend warning was subsequently corrected as a target-registration-order error;
these compiler-reference builds do not establish the complete authoring pipeline.

Earlier attribute-key/delegate checkpoint: actual key, flag, both delegates,
factories and nullable/property operations have source bodies. No fake owner,
key or weak/property definition was supplied. Strict Clang stops at missing
Any.hpp; real keys/nodes and weak/property/map behavior remain unexecuted.
26 ranges and seven KDoc blocks retain pinned provenance. Both full-root --deep
reports retain 7/14 function matches, seven unmatched nested methods, three missing
nested types and provisional logic/span zeros. Inventories: compiler 645, library
354, complete src 512. Receipts: build/ir-recovery/ir-attribute-keys/.
Two earlier element projection source ranges were corrected after full source reading.
Return to actual object/weak/property/map and declaration/binding dependencies,
then Native scopes/buildStateMachine. Both required MLX demonstrations remain open.

Earlier IR element/initializer checkpoint: actual dense-slot algorithms and the
generated expression hierarchy are written. Strict Clang compiles the core element,
expression, existing variable and variable-symbol bodies. Snapshot/copy maps, actual
keys/debug delegates, empty-list, rendering and descriptors remain genuine dependencies;
no concrete node/key construction, binding or production integration is established.
41 source ranges and four KDoc blocks resolve to four exact pinned files. Both
full-root --deep reports retain missing APIs and provisional logic zeros; inventories
are compiler 645, library 354, complete src 511. Receipts:
build/ir-recovery/ir-element-storage/. Return to real variables and Native scopes/
buildStateMachine. Standalone C++ remains independent of Kotlin tools/runtime;
both complete C++/MLX and shared Native/C++ state-machine/MLX demos remain open.

Earlier null-array growth checkpoint: the consumed arrayOfNulls, both copyOfNulls
overloads and object-array copyOf(newSize) execute with readable null padding,
nullable type idempotence, source error order, independent copy storage and retained
reference identity. 930 observations match actual Native 2.4.10 execution; strict
Apple Clang and address/undefined sanitizer execution report no diagnostics. Four
affected CTest checks have zero failures. 41 provenance ranges resolve to six exact
pinned files; 16 KDoc blocks retain their source text with comment whitespace
normalized, and four consumed blocks match installed Native source exactly.
Both full-root --deep scans retain missing operations and provisional logic zeros.
Receipts: build/ir-recovery/ir-null-array-growth/. Return to IrElementBase dense
attributes and its actual key/map dependencies, real variables and suspension scopes.
This is compiler-storage evidence, not Native array layout, shared coroutine states
or either complete C++/Native/MLX demonstration.

The current Native reference boundary is `src/kotlinx/coroutines/KotlinGCBridge.hpp/.cpp`:
actual runtime declarations, FrameOverlay and ObjHolder bodies, without weak/no-op
substitutes. A real Native host fixture verifies rooted lifetimes, atomic operation
results on rooted slots and object return slots. It does not verify heap/global fields
or shared coroutine frames. The installed runtime lacks the pinned RegisterGlobal;
a matching pinned runtime is required. Receipts are in
`build/ir-recovery/native-references/`. Native AtomicReference, CurrentThread, Lock and
Lazy remain the next genuine parameter dependencies.

Current concrete parameter checkpoint: four source-bodied descriptor implementation
pairs are present under `descriptors/impl/`. Headers and the NonRoot body compile;
the other bodies still require genuine renderer, type-update, visibility, collections,
substitution and Native Lazy dependencies. They have no concrete-instance execution
acceptance. Member/CallableMember contracts and Kind/Modality helpers compile; eight
unchanged Kotlin/Native Modality observations match C++. The identity ledger and
`build/ir-recovery/concrete-parameters/` record exact source provenance, individual
body diagnostics and raw deep findings. General generic variance remains incomplete.

Current IR declaration checkpoint: eight actual abstract root/declaration contracts
and the full IrVisitor are written. All 89 signatures compile; all 88 source default
routes and argument forwarding match mechanically. The non-template C++ dispatch
body is compiled into three existing compiler targets. Concrete attributes/nodes,
transformer behavior, generic star views and real owner binding remain unfinished.
No replacement IR object was manufactured. The scanner's imported-forward namespace
identity rejection and raw missing/zero findings are recorded in the identity ledger.
Current receipts: build/ir-recovery/ir-declaration-contracts/.

Earlier concrete variable source checkpoint: five actual declaration/variable/symbol
pairs have source parent errors, properties/defaults, initializer traversal and
constructor binding. New bodies stop at real IrElementBase or IR-based descriptor
dependencies and are not in production targets or exercised with real instances.
No fabricated ancestor or collection was supplied. Descriptor narrowing now retains
one abstract root dispatch across value/bindable interfaces. Four existing boundary
units and fifteen genuine contract checks compile; invalid owner/descriptor rejected.
Three compiler targets rebuilt; two affected injection regressions have zero failures.
157 ranges resolve to seven pinned source files. Both full-root --deep reports retain
21 missing generated interfaces, 16 implementations, three unmatched variable
operations and provisional logic zeros. Receipts: build/ir-recovery/ir-concrete-variable/.
Return through genuine ancestors/attributes/collections/descriptors to these
constructors and variable/suspension scopes, then Native buildStateMachine.
Neither complete standalone C++/MLX nor actual Native/C++ state-machine/MLX
execution is established.

Earlier typed value declaration/symbol checkpoint: the real IrValueDeclaration
and IrValueSymbol interfaces and actual typed C++ virtual property boundary compile
in three compiler targets. Mutual C++ covariance failure is recorded; typed public
getters preserve the root object's identity through one abstract dispatch. Source
bounds, twelve consumed KDoc blocks and 58 exact provenance ranges are checked.
Thirteen actual-type contract checks compile and invalid Name ownership is rejected;
no concrete owner was made for these checks. The actual source symbol implementation
still stops at IrBasedDescriptors.hpp. Concrete variable/descriptor/type/attribute
and scope execution remain unfinished. Both project-wide deep reports retain raw
property-only zeros, 22 missing generated symbol interfaces and unsupported emission.
Receipts: build/ir-recovery/ir-value-identity/. Ordinary C++ use remains independent
of Kotlin application runtime; this is compiler prerequisite evidence only.

Current transformer source checkpoint: the real IrTransformer generic base and
IrElementTransformerVoid source algorithms are written, including child-first
rewriting, typed node results, postfix and free child helpers, final forwarding
and checked package/file casts. Their consumed legacy transformer is the actual
methodless source marker interface. 457 source ranges and three KDoc blocks are
verified against the pinned source. The marker header compiles; transformer
headers/bodies stop at genuine untranslated node headers and are not production
integration or real-node execution acceptance. No fabricated hierarchy was used.
The full-root oracle inventories 89/89 and 181/181 methods/helpers respectively,
with 0.86/0.68 body similarity and provisional normalized-logic zeros. Raw lint,
missing aliases/extensions, source grammar and emission findings remain visible.
Receipts: build/ir-recovery/ir-transformers/. Return to actual declaration/value/
symbol and suspension scope closure, then Native buildStateMachine and its frontend
consumer. Ordinary C++ use must remain independent of any Kotlin application runtime.

The pinned Native host distribution probe stops before compilation because the
sparse checkout lacks repo/kotlin-build-helpers. Its documented host Xcode requirement
is 27; installed Xcode is 26.6. That probe did not reach Xcode validation. Completing
this exact source/build closure and a matching runtime remains required; an installed
Native host runtime does not establish bare-metal execution or shared coroutine frames.

Current Native collection dependency checkpoint: Map/MutableMap and actual Entry
contracts, Set/MutableSet, mutable iterable/collection and iterator interfaces now
exist with all source KDoc and 140 pinned provenance ranges. Strict consumers compile
37 static checks and source calls with primitive/nullable and actual descriptor types.
Cpp namespaces for static nested entries preserve independent entry identity. These
are compiler interface/type boundaries; no replacement collection instance was made.
Backing HashMap/build-map/empty-map algorithms, actual view lifetimes/mutation and
IrElementBase/IrAttribute execution remain unfinished. Current full-root --deep reports
retain their raw zero and split-Collection pairing diagnostics. Current receipts:
build/ir-recovery/ir-attribute-dependencies/.

Current Native array dependency checkpoint: source object-array uninitialized
allocation/reset/copy/default/range/resize algorithms execute on compiler storage.
NativeArrayUtil.hpp/.cpp supplies actual strong Native array runtime declarations
and reset bodies over ObjHeader arrays. 862 source-algorithm observations agree with
Native; a separate linked fixture verifies real Native object identity, overlaps,
C++ array writes/resets and result-slot retention. These are distinct storage paths;
no Native object is reinterpreted as a C++ array. Raw similarity 0.19/logic zero and
false primitive-overload pairing remain open. Actual HashMap/view/ancestor algorithms,
IR attributes/owners/scopes, pinned runtime and bare-metal frames remain unfinished.
Current receipts: build/ir-recovery/native-map-dependencies/.

Current Native integer-array checkpoint: IntArray, its actual specialized iterator,
all eight PrimitiveIterators contracts/forwarders, consumed IntArray generated
copy/range/resize/fill operations and five AbstractList companion helpers are
translated. 5,367 observations agree with actual Native stdlib. A separate Native
fixture exercises real IntArray get/set/length/copy/overlap/fill through mandatory
runtime symbols. 163 exact source ranges and 30 KDoc blocks are checked.
Compiler-owned fixed-length C++ IntArray storage is not Native ArrayHeader layout.
Raw PrimitiveIterators is 8/8 functions/types but normalized logic provisional zero;
Arrays.kt is 3/24 functions, generated ArraysNative 7/240. Full AbstractList/map and
IR identity/scopes remain open; seven other primitive arrays and fourteen primitive
fill/copy overloads remain unported. Receipts: build/ir-recovery/native-int-array/.

Files:
- `kotlin/concurrent/atomics/Atomics.hpp/.cpp` — complete consumed Native AtomicInt
  operations and integer extensions/retry loops. Forty-two observations agree with
  the unchanged source compiled by Kotlin/Native on macOS ARM64, including concurrent
  increments, compare-and-set contention, overflow and update failure/retry. Strict
  sanitizer execution and the integrated `kxs_atomic_int_contract` have current
  receipts in `build/ir-recovery/native-lazy/`. This is compiler-owned primitive
  storage: Native object layout, reference/root operations, Lock and Lazy remain
  unfinished. Whole-file deep logic is provisional zero; raw missing classes remain.
- `org/jetbrains/kotlin/ir/symbols/IrSymbol.hpp/.cpp` — source abstract symbol
  contracts and the compiled `is_public_api` helper. The concrete source-bodied
  `symbols/impl/IrSymbolImpl.hpp` draft is not compiled: real descriptors,
  declarations, signatures and rendering dependencies remain untranslated.
  Binding and scope behavior have no execution acceptance yet.
- `org/jetbrains/kotlin/descriptors/DeclarationDescriptor.hpp/.cpp`,
  `DeclarationDescriptorVisitor.hpp` and `DescriptorVisitorDispatch.hpp` — real
  root descriptor contracts and a typed C++ boundary for the source generic
  virtual visitor. Fifteen methods compile for four result/context configurations.
  Transport tests preserve reference/move-only/null results; real node routing
  and IR owner binding remain unverified.
- `descriptors/annotations/Annotated.hpp`, `AnnotatedImpl.hpp/.cpp` and
  `ValidateableDescriptor.hpp/.cpp` — source getter/storage and genuine default
  validation. Full annotation collections and concrete descriptors remain open.
  `DeclarationDescriptor` and parameter contracts now compile; `IrSymbolImpl`
  first stops at the actual `IrBasedDescriptors` dependency. See the identity ledger.
- `descriptors/SourceElement.hpp/.cpp`, `SourceFile.hpp/.cpp` — genuine
  absent-source objects, nullable UTF-16 file-name contract and stable source-file
  routing. Their singleton trace matches pinned Java execution; these objects
  supply source metadata rather than stand-in IR declarations.
- `descriptors/Visibility.hpp/.cpp`, `Visibilities.hpp/.cpp` — source base
  behavior, all nine actual visibility objects, import/display rules and partial
  ordering. `kxs_visibility_contract` records a trace that matches 91 unchanged
  Kotlin source observations, including all 81 singleton comparisons.
- `descriptors/DescriptorVisibility.hpp/.cpp` — full genuine source base with
  abstract accessibility contracts and implemented delegate/default behavior.
  Concrete accessibility rules, DelegatedDescriptorVisibility, obsolete-API
  metadata and actual descriptor integration remain untranslated.
- `descriptors/DeclarationDescriptorWithSource.hpp`,
  `DeclarationDescriptorNonRoot.hpp`, `DeclarationDescriptorWithVisibility.hpp`,
  `Substitutable.hpp` — source ancestors, covariant original reference, Clang
  nonnull metadata and substitution upper bound. Actual callable/parameter/type
  consumers, general generic variance and concrete substitution remain open.
- `descriptors/{CallableDescriptor,ValueDescriptor,VariableDescriptor,ParameterDescriptor,ValueParameterDescriptor}.hpp`
  — real source contracts, narrowed return types and typed collection boundaries.
  `ValueParameterDescriptor.cpp` retains the source false default for late init.
  Consumed covariance compiles; concrete descriptor implementations, actual
  binding, generalized variance and nullable user-data semantics remain open.
- `kotlin/collections/{Collections,Iterator,CollectionElement}.hpp` — consumed
  read-only generic interfaces and private C++ virtual-template boundary.
  Their full source KDoc remains; mutable/set/map algorithms are untranslated.
- `kotlin/Array.hpp` — Native array initializer and private ArrayIterator algorithm,
  with per-function Native provenance. Exhaustion prechecks and reports its source
  index; the former JVM iterator/factory are removed. Nineteen observations agree
  with an ARM64 native executable containing the unchanged pinned Native iterator
  class and the installed Native 2.4.10 stdlib. C++ array storage is compiler-owned
  storage; Native object headers, GC roots, runtime entries and direct object/frame
  handoff remain required and unverified. Bare-metal execution is unverified.
  Raw normalized deep logic remains provisional zero. Receipts:
  `build/ir-recovery/native-runtime-correction/`.
- `org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.hpp`,
  `ir/declarations/IrParameterKind.hpp`, and `descriptors/Named.hpp` — the
  methodless marker hierarchy, parameter kinds and abstract naming contract
  required by the actual IR declarations and descriptors.
- `org/jetbrains/kotlin/name/Name.hpp/.cpp` — full method translation of the
  compiler's Java `Name` dependency, preserving UTF-16 comparison, nullable
  results, ordinary/special distinction and wrapping hashes. The
  `kxs_ir_name_contract` test checks this prerequisite. Actual declaration root contracts now exist; concrete owners,
  binding and scopes remain unfinished; see
  `docs/audits/IR_IDENTITY_DEPENDENCIES.md`. The Kotlin deep scanner has no Java
  parser, so this dependency's evidence includes pinned Java execution.
- `CodeGenerator.hpp/.cpp` — translated branch emission, phi input assignment and instruction-position
  state from FunctionGenerationContext and PositionHolder. Each context binds
  one LLVM function definition for address generation; block layout follows
  Kotlin's insertion-after-current algorithm. Conditional value/effect helpers
  retain source phi and terminator handling; raw_ret marks returns as terminators.
  Integer comparisons retain Kotlin’s signed and unsigned LLVM predicates;
  switch_ retains ordered cases and the explicit default destination.
  Runtime-cleanup return methods remain untranslated. Mandatory injection
  and the coroutine expression helpers share this implementation. Kotlin source
  location maps, runtime frame generation and full context initialization remain
  unported.
- `IrToBitcode_coroutines.hpp/.cpp` — real LLVM types, resume-point collection,
  null-label dispatch, and normal/resumed result merging through phi nodes.
  Merge phis and dispatch comparisons retain Kotlin’s unnamed instructions.
  Continuation-block construction invokes its source code-generation callback;
  the exception-handler caller and location/type lowering remain untranslated.
  Methods use snake_case and retain exact source ranges. The mandatory injector
  uses the translated resume-point scope. The expression emitters receive actual
  LLVM operands and callbacks from the compiler boundary; complete Kotlin IR
  expression evaluation, source-location metadata and general declaration/scope
  resolution remain unported.
- `IrToBitcodeCoroutinesTest.cpp` — verifies the generated module and emits it
  for execution checks of distinct normal/resumed results, Unit without a phi,
  phi predecessors when normal generation has already terminated, and two
  sequential suspension points with distinct saved resume addresses.
- `translate_ir_to_cpp.py` — naive whole‑file transliterator for
  `IrToBitcode.kt`. This emits a C++‑ish corpus for manual cleanup.

Generate a raw transliteration:

```bash
python3 tools/kotlinc_native_ref/translate_ir_to_cpp.py \
  tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt \
  tools/kotlinc_native_ref/IrToBitcode_full.cpp
```

The generated file is *not* committed by default.
