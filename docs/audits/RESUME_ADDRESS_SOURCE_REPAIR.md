# Compiler resume-address source repair — 2026-10-08

## Callable declaration ownership and invoke identity — 2026-10-08

Continuation from `827b1c9c` repairs the production Clang integration of
AbstractFunctionReferenceLowering.kt:261-288's specific invoke declaration and
parent remapping. The previous turn's saved default-callable failure came from
an intermediate frontend: the final tail-only default lambda already passes.
A newly added non-tail default lambda reproduced the real retained-frame defect.
Its implicit Clang closure record ends at the capture introducer; using that
range truncated generated frame construction. getLambdaContextDecl is optional
and was null in the concrete default-parameter case.

CompilerFrameLowering.cpp:118 now resolves the actual containing declaration
from the host AST ancestry when the explicit lambda context is absent. At :215
it preserves the owning function/class/variable declaration, its namespaces and
its complete source boundary; prototype/variable/record semicolons are restored.
This is Clang infrastructure for the source parent-remapping contract, not an
invented Kotlin IR owner or alternate frame. Runtime checks then exposed a second
mismatch: sibling lambdas in one complete class can have identical invoke names
and canonical types. At :423 the importer also compares original source identity
before selecting the replacement body. It no longer installs a sibling's original
body over the requested lowered invoke. Existing explicit template-instantiation
entry selection stays separate. Temporary debugging diagnostics were removed.

The existing default-callable fixture now covers a tail default plus non-tail
function-prototype, function-definition, namespace, global initializer, member
initializer and member-parameter contexts. Its runtime branch is enabled only
by KXS_TEST_DEFAULT_CALLABLE_RUNTIME; the normal syntax gate still checks the
source declarations. The test driver registers strict ordinary in-compiler
compilation and execution of all35 outcome cases. Seven contexts each exercise
immediate success, resumed success, resumed failure, resumed cancellation and
immediate failure; the tail entry forwards the parent, the six non-tail entries
hand off their current frame, and each completes once. Result boxes have explicit
unique_ptr deletion. Empty closure contexts are tested; arbitrary captured-object
and temporary callable ownership is not established by this fixture.

ContinuationImpl.cpp:6 removes its unused DispatchedContinuation include after
comparison with the actual Native ContinuationImpl.kt imports/body. No runtime
algorithm or substitute implementation was added. Fresh actual context_impl.cpp,
ContinuationInterceptor.cpp, ContinuationImpl.cpp and the stdlib
CancellationException.cpp form the bounded runtime dependency archive used here.
They build without Kotlin tools/runtime. The context/interceptor/continuation
units report eight existing unused-parameter warnings under -Wall/-Wextra/
-Wpedantic; those warnings remain unsuppressed and are not a strict dependency
build pass. CancellationException and the generated fixture compile strictly.

The frontend builds against LLVM23.1.2 with Native runtime OFF. The default-callable
fixture passes -Wall/-Wextra/-Wpedantic/-Werror compilation and ASan/UBSan execution.
otool lists libc++, libSystem and Clang's ASan runtime, with no Kotlin/JVM link.
The Unit-tail fixture also passes fresh sanitizer execution, and the earlier
expression-slicing fixture now executes its comma temporary destruction, borrowed
resource identity, repeated suspension, resumed failure/cancellation and ordered
cleanup assertions successfully against the same actual dependency units. This
supersedes the last checkpoint's lack of runtime evidence for those bounded cases;
it does not prove a fresh full-core build, MLX GPU execution or actual Native
shared-frame interoperability. An additional strict qualified_locals.cpp compile
fails before import on unremapped structured-binding names head/resource in
static_assert(noexcept(...)); no qualified-local runtime pass is claimed here.
That source query/declaration remapping remains another lowering gap.

The full authoring driver advances through its warning controls, source rejection
checks, new syntax gate, direct extraction/in-process execution,35 default-callable
cases and Unit fixture. It now stops at default_arguments/factory.cpp:22:
a relocated nonsuspending default-expression lambda prints make() in the global
caller frame, losing its actual defaults::make symbol context. Continue the
AbstractFunctionReferenceLowering.cpp:74 BoundValues printer's ordinary declaration
reference remapping, compared with the existing DefaultArgumentReferences printer
and DefaultArgumentStubGenerator.kt:91-108. Do not fix that fixture by manually
qualifying its upstream-context source or by replacing the default expression.

Final compiler/library/frontend deep scans completed. Compiler reference root:
286/7163 bodies,132/1617 types,0.28 similarity,10 scoring failures,695 sparse files,
193 paired units/285 target files. Coroutine root:663/2918 bodies,178/560 types,
0.24 similarity,12 failures,412 paired units/614 target files. Frontend root:
14/7163 bodies,6/1617 types,0.04 similarity,0 scoring failures,15 paired units/
25 target files. Clang AST infrastructure changes do not certify additional Kotlin
IR/classifier/metadata parity. Current reports remain in build/source-continuation/
{compiler,library,frontend}-source-distance. Build/runtime evidence is in
build/source-continuation/default-callable-runtime; the full driver failure is in
default-callable-full-regression.log. Full transliteration, general callable/IR
lowering, both required MLX/Native executable paths and complete lifetime contracts
remain unfinished. The original goal remains active.

## Tail expression containers and C++ temporary retention — 2026-10-08

Continuation from `9cd3e1b3` returns to the production frontend's state-machine
selection. TailSuspendCallsCollector.kt:64-79 gives an expression container's last
statement its enclosing tail position. TailSuspendCallsCollector.cpp:45,78 now
maps built-in C++ comma expressions to that rule: the left operand is non-tail
and the right operand inherits the enclosing state. Nested containers, conditions,
try regions and ordinary enclosing calls retain their source visitor state.
Overloaded comma remains an ordinary call. is_unit_read at :160 additionally
unwraps parentheses and Clang cleanup nodes, matching the existing transparent
AST traversal; a parenthesized null Unit result no longer forces a needless frame.

KotlinxSuspendPlugin.cpp:499 tracks actual comma-prefix evaluation when selecting
a direct entry. Destructor-bearing materialized/bound temporaries in that prefix
require retained frame storage across the tail operand. By-value arguments of the
tail call do not acquire that prefix state. This is a NOTE(port) C++ lifetime
adaptation to NativeSuspendFunctionLowering.kt:55-91, not a substitute state
machine. Existing borrowed references and captured object ownership are unchanged.
NativeSuspendLowering.cpp:1102 now saves the actual PrintingPolicy while enabling
canonical default-argument types: Clang's PrintAsCanonical bit-field cannot bind
to SaveAndRestore<bool>. This repairs a concrete production plugin build error.

The Native-OFF LLVM23.1.2 frontend and collector targets build. All26 AST cases
pass, including nested comma tails, non-tail prefixes, overloaded comma, try and
conditional state, and parenthesized Unit returns. The existing driver now checks
nine direct entries and seven suspending variants through28 immediate/resumed/
immediate-failure/resumed-failure cases. Its direct portion was run separately
from the broad driver's earlier failing gate. Both extracted diagnostic output
and ordinary in-compiler compilation/execution pass with -Wall/-Wextra/-Wpedantic/
-Werror. ASan/UBSan direct execution passes and confirms exact caller continuation
forwarding, one completion and released continuation handles. No cancellation or
retained resource-destruction claim follows from those direct cases.

The direct executable links fresh actual context_impl.cpp and
ContinuationInterceptor.cpp dependencies only. Their separate strict compile
fails on four existing unused-parameter diagnostics; ordinary dependency
compilation with -Wall/-Wextra/-Wpedantic succeeds and reports all four warnings.
No warning was suppressed or source dependency replaced. This bounded executable
is not a fresh full-core build. The available core archives still expose the older
kotlinx::coroutines ABI; linking current frame fixtures against them fails on
current kotlin::coroutines context/continuation definitions.

unit_tail.cpp and expression_slicing.cpp compile to LLVM with strict warnings,
mandatory frontend/pass injection and O1 ASan/UBSan instrumentation. The Unit
comma entry has no generated frame; comma_temporary owns the actual immovable
TemporaryArgument in its frame and emits its cleanup paths. Its O0 module has
blockaddress stores and indirectbr dispatch with consumed marker calls. O1 folds
its single-destination dispatch into a direct resume branch; the stored block
address remains. Added destruction/failure/cancellation runtime assertions are
not freshly executed. expression_slicing now includes the actual continuation
and DSL headers it uses rather than the unrelated cancellable DSL umbrella.
The full driver still fails earlier in suspend_default_callable.cpp:8 while
parsing a generated default-lambda frame with a truncated/unbalanced enclosing
source context. That lowering dependency remains a concrete next repair.

All final deep scans completed. Compiler reference root:286/7163 bodies,
132/1617 types,0.28 similarity,10 scoring failures,695 sparse source files,
193 paired units/285 target files. Coroutine root:663/2918 bodies,178/560 types,
0.24 similarity,12 failures,412 paired units/614 target files. A third scan pairs
the full pinned compiler corpus with the actual frontend root:14/7163 bodies,
6/1617 types,0.04 similarity,0 scoring failures,15 paired units/25 target files.
TailSuspendCallsCollector reports8/14 bodies,2/2 types,0.13 similarity. Its reported
missing receiver helpers must be compared with actual is_unit_read and
is_return_if_suspended_call; IR returnable-block symbols and IR-specific body
visitor contracts remain untranslated. The broad root metrics do not certify
this frontend's full source parity. Reports are under build/source-continuation/
{compiler,library,frontend}-source-distance; bounded execution/IR logs are in
{tail-container-focused,tail-runtime-fixtures}, and the broad failure is in
tail-container-regression.log.

Continue the default-callable source-context repair and the remaining actual IR
container/returnable-block, spilling and compiler dependency translations. Any
identity/hash/text operations already exist in AnyIdentity.cpp, AnyToString.cpp
and native/Runtime.cpp; the collection gap is arbitrary element operations at
the actual std::any erasure boundary, not absence of those Any methods. Complete
standalone C++/MLX and actual Native shared-frame GPU handoffs, retained resource
identity/cleanup and full-library translation remain unproved. The original goal
remains active.

## Standard and Native scalar identity catalogs — 2026-10-08

Continuation from `437fc2e7` translates StandardClassIds.kt's178 scalar properties,
six public naming/arity bodies and22 private concrete naming helpers into
StandardClassIds.hpp:9 and StandardClassIds.cpp. Package/class, annotation,
annotation-parameter and callable catalogs preserve source nesting. FunctionN,
SuspendFunctionN, KFunctionN and KSuspendFunctionN build their exact source names,
including negative inputs without introducing validation. Unsigned IDs derive
from the actual signed ID's short name. Nested Map entries and JsExport annotation
IDs derive from their actual parent IDs. ParameterNames.retentionValue returns
the same retained Name reference as value, preserving the source alias.

NativeRuntimeNames.hpp:9/.cpp now translate all31 public scalar properties:
six atomic class IDs, five callable IDs and20 annotation IDs, plus the two private
package properties and two private String.callableId helper bodies. Callables:24
and Annotations:38 reuse the actual StandardClassIds and CallableId contracts.
The source standard atomic package is kotlin.concurrent.atomics; Native runtime
atomics are in kotlin.concurrent. CompareAndSetAt versus compareAndSet names and
the top-level AtomicArray factory versus member methods remain distinct. Native
annotation aliases retain their separate qualified names; Escapes.Nothing is a
nested ID from the actual Escapes class. These identities do not install annotation
effects or provide actual IR declarations, JVM metadata or Native object layout.

All algorithms and private helpers stay in .cpp; headers contain declarations and
public nested types. Source provenance accompanies each translated function and
class. Object properties use retained first-access getters, marked NOTE(port),
and C++ keyword collisions use _name suffixes. No replacement set/map/container,
IR descriptor, coroutine state machine or Kotlin runtime dependency was introduced.
StandardClassIds still lacks17 set/map/derived collection properties, its Collections
object, primitiveArrayId and inverseMap. NativeRuntimeNames still lacks its two
primitive-to-atomic maps. Their actual collection dependency remains untranslated;
these absent contracts are not represented by stubs or stand-in algorithms.

CMakeLists.txt:48 registers StandardClassIds on the three production LLVM targets;
:104-107 connects the Native catalog's actual dependencies to the Konan fixture;
:129-137 connects both catalogs to the existing naming fixture. An initial link
exposed an incorrectly edited source-list occurrence; the CMake registration was
corrected and all five affected targets then built with Native runtime OFF on
LLVM23.1.2. Strict -Wall/-Wextra/-Werror debug ASan/UBSan compile and execution,
strict release syntax, six focused CTests and the existing LLVM module-generation
fixture pass. Added cases exercise source package distinctions, nested IDs, arity
names, alias identity, top-level/member callables and annotation aliases. No
warnings are suppressed. This is bounded catalog/LLVM dependency evidence, not
actual IR classification or complete coroutine/Native/MLX executable acceptance.

Both final-root deep scans completed. Compiler corpus remains695 sparse files:
286/7163 bodies,132/1617 types,0.28 similarity,10 scoring failures,193 paired units/
285 target files. StandardClassIds reports7/30 explicit bodies,4/5 types,0.04 body
score,206 target bodies. Its Collections type and two private collection-dependent
helpers are genuinely absent; most translated String receiver helpers remain
unmatched against C++ free-function forms. NativeRuntimeNames reports0/2 bodies,
3/3 types,0.00,35 target bodies: both actual private callable_id overloads remain
unmatched. Object-property inventories also report present getters as missing.
Source object/function emission falls back, generated source has parse errors and
normalized logic0; target parse errors are absent. Counts therefore do not certify
property/constructor/metadata parity. Full coroutine root remains663/2918 bodies,
178/560 types,0.24,12 failures,412 paired units/614 target files. Reports in
build/source-continuation/{compiler-source-distance,library-source-distance}
remain the oracle. The full transliteration/state-machine goal remains active.

The next missing dependency chain is the real collection/object and HashMap
implementation, the catalog sets/maps and Grouping/byFqNameParts. Actual callable,
InlineClassesSupport/classifier/cache/frame consumers and complete executable
acceptance still require translation and wiring.

## Callable identity and special-name source contracts — 2026-10-08

Continuation from `0c543eee` translates core/names CallableId.kt:45-141 and
SpecialNames.kt:18-119. CallableId.hpp:9 exposes all six public constructors,
package/class/callable/class-ID properties, locality, debug and single qualified
names, copy, equals/hash/text and the three extension contracts. CallableId.cpp:11,
13 keeps private companion name/class-ID calculations out of the header. The
local package is built from the actual SpecialNames.LOCAL property. Nullable
class/path values use optional and nullable extension receivers borrow pointers,
following the existing compiler naming value boundary. Ordinary application
objects gain no compiler superclass or Native runtime representation.

Callable equality compares package/class/callable name and intentionally ignores
class-ID locality and pathToLocal. Hash accumulation preserves source Int wrapping
and null's zero contribution. Copy preserves the original class ID and debug
path; with_class_id constructs a fresh ID and drops that path as the source does.
Debug naming selects pathToLocal first; normal naming selects the actual ClassId
qualified name when present. Text replaces package dots with slashes, preserving
class dots and the leading slash for the root package. Compiler opt-in locality
metadata and actual Native object identity are not supplied by these value APIs.

SpecialNames.hpp:8 exposes the source anonymous string constant,18 retained
Name/FqName object properties and all six function bodies. SpecialNames.cpp owns
the private anonymous-parameter prefix and implementations. Properties use
first-access retained storage to avoid cross-unit initialization hazards, marked
NOTE(port). THIS maps to this_name to escape the C++ keyword. Index validation
retains the source diagnostic, anonymous parameter recognition uses its exact
prefix test, and nullable safeIdentifier overloads preserve the distinction
between identifier construction and safe-identifier validation.

CMakeLists.txt:46-47 registers both units on all three production LLVM targets;
:125-131 adds them to the existing naming fixture. Native-OFF configure and
kxs_fq_name_test/KotlinxCoroutinePass/kxs-inject/kxs_codegen_test builds pass.
Strict debug -Wall/-Wextra/-Werror ASan/UBSan compile and execution, strict release
syntax, six focused CTests and the existing actual LLVM module-generation fixture
pass. The extended fixture exercises pre-main local-name construction, nullable
and nested IDs, differing class locality/debug paths with equal hashes, copy and
class replacement, root formatting, special-name errors and nullable overloads.
No warnings are suppressed. These tests cover naming dependencies, not actual IR
classifier execution or complete coroutine/Native/MLX acceptance.

Both final-root deep scans completed. Compiler corpus remains695 sparse files:
279/7163 bodies,127/1617 types,0.28 body similarity,10 scoring failures,192 paired
units/283 target files. CallableId reports7/8 explicit bodies,1/1 type,0.34 score;
its actual private calculate_class_id implementation is unmatched against the
source companion owner. Companion property matching also leaves LOCAL_NAME and
PACKAGE_FQ_NAME_FOR_LOCAL unrecognized despite their private/public implementations.
SpecialNames reports6/6 explicit bodies,1/1 type,0.41 score. These body counts do
not certify property/constructor/metadata parity. Source class/object emission
falls back, generated source has parse errors and normalized logic0; target parse
errors are absent. Full coroutine root remains663/2918 bodies,178/560 types,0.24,
12 failures,411 paired units/612 target files. Generated reports remain the oracle.

The production targets compile these dependencies but real callable/IR consumers
remain to be translated and wired. HashMap and its collection/object dependencies,
fresh-map Grouping/byFqNameParts, StandardClassIds/NativeRuntimeNames callables,
InlineClassesSupport, classifier/cache/frame lowering and both complete executable
paths remain open. The full translation/state-machine goal remains active.

## Numbers and Native HashMap sizing dependency — 2026-10-08

Continuation from `490cb209` translates the26 public scalar operations in Native
Numbers.kt:12-265, together with the six private intrinsic/runtime helpers that
supply them. Numbers.hpp keeps the public declarations and short source comments;
Numbers.cpp owns implementations and private helpers. Int/Long bit counts, zero
cases, highest/lowest set bits and rotations follow the pinned source. Unsigned
arithmetic preserves Kotlin wrapping and masked shifts without signed-overflow
or shift-width undefined behavior, including INT_MIN rotation counts. Float/Double
classification and raw/canonical bits use the actual Operator.cpp implementations,
Primitives.kt NaN definitions and scalar reinterpret contracts. Companion from_bits
functions call private source intrinsic implementations. Native GCUnsafeCall and
TypedIntrinsic compiler metadata/exported Native ABI are not established here.

HashMapFunctions.hpp/.cpp translate HashMap.kt:599-601 computeHashSize and
computeShift in the existing private-companion namespace convention. Sizing uses
capacity.coerceAtLeast(1), wrapping multiplication by3, and the actual highest-bit
operation; shift uses the actual leading-zero operation plus1. These are source
algorithms, not a replacement HashMap. The917-line concrete HashMap, storage,
hash/equality, view and iterator contracts remain untranslated. Its source
AbstractMutableCollection and AbstractMutableSet bases have been opened; the
AbstractCollection/AbstractSet dependencies must also be translated. Fresh-map
Grouping operations and byFqNameParts remain absent, followed by actual
InlineClassesSupport/classifier/cache/frame consumers.

CMakeLists.txt:52-53 registers both implementation units on KotlinxCoroutinePass,
kxs-inject and kxs_codegen_test; :85 registers kxs_numbers_test. Native-OFF
LLVM23.1.2 configure and all four affected target builds succeeded. Strict debug
-Wall/-Wextra/-Werror ASan/UBSan compile and execution, strict release syntax,
six focused CTests and the existing actual LLVM module-generation fixture pass.
The numeric fixture covers all single-bit positions, zero/sign bits, extreme
rotation counts, wrapping map sizing, NaN payload/canonicalization, negative zero,
infinity and finite limits. No warnings are suppressed. This is scalar dependency
and existing LLVM fixture evidence, not full coroutine/Native/MLX acceptance.

Both final deep scans completed. Restoring Native/Wasm collection sources expands
the compiler corpus from673 to695 sparse files:266/7163 matched bodies,
125/1617 types,0.28 body similarity,10 scoring failures,190 paired units/279 target
files. Scope changed; these totals do not measure improvement against the earlier
corpus. Numbers reports6/20 explicit bodies,0/0 types,0.05 body score,32 target
bodies. Fourteen Int/Long receiver and Float/Double companion bodies remain
unmatched against the actual C++ overload/namespace forms. The extractor records
Kotlin extension receivers as parents (symbol_extraction.cpp:662-688); source
emission also falls back on function nodes. Target parser errors are absent, while
normalized source logic/text is provisional. Do not infer absent implementation
or completed parity solely from that score.

The scanner explicitly rejects HashMap.kt [kotlin.collections] versus
HashMapFunctions.hpp [kotlin.collections.hash_map.detail] with IDENTITY_MISMATCH.
The two private sizing bodies therefore have no certified match count, and the
HashMap source file remains missing in the oracle. This is an observed namespace
pairing limitation, not permission to report full HashMap parity. Full coroutine
root remains663/2918 bodies,178/560 types,0.24 similarity,12 failures,409 paired
units/608 target files. Generated evidence and priorities remain under
build/source-continuation/{compiler-source-distance,library-source-distance}.
The full transliteration/state-machine goal remains active.

## Grouping destination operations — 2026-10-08

Continuation from `e5392502` translates Grouping.kt's public interface and all
five destination-taking bodies: aggregateTo, both foldTo overloads, reduceTo and
eachCountTo. Grouping.hpp:71,93,119,132,143,156 exposes those contracts. Public
Kotlin generics remain header templates; private boxing/projection helpers are
necessary for the public generic variance and reuse existing collection codecs.
The key out-type exposes genuine supertypes while the source element type remains
invariant. MutableMap<in K,R> destinations accept real key supertypes and preserve
R's invariant type. Mutation goes through the public typed put method, decoding
the projected key through its existing codec; protected map mutation dispatch
was not made public. The original destination is returned by reference.

The aggregation loop follows source order: iterate, select key, look up current
accumulator, distinguish missing key from present-null, call operation, store
result. Fold selects an initial value only for a new key; reduce uses the first
element only for a new key. NullableMapValue's existing nullable representation
preserves pointer/shared/optional/Any nulls without an extra nullable layer.
Non-null accumulator casts use bad_any_cast in the private C++ codec boundary.
Count uses uint32 addition/bit_cast to preserve Kotlin Int wrapping without C++
signed overflow; Primitives.kt:975-976 declares PLUS and IntrinsicGenerator.kt:
560-567 selects ordinary LLVM add for integer inputs. This does not translate the
whole intrinsic generator or establish source Native exception representation.

The four fresh-map operations aggregate/fold/fold/reduce remain absent until
mutableMapOf's concrete dependency is translated. Source Maps.kt:85 constructs
LinkedHashMap; Native/Wasm HashMap.kt:917 aliases LinkedHashMap to the actual
insertion-ordered HashMap implementation. That917-line source, its array/hash/view
and iterator contracts, groupingBy adapters and actual enum values input remain
required for byFqNameParts. No std::map substitute, fake map implementation or
placeholder was introduced. InlineClassesSupport/classifier/cache/frame work
continues after these source dependencies.

Native-OFF CMake configure and kxs_grouping_test build pass. Strict debug
-Wall/-Wextra/-Werror ASan/UBSan compile and execution pass. Strict release header
syntax passes when included from a translation unit; compiling the header as
main initially triggered pragma-once-outside-header, corrected by checking its
actual include form without suppressing the warning. Five focused CTests pass.
The new fixture executes source iterator/key covariance and nullable/non-null
accumulator conversions. Eight explicit instantiations compile the destination
algorithms against actual abstract MutableMap interfaces: int, borrowed pointer,
shared owner, Any, optional, projected Any keys, reduce and count. Those map
algorithms are NOT executed because concrete source maps are still untranslated.
The covariance provider is an ordinary fixture, not an IR descriptor/classifier.
This is bounded source/type evidence, not complete coroutine/Native/MLX acceptance.

Both final-root deep scans completed. Compiler source remains673 sparse files:
253/6847 bodies,111/1575 types,0.31 average,5 scoring failures. Grouping:5/9 explicit
bodies,1/1 type,0.08 reported body score. Target has9 bodies/5 types including
representation bridges. Kotlin emission falls back on interface/function/annotation
nodes, has generated parse errors and normalized logic/text0; target parse errors
are absent. Thus the score is provisional. Full coroutine root:663/2918 bodies,
178/560 types,0.24,12 failures,406 paired units/603 target files. The reports,
missing inventories and priorities in build/source-continuation/{compiler-source-distance,
library-source-distance} remain the current oracle; neither root is complete.

CMakeLists.txt:83 registers the header-only dependency's focused target. The
production catalog does not yet call Grouping or its destination algorithms;
this checkpoint does not claim that wiring. API rows and continuation evidence
are updated. The full translation/state-machine goal remains active.

## Native primitive catalog and runtime-name dependency — 2026-10-08

Continuation from `6f9875bf` translates InlineClasses.kt:50-70 into
InlineClasses.hpp:8-18 and InlineClasses.cpp. The ten KonanPrimitiveType values
retain their source ClassId and BinaryType.Primitive constructor properties.
Enum property getters follow the existing Variance convention; their references
borrow private, retained catalog values. CHAR maps to SHORT while remaining a
distinct ClassId and Primitive box. NonNullNativePtr maps to POINTER and Vector128
to VECTOR128 through the actual KonanFqNames operations. No LLVM shape-based
classification or fabricated IR declaration was introduced.

KonanFqNames.hpp:14/.cpp translate all26 scalar object properties and the three
source constants. Local-static getters preserve retained name values; C++ keyword
collisions use thread_local_name/volatile_name. gcUnsafeCall is computed through
NativeRuntimeNames::Annotations::gc_unsafe_call_class_id(), not an independently
guessed string. NativeRuntimeNames.hpp:7,10/.cpp translate only the two private
package properties and GCUnsafeCall annotation ClassId (source:11-12,49); the
other atomic/annotation/callable/map APIs remain absent. InternalKotlinNativeApi
metadata and compiler opt-in behavior remain untranslated.

The new pre-main catalog test exposed a real cross-unit initialization defect
in the preceding PrimitiveType implementation: the executable aborted with
"Class name must not be root: kotlin" because the catalog read unconstructed
builtin names. konan_global_before.log records that failure. PrimitiveType.hpp
now has a minimal constexpr literal/atomic constructor; the eight instances are
constinit. Their owned properties are constructed/published on first access in
PrimitiveType.cpp, with losing candidates destroyed and winning storage freed
by the instance destructor. This deliberate C++ initialization adaptation is
marked NOTE(port); it changes eager name-property construction timing. It retains
canonical instance/name references and the source FqName PUBLICATION behavior.
No alternate state machine/runtime is involved. Source NUMBER_TYPES and generated
enum APIs still require translation.

CMakeLists.txt:48-50 registers the new units on all three production LLVM targets;
:83 registers kxs_konan_primitive_test. Native-OFF Release configure and all five
affected target builds pass using LLVM23.1.2. Strict release syntax and strict
ASan/UBSan debug compiles pass. Four focused CTests pass. Fresh sanitizer runs for
both primitive fixtures pass, including pre-main catalog construction and the
separate fixture's16-reader first-publication race. Existing actual LLVM module
codegen/verification also passes. Compiler metadata tests establish neither
actual IR classifier execution nor the two complete Native/MLX acceptance paths.

Both final-root deep scans completed after the initialization repair. Restoring
core/compiler.common.native/name expands the sparse source corpus669→673 files.
Compiler:248/6847 bodies,110/1575 types,0.31 average,5 scoring failures.
InlineClasses:0/30 explicit bodies,1/3 types,0.00; eight target bodies implement
only enum properties/constructors. KonanFqNames:0/0 explicit source bodies versus
26 target getters,1/1 type; scorer forces0 because it inventories properties
without getter bodies. This is the added scoring failure, not a completed-body
claim. NativeRuntimeNames:0/2 bodies,2/3 types,0.00. Generated emission falls back
on enum/object/property nodes and reports parse errors; target parse errors are
absent. Normalized logic/text remain0 and provisional. Full library:663/2918
bodies,178/560 types,0.24,12 failures,404 paired units/601 target files. Evidence
and priority inventories are under build/source-continuation/{compiler-source-distance,
library-source-distance}; these measured criteria do not certify completion.

Next: actual byFqNameParts grouping and its concrete collection dependencies
(Map interface exists; Grouping.kt, Maps.kt and HashMap/LinkedHashMap bodies are
not translated), then InlineClassesSupport and actual IR classifier hooks/cache
keys consumed by frame/call lowering. All30 InlineClasses explicit algorithms,
the two missing support types, remaining name/enum APIs and complete executable
acceptance remain required. The full translation/state-machine goal stays active.

## Primitive-name catalog dependency — 2026-10-08

Continuation from `50c511a2` translates `PrimitiveType.kt:11-28,34-58` into
`org/jetbrains/kotlin/builtins/PrimitiveType.hpp:13` and its matching `.cpp`.
All eight named instances, primitive/array Name properties, lazily published
FqName properties and both exact short-name lookups follow the pinned source.
Nullable lookup results borrow canonical immutable instances. Property-bearing
Kotlin enum instances use a concrete singleton class; generated enum APIs
(name/ordinal/values/entries/valueOf/comparison) remain untranslated. NUMBER_TYPES
still requires actual setOf/LinkedHashSet implementation; no alternate set API
or placeholder was added. StandardNames.hpp:8 and .cpp translate only the two
built-in package properties at StandardNames.kt:73-81, with static getters to
avoid translation-unit initialization order. The remaining object is incomplete.

Lazy getters also reference SafePublicationLazyImpl's actual CAS algorithm,
LazyJVM.kt:114-133. Multiple candidates may compute, but all callers receive the
winner. Selected libc++ rejects atomic<shared_ptr>; the final implementation uses
atomic owned FqName pointers. unique_ptr frees losing candidates; the instance
destructor frees the two winning boxes. Getter references are borrowed for the
instance lifetime. This is internal C++ compiler metadata ownership, not a Native
GC object/frame representation or a replacement coroutine state machine.

CMakeLists.txt:46-47 registers both source units on kxs-inject,
KotlinxCoroutinePass and kxs_codegen_test; :80 registers the focused contract test.
Native-OFF Release configure and all four target builds pass with LLVM23.1.2.
Strict -Wall/-Wextra/-Werror debug sanitizer compile and release syntax pass.
Three focused CTests pass; primitive_type_sanitized passes ASan/UBSan. The new
fixture races16 first readers, checks published identity for both properties,
all eight names/array names and exact rejection of qualified/unsigned/incorrect
names. Existing actual LLVM codegen fixture emits/verifies its module. These
checks do not establish actual IR classification or either complete acceptance
path in docking_ring.md.

Both final-root deep scans completed after the storage repair. Restoring the
pinned builtin directory expands the sparse Kotlin corpus661→669 files; this
changes the measurement scope. Compiler root:248/6845 bodies,106/1568 types,
0.34 average,4 scoring failures. PrimitiveType:2/2 explicit bodies,1/1 type,
0.08 reported body similarity; target has8 bodies. StandardNames:0/19 bodies,
1/2 types,0.00. Both generated emitters fall back on the enclosing enum/object
and report generated parse errors; target parse errors are absent. Their
normalized logic/text scores are0 and provisional. The inventories do not count
missing enum-generated/property/set contracts as missing explicit bodies, so
2/2 cannot imply complete PrimitiveType translation. Evidence is in
build/source-continuation/compiler-source-distance/{scan.log,
deep_symbol_inventory.txt,deep_transliteration_evidence.txt,port_status_report.md}.
Full coroutine root:663/2918 bodies,178/560 types,0.24,12 scoring failures,
400 paired units/594 target files; ChannelFlow and Channels remain unchanged.

Next source consumers are KonanFqNames and KonanPrimitiveType in InlineClasses.kt,
including its source CHAR→SHORT mapping and structural byFqNameParts grouping.
Their actual collection dependencies, InlineClassesSupport, real IR classifier
hooks/cache keys and VariableManager/frame/call consumers remain required. No
fake IR descriptors or LLVM-pointer shape classification was introduced. The
full translation/state-machine goal remains active.

## Qualified-name and class-ID classification dependencies — 2026-10-08

Continuation from48959b4a restores core/names at the same pinned Kotlin revision
and translates the scalar contracts in FqName.kt, FqNameUnsafe.kt and ClassId.kt.
name/FqName.hpp:14, FqNameUnsafe.hpp:15 and ClassId.hpp:16 expose the concrete
types. Implementations stay in the matching .cpp files. Names keep UTF-16 units,
backtick-aware last-dot parsing, cached child short names and parents, root
diagnostics, both prefix boundary checks and source structural hashing/equality.
Unsafe-to-safe conversion preserves the source's cache-dependent isSafe behavior,
including the fact that conversion does not validate the text.

Like existing Name, C++ names use immutable value handles. A private shared
backing retains the unsafe text/parent/short-name caches; a safe view uses the
same backing rather than creating a strong ownership cycle. This deliberate
internal representation adaptation is marked in FqNameUnsafe.cpp:10. It is not
a Native object/frame representation. It does not establish reference identity
of actual Kotlin/Native objects or coroutine/frame interoperability.
The companion ROOT field maps to a static root() getter with local initialization,
avoiding C++ translation-unit initialization order. A catalog-style ClassId
initializer before main exercises that path; the getter keeps the canonical
root backing. No global constructor may dereference a not-yet-built root.

ClassId.cpp translates nested/outer/outermost IDs, locality propagation, slash
escaping, qualified text, top-level construction and backtick-aware from_string.
Data-class components/copy/equality/hash are tied to the actual IR generator.
The initial boolean hash term was corrected after reading
DataClassMembersGenerator.kt:178-221,269-307 and Boolean.kt:66-67: source Boolean
hashes are1231/1237. Source assertions map to NDEBUG/compiler logic_error, with
UTF-8/WTF-8 diagnostics. The private encoder is compiled only with its assertion,
resolving a fresh release unused-function error without suppressing warnings.

tools/kxs_inject/CMakeLists.txt:42-45 registers Name/FqName/FqNameUnsafe/ClassId
on all three production LLVM targets. The new standalone contract test at :78
ports all64 prefix cases from FqNameUnsafeTest.java and adds quoted-dot parsing,
cached vs reparsed child names, root failures/empty child, safe-view backing
identity, UTF-16 retained values, escaped class-ID round trips, nested/local
propagation, source Boolean hash difference, data-class copy/components and the
non-ASCII assertion diagnostic. Strict compile with/without NDEBUG, Native-OFF
plugin/injector/helper builds, existing LLVM codegen fixture, CTest and ASan/UBSan
pass. No actual IR class or classifier is fabricated in these tests.

Both root deep scans completed. FqName:12/14 bodies,1/1 type,similarity0.71;
FqNameUnsafe:15/17,1/1,0.22; ClassId:8/9,1/2,0.57. No target name parse errors.
ClassId's nested extension escapeSlashes is present as a private namespace helper;
the lexical-scope name matching does not pair it. Its locality opt-in annotation
and compiler warning policy remain untranslated. FqName.pathSegments/fromSegments
and FqNameUnsafe.pathSegments/collectSegmentsOf remain absent until their actual
List/ArrayList dependencies are translated; no vector API or placeholder was
substituted. The unused SPLIT_BY_DOTS regex cache is also untranslated. Source
emission limitations and unrelated StandardClassIds parser errors remain visible.

Current sparse compiler root:246/6807 bodies,104/1554 types,similarity0.36,
4 scoring failures,661 source files after restoring9 core/names Kotlin files.
These counts have a different source scope from the previous652-file corpus.
Full kotlinx.coroutines remains663/2918 bodies,178/560 types,0.24,12 failures.
Evidence is under build/source-continuation/{compiler,library}-source-distance.
Next translate the primitive catalog and remaining list/metadata dependencies,
InlineClassesSupport/IR classification, the structural type cache and frame
consumers. Generation-state entry and both complete MLX/Native acceptance paths
remain unfinished. Goal active.

## Binary classification result and lazy sequence contracts — 2026-10-08 (historical 43eb5c38 receipt)

Continuation from 6cf05519 translates native/base/src/main/kotlin/org/jetbrains/
kotlin/backend/konan/BinaryType.kt:8-20. BinaryType.hpp:11,19,31,41 and
BinaryType.cpp:11,15 provide the nine primitive kinds, sealed result family,
Primitive's value, Reference's actual typed sequence/nullability and primitive
extraction. The internal common family uses a type-erased C++ base with typed
Reference<T> variants, allowing Primitive to participate without a fabricated
Nothing type/object. Source ownership is retained with supplied shared sequence
handles. Element pointers remain borrowed. These adaptations are marked in source.

kotlin/sequences/Sequence.hpp:40,49 translates stdlib Sequence.kt:21-28.
The abstract factory and typed iterator bridge follow this port's existing
Iterable/Iterator covariance boundary and transfer each created iterator to the
caller. The sequence remains potentially infinite and lazy; no vector replacement,
snapshot or unconditional reusable-iteration policy is introduced. Public generic
code stays in its header; non-generic BinaryType operations live in the .cpp.

tools/kxs_inject/CMakeLists.txt:42 registers BinaryType.cpp on all three LLVM
consumers. The new contract test at :74 verifies all nine primitive results,
shared provider identity without iteration, mutation observed at iteration time,
typed/covariant iterator element identity, provider retention/release, borrowed
elements surviving provider destruction and propagation of constrained-once
provider errors. Its fixture uses ordinary borrowed C++ objects, not fabricated
IR classes or a replacement classifier. Strict -Wall -Wextra -Werror compile with
and without NDEBUG passes. Native-OFF plugin/helper builds, the existing LLVM
code-generation fixture, CTest and ASan/UBSan pass. These prove result/sequence
contracts only; actual classification and frame integration remain unfinished.

Both root deep scans completed. BinaryType:1/1 explicit body,4/4 types,similarity
0.07. Its emitted normalized logic is0 because the sealed class, enum and
star-projected when function are unsupported emission nodes; exact fallback
spans are preserved in deep_transliteration_evidence.txt. Target parsing has no
errors. Sequence:0/0 source bodies,1/1 type,forced0 score: the body-only criterion
penalizes the C++ covariance bridge against the abstract Kotlin iterator method.
This is not evidence of a missing source iterator implementation. Current sparse
compiler root:211/6715 bodies,101/1542 types,similarity0.34,4 scoring failures,
652 source files after restoring the48-file stdlib collection directory at pinned
fee29910d8dddd2b1f7b44036c00533cee493351. Earlier604-file measurements have a
different source scope. Full kotlinx.coroutines root remains663/2918 bodies,
178/560 types,0.24,12 scoring failures. Reports and inventories are under
build/source-continuation/{compiler,library}-source-distance.

KonanPrimitiveType's real ClassId/FqName catalog, InlineClassesSupport and
IrTypeInlineClassesSupport still need translation before DataLayout's IR-derived
type mapping is implemented. Runtime's IR caches must preserve source structural
equality. VariableManager and full frame/root/call/exception consumers,
generation-state entry and both complete standalone MLX/Native handoff acceptance
paths remain open. No alternate classification/state machine is supplied.

## Actual debug bridge source dependencies — 2026-10-08 (historical 6cf05519 receipt)

Continuation from c233adf7 translates upstream
kotlin-native/llvmDebugInfoC/src/main/cpp/DebugInfoC.cpp:253-286.
DebugInfoC.hpp:16 and DebugInfoC.cpp:14 define auto-variable creation;
the remaining three operations create parameter metadata, an empty expression
and an inserted declaration. Source default debug options are preserved by
calling the actual selected LLVM DIBuilder overloads. Context-owned metadata,
builder and storage remain borrowed. The signed expression operands are copied
to uint64_t in order, as upstream does. C exports use snake_case functions in
the existing compiler namespace; the expression pointer is const because the
source only reads it. Both adaptations are explicitly marked in the header.

tools/kxs_inject/CMakeLists.txt:42 registers the bridge on kxs-inject,
KotlinxCoroutinePass and kxs_codegen_test. DebugInfoContractTest.cpp constructs
real local/parameter metadata and an LLVM function with storage and a terminator.
It checks metadata/storage identity, source defaults, parameter numbering,
nonempty/empty DWARF expressions, declaration insertion before ret and
LLVMVerifyModule after DIBuilder finalization. LLVM23 emits debug records here;
the bridge uses the same insertDeclare operation as source. The test is registered
at CMakeLists.txt:74. Strict -Wall -Wextra -Werror compile, Native-OFF plugin/helper
builds, the existing code-generation fixture, the new CTest and ASan/UBSan pass.
The first multi-target build regenerated CMake and then could not resolve the
new test target; a fresh explicit target build passed. These are compiler/debug
dependency checks, not either complete runtime/MLX acceptance path.

The prior source-absence claim was incorrect: tmp/kotlin's working tree is sparse.
Pinned Git HEAD fee29910d8dddd2b1f7b44036c00533cee493351 already contains
native/base/.../BinaryType.kt and InlineClasses.kt, compiler/ir/backend.native/
.../IrTypeInlineClassesSupport.kt and kotlin-native/llvmDebugInfoC. Those paths
are now checked out at the same revision; no upstream code was modified and
existing untracked libraries/stdlib/native files remain. IrType binary
classification calls the actual InlineClassesSupport contract; it must not be
replaced by pointer-shape inference. The broad Git grep was deliberately stopped
after verifying its live process and locating the exact source paths.

Deep scans completed for the C++ debug source root, full kotlinx.coroutines
root and current sparse tmp/kotlin root. The debug scan reports no paired files: codebase.hpp:1505 rejects namespace
context because the source conversion helpers occupy namespace llvm while the
binding functions are global; target bindings occupy the compiler namespace.
The direct function comparison inventories28 source and4 target bodies but its
strict C++ name policy does not map PascalCase C exports to snake_case. Upstream
DEFINE_SIMPLE_CONVERSION_FUNCTIONS invocations also produce parser missing-semicolon
diagnostics. Target debug source parsing has no errors. No matched-body score is
claimed for this bridge. Full library:663/2918 bodies,178/560 types,similarity0.24,
12 scoring failures; ChannelFlow18/19,6/6,0.25 and flow Channels12/12,0.22 remain.
The sparse compiler working-tree scan reports196/6201 bodies,88/1441 types,
similarity0.36,3 scoring failures and604 source files. It does not cover the full
pinned Git tree. CodeGenerator remains73/145 bodies,6/15 types,similarity0.24;
ContextUtils38/51,31/36,0.47; LlvmUtils21/49,7/10,0.20. Reports, inventories and direct comparison are in
build/source-continuation/{debug,library,compiler}-source-distance. Scoring failures and
unsupported source emission require investigation; these are not completion
measurements. VariableManager/debug-location consumers, DataLayout's actual
classification/cache, full frame/root/call/exception translation and both complete
runtime acceptance paths remain open.


## LLVM aggregate constants and RuntimeAware type contracts — 2026-10-08

Continuation from 62a3a77b translates LlvmUtils.kt:45-54,82-110.
ConstArray/Zero at LlvmUtils.hpp:22,53 and LlvmUtils.cpp:34,55 retain actual LLVM
constant values and source types. ConstArray retains the shared source element
list and its descriptor owners, validates element types under the documented
NDEBUG assertion mapping, preserves the source diagnostic, and emits its array
eagerly. Later list mutations do not re-emit the constant. Zero uses the actual
LLVMConstNull for its supplied type. The private ConstantValue implementation
backs const_value at LlvmUtils.cpp:139; its LLVM value remains borrowed and the
source constant predicate is retained.

RuntimeAware extensions at LlvmUtils.hpp:68-78 and LlvmUtils.cpp:12-25 return
actual runtime TypeInfo/ObjHeader/ArrayHeader types, a LlvmRetType with the explicit
source isObjectType=true flag, and source LLVM undef for Nothing's unreachable
value. No pointer-shape inference or alternative result ABI was added.
extract_const_unsigned_int retains source constant validation, LLVM zero extension
and the Long bit representation. The return descriptor is forward-declared in
the header to avoid the existing LlvmAttributes/LlvmUtils include cycle; the
implementation includes its actual definition.

Fresh strict -Wall -Wextra -Werror compilation passed with/without NDEBUG.
KotlinxCoroutinePass/kxs_codegen_test rebuilt with LLVM23.1.2 and Native runtime
OFF; the existing LLVM fixture verified/emitted its module. The bounded
aggregate_contract harness passed ASan/UBSan: retained element-list identity,
array order and eager snapshot after mutation, empty arrays, typed mismatch
message, integer/pointer/struct zero constants, arbitrary constant identity,
8/32/64-bit unsigned extraction, runtime type identity, actual object-result
metadata and the upstream Nothing undef. Earlier constant/lazy-import checks
remain in that fixture. The input is an LLVM declaration/type fixture, not actual
Native runtime execution or either complete MLX/shared-state-machine path.

Scoped ast_distance --deep completed. LlvmUtils:21/49 matched explicit bodies,
7/10 types,similarity0.20,target44 bodies. Target parsing reports no errors;
unsupported source emission/generated parse errors keep criteria provisional
(normalized logic0.174421, supported span0.085799). Full-root measurements remain
required and open.

Tracing VariableManager.kt and DataLayout.kt confirms that actual variable and
frame translation still needs computePrimitiveBinaryTypeOrNull/binaryTypeIsReference,
IR-keyed type caching and the debug bridge contracts. The working-tree search
found calls but no definitions because the source checkout is sparse; the
earlier missing-snapshot conclusion is corrected by the current receipt above.
No enum classification, pointer-keyed cache or callback allocator was guessed
from those call sites. The restored source contracts still require translation
and integration beyond the four debug operations now implemented. Other LLVM/IR source work remains available;
this is not an overall goal blockage. Frame/root/call/exception consumers,
generation-state entry, Runtime loading/caches and both full acceptance paths
remain unfinished. Evidence is under build/source-continuation/llvm-source-distance.

## LLVM type/constant and Struct source dependencies — 2026-10-08 (historical 62a3a77b receipt)

Continuation from 0ed4ddd4 translates ContextUtils.kt:264-298,514-571 and
LlvmUtils.kt:56-82,251-254. Nine concrete ConstInt/ConstUInt/ConstChar/ConstFloat
classes start at ContextUtils.hpp:293 and ContextUtils.cpp:12. They retain their
source values and actual LLVM constants; signed integers convert through signed
64-bit values before the LLVM C API, and Char uses char16_t. No runtime box or
substitute object representation was introduced.

CodegenLlvmHelpers at ContextUtils.hpp:437 and ContextUtils.cpp:390 initializes
the eleven source LLVM types, including target-data-dependent intptr and vector128.
Source constant factories and raw-value wrappers, intptr sign extension,
null/boolean/integer singleton properties and the ConstPointer null descriptor
are implemented. Struct type construction at ContextUtils.cpp:790,792,804 and
constant struct construction at :798 retain packing, element order and flexible
array replacement. Empty vararg forms remain available through explicit empty
vector defaults. Computations and lazy storage remain in .cpp.

Struct at LlvmUtils.hpp:20 and LlvmUtils.cpp:18,44 preserves the actual shared
source element-list identity, retains element descriptors and emits the LLVM
aggregate eagerly. Later mutation of a supplied list does not re-emit that
aggregate, matching the source property initialization. Null elements use the
expected field type's zero constant. Source mismatch/count diagnostics are
translated. NOTE(port) records that source assertions follow C++ NDEBUG and
throw compiler logic_error failures when enabled. to_type_string at
LlvmUtils.cpp:11 retains the source null marker and explicitly owns the LLVM
print buffer through string construction/failure.

Strict -Wall -Wextra -Werror syntax compilation passed with and without NDEBUG.
KotlinxCoroutinePass/kxs_codegen_test rebuilt with LLVM23.1.2 and Native runtime
OFF; the existing LLVM fixture verified/emitted its module. The bounded
constant_contract harness passed ASan/UBSan for integer extremes, unsigned byte,
UTF-16, floating negative zero/infinity/NaN, 32/64-bit intptr conversion, literal
identity, zero-argument forms, shared element-list mutation versus emitted
snapshot, typed null fields, packed/flexible-array structure and invalid-type
messages. It also retains the previous public lazy-import checks. These LLVM
declaration/type fixtures prove neither complete Native runtime execution nor
MLX/shared-state-machine acceptance.

The fresh scoped deep comparison reports ContextUtils:38/51 matched explicit
bodies,31/36 types,similarity0.47; LlvmUtils:19/49 bodies,5/10 types,0.19.
Target parse errors were investigated with the tool's bundled C++ grammar.
A bounded parser probe reports missing type identifiers on two pre-existing
const-reference defaults spelled '= {}' in the previous committed headers.
Equivalent explicitly typed empty-vector defaults parse successfully. Both
updated targets now report no parse errors in the refreshed scan; source class
emission/generated parse errors still keep normalized criteria provisional.
Full-root compiler/library measurements remain required and open.

The next consumers remain actual frame/prologue construction and allocation,
root updates, VariableManager, public object-result call slots and exception
emission. Generation-state entry, Runtime bitcode loading/IR-keyed caches and
remaining ContextUtils/LlvmUtils algorithms are unfinished. Both complete
acceptance paths remain open. Evidence is under
build/source-continuation/llvm-source-distance (ignored build artifacts).

## Lazy runtime bindings for frame/root consumers — 2026-10-08 (historical 0ed4ddd4 receipt)

Continuation from 63eeae2f translates all forty source lazy runtime-function
properties at ContextUtils.kt:445-504. ContextUtils.hpp:377-451 exposes the
thirty-eight public getters; the two type-provider getters remain private.
ContextUtils.cpp:193 contains private synchronized lazy storage, with getter
bodies at :403-681. Source function names and explicit object-result flags are
preserved for external RC references, Native/ObjC conversion and continuation
operations, safepoints/mark traversal, volatile heap-reference operations and
array element addresses. LLVM handles stay borrowed; actual callable descriptors
and their attribute providers are retained by the helper.

CodegenLlvmHelpers construction at ContextUtils.cpp:317 now requires an explicit
should_optimize policy from the enclosing compiler boundary. No default was
invented. Thread-state getters at :445,452 select the source optimized or _debug
symbol names. Lazy imports use the existing actual module/type/attribute import
operation; failed evaluation can retry and repeated getters preserve descriptor
identity. Private import helpers and getters are const, while their synchronized
lazy storage retains the source property behavior. This supplies the missing
UpdateVolatileHeapRef and thread-state/safepoint binding dependencies without
inventing root-update or frame-call adapters.

Fresh strict -Wall -Wextra -Werror syntax compilation and builds of
KotlinxCoroutinePass/kxs_codegen_test passed with LLVM 23.1.2 and Native runtime
OFF. The existing code-generation fixture verified/emitted its module. A bounded
lazy_import_contract harness passed ASan/UBSan: it checks all thirty-eight public
getters against source symbol names/object-result flags, imported LLVM type and
nounwind metadata, descriptor identity, deferred imports, failed lookup/retry,
and both optimized/debug thread-state selections. The two private type-provider
getters are compiled but not executed by this harness. Input contains declaration
fixtures only, not actual Native runtime implementations. No full Native or MLX
shared-state-machine execution is claimed.

Scoped ast_distance --deep completed after the source changes. ContextUtils
remains 16/51 matched explicit bodies,22/36 types,similarity0.21, with 113 target
function bodies. Kotlin property initializers and C++ getters are counted
differently; the unchanged matched-body count does not erase the added source
properties or prove them complete. Source emission remains provisional due to
unsupported classes/generated parse errors; target parsing has no errors.
Full-root measurements and both complete acceptance paths remain open.

Next source dependencies remain the constant/type helpers, actual frame
construction/allocation, root-update consumers, VariableManager, public
object-result slot handling and exception emission. The generation-state entry,
Runtime bitcode loading and IR-keyed caches are also incomplete. No substitute
runtime, callback-based frame allocator or alternate state machine was added.
Evidence is under build/source-continuation/llvm-source-distance.

## Compiler Runtime metadata and RuntimeAware wiring — 2026-10-08 (historical 63eeae2f receipt)

Continuation from b5b16a95 translates Runtime.kt:14-23,27-122 into
Runtime.hpp:12,123 and Runtime.cpp:49-218. This is the compiler's LLVM metadata
object, not a Native runtime implementation. Required and optional named types
retain source lookup order, including the touch-global fallback. Source target
and data-layout snapshots, actual LLVM target data, eager header/frame types,
synchronized lazy ObjC/block types, ABI size/alignment/offset calculations,
pointer properties, string-header extra size, object alignment and byte order
are implemented. Created target data is owned and disposed; LLVM module/context
and types stay borrowed. All algorithmic bodies are in Runtime.cpp.

CodegenLlvmHelpers now implements RuntimeAware at ContextUtils.hpp:317 and
ContextUtils.cpp:228. Its explicit compiler boundary takes a borrowed Runtime
metadata object instead of a raw runtime-module argument. Layout/target copying
uses the source metadata snapshots, and import_rt_function at :312 uses the
metadata's actual module. CMake registers Runtime.cpp on the three LLVM consumer
targets. No Kotlin compiler invocation or Native runtime link was added.

The phase-context/diagnostic bitcode-loading constructor and the two IR-keyed
caches at Runtime.kt:24-25 are still missing. No pointer-keyed replacement cache
was introduced. The complete generation-state constructor, frame allocation,
root-update consumers, VariableManager, public object-result calls and exception
emission remain unfinished. This source advance supplies frameOverlayType,
pointerSize and pointerAlignment dependencies; it does not complete their callers.

Fresh -Wall -Wextra -Werror syntax compilation passed. KotlinxCoroutinePass and
kxs_codegen_test rebuilt under LLVM 23.1.2 with Native runtime OFF, and the existing
code-generation fixture verified/emitted its module. The bounded runtime_contract
harness passed ASan/UBSan using LLVM layout/type fixtures: 32-bit big-endian and
64-bit little-endian layout, touch-global fallback, missing required/optional
types, metadata snapshots, ObjC shapes/identity, lazy failure/retry and borrowed
module survival. The updated import_contract passed ASan/UBSan and verifies the
actual RuntimeAware object identity plus the previous declaration-import behavior.
These fixtures contain no actual Native runtime bodies and prove neither required
MLX/shared-state-machine acceptance path.

Scoped ast_distance --deep completed against the LLVM Kotlin source directory and
kotlinc_native_ref target root. Runtime:7/7 matched explicit function bodies,
2/2 types, body similarity0.36; ContextUtils:16/51 bodies,22/36 types,0.21.
Runtime's complete property/constructor/cache parity is not represented by the
7/7 count. deep_transliteration_evidence.txt explicitly reports unsupported class
emission, generated parse errors, zero supported span/normalized logic and a
provisional score; target parsing reports no errors. Full-root compiler/library
measurements and both complete runtime acceptance paths remain open. Evidence is
under build/source-continuation/llvm-source-distance (ignored build artifacts).

## LLVM runtime-function imports — 2026-10-08 (historical b5b16a95 receipt)

Continuation from df67badd translates ContextUtils.kt:329-365,411-441,573-579
and LlvmUtils.kt:128-134. CodegenLlvmHelpers at ContextUtils.hpp:316 and
ContextUtils.cpp:193 binds actual compiler-owned LLVM context/output/runtime
modules. The module boundary supplies the runtime module explicitly and does
not parse, invent or link a Native runtime. Layout and target are copied before
eager imports. Twenty-four source bindings retain actual symbol names and
explicit object-result flags for allocation, root updates, frame operations,
initializers/TLS and exception retrieval. Returned descriptors own their source
attribute provider while LLVM values remain borrowed.

Private import_function at :280 rejects duplicate/missing symbols, obtains the
actual global function type and copies declaration attributes in full. The
existing provider preserves filtered call-site attributes. import_memset at :293,
llvm_intrinsic at :301 and import_rt_function at :310 translate source naming,
signatures and enum attributes; llvm.trap retains cold/noreturn/nounwind.
get_global_function_type/get_global_type at LlvmUtils.cpp:115,117 delegate to
LLVMGlobalGetValueType. The complete generation-state constructor, RuntimeAware,
externalFunction/prototypes/dependency tracking and later lazy Native/ObjC
bindings remain untranslated. This LLVM-module entry is an explicit partial
compiler boundary, not a completed CodegenLlvmHelpers or Native pipeline.

Strict syntax compilation with -Wall -Wextra -Werror passed. KotlinxCoroutinePass
and kxs_codegen_test rebuilt with LLVM 23.1.2 and Native runtime OFF; the existing
code-generation fixture verified/emitted its module. The bounded import_contract
harness compiled with ASan/UBSan and exited zero. Its input is an LLVM declaration
fixture using source symbol names; it tests actual type/attribute copying,
object-result metadata independent of pointer shape, layout/target, intrinsic
contracts, duplicate/missing diagnostics and LLVMVerifyModule. It does not run
Native runtime functions, use an actual runtime bitcode module or prove Native
interop/frame ownership. Source/executable remain under
build/source-continuation/llvm-source-distance/import_contract.*.

The refreshed scoped deep scan exited zero: ContextUtils16/51 bodies,22/36 types,
function similarity0.20; LlvmUtils18/49 bodies,4/10 types,similarity0.19.
Generated-emission criteria remain provisional; full-root measurements remain
open. Existing CMake registration already includes these implementations, so
no source-list or new runtime-link dependency was added. Continue allocation/
root operations and VariableManager, public object-result calls and genThrow,
with actual Native imports supplied at the explicit interop boundary. Complete
standalone/MLX and Native handoff acceptance remain open; the full goal is active.

## LLVM module context and runtime annotations — 2026-10-08

Continuation from 61e3e6fe translates BasicLlvmHelpers from
ContextUtils.kt:300-326 into ContextUtils.hpp:289 and ContextUtils.cpp:155-191.
The LLVM boundary receives the actual compiler-owned context/module and
useLlvmOpaquePointers policy. The target triple and runtime annotation map
retain Kotlin's synchronized lazy snapshots. Annotations come from actual
llvm.global.annotations initializer operands: both source pointer branches,
null-initializer/empty-key filtering, grouped values and operand order are
preserved. Partial construction remains local so failed lazy initialization can
retry. The compiler continues to own every returned LLVM handle.

LlvmUtils.cpp:98,106 ports getAsCString and getOperands from
LlvmUtils.kt:118-125,358-359. The source null-termination requirement and error
text remain; nonterminated data is rejected rather than silently converted.
Private lazy storage stays in the implementation file. Existing CMake source
registration covers both edited implementation files; no new dependency is added.

Strict syntax compilation with C++20,-Wall,-Wextra,-Werror passed for both
implementations. KotlinxCoroutinePass and kxs_codegen_test rebuilt successfully
with LLVM 23.1.2 and Native runtime OFF. The existing code-generation fixture
verified/emitted its module. An LLVM-backed ASan/UBSan harness passed actual
annotation grouping/order, missing initializer filtering, absent/present lazy
snapshots, target-triple snapshot and nonterminated-string rejection; its module
also passes LLVMVerifyModule. Source/executable are under
build/source-continuation/llvm-source-distance/annotation_contract.*.
Only the opaque-pointer branch was exercised. The first harness used a zero-filled
empty-string constant that LLVMGetAsString did not expose as data; that input
triggered the preserved source requirement. The fixture now uses the actual
missing-initializer source branch. Zero-filled string representation compatibility
and the older typed-pointer execution path remain unverified.

The scoped deep scan was refreshed. Initially it classified ContextUtils as
MISSING_FILE despite the compiled types/bodies. Inspection of
 tools/ast_distance/include/porting_utils.hpp:311-331 shows the first provenance
payload is returned as written; explicit line-free port-lint source headers on
ContextUtils and LlvmUtils restored deterministic pairing while retaining all
per-function ranges. Final measurements are ContextUtils 12/51 bodies,21/36 types,
function similarity0.17; LlvmUtils16/49 bodies,4/10 types,similarity0.17.
The new ContextUtils properties are getters, not additional explicit Kotlin
function bodies. Generated-emission criteria remain provisional. No full-root
measurement or complete runtime acceptance is established by these scoped checks.

Continue CodegenLlvmHelpers/runtime imports, allocation/root operations,
VariableManager and public object-result call lowering. BasicLlvmHelpers is a
translated dependency; the public call path and complete state-machine/interop
paths remain unfinished. The full goal remains active.

## Escape-analysis lifetime and slot classes — 2026-10-08

Continuation from 7e9d2aec translates ContextUtils.kt:21-130 into
ContextUtils.hpp:9,108 and ContextUtils.cpp:8-142. All eight SlotType variants
and twelve Lifetime variants retain source classification, constructor properties,
singleton identities and diagnostic strings. Concrete nested classes preserve
parameter and array-lifetime types rather than reducing them to an enum.
Singleton slots remain borrowed. Each parameter-dependent lifetime owns the
slot object created by its source constructor. ParametersField and ParamsIfArena
share the same mutable parameter-index array; mutation through either view
remains visible to the other. Compiler-side storage adds no Native runtime or
Kotlin compiler dependency.

The private implementations stay in ContextUtils.cpp; the header contains the
actual type/API surface. kxs_inject/CMakeLists.txt:42 registers this dependency
in kxs-inject, KotlinxCoroutinePass and kxs_codegen_test. This supplies the
lifetime input needed by CodeGenerator.call but does not yet connect that public
call algorithm, VariableManager or Native frame/root emission.

Checks completed:

- ContextUtils.cpp strict C++20 syntax compilation with -Wall -Wextra -Werror
  exited zero.
- Root CMake regenerated and rebuilt KotlinxCoroutinePass and kxs_codegen_test
  in build/source-continuation with LLVM 23.1.2, Native runtime OFF. No
  kotlinc/konanc invocation or Native runtime link was introduced.
- The bounded C++ lifetime_contract harness compiled with AddressSanitizer and
  UndefinedBehaviorSanitizer and exited zero. It checks shared array identity,
  mutation through the slot view, retention after the external handle is reset,
  release after lifetime destruction and distinct allocated parameter slots.
  Its source/executable are under build/source-continuation/llvm-source-distance.
- The existing code-generation helper exited zero and verified/emitted its LLVM
  module. It does not exercise the unconnected public object-result call path.
- The scoped LLVM deep scan exited zero. ContextUtils now measures 12/51 bodies,
  20/36 types and function similarity 0.17. The twelve explicit Lifetime string
  bodies are matched; other ContextUtils/CodegenLlvmHelpers/import/initializer
  functions and types remain absent. Source-emission criteria remain provisional:
  translated text cosine 0.003401, AST cosine 0.221225, normalized logic 0,
  supported span coverage 0, unsupported fraction 1, generated parse errors yes,
  target parse errors no. These limitations do not establish full source parity.

The scan uses the same scoped command recorded below. Full-root compiler/library
measurements and both complete executable acceptance paths remain open. Next
source work is actual allocation/root operations and VariableManager, then public
object-result call selection and genThrow, with real LLVM/runtime imports from
ContextUtils and the Native compiler dependencies. The full goal remains active.

## Call/invoke selection and bit operations — 2026-10-08

Continuation from f98313b8 translates CodeGenerator.kt:87-92,850-886,966-998.
ExceptionHandler preserves None/Caller singleton types and the abstract Local
unwind-block getter in CodeGenerator.hpp:15. Its genThrow body remains absent
pending the actual call/runtime dependencies. CodeGenerator.cpp:239 translates
private call_raw: nounwind direct calls, caller propagation without a cleanup
landingpad, caller cleanup-use tracking, local unwind selection, the source
missing-handler diagnostic and call_success invoke/positioning. The enclosing
LLVM boundary borrows an explicitly supplied actual cleanup block; source
Native prologue/frame construction and cleanup epilogue are still untranslated.
The public object-result call/slot algorithm is absent, so call_raw has no
production caller yet and is not presented as wired exception lowering.

CodeGenerator.cpp:304-349 translates not/and/or/xor, zero/sign extension,
extension selection, truncation and left/arithmetic/logical right shifts.
Operator keywords receive trailing underscores. Private shift preserves the
zero-amount operand identity and constructs nonzero amounts with the actual
operand type and Kotlin Int-to-Long sign conversion. No Native roots or runtime
functions are inferred from pointer shapes.

Strict CodeGenerator.cpp syntax compilation passed with C++20, -Wall -Wextra
-Werror and the actual LLVM headers. KotlinxCoroutinePass and kxs_codegen_test
rebuilt successfully in build/source-continuation; the existing executable exited
zero and verified/emitted coroutine_codegen.ll. Those existing fixture checks
do not exercise private call_raw or all new bit operations. The refreshed scoped
LLVM deep scan exited zero and measures CodeGenerator at 73/145 matched bodies,
6/15 types and 0.24 function similarity (previously 61/145,2/15,0.19).
The same matching/emission limitations recorded in the following checkpoint
remain; this does not refresh full-root measurements or establish complete
runtime acceptance. Continue actual object-result slots, frame/root and exception
runtime bindings, VariableManager and the connected expression driver. The full
goal remains active.

## LLVM value operations and focused build repair — 2026-10-08

Continuation from bc4c1b51 translates CodeGenerator.kt:1014-1033 into
CodeGenerator.cpp:300-352 and the public header: ordered floating comparisons,
integer/floating addition and subtraction, floating negation, select, bitcast,
integer-to-pointer and pointer-to-integer conversions. All 14 bodies use the
actual LLVM builder and preserve source operand order and instruction names.
LlvmAttributes.hpp/.cpp now preserve the ten distinct nested object types from
LlvmAttributes.kt:83-84,97-104, with private construction and existing static
singleton identity. The kind caches still borrow those static objects.

Focused checks support translation. The earlier blanket deferral applied the
user's instruction too broadly: full runtime acceptance requires the connected
implementation, while source comparisons and focused compilation remain useful
and required. Historical receipts below describe their own checkpoints only.

The six connected implementations (CodeGenerator, LlvmAttributes, LlvmParamType,
LlvmUtils, LlvmFunctionPrototype and LlvmCallable) pass clang++ -std=c++20
-Wall -Wextra -Werror -fsyntax-only with the actual LLVM include directory.
The root CMake build in build/source-continuation reconfigured and built
KotlinxCoroutinePass and kxs_codegen_test successfully with LLVM 23.1.2 and
KOTLIN_NATIVE_RUNTIME_AVAILABLE=OFF. Neither command invoked kotlinc/konanc.
The code-generation executable exited zero and emitted
build/source-continuation/llvm-source-distance/coroutine_codegen.ll; its existing
checks include LLVMVerifyModule, suspension-block joins and insertion/scope
restoration. It does not directly exercise every new floating operation or
establish retained ownership, cancellation or either complete MLX acceptance path.

The build exposed two integration issues. LLVM/Clang SDK include directories
are now SYSTEM in the plugin/injector CMake targets; project warning policy
remains strict, with no added -Wno options. CoroutineInjection.cpp:134 uses
LLVM 23's CondBrInst while older LLVM retains BranchInst. Both preserve the
single direct condition consumer and true-successor resume target; only the
LLVM 23 branch was freshly compiled here.

Two scoped deep scans completed; the second follows the nested-type repair.
From build/source-continuation/llvm-source-distance, the command was:

```sh
../../../tools/ast_distance/ast_distance --deep ../../../tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm kotlin ../../../src/kotlinx/coroutines/tools/kotlinc_native_ref cpp
```

| Source | Function similarity | Matched bodies | Matched types |
|---|---|---|---|
| CodeGenerator | 0.19 | 61/145 | 2/15 |
| IrToBitcode | 0.01 | 3/184 | 6/26 |
| LlvmCallable | 0.54 | 9/10 | 5/5 |
| LlvmAttributes | 0.01 | 2/6 | 13/13 |
| VariableManager | 0.00 | 0/27 | 1/6 |
| LlvmParamType | 0.00 | 0/0 | 2/2 |

Generated inventories, criteria and repair priorities remain in that build
directory. LlvmParamType's forced-zero score explicitly reports no source
function bodies despite eight target constructor/accessor bodies. The inventory
also reports LlvmFunctionPrototype.kt as MISSING_FILE although supplied-signature
and attribute-provider bodies exist and compile. This is a demonstrated matching
discrepancy; its root cause remains to be investigated. The actual IR signature
factory, FunctionOrigin and LlvmFunctionProto are still missing. Generated Kotlin
emission limitations keep affected deep criteria provisional. This scoped scan
does not refresh full-root library/compiler measurements or waive those checks.

Continue source translation of actual call/exception/frame/root operations,
VariableManager, IR-derived signatures and the connected expression/initializer
driver. Target/default attributes and bridge debug metadata remain incomplete.
The full translation goal remains active.

## Typed LLVM signatures and attribute kinds — 2026-10-08

The continuation after f542b371 translates LlvmAttributes.kt:67-105,
LlvmParamType.kt:10-18, LlvmFunctionPrototype.kt:94-106,140-174 and
LlvmUtils.kt:235-243,307-351. LlvmAttributes.hpp/.cpp keeps the source attribute
classes and static singleton identities, with separate kind-ID caches for
parameter and function attributes. The actual LLVM lookup retains its missing-kind
error. A NOTE(port) explains serialized cache access by compiler threads.

LlvmParamType and LlvmRetType preserve actual LLVM types, attribute lists and
explicit object-return metadata. Attribute lists borrow the static singletons.
LlvmFunctionSignature at LlvmFunctionPrototype.cpp:123 owns its immutable
containers and synchronized lazy LLVM function type (:142). Calls (:151) and
declarations (:164) receive function, return and ordered one-based parameter
attributes. Concrete helpers adapt Kotlin's covariant list traversal without
introducing generic internal templates.

The signature-based pointer/declaration/definition constructors at
LlvmCallable.cpp:89,130,138 now retain the actual signature as their provider.
Nounwind lookup uses the source attribute helpers. Function type construction,
kind IDs, enum-attribute creation and function-attribute setters are implemented
in LlvmUtils.cpp:43-95. CMake adds LlvmAttributes.cpp and LlvmParamType.cpp to
the existing LLVM targets; the existing LLVM/Threads linkage supplies their
host dependencies. No Kotlin compiler or runtime dependency was added.

No configure, build, test, AST emission or deep scan was run. The IR-derived
signature factory at LlvmFunctionPrototype.kt:108-137, including type conversion,
ABI parameter attributes and the object-return slot parameter, remains untranslated.
Target/default attributes, function origins/prototypes, bridge debug metadata,
exception/call/frame/root-update lowering, VariableManager and the normalized
expression/initializer driver remain connected work. These supplied-LLVM
signature bodies do not establish either executable acceptance path. The full
translation goal remains active.

## LLVM callable and attribute dependency translation — 2026-10-08

References in this section describe checkpoint f542b371.

The continuation after d63d98ba translates the callable dependency used by
CodeGenerator.call and Native reference updates. LlvmCallable.hpp/.cpp mirror
LlvmCallable.kt:11-103: actual function types/values, object-return metadata,
synchronized lazy properties, call/invoke emission with provider attributes,
constant/callback conversion, function/pointer/declaration/definition classes,
nounwind lookup, parameter range checks, landingpads, basic blocks, block
addresses and subprogram attachment. LLVM operands remain compilation-owned;
the explicitly shared provider retains its policy. No runtime function or
object-return convention is inferred from pointer shape.

LlvmFunctionPrototype.cpp:10,23,94,99 ports the actual empty provider and lazy
external copier from LlvmFunctionPrototype.kt:24-92. Function, return and
parameter indices preserve the signed -1..parameter-count walk. Declaration
attributes are copied in full; call-site lists filter through LLVMIsEnumAttribute,
as upstream does. The two empty provider methods are the exact source defaults.
LlvmUtils.hpp/.cpp translates ConstValue, ConstPointer and constant indexed
pointers from LlvmUtils.kt:18-43, using actual LLVM constants. The owning LLVM
context supplies int32 operands at the existing LLVM boundary.

CodeGenerator.cpp:77 binds a borrowed actual LlvmFunction.Definition. Typed
parameter and block-address operations use it. basic_block_in_function at :156
ports source block creation and location registration. gep/struct_gep/
extract_value at :300,304,308 port CodeGenerator.kt:1035-1042. Conditional blocks
at :312,334 now carry the current start/end locations from source position(),
closing the preceding source-location dependency at those consumers.
The production injector still enters through its LLVM operand adapter; the
normalized typed compiler driver remains untranslated.

The three new implementations are registered in the existing shared-LLVM
kxs-inject, KotlinxCoroutinePass and kxs_codegen_test targets. Their synchronized
lazy snapshots use the explicit Threads::Threads dependency. No configure,
build, test, AST emission or deep scan was run. Signature-based constructors,
LlvmFunctionSignature/attribute kinds/prototypes and bridge-function debug
metadata still need their source dependencies. These are recorded gaps, with
no substitute bodies or declarations presented as finished implementations.
Next work remains CodeGenerator.call/callRaw, exception handlers, actual frame
allocation and Native root-update bindings, VariableManager and the connected
typed expression/initializer emitter. The full translation goal remains active.

## LLVM memory and source-location translation — 2026-10-08

References in this section describe checkpoint d63d98ba.

The continuation after 1da27774 translates CodeGenerator.kt's param, load,
store, optional ordering/alignment, source-location map and builder debug
operations. LocationInfo.hpp:11 mirrors IrToBitcode.kt:2884-2887;
LocationInfoRange at :24 mirrors CodeGenerator.kt:390-393. LLVM scope metadata
and inline descriptors remain borrowed from the owning compiler. Each map
entry owns its range, preserving a returned range's identity after replacement.
VariableDebugLocation.hpp:9 supplies the actual metadata fields from
VariableManager.kt:150; the manager algorithms are not yet translated.

CodeGenerator.cpp:25 constructs actual LLVM debug metadata recursively for
inline chains. basic_block at :151,155,159 carries start/end locations into the
map. debug_location at :166 ports the source's zero-line reuse rule; reset at
:174 and position at :179 expose the source operations. PositionHolder at :89
carries the prior range into an unreachable block after a terminator and at
:105 restores the destination block's location. The owning compiler's explicit
location-debug policy is supplied at the LLVM boundary; no LLVM metadata
inference substitutes for ConfigChecks.kt:30.

param/load/store at :184,195,200 port CodeGenerator.kt:720-732,754-758.
CoroutineInjection.cpp:205,212 now uses those memory operations for its actual
persistent label field. IrToBitcode_coroutines.cpp:140,201,223 uses actual IR
start offsets for start/dispatch/resume/continuation block locations, following
IrToBitcode.kt:2152-2154,2289-2340. General expression-location updates still
belong to the untranslated enclosing expression emitter.

Source review of tools/kxs_inject/CMakeLists.txt:14,22,31 confirms both edited
compiler implementations already belong to kxs-inject, KotlinxCoroutinePass and
kxs_codegen_test and link the selected shared LLVM target. New headers need no
source-list change or Kotlin tool/runtime dependency. The frontend CMake target
retains its existing Clang/LLVM linkage and explicit Native test boundary.
No configure, build, AST emission, runtime check or deep scan was run.

Next connected dependencies are actual frame allocation, load_slot/store_any
and Native reference-update calls, concrete function/variable/debug/exception
contexts, type lowering and the normalized Clang-to-IR driver. Plain LLVM stores
have not been substituted for Native object reference updates. Suspending typed
initializers and partial construction remain unfinished. This is a source
translation checkpoint; the full translation and executable paths remain open.

## Typed IR code contexts and suspension-point lookup — 2026-10-08

The continuation after 79e1c3c9 translates the code-generation scope dependency
from IrToBitcode.kt:120-212,300-326,2281-2340. Resolved initialization needs a
normalized expression emitter with actual declaration-bound contexts; the
existing LLVM operand callbacks alone do not provide that source contract.

CodeContext.hpp:24 now declares the complete source interface: returns and
return slots, loop jumps, exception handler, variable declaration/lookup,
function/file/class/returnable-block scopes, resume-point registration, source
locations, debug scope, lifecycle hooks and exception wrapping. Actual IR
objects and contexts are borrowed. ExceptionHandler remains a source-type forward declaration. The subsequent
memory/location checkpoint supplies VariableDebugLocation and LocationInfo.
The source's empty lifecycle defaults are implemented at
IrToBitcode_coroutines.cpp:11,13.

The private InnerScope at IrToBitcode_coroutines.cpp:19 spells Kotlin delegation
as forwarding methods. SuspendableExpressionCodeScope at :66 combines that
lexical context with the existing resume-point list adapter. SuspensionPointScope
at :78 retains the actual IrVariable identity, resume block and source metadata
index. gen_get_value returns the compiler-owned block address only for that
variable; other reads delegate to the outer context with their actual result slot.
The index remains code-generation metadata and is not a runtime dispatch state.

using_context at :106 ports the enter, exception-wrap and finally-exit order.
A NOTE(port) explains the explicit context parameter in place of Kotlin's mutable
currentCodeContext. Concrete typed overloads at :201,223 consume the actual
IrSuspendableExpression/IrSuspensionPoint getters, emit start/address dispatch,
evaluate normal/resume results in their scopes and join through the existing phi
helper. Block creation and scope entry preserve the source order.

No build, AST emission, runtime check or deep scan was run. Production marker
injection still uses the existing LLVM operand entries; there is no new normalized
Clang-to-IR expression driver invoking these typed overloads yet. General function,
variable, source-location and exception contexts, IrType lowering and typed
initializer/partial-construction emission remain incomplete dependencies.
CMake source review confirms the edited IrToBitcode_coroutines.cpp is already
registered in kxs-inject, KotlinxCoroutinePass and kxs_codegen_test; its new header
requires no new target or runtime dependency. Both complete executable acceptance
paths remain unproven, and the full translation goal remains active.

## Resolved initializer operands in the coroutine walks — 2026-10-08

The continuation after 365a046b translates the evaluated initializer contract
from InitializersLowering.kt:34-55 into the Clang adapter. Kotlin extracts field
initializers and copies the resulting initialization block into a constructor
before coroutine slicing. Clang retains both written and semantic InitListExpr
forms; its default RecursiveASTVisitor selects the written form, which may omit
selected member defaults and other implicit initialization operands.

SuspendFunctionAnalyzer::evaluated_initializer_list at
SuspendFunctionAnalyzer.cpp:47 selects the semantic form when present. The
local collector (:59), overload-resolution visitor (:284) and suspension-point
visitor (:400) each traverse its operands once, without traversing the alternate
written tree or enabling all implicit declaration bodies. Local collection also
follows selected defaults and deduplicates actual VarDecl identities. The
backward liveness visitor at :620 traverses the same selected operands in reverse
initialization order, including the array-filler expression. Existing default-use
paths remain attached to discovered suspension occurrences.

NativeSuspendLowering.cpp:606,882 applies that selection to suspension and
materialization discovery. Extended-owner reservation at :1403 uses the same
semantic operands. This avoids treating a list with an implicit selected suspend
call as a nonsuspending native declaration initializer.

TailSuspendCallsCollector.cpp:50 follows selected list operands and default
expressions with non-tail state, matching TailSuspendCallsCollector.kt:35-37.
An initialization operation remains after their result, so an implicit default
suspension cannot inherit an enclosing return's tail-call optimization.

No compilation, AST emission, runtime check or deep scan was run. This source
checkpoint repairs discovery and dataflow; it does not finish emission of
suspending aggregate/array initialization. Typed partial construction,
member-default receiver binding, implicit initializer source emission and repeated
array-filler lowering still need translation. initializer_list backing arrays,
immovable default parameters, access context, local nominal integration,
optimized spilling and both complete executable paths remain unfinished.
The existing frontend target already contains all three edited implementation
files; no build-system or runtime dependency was introduced.

## Extended aggregate referents in native initialization — 2026-10-08

The continuation after 425fb3bf grows the typed storage translation from
CoroutinesVarSpillingLowering.kt:49-105. C++ adds a lifetime contract absent from
Kotlin GC locals: temporaries bound to aggregate reference members can belong
to the aggregate variable's lifetime. The translated placement new-expression
must preserve the original declaration's lifetime extension explicitly.

NativeSuspendLowering.cpp:1370 adds reserve_extended_temporaries. It walks
evaluated initializers and records only MaterializeTemporaryExpr nodes whose
actual extending declaration is that variable. Postorder reservation places
nested owners before their enclosing owner, and all these fields before the
aggregate field. Owners participate in the existing lexical scope and
slot_variables_ maps, so normal exits, declaration-bound jumps and terminal
frame cleanup destroy the aggregate before its referents. External references
are not given owners.

collect_references at NativeSuspendLowering.cpp:368 inserts each reserved
object's placement construction at its original operand. The engagement bit is
set after successful construction, followed by the original lvalue/xvalue
category. Nested references and variable substitutions still use the existing
rewriter. generated_expression at :762 emits the same operation for compiler
expressions without independent source tokens.

Nonsuspending aggregate and array declaration lists at :1473,1495 now remain
native initialization expressions. Their constructors, side effects and partial
construction unwinding retain native list order; extended owners are constructed
inside the selected operand rather than eagerly in a preceding statement. These
owners survive subsequent suspension in the declaration's scope and do not join
the initialization full-expression temporary list.

InitializersLowering.kt:34-55 also supplies the selected member-initializer
contract. Suspension and materialization discovery at NativeSuspendLowering.cpp:
602,872 now unwrap CXXDefaultInitExpr, and overload-resolution deferral at
SuspendFunctionAnalyzer.cpp:260 follows that same selected expression.

No compilation, AST emission, runtime check or deep scan was run. This is source
translation, not evidence of executed destruction or resource retention. Lists
containing suspension still require typed aggregate/array operand slicing,
including direct construction of immovable subobjects. std::initializer_list
backing-array lifetime transport, implicit member expression emission, default
parameter construction/access context, local nominal integration, optimized
spilling and both complete MLX executable paths remain unfinished. Existing
frontend registration consumes these sources; no CMake dependency was added.

## Default-expression occurrence identity — 2026-10-08

The continuation after ca15c58b translates the identity contract used by
DefaultArgumentStubGenerator.kt:91-108, InitializersLowering.kt:34-55 and
LivenessAnalysis.kt:47,79-89. Kotlin prepares default arguments in the selected
function and copies instance initializers into their constructor. Clang can
instead share a declaration expression between different default-use nodes.
A raw expression pointer alone therefore does not identify the evaluated use.

SuspendPointInfo at SuspendFunctionAnalyzer.hpp:17 now records the enclosing
CXXDefaultArgExpr/CXXDefaultInitExpr use path. Suspension discovery at
SuspendFunctionAnalyzer.cpp:357,364 pushes and restores those actual AST nodes
while visiting each selected expression. Nested uses include the whole path,
so a shared inner default wrapper cannot collapse different outer call sites.
The ASTContext continues to own these nodes; this metadata borrows them.

The private SuspensionOccurrence key at SuspendFunctionAnalyzer.cpp:413 pairs
that path with the actual suspension statement. LivenessAnalysisVisitor::save
at :451 unions snapshots only for the same occurrence, preserving Kotlin's
loop-iteration merge without conflating independent uses. Both default visitors
at :605,612 use the same path, and compute_liveness at :712 attaches snapshots
by the complete key. Calls without defaults retain an empty path.

This changes analyzer metadata and its attachment algorithm. Native frame
emission already allocates fresh slots/resume markers on each emitted use;
optimized spill allocation is still unfinished. No compilation, AST emission,
runtime check or deep scan was run. Implicit operator/literal bindings,
private/protected access context, immovable by-value parameter construction,
aggregate/member expression slicing and earlier local integration remain
unfinished. Both complete MLX execution paths remain unproven.

CMake source review confirms SuspendFunctionAnalyzer.cpp remains registered in
KotlinxSuspendPlugin and the existing kxs_enable_suspend_frontend chain loads
that frontend with the mandatory LLVM module pass. This source change requires
no additional target, runtime library or Kotlin compiler invocation.

## Selected default operands in the connected expression slicer — 2026-10-08

The continuation after 535f304a translates selected default operands from
DefaultArgumentStubGenerator.kt:91-108 through the existing Native expression
slicer (NativeSuspendFunctionLowering.kt:215-250). C++ defaults are selected by
Clang rather than Kotlin's argument mask; their actual parameter declarations
supply the retained value/reference category.

NativeSuspendLowering.cpp:1038 adds emit_default_argument. Ordinary defaults
use semantic AST printing with resolved types; defaults containing suspension
or materialization enter emit_expression. Constructors and call operands both
consume that selected value. emit_call now evaluates omitted operands for ordinary
calls as well as implicit-continuation calls, appends those values explicitly,
and appends the continuation only through the existing annotated ABI path.
Suspension/materialization discovery traverses CXXDefaultArgExpr's selected
expression, which is not an ordinary Clang statement child. Default evaluation
therefore participates in suffix suspension analysis and occurs before saving
the enclosing suspend-call address.

NativeSuspendLowering.cpp:332 preserves named default declaration references
in their originating namespace/class/enum context. At :456, substituted type
parameter locations use Clang's concrete canonical type rather than an unavailable
callee parameter spelling. Default constructor expressions also use their resolved
type and selected argument list in the caller helper.

SuspendFunctionAnalyzer.cpp:110 expands semantic default printing to actual
class-template scopes, anonymous namespace visibility and explicit template
arguments. At :199-203, canonical types retain their scope during printing.
Dependent calls in selected defaults participate in overload-resolution deferral
at :253. Both argument suffix paths exclude the operator receiver from the list
of arguments written inside parentheses.

No build, runtime check or deep scan was run. Reused default AST occurrence
identity, implicit operator/literal binding, private/protected declaration access
from re-emitted caller expressions, direct parameter construction for immovable
by-value defaults, and aggregate/member initialization context remain unfinished
compiler dependencies. Result boxing and borrowed-reference ownership are not
changed by this checkpoint. Optimized spilling, earlier local/aggregate gaps and
both complete MLX execution paths remain open. These private source changes use
the existing frontend CMake target and introduce no Kotlin runtime dependency.

## Indirect source jumps and statement metadata continuation

The continuation after 8e57823f carries the existing declaration-bound target
and lexical lifetime operations through computed source jumps and attributed
statements. Kotlin provenance is LivenessAnalysis.kt:123-136,248-265,
CoroutinesLivenessAnalysis.kt:64-106 and NativeSuspendFunctionLowering.kt:119-170.
GNU computed jumps and Clang statement attributes are explicit C++ adaptations.

NativeSuspendLowering.cpp:1373 records actual AddrLabelExpr declarations during
the evaluated lexical walk. emit_indirect_goto at :1493 evaluates the target
once into existing retained storage and finishes its full-expression temporaries
before target cleanup. Deterministically ordered comparisons use only labels
whose addresses occur in the source; the selected path runs emit_goto's scope
and catch cleanup. Other addresses retain the original computed-goto undefined
behavior. GNU label-address syntax is emitted only for an authored indirect jump;
coroutine resume addresses still come from the mandatory LLVM pass.
SuspendFunctionAnalyzer.cpp:527-537 tracks those same address-taken declarations
for indirect successor liveness rather than unioning unrelated labels.

emit_statement at NativeSuspendLowering.cpp:1631 lowers attribute wrappers.
Branch/fallthrough hints and loop pragmas precede their actual control operation,
including after a loop's initializer. Resolved loop-hint values use Clang's actual
integer constant in concrete helper contexts. Ordinary attributed expressions
keep their native source/full-expression form. Common statement call policies
(noinline, always_inline, nomerge) surround the full lowered operation; this also
applies them to generated storage calls, a deliberate C++ adaptation recorded
in NOTE(port). TailSuspendCallsCollector.cpp:27-30 preserves tail state through
attribute wrappers. Other native attribute contracts, including musttail when a
frame is required, are not translated by this checkpoint and produce an explicit
lowering diagnostic rather than silently losing the attribute.

The liveness provenance ranges were corrected against the pinned Kotlin source.
No compilation, runtime check or deep scan was run. Call-policy isolation from
generated helper calls, remaining attribute contracts, shared default-expression
identity, optimized spilling and prior aggregate/local integration gaps remain
unfinished. Both complete MLX execution paths remain unproven. Existing CMake
registration consumes these edited sources; no target or runtime dependency was
added.

## Declaration-bound source jumps and retained scope cleanup

The continuation after efde2c6c extends the translated target bookkeeping in
LivenessAnalysis.kt:123-136,248-265 and lexical visibility walk in
CoroutinesLivenessAnalysis.kt:64-106. C++ labels are an explicit Clang adaptation;
they do not introduce another coroutine frame or resume dispatch representation.

SuspendFunctionAnalyzer.cpp:347 saturates declaration-bound label live sets over
the whole body, retaining suspension snapshots across iterations. At :517-529,
a direct goto reads its actual target set, a label records the live values before
its statement, and indirect-goto analysis unions potential label targets before
reading the computed address. Lexically following statements are not successors
of an unconditional jump.

NativeSuspendLowering.cpp:1370 records the declarations and catch handlers active
at each source label before emitting the state-machine body. It follows compound,
branch, loop, range-for and catch scopes; switch labels and attributes are
transparent. Tuple holding variables retain the containing declaration's scope.
At :1248 and :1295, retained locals and lifetime-extended reference owners record
their source declaration. Catch exception, variable and context storage record
the owning handler at :1538-1560.

emit_goto at NativeSuspendLowering.cpp:1466 destroys currently scoped objects
absent from the target, in reverse construction order. It keeps objects active
at that label and uses existing catch-context transitions when exiting a handler.
A backwards jump before a declaration therefore releases its old construction
before the declaration executes again. Source labels at :1607 now lower their
statements, including suspension, instead of passing through the raw statement
path or failing when the labelled body contains a suspend call.

These are unvalidated source changes. No compilation, runtime check or deep scan
was run, following the user's source-first direction. Indirect-goto ownership
cleanup, attributed control-statement emission, shared default-expression
identity, optimized spill allocation and prior aggregate/local integration gaps
remain incomplete. Actual Native/MLX and standalone MLX acceptance remain open.

## Connected suspend-call and liveness translation

The user directed source translation to continue before compilation, runtime
checks or deep scans. Those acceptance operations are deferred for this source
checkpoint; no new validation or completion result is claimed.

SuspendFunctionAnalyzer.cpp:284 translates the evaluated suspend-call walk from
NativeSuspendFunctionLowering.kt:367-388. Calls are collected after their children;
implicit wrappers and a DSL wrapper around an already annotated call do not add a
second suspension site. Nested class/function bodies, unevaluated operands and
discarded constexpr arms do not participate. Lambda capture initializers and
selected default expressions execute in their enclosing call context.

SuspendFunctionAnalyzer.cpp:342 translates the connected LivenessAnalysisVisitor
from compiler/ir/backend.common/.../optimizations/LivenessAnalysis.kt:44-267.
Reverse child propagation, variable reads/declarations/assignments, function
returns, conditional branches, throws, catch-live propagation, loop fixed points,
and break/continue targets now replace the prior block GEN/KILL approximation.
The Clang adaptation also handles short circuit operands, for/range-for increments,
switch fallthrough, reference writes and decomposed declaration identity.
compute_liveness at :576 attaches the resulting sets to the existing suspension
metadata and excludes completion, which the base continuation retains.

The production frontend consumes this analyzer before frame installation.
NativeSuspendLowering still retains actual C++ local storage independently of
value liveness because destruction and borrowed referents have separate lifetime
requirements. This checkpoint does not claim optimized spill-field allocation or
completion of CoroutinesVarSpillingLowering. Unstructured C++ label/goto liveness,
shared default-expression identity and the previously documented aggregate/local
integration gaps remain unfinished source work. Both compiler targets and their
CMake integration were read; this checkpoint changes the already registered
frontend source and requires no new target or runtime dependency.

## Direct aggregate spill construction continuation

Source checkpoints ea4a97db and d97189ee repair local aggregate construction
against the typed field contract in CoroutinesVarSpillingLowering.kt:49-105
and NativeSuspendFunctionLowering.kt:201-252. C++ aggregate lists additionally
require direct destination construction with their original braces.

NativeSuspendLowering.cpp:1284-1289 selects the existing aligned owning storage
for a non-reference record local initialized by InitListExpr. At :1293-1298 it
constructs that aggregate directly at its destination with braces.
construct at :627-647 preserves list initialization for this path. Previously
the local list was forwarded to optional::emplace, imposing initializer
deduction/move requirements absent from ordinary C++ aggregate initialization.
The engagement bit is set only after successful native construction; existing
scope/frame cleanup destroys the actual object. No public ownership API or
alternate state-machine representation changes.

qualified_locals.cpp adds AggregateOwner with a const unique_ptr member and a
const tag, making it an immovable aggregate. Static assertions establish its
source category. The aggregate's resource is initialized by an ordinary C++
factory before repeated suspension. Pending/resumed assertions check retention
and actual resource identity; its destructor checks member values and terminal
assertions require exactly one destruction. Existing modes include immediate/
suspended completion, immediate/resumed failure, cancellation and later partial
array-construction failure. The result-box delete policy remains unchanged.

Strict fixture syntax and actual Clang AST emission exit 0; the AST contains
the aggregate InitListExpr. Final strict lowering-unit checking exits 1 in
LLVM/Clang dependency headers, with no source-local diagnostic. Fresh plugin
build exits 2 in the metadata plugin's dependency headers. The installed older
frontend exits 1 at the existing alias declaration. No warning was suppressed.
Receipts are build/ir-recovery/aggregate-storage-{fixture,ast,lowering,
lowering-final,plugin-build,plugin-build-final,installed-frontend}.log.
No fresh runtime validates native construction, resource identity or destruction.

Extended lifetimes of materialized temporaries bound to aggregate reference
members remain unfinished. Such a temporary can belong to the containing
aggregate local's lifetime rather than its initialization full expression;
its owner must be mapped and cleaned after the aggregate, preserving evaluation
order. Aggregate/array expression slicing and local nominal/dependent integration
also remain incomplete. Standalone MLX and actual Native/MLX shared-state-machine
execution acceptance remain unproven.

## Constructor reference-materialization continuation

Source checkpoint 2e4eb601 repairs constructor-child slicing after reading
NativeSuspendFunctionLowering.kt:215-250. Kotlin treats the allocated instance
as the first constructor operand; the C++ adaptation additionally preserves
materialized reference arguments and guaranteed elision of ordinary prvalues.

NativeSuspendLowering.cpp:829-834 recognizes Clang's constructor-conversion
functional cast as the selected constructor's wrapper. Its child is lowered
directly, preserving a single prvalue construction rather than introducing a
second value slot/move. At :994-1006, materialized constructor reference arguments
are retained even when their source expression is pure. Source glvalues still
bind as borrowed references. The existing full-expression storage supplies
the scalar argument lifetime; the constructed receiver borrows that actual
object and does not acquire ownership of the reference.

qualified_locals.cpp:284-299 adds an immovable ConstructorCondition receiver
whose const-reference constructor binds literal 17. Operation 7 in condition_flow
uses that receiver before a suspending sibling. The sibling checks the literal's
referent; receiver destruction also checks its value and identity, requiring
the receiver to be destroyed while the referent still lives. Existing fixture
modes cover immediate/suspended completion, resumed failure, cancellation and
immediate failure, with pending retention and terminal destruction assertions.

Strict fixture syntax and Clang AST emission exit 0. The AST shows the selected
ConstructorConversion, constructor call and const-int MaterializeTemporaryExpr.
Strict lowering-unit checking exits 1 in LLVM/Clang dependency headers with no
source-local diagnostic. Fresh plugin build exits 2 in the metadata plugin's
dependency headers; the older frontend exits 1 at the existing alias declaration.
No warning was suppressed. Receipts are
build/ir-recovery/constructor-temporaries-{fixture,ast,lowering,plugin-build,
installed-frontend}.log. No fresh runtime validates construction, retention,
destructor ordering or failure/cancellation cleanup for this repair.

Local nominal/dependent integration, aggregate/array materialization and
broader ordinary C++ expression coverage remain incomplete. Standalone MLX and
actual Native/MLX shared-state-machine execution acceptance remain unproven.

Both exact full-root deep scans completed with exit 0 after the source checkpoint
without concurrent source edits. Generated inventories/reports are unchanged:
library 832/2918 bodies, 359/560 types, body similarity 0.26; compiler 592/7657
bodies, 174/1727 types, body similarity 0.36.

## Non-suspending operand temporary continuation

Source checkpoints deb8848c and ae390be1 extend the evaluated-expression
contract after reading NativeSuspendFunctionLowering.kt:201-252,367-388 and
CoroutinesVarSpillingLowering.kt:49-105. C++ full-expression materialization
requires additional lifetime preservation even when an individual operand
contains no suspend call.

NativeSuspendLowering.cpp:758-770 detects evaluated materialized record/scalar
objects. It visits lambda capture initializers separately from invoke bodies
and ignores unevaluated operands. At :824-828, a non-suspending operand containing
such a temporary enters the existing call/branch slicing paths. Previously a
temporary receiver used by the left operand of &&/|| or the condition of ?: could
be destroyed at the generated if statement before a selected sibling suspended.
The actual receiver now occupies the existing full-expression owning storage.
Short-circuit and conditional lowering still construct only the executed path.

At :1108-1130, materialized reference arguments are retained even when their
source value is pure, such as an integer literal. A pure value is not evidence
that its materialized object's identity/lifetime can be discarded. Borrowed
source glvalues retain the existing reference policy; no owning handle is
invented for them. Cleanup uses the prior full-expression/frame cleanup paths.

qualified_locals.cpp:267-301 adds an immovable condition temporary and a scalar
reference case. Seven operations cover skipped and executed &&/|| operands,
both selected conditional arms, and an ordinary read_integer(17) that exposes
the temporary integer's address to a suspending sibling. Pending/resumed
assertions check object retention and the integer referent value; terminal
assertions check destructor count and cleanup. Modes include immediate and
suspended completion, resumed failure, cancellation and immediate failure.
The executable remains registered with strict warnings and ASan/UBSan.

Final strict fixture syntax exits 0. Clang AST emission also exits 0 and shows
the actual MaterializeTemporaryExpr below the temporary receiver's member read.
Final AST emission additionally shows the const-int MaterializeTemporaryExpr
binding read_integer's literal argument.
Final strict lowering checking exits 1 in LLVM/Clang dependency headers, with
no source-local diagnostic. Fresh plugin build exits 2 in the metadata plugin's
dependency headers. The installed older frontend exits 1 at the existing alias;
it does not validate these changes. No warning was suppressed. Receipts are
build/ir-recovery/condition-temporaries-{fixture,fixture-final,ast,ast-final,lowering,
lowering-final,plugin-build,plugin-build-final,installed-frontend}.log.
No fresh runtime establishes the new lifetime or skipped-path assertions.

Local nominal/dependent integration, constructor/aggregate materialization
and broader ordinary C++ expression coverage remain incomplete. This source
repair does not establish full standalone MLX or actual Native/MLX execution.

Both exact full-root deep scans completed with exit 0 after the final source
checkpoint, with no concurrent source edits. Generated inventories/reports
remain unchanged: library 832/2918 bodies, 359/560 types, body similarity 0.26;
compiler 592/7657 bodies, 174/1727 types, body similarity 0.36.

## Borrowed argument temporary lifetime continuation

Source checkpoints 0f60aa5b and ee88e8f5 repair argument lifetime around the
NativeSuspendFunctionLowering.kt:201-252 expression slicing and :254-335
suspension-point contract, alongside CoroutinesVarSpillingLowering.kt:49-105.
ABI return of COROUTINE_SUSPENDED is not completion of the original C++ call
or its containing full expression.

NativeSuspendLowering.cpp:649-663 now registers owned sliced values in the
existing full-expression temporary storage. A record prvalue is constructed
directly in aligned owning storage rather than passed through optional::emplace,
preserving guaranteed elision for immovable ordinary C++ objects. Borrowed
glvalues remain borrowed and acquire no ownership. At :1121-1154, transient
argument bindings are released after the immediate/resumed logical completion
join, rather than before the suspension result check. Owned full-expression
temporaries are excluded from that transient cleanup and remain alive through
any enclosing ordinary call. Failure/cancellation still uses frame cleanup.

Previously a const-reference argument backed by a newly materialized object
could be destroyed before returning COROUTINE_SUSPENDED, while the callee's
frame still held a borrowed reference to it. There is no new runtime, adapter
frame, ownership transfer for references, or replacement of LLVM saved-address
dispatch in this repair.

expression_slicing.cpp:54-92 adds an immovable TemporaryArgument. An enclosing
ordinary call retains one temporary while a suspend callee borrows a second
through two suspensions. Assertions check actual reference identity, both
objects alive and neither destroyed while pending, completion value, and
exact reverse destruction order (2,1). Modes cover immediate completion,
repeated suspended completion, resumed failure, cancellation on the second
suspension, and immediate callee failure. Each result box is owned/unboxed by
the existing unique_ptr receivers. test_suspend_plugin.py now applies strict
warning flags to this registered executable with its existing ASan/UBSan run.

Default fixture syntax and Python AST parsing pass; these checks do not lower
or execute the state machine. Strict direct lowering checking exits 1 in
LLVM/Clang dependency headers with no source-local diagnostic. Fresh plugin
build exits 2 in the metadata plugin's dependency headers. The installed older
frontend's strict attempt exits 1 on existing coroutine-header unused parameters,
with further generated-frame errors; it cannot validate this source repair.
No warning was suppressed. Receipts are
build/ir-recovery/argument-lifetime-{lowering,fixture,fixture-final,plugin-build,
installed-frontend}.log. All new runtime assertions remain unverified by a fresh
frontend/pass build, including object identity and cancellation cleanup.

Both exact full-root deep scans completed with exit 0 after both source
checkpoints, without concurrent source edits. Generated inventories/reports
are unchanged: library 832/2918 bodies, 359/560 types, body similarity 0.26;
compiler 592/7657 bodies, 174/1727 types, body similarity 0.36. Standalone
MLX and actual Native/MLX shared-state-machine acceptance remain unproven.

## Wider constant-value spilling continuation

Source checkpoint 1e50f64a continues the typed local/property contract after
reading CoroutinesVarSpillingLowering.kt:49-105 and
AbstractSuspendFunctionsLowering.kt:55-91. Original C++ declared types and
address/reference uses retain their frame objects; compiler-proven constant
reads must also remain usable in constant-expression/type contexts.

NativeSuspendLowering.cpp:277-330 previously abandoned constant rewriting for
values wider than 64 bits. That left a runtime frame access where a template
argument or static assertion requires a constant expression. It now assembles
the actual Clang-evaluated bits from unsigned 64-bit literal limbs with shifts
in the destination integer type. Signed negative values use -1 - complement;
this includes the signed minimum without an unrepresentable positive
intermediate or negation overflow. Enum values use their declared underlying
integer type for arithmetic and cast back to the original enum type. This is
C++ constant-expression preservation around Kotlin-derived spill storage,
not a new Kotlin integer API or alternate coroutine runtime.

qualified_locals.cpp adds signed/unsigned 128-bit constants, a signed minimum,
the unsigned maximum, and a scoped enum with that wider underlying type.
Template arguments and static assertions check their value/type use; the
existing repeated-suspension fixture additionally checks the retained wide
object's address and value, plus enum/integer reads after suspension. Its
completion, immediate/resumed failure and cancellation modes remain registered.
These executable assertions have not passed with a freshly built frontend.

Strict fixture syntax checking exits 0, including the final enum cases.
Direct strict lowering checking exits 1 in LLVM/Clang dependency headers,
with no diagnostic located in NativeSuspendLowering.cpp. Fresh plugin build
exits 2 in the metadata plugin's dependency headers. The installed older
frontend exits 1 at the existing alias declaration; it does not exercise the
new lowering. No warning was suppressed. Receipts are
build/ir-recovery/wide-constants-{fixture,fixture-final,lowering,plugin-build,
installed-frontend}.log. No fresh runtime establishes object identity or cleanup.

Local enum declarations remain rejected by emit_declaration. Copying such an
enum into the frame would change its nominal identity without explicit importer
mapping to the original declaration. That lexical/nominal integration, local
class/lambda dependencies, non-integral constants and full executable acceptance
remain unfinished; this constant-value repair does not reclassify those gaps.

Both exact full-root deep scans completed with exit 0 after the source checkpoint
without concurrent source edits. Generated inventories/reports are unchanged:
library 832/2918 bodies and 359/560 types, body similarity 0.26; compiler
592/7657 bodies and 174/1727 types, body similarity 0.36. Neither scan establishes
standalone MLX or actual Native/MLX shared-state-machine execution acceptance.

## Complete lexical-container parsing continuation

Source checkpoint 0f47180e repairs helper parsing and declaration reuse in
CompilerFrameLowering.cpp after reading LocalDeclarationPopupLowering.kt:26-122
and the Native suspend entry/body replacement dependencies. Kotlin moves local
declarations into their declaration container after local-declaration lowering.
C++ integration must preserve actual lexical/nominal identities instead of
moving local classes into an unrelated namespace.

CompilerFrameLowering.cpp:181-211 now finds the outer enclosing lexical class or
function for an in-class/local method body, reparses through that complete
declaration, and closes its namespaces. Previously the in-class path parsed the
rest of the main file, including definitions the host parser had not reached.
Record source ranges end at the brace, so the helper supplies the declaration
semicolon; function definitions need none. Out-of-class/free entries retain
their existing body-boundary parsing. No declaration is hoisted by this change.

At :48-68, generated declaration offsets are mapped back through the body rewrite
before applying the original parser boundary. At :359,373, source/host inventories
exclude the replaced body and include original members after it within the
completed lexical container. This allows a later field or method to reuse its
actual host AST node rather than being excluded solely because it follows the
suspend method's body. Generated frame declarations still have no host twin.
The instantiated-local-method namespace restriction remains unresolved; this
change does not claim broader local class/lambda lowering completion.

test_suspend_plugin.py:74-82 extends the registered late-include regression with
an in-class suspend method that accesses a field declared after its body and a
later ordinary accessor. The later DispatchedContinuation include and Result
assignment remain present. Both suspend definitions use the supported Clang
annotation spelling, allowing ordinary source syntax checking without the
attribute-registration plugin.

The extracted registered source passes ordinary default syntax checking (exit
0). Strict source syntax exits 1 on existing unused parameters in JobSupport.hpp
and CancellableContinuationImpl.hpp reached through the later include. Direct
strict importer checking exits 1 in LLVM/Clang dependency headers, with no
diagnostic located in CompilerFrameLowering.cpp. Fresh CMake plugin build exits
2 in the metadata plugin's dependency headers. The installed older frontend exits
1 on generated GNU address-of-label code in the member frame; no fresh runtime
or current-helper integration result is established. Python AST parsing and
git diff --check pass. No warning was suppressed. Receipts are
build/ir-recovery/lexical-prefix-{importer,plugin-build,regression-syntax,
regression-default-syntax,installed-frontend}.log.

Both exact full-root deep scans completed with exit 0 against the committed
source without concurrent source edits. Generated inventories/reports are
unchanged: library 832/2918 bodies and 359/560 types, body similarity 0.26;
compiler 592/7657 bodies and 174/1727 types, body similarity 0.36. Standalone
MLX and actual Native/MLX shared-state-machine acceptance remain unproven.

## Instantiated body delivery continuation

Source checkpoints 077818e0 and 01b09e0d repair the Clang integration around
the translated coroutine-body replacement after reading
AbstractSuspendFunctionsLowering.kt:55-91. Kotlin replaces the original body
with coroutine construction; Clang additionally delivers instantiated definitions
through ASTConsumer callbacks. Its HandleCXXImplicitFunctionInstantiation contract
states that the body is not yet available and completed bodies subsequently arrive
through HandleTopLevelDecl. A translation-unit fallback would miss the required
pre-CodeGen ordering and was not introduced.

KotlinxSuspendPlugin.cpp:250-298 tracks actual canonical FunctionDecl/body pairs.
Completed bodies are not lowered again. During recursive importer delivery, a
replaced body can pass onward; re-entry with the original active body produces
a diagnostic rather than allowing the unlowered definition into CodeGen. Scope
exit removes the active entry on both success and failure. The initial deprecated
LLVM make_scope_exit use was caught by strict checking and changed to the current
scope_exit constructor. This is C++ compiler integration, not an additional
Kotlin state machine or a claimed translation of a Kotlin callback.

CompilerFrameLowering.cpp:467,507,524 now propagates failed host-consumer callbacks
at each explicit referenced-definition delivery. Previously all three return
values were ignored. Local class/lambda lexical contexts and broader dependent
import remain incomplete; their nominal identity is not flattened into a namespace.

unit_tail.cpp:22-36 adds recursive template specializations with constexpr branch
selection and retained shared ownership through nested continuation frames. Its
registered executable asserts immediate/resumed completion and failure, object
identity after suspension, and owner expiration after cleanup. The existing
test_suspend_plugin.py invocation now supplies strict warning flags for this
fixture. These assertions have not passed with a freshly built frontend.

Direct strict compiler-unit checks exit 1 in dependency headers. The first driver
check additionally found the deprecated helper; the final driver check has no
diagnostics located in the changed driver. The importer check likewise has no
source-local diagnostic. Fresh CMake plugin build exits 2 in the metadata plugin's
LLVM/Clang dependency headers. Strict fixture syntax without its attribute plugin
fails on the unknown suspend attribute. The installed older frontend exits 1 on
generated GNU address-of-label expressions when it reaches recursive_unit; it
does not establish behavior of the current source. No warning was suppressed.
Receipts are build/ir-recovery/body-delivery-{driver,driver-final,importer,
plugin-build,fixture,installed-frontend}.log. Standalone MLX and actual Native/MLX
shared-state-machine acceptance remain unproven.

CMakeLists.txt:108-113 and KotlinxCoroutines.cmake:110-113 were reviewed: library
and executable targets require both frontend construction and LLVM address
injection. Disabling the in-tree frontend selects an external frontend. No CMake
source change was needed for this callback repair, and the fresh build failure
was retained rather than changing its warning policy.

Both exact full-root deep scans completed with exit 0 after the source checkpoints
and no concurrent source edits. Generated inventories and reports are unchanged:
library 832/2918 bodies, 359/560 types, body similarity 0.26; compiler 592/7657
bodies, 174/1727 types, body similarity 0.36. These measurements do not establish
executable acceptance or completion of the translation.

## Compile-time branch selection continuation

Source checkpoint 7d077d9a continues the Kotlin expression/control traversal
contract after reading NativeSuspendFunctionLowering.kt:197-250 and the matching
TailSuspendCallsCollector.kt:81-86. C++ if constexpr additionally discards one
arm before runtime coroutine construction; its init statement still executes.

NativeSuspendLowering.cpp:495 excludes the discarded arm from suspension
discovery. At :1427, statement emission uses Clang's getNondiscardedCase instead
of constructing a runtime boolean slot and lowering both bodies. Init statements
and condition declarations keep the actual construction/cleanup scope, including
owning objects that must survive a suspension in the selected arm. A false
condition without an else has an empty selected arm, while its init still runs.

SuspendFunctionAnalyzer.cpp:224 defers unresolved constexpr conditions until
instantiation and ignores discarded calls when deciding overload readiness.
TailSuspendCallsCollector.cpp:68 visits only the selected result arm. Direct
tail continuation edits at KotlinxSuspendPlugin.cpp:304 and retained-storage
eligibility at :474 use the same selection. Owning init objects still require
retained storage; discarded local types do not force it. No alternate frame or
runtime branch selector was introduced.

qualified_locals.cpp:190-225 now includes an always-false branch without an else
whose init expression increments a counter, and a selected guarded branch with
an immovable init object spanning actual suspension. The discarded arms contain
suspend calls; one also contains a local class. Completion, immediate/resumed
failure and cancellation assert exact guard construction/destruction counts and
no extra calls. These are pending executable assertions, not a fresh runtime
result.

Strict ordinary fixture syntax and actual Clang AST emission exit 0. Direct
strict checking of the four changed compiler translation units exits 1 in
dependency headers, with no diagnostics located in those compiler files. The
final plugin-unit check covers the subsequent storage eligibility adjustment.
Fresh CMake frontend build exits 2 in dependency headers; the installed older
plugin exits 1 at the earlier alias declaration. Receipts under build/ir-recovery:
constexpr-branch-source-final.log, constexpr-branch-lowering.log,
constexpr-branch-plugin-final.log, constexpr-branch-plugin-build.log and
constexpr-branch-fixture.log. No warnings were suppressed.

No fresh executable validates branch omission or guard retention/cleanup.
Deferral does not establish the complete template specialization revisit/import
pipeline. Dependent fields/bindings, local nominal declarations, nested invokes
and both complete executable acceptance paths remain incomplete. Historical
sections below describe their earlier checkpoints; the full goal stays active.

Both exact full-root deep scans exit 0 without concurrent source edits. Receipts:
constexpr-branch-library-deep.log and constexpr-branch-compiler-deep.log. Compiler
detail inventories/evidence refresh the tail visitor's changed bodies and source
locations. Aggregate reports remain unchanged: library 832/2918 bodies, 359/560
types, similarity 0.26 with 123 scoring failures; compiler 592/7657 bodies,
174/1727 types, similarity 0.36 with 24 failures. The measurements do not prove
compiled branch selection or resource lifetimes through the complete pipeline.

## Retained local constant-value continuation

Source checkpoint 12589756 continues the original-variable binding work after
rereading NativeSuspendFunctionLowering.kt:368-410, including the IrConst and
immutable-value purity contract. C++ additionally distinguishes constant reads
that do not require a variable's identity from reads that require its object.

NativeSuspendLowering.cpp:277 now uses Clang's NOUR_Constant classification,
constant-expression usability and actual integral evaluation for retained
locals/parameters. It prints the evaluated value with the original unqualified
expression type, including enum types. Standard signed/unsigned 64-bit literals
retain their full range; the signed minimum uses a representable literal pair.
At :324, these constant reads retain their compile-time value; address-taking
and reference uses continue to use the actual stored variable. At :377, the
type-location visitor also handles constant references inside template arguments
and array bounds. Global traits and template members are not folded by this
local binding adaptation, and no second constexpr object is introduced.

qualified_locals.cpp:112-125 adds constexpr and constant-initialized integer
locals, an enum constant, uint64 maximum and int64 minimum, an actual std::array
template argument, static assertions and a retained pointer to the count object.
At :195, repeated suspension checks the same object's address and values.
The actual Clang AST marks the value reads non_odr_use_constant, while the
address-taking reference retains its ordinary identity-bearing use.

Strict ordinary fixture syntax/AST emission exits 0. Final direct strict compiler
translation-unit checking exits 1 in LLVM/Clang dependency headers, with no
diagnostics located in NativeSuspendLowering.cpp. Fresh CMake frontend build
exits 2 in dependency headers. The older installed plugin rejects the fixture's
earlier alias declaration, so its exit 1 does not execute this repair. Receipts
under build/ir-recovery: constant-locals-source.log,
constant-locals-lowering-final.log, constant-locals-plugin-build.log and
constant-locals-fixture.log. No warnings were suppressed; no fresh runtime
establishes constant object identity or cleanup for this changed compiler.

Non-integral and wider extension constant values, dependent type/binding cases,
evaluated polymorphic typeid with suspension, local classes and nested invoke
lexical binding remain incomplete. Both complete executable acceptance paths
remain unproven. Older sections below describe their source checkpoints.

Both exact full-root deep scans exit 0 without concurrent source edits. Receipts:
constant-locals-library-deep.log and constant-locals-compiler-deep.log. Generated
reports remain unchanged: library 832/2918 bodies, 359/560 types, similarity 0.26
with 123 scoring failures; compiler 592/7657 bodies, 174/1727 types, similarity
0.36 with 24 failures. Those measurements do not validate constant expression
preservation or runtime identity through the compiler pipeline.

## Unevaluated type query continuation

Source checkpoint 05a52132 continues the runtime-call traversal contract after
reading NativeSuspendFunctionLowering.kt:368-410 and the complete matching
TailSuspendCallsCollector.kt. C++ sizeof/noexcept and non-evaluated typeid
operands introduce no runtime calls. This C++ evaluation-context adaptation is
additional to Kotlin's IR visitor rather than a new suspension mechanism.

SuspendFunctionAnalyzer.cpp:203 identifies the unevaluated query expressions,
retaining potentially evaluated polymorphic typeid and variably modified
operands. NativeSuspendLowering.cpp:457 and TailSuspendCallsCollector.cpp:26
exclude unevaluated operands from suspension discovery; direct tail continuation
editing at KotlinxSuspendPlugin.cpp:302 does likewise. Overload-resolution
deferral ignores calls inside these queries and decltype. The source checker at
KotlinxSuspendPlugin.cpp:158-172 retains AST traversal with an evaluation context;
nested function declarations reset to their independent body context.

NativeSuspendLowering.cpp:350 replaces resolved decltype type locations with
Clang's canonical underlying type through std::type_identity_t. This preserves
array/reference type syntax, cv-qualification and the special declared type of
an unparenthesized structured binding. It does not infer the type from a rewritten
frame getter. At :278, unevaluated operand references use their original type
and value category, keeping a getter's exception specification out of noexcept.
At :1086, local StaticAssertDecl emits the original typed assertion without
construction or cleanup storage.

qualified_locals now asserts ordinary/parenthesized decltype for cv-qualified
locals, array components and owned tuple components, array extents, unevaluated
suspend result sizes and noexcept. Its ordinary_type_queries function uses
sizeof/noexcept/decltype/non-polymorphic typeid without coroutine annotation;
runtime calls must remain zero before the actual authoring entry starts.

Strict ordinary fixture syntax exits 0. Direct strict checking of the four
changed compiler translation units initially found two Clang type-location API
arity mismatches; both were corrected. The repeated check exits 1 in dependency
headers with no diagnostics located in the changed compiler files. The final
Native-only check covers the subsequent query-reference adjustment. Fresh CMake
plugin build exits 2 in dependency headers. The older plugin rejects the fixture's
earlier alias declaration, so its exit 1 does not validate these changes.
Receipts under build/ir-recovery: type-query-source-final.log,
type-query-lowering-final.log, type-query-native-final.log,
type-query-plugin-build.log and type-query-fixture.log. No warnings were suppressed.

No fresh executable validates the changed compiler pipeline. Dependent decltype,
constexpr-value assertions referencing spilled locals, evaluated polymorphic
typeid with suspended operands, local classes and nested invoke lexical binding
still need implementation/verification. Both complete executable acceptance paths
remain unproven; the full transliteration/state-machine goal remains active.

Both exact full-root deep scans exit 0 without concurrent source edits. Receipts:
type-query-library-deep.log and type-query-compiler-deep.log. Compiler detailed
inventory/evidence refreshes the changed tail traversal; aggregate reports remain
unchanged: library 832/2918 bodies, 359/560 types, similarity 0.26 with 123 scoring
failures; compiler 592/7657 bodies, 174/1727 types, similarity 0.36 with 24 failures.
These measurements do not establish fresh lowering execution or query behavior.

## Copied array construction and cleanup continuation

Source checkpoint e522dd8c continues the original-variable-to-field contract
after rereading CoroutinesVarSpillingLowering.kt:49-105 and the source expression
slicing in NativeSuspendFunctionLowering.kt:210-252. C++ arrays require actual
element construction and destruction; Kotlin variable spilling itself does not
define those C++ lifetimes.

NativeSuspendLowering.cpp:1156 handles Clang's ArrayInitLoopExpr with the actual
OpaqueValueExpr source captured once by reference. The source expression is
evaluated before its component initializers. At :594, implicit element indices
and nested copy loops print native brace initializers with their actual AST
element expressions. Native array construction owns partial-construction cleanup
when an element constructor throws. Engagement is recorded only after success.
This path also applies to compiler-generated decomposition declarations.

At :486, array storage emits reverse loops for every constant array dimension
before destroying the actual element. Previously, std::destroy_at on the array
visited elements forward, unlike ordinary C++ array destruction. Borrowed array
references keep their existing pointer binding and acquire no cleanup ownership.
No substitute array type, coroutine frame or runtime was added.

qualified_locals adds independent scalar-array copies, nested-array copies,
source evaluation counts and class-array copy identity across repeated suspension.
The sixth mode throws from the second element copy: the completed first copy
must be destroyed before the two originals. All other success/failure/cancellation
modes check four class-array destructions in reverse native order, alongside
the existing owning tuple, cv-qualified object and static initialization checks.
These assertions are pending fresh compiler execution.

Strict source syntax and the actual Clang AST dump exit 0. The dump contains the
expected implicit loops, opaque source expressions and class copy constructor.
Direct strict compiler translation-unit syntax exits 1 in dependency headers,
with no diagnostics located in NativeSuspendLowering.cpp. Fresh CMake frontend
build exits 2 in dependency headers. The installed older plugin exits 1 at the
earlier alias declaration. Receipts under build/ir-recovery:
array-decomposition-source-final.log, array-decomposition-lowering.log,
array-decomposition-plugin-build.log and array-decomposition-fixture.log.
No warnings were suppressed, and no fresh runtime result proves construction,
destruction order, resumed failure or cancellation for these source changes.

Dependent decomposition, local nominal classes and nested invoke lexical binding
remain incomplete. Both complete executable acceptance paths remain unproven.
The historical sections below describe their earlier source checkpoints.

Both exact full-root deep scans exit 0 without concurrent source changes.
Receipts: array-decomposition-library-deep.log and
array-decomposition-compiler-deep.log. Generated reports remain unchanged:
library 832/2918 matched bodies, 359/560 types, body similarity 0.26 with 123
scoring failures; compiler 592/7657 matched bodies, 174/1727 types, body
similarity 0.36 with 24 failures. These metrics do not validate C++ lifetime
behavior or the new compiler AST adaptation.

## Structured binding continuation

Source checkpoint 90204a13 grows the same declaration/storage lowering after
rereading NativeSuspendFunctionLowering.kt:119-170,197-335 and the complete
CoroutinesVarSpillingLowering.kt. The Kotlin source maps original variables to
their retained fields. C++ decomposition requires additional actual compiler
bindings; it must not synthesize new component objects or transfer borrowed
component ownership.

NativeSuspendLowering.cpp:1042 extracts the shared variable construction path.
At :1123, actual DecompositionDecl bindings register their compiler expressions.
Array/member bindings refer directly to their underlying retained object,
including bit-fields, without pointer storage that cannot represent a bit-field.
Tuple holding variables use the same construction/cleanup path, in declaration
order; a borrowed tuple component does not acquire ownership. Lifetime-extended
holding initializers retain the actual owner using existing delayed storage.
At :578, compiler-generated lvalue-to-xvalue casts print std::move so implicit
tuple decomposition retains its selected rvalue get overload. This is a C++ AST
adaptation, not a replacement state machine or a new tuple protocol.

The existing qualified_locals execution fixture now covers array reference
identity, member bit-field mutation, rvalue-qualified user get calls that must
execute exactly twice, and a tuple owning a unique_ptr to an immovable resource.
Four resources must remain alive during repeated suspension and all must be
destroyed after success, immediate/resumed failure or cancellation. These are
pending executable assertions, not a fresh runtime result.

Strict ordinary fixture syntax and its actual Clang AST dump exit 0. The dump
confirms direct member/array bindings, value holding variables for user get, and
rvalue reference holding variables for the pair's owned component. Final direct
strict compiler translation-unit syntax exits 1 in dependency headers, with no
diagnostics located in NativeSuspendLowering.cpp. Fresh CMake frontend build
exits 2 in dependency headers. The older installed frontend still rejects the
earlier alias declaration, so its fixture check exits 1 before testing this
repair. Receipts under build/ir-recovery: decomposition-source-syntax.log,
decomposition-source-ast.log, decomposition-lowering-final.log,
decomposition-plugin-build.log and decomposition-fixture-final.log.

No warning suppression was added. No fresh plugin executable validates cleanup
or overload preservation. Dependent decomposition, copied array decomposition,
local nominal classes and separately lowered invoke lexical integration remain
incomplete. Both full executable acceptance paths remain unproven.

Both exact full-root deep scans exit 0 without concurrent source edits. Receipts:
decomposition-library-deep.log and decomposition-compiler-deep.log. Generated
reports remain unchanged: library 832/2918 matched bodies, 359/560 types, body
similarity 0.26 with 123 scoring failures; compiler 592/7657 matched bodies,
174/1727 types, body similarity 0.36 with 24 failures. These coarse measurements
do not validate the compiler binding adaptation.

## Local alias binding continuation

Source checkpoint c57aa777 extends the same Native-derived frame lowering.
LocalDeclarationPopupLowering.kt:39-117 was reread with CompilerFrameLowering.cpp
and NativeSuspendLowering.cpp. Kotlin preserves declaration bindings when moving
local declarations; C++ type aliases additionally introduce no nominal type or
runtime object. This adaptation does not implement Kotlin's entire popup phase.

NativeSuspendLowering.cpp:1002 now accepts actual TypedefNameDecl declarations,
assigns each a unique frame alias and emits its canonical underlying type.
The type-location visitor at :335 rewrites alias uses by declaration identity,
including nested shadowing. Spill storage at :462 canonicalizes local alias
spellings while retaining the actual type and cv-qualification. Lambda capture
initializers retain their existing traversal boundary; separate invoke bodies
remain part of the incomplete lexical declaration integration.

qualified_locals.cpp:51-86 uses chained aliases, a typedef, const/volatile types
and a nested same-named int alias before and after actual suspension. It retains
the existing immovable object identity, repeated suspension, static initialization,
completion, immediate/resumed failure and cancellation checks.

Strict ordinary C++ source syntax exits 0. Strict direct compiler translation-unit
syntax with unlimited errors exits 1 on LLVM/Clang dependency headers, with no
diagnostics located in NativeSuspendLowering.cpp. Fresh CMake frontend build exits
2 in dependency headers. The installed older frontend exits 1 at the original
non-variable declaration rejection. No fresh executable validates this change.
Receipts under build/ir-recovery: local-alias-source-syntax.log,
local-alias-lowering-syntax.log, local-alias-plugin-build.log and
local-alias-fixture.log. No warning suppression or system-header workaround was
added. Local nominal classes, inherited alias scope in separately lowered invokes
and local-class/lambda template namespace integration remain unresolved.

Root CMakeLists.txt:108-115 and cmake/Modules/KotlinxCoroutines.cmake:110-113 were
reviewed again: core/tests retain frontend plus LLVM injection when an external
frontend is selected. No CMake source change was needed for this binding repair.
Both exact required full-root scans exit 0 with no concurrent source edits;
receipts are local-alias-library-deep.log and local-alias-compiler-deep.log.
Generated reports remain unchanged. Library: 832/2918 matched bodies, 359/560
types, body similarity 0.26, 123 scoring failures. Compiler: 592/7657 matched
bodies, 174/1727 types, body similarity 0.36, 24 failures. Those measurements do
not establish execution or completion of the local declaration pipeline.

Source commits d7feb4fb and a4e0c319 continue the active compiler state-machine
translation. The pinned LocalDeclarationPopupLowering.kt, CoroutinesVarSpillingLowering.kt
and consumed IrToBitcode.kt:2281-2337 were read with the frontend/frame importer,
Native lowering and mandatory LLVM injection. CMake plugin modules and the
Native-disabled target's actual compile commands were reviewed.

## Source repairs

NativeSuspendLowering.cpp:995 now rewrites a static declaration's initializer
through the actual retained binding map. Its C++ static storage duration and
initialization guard remain intact; parameters, automatic locals and receivers
refer to their actual frame bindings. The qualified_locals fixture adds varying
arguments, an automatic local used by grouped static initializers, one-time
initialization and stable static addresses alongside repeated suspension,
cv-qualified identity, completion, immediate/resumed failure and cancellation.
Existing binaries reproduce stale parameter/local names and duplicate static
emission; they do not contain either repair.

IrToBitcode.kt creates bbResume in code generation and resolves its suspension
identity to blockAddress(bbResume). The adapted frontend now emits ordinary C++
conditional marker branches instead of GNU label addresses. Suspend.hpp:25,27
adds declaration-only compiler contracts; :106,129 updates the actual macros.
NativeSuspendLowering.cpp:242,968,1118 updates suspension, handler-context and
unwind regions. The same generated frame, void* label and Continuation ABI remain.

CoroutineInjection.cpp:56,109 pairs each __kxs_suspend_site with a constant-ID
__kxs_resume_point conditional branch in that function. Its true successor is
an actual LLVM block. The injector creates its BlockAddress, stores it in the
exact supplied persistent field, registers it for indirectbr, replaces the
marker condition with false and removes both calls. IDs are compile-time pairing
data and are erased; no integer state or marker runtime is introduced. Signature,
direct-call/branch, unique/matched ID, persistent field, same field and local
address checks reject malformed contracts. The earlier explicit-address marker
remains accepted for existing LLVM inputs. KotlinxCoroutinePass.cpp:26 recognizes
all marker declarations before optimization. No compiler launcher or serialized
IR stage is added to production.

The existing test_kxs_inject suite gains repeated two-point, non-monotonic ID,
independent-frame address tests and malformed pairing/branch/field checks.
test_llvm_pass adds ordinary strict C++ compilation, optimized/unoptimized
sanitizer execution and emitted marker-erasure/address checks. These are pending
execution against fresh binaries; they do not establish complete Native or MLX
acceptance.

## Verification

- Strict frontend emission of the actual new standalone test source exits 0.
  Its emitted LLVM has the expected direct i1 marker condition and true branch.
- opt -passes=verify accepts the new LLVM test input (exit 0). This verifies input
  structure, not the changed injection implementation.
- Python test source syntax parses successfully.
- Strict actual test_suspension_core.cpp syntax check exits 1 on five dependency
  unused-parameter diagnostics. GNU label-address diagnostics are absent from
  the changed authoring macros.
- Fresh KotlinxSuspendPlugin and KotlinxCoroutinePass CMake builds each exit 2
  in KotlinxClassMetadataPlugin's LLVM/Clang dependency headers. No warning
  suppression, system-header workaround or runtime substitute was added.
- The new compiler-pass regression run against the older installed pass exits 1
  at link time with unresolved site/resume markers. It is evidence that the old
  pass cannot execute the new contract, not execution of the source repair.

Receipts under build/ir-recovery use static-binding- and resume-markers- prefixes.
Specific files: static-binding-fixture.log, static-binding-plugin-build.log,
resume-markers-frontend.log, standard-resume-markers.ll,
resume-markers-test-ir-verify.log, resume-markers-core-source.log,
resume-markers-plugin-build.log, resume-markers-ir-build.log and
resume-markers-existing-pass.log. No fresh runtime validates either source repair.

CMakeCache retains KOTLIN_NATIVE_RUNTIME_AVAILABLE=OFF. The actual core command
contains both the frontend plugin and -fpass-plugin. CMake production remains
in-compiler frontend/LLVM lowering; no CMake source change was needed this batch.
Both exact full-root scans exit 0 without concurrent source changes. Logs:
resume-markers-library-deep.log and resume-markers-compiler-deep.log. Reports
remain unchanged: library 832/2918 bodies, 359/560 types, similarity 0.26 with
123 scoring failures; compiler 592/7657 bodies, 174/1727 types, similarity 0.36
with 24 failures. Those coarse matches do not validate this compiler adaptation.

Local class/lambda lexical declaration integration remains incomplete. Strict
fresh dependency compilation still prevents building the changed plugins. Both
complete standalone C++/MLX and actual Native shared-frame GPU acceptance remain
unproven. The full transliteration/state-machine goal remains active.
