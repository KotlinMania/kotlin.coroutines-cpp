# IR declaration identity and scope dependency ledger

Updated 2026-10-07. First-priority compiler card: `t_16bf1579`.
The implementation sequence and acceptance remain in
[ir_identity_and_scopes.md](../architecture/ir_identity_and_scopes.md).

Compiler source is pinned to `fee29910d8dddd2b1f7b44036c00533cee493351` in
`tmp/kotlin`. Twenty-nine required files were materialized from local Git
objects without overwriting existing files or changing sparse-checkout
configuration. `build/ir-recovery/ir-identity/source-manifest.json` records
paths, hashes, line counts and exact comparison with the pinned revision.
Twenty-six files are Kotlin; three are Java compiler dependencies. This is
the investigated dependency frontier. Dependency closure is still incomplete.

The next checkpoint materialized `DeclarationSymbolMarkers.kt` and
`IrParameterKind.kt` from that same revision. Their receipt is
`build/ir-recovery/ir-symbol-contracts/source-manifest.json`. The investigated
materialized frontier at that checkpoint was 31 files: 28 Kotlin and three Java.

The descriptor checkpoint adds five pinned files: two Kotlin and three Java.
Its `build/ir-recovery/descriptor-visitors/source-manifest.json` records exact
content matches. The investigated frontier at that checkpoint was 36 files: 30 Kotlin and six
Java. Materialization exposes dependencies; it does not translate them.

The source-location/base-contract checkpoint materializes eleven further pinned
files: two Kotlin and nine Java. The frontier is now 47 files: 32 Kotlin and
fifteen Java. Its manifest is `build/ir-recovery/descriptor-sources/source-manifest.json`.
The callable/value/variable/parameter files were read in full; they are not yet
translated. No existing source or sparse-checkout configuration was overwritten.

The visibility checkpoint materializes five more pinned dependencies: four
Kotlin files and one Java receiver interface. Its manifest is
`build/ir-recovery/descriptor-visibility/source-manifest.json`; the existing
FqName source was preserved. The investigated frontier is 52 files: 36 Kotlin
and sixteen Java. EffectiveVisibility and ReceiverValue were read for their
actual type contracts; their algorithms remain untranslated. Source exposure
expands the oracle inventory without establishing port completion.

The callable/parameter checkpoint materializes nine more pinned dependencies:
six Kotlin files and three Java files, including the source array/iterator
contracts and concrete descriptor implementation sources. The investigated
frontier is 61 files: 42 Kotlin and nineteen Java. Both manifests and exact
pinned-source revalidation are in `build/ir-recovery/parameter-descriptors/`.
Concrete descriptor implementation classes were read in full but remain
untranslated. Only the consumed read-only Collection/List/Iterator interfaces
are translated; mutable/set/map algorithms remain required. ConstantValue's
source base was read for its type contract; concrete constants remain open.

## Bounded transformer dependency for coroutine construction: 2026-10-06

Scope recorded before source translation: the constrained docking-ring objective
and standalone C++ requirement were revalidated. That design checkpoint claimed
no new source algorithm or MLX execution. The subsequent source implementation
and its unresolved compilation/consumer limits are recorded below.

Named source chain for the bounded dependency work:

- NativeSuspendFunctionLowering.kt:112-175, buildStateMachine, transforms the
  original body through ExpressionSlicer (:183-190) and a real transformer that
  remaps return/get/set nodes (:136-162). This creates the actual suspendable
  expression and label-field accesses used by Native coroutine construction.
- The consumed concrete transformer base is generated IrElementTransformerVoid.kt,
  which extends IrTransformer<Nothing?>. IrTransformer.kt:20-311 supplies the actual
  child-transforming and typed parent-delegation algorithms. The exact source
  Deprecated.kt defines IrElementTransformer as a methodless marker interface;
  it must not be invented as a replacement alias or fake node implementation.
- Docking-ring requirement: the plugin must build and rewrite real declarations,
  fields and suspension points, preserving declaration identity and child mutation.
  The current callback-supplied LLVM resume address does not satisfy that contract.

Bounded missing behavior: translate the actual transformer algorithms and required
source marker/interface, preserving all node-specific result types and child
transformation order, then its actually consumed void/context specialization.
Do not introduce dummy concrete IR nodes to instantiate visitor tests. The genuine
IR hierarchy and its typed covariance remain required source dependencies.

Stopping condition: all consumed transformer algorithms are present with exact
provenance/comment parity and are exercised against real source IR nodes, then
integrated into buildStateMachine and its production frontend consumer. A header
that merely parses, abstract-interface checks or source route comparisons do not
end this dependency task. Continue back to the concrete declaration/symbol and
state-machine consumers; do not expand into unused compiler visitor families.

The lookup-table/IR-attribute dependency chain remains unfinished and may proceed
only where its actual named consumer requires it. Standalone C++ execution must not
acquire a Kotlin runtime requirement through this compiler dependency work.

## Bounded value declaration/symbol dependency: 2026-10-06

Recorded before source changes. NativeSuspendFunctionLowering.buildStateMachine:
136-162 reads expression.symbol.owner and uses that exact value declaration as the
captured-field map key. IrToBitcode.evaluateGetValue/evaluateSetValue:1258-1278 and
SuspensionPointScope:2308-2317 requires the same declaration identity for scope lookup
and the actual LLVM suspension address. The direct source contracts are generated
IrValueDeclaration.kt:19-26 and generated symbols/IrSymbol.kt:146-152 (IrValueSymbol).
Their roots are IrSymbolOwner.kt:17-19 and src symbols/IrSymbol.kt:56-67,126-127.

Missing behavior: the typed declaration-to-symbol and symbol-to-owner contracts,
preserving exact borrowed object identity rather than names, IDs or substitute
objects. Clang's actual-interface probe rejects the direct narrowed virtual symbol
return because IrValueSymbol is incomplete. Reversing class definition order makes
the owner return incomplete instead. This is a C++ recursive covariance constraint;
supplying dummy class definitions or unchecked casts would not preserve the source.

The bounded language adaptation keeps source-named typed public getters and their
actual abstract virtual property dispatch through the real root reference types.
Typed narrowing occurs after both real interfaces are defined, using checked C++
reference conversion. There is one symbol/owner object and no separate binding
state, fallback, name dispatch or runtime simulation. Existing IrSymbolBase's real
unbound/rebinding algorithms must remain the sole binding behavior; adapt its
virtual boundary without changing its owner field or failure conditions. Preserve
all source generic bounds and source KDoc/provenance. This compiler type adaptation
does not define Native object/frame ABI or add Kotlin runtime dependency to C++ use.

Stopping condition: the actual typed contracts and boundary compile in real compiler
consumers, then actual IrVariableImpl and source symbols exercise one-time binding,
owner/declaration identity and source scope resolution before integration into the
Native builder/frontend. Header/type checks alone do not establish actual object
execution or finish this dependency. Continue actual declaration/node/attribute,
concrete descriptor and symbol implementation closure; do not expand unconsumed
symbol families. Exact probe diagnostics: build/ir-recovery/ir-value-identity/.

## Bounded IR element storage dependency: 2026-10-07

Recorded before source changes. Translate all of
`compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/IrElementBase.kt:23-190`, consumed
by IrDeclarationBase and IrVariable/IrVariableImpl. Their actual declarations
feed NativeSuspendFunctionLowering.buildStateMachine:136-162 and
IrToBitcode.VariableScope:549-568/SuspensionPointScope:2308-2317. Preserve dense
nullable slots, reference-identity lookup, complemented free index, previous
typed values, one-pair initialization, two-pair growth, swap-last removal,
debug snapshots and destination/source copyByDefault/includeAll merging.
Preserve the source's genuinely childless visitor defaults and transform-to-accept.

C++ representation: the compiler node uses the existing owning array with an
outer nullable allocation and initialized nullable Any slots. Keys retained by
source arrays/maps use shared handles to the same real compiler attribute object;
identity compares its canonical address. Kotlin's IrAttribute<*, *> projection
needs a private abstract C++ property boundary, constructible only by the genuine
IrAttribute<E,T> class. It exposes only the real copyByDefault property, supplies
no fabricated key instance, and does not implement the untranslated IrAttribute
debug/delegate behavior. Typed get/set retain the source key type and the existing
nullable/element boxing contracts. Source references are retained without claiming
Native runtime object layout or adding a Kotlin application dependency.

Split C++ implementation units for compilation, not behavior: lookup/mutation and
the source virtual bodies are independent of snapshot/copy map dependencies. Write
the complete map-consuming bodies against actual buildMap/emptyMap and imported
java.util.IdentityHashMap contracts; do not substitute a standard-container map,
omit the copy algorithm, or fabricate declarations for absent backing classes.
Full attributes, snapshots and copying remain uncompiled/unexecuted until those
genuine map/key/debug-owner dependencies close. Any further prerequisite expansion
must record its exact source/consumer and stopping condition before translation.

Stopping condition: real IR nodes and genuine attribute keys execute every source
operation, including copy flags, key/value lifetime, previous values and snapshots,
then actual variables exercise binding and enter the named Native scopes/builder.
Syntax/type checks alone do not finish this dependency or the docking-ring goal.

## Bounded initializer expression type dependency: 2026-10-07

Recorded before materialization/translation. Generated
`expressions/IrExpression.kt:19-24` is the actual type of
`IrVariable.initializer` and its traversal/replacement bodies (:33,38-44).
It inherits the actual IrElementBase, IrStatement and generated IrVarargElement,
and retains a mutable IrType plus a checked expression-returning transformation.
Translate that complete contract and its actual vararg-element ancestor, including
typed transforms through the existing generic visitor boundary. Do not invent a
concrete expression, dummy type or visitor to execute it. The consuming variable
feeds NativeSuspendFunctionLowering.buildStateMachine and the real value/suspension
scopes described above.

Stopping condition: actual variable traversal compiles with the real expression
inheritance and typed transformation bodies, then execute replacement against real
source expression nodes once their constructor/type dependencies close. Return to
real variables and scopes; do not expand unrelated expression families.

## Bounded actual attribute-key/delegate dependency: 2026-10-07

Recorded before source translation. IrAttribute.kt:42-160 supplies the actual
typed keys, nullable get/set, Boolean flag semantics, key creation and property
delegates consumed by IrElementBase.kt:48-189. Those actual element/declaration
objects feed IrVariableImpl and Native buildStateMachine/value/suspension scopes
through the chain already recorded above. Preserve key identity, copyByDefault,
weak debug-owner lifetime, both source weak reads in diagnostic text, and the
actual inherited object text for unnamed keys. Do not substitute an owner wrapper,
finite type probes, fabricated keys or a simplified diagnostic.

Translate complete key/delegate bodies against the real Any, WeakReference and
KProperty dependencies. Their genuine implementations remain unclosed; references
to those missing source dependencies must remain visible in compiler diagnostics.
No fabricated definitions or Kotlin application runtime requirement are authorized.
Map snapshot/copy dependencies remain separate unfinished work. C++ independent
static nested generic types use an explicit namespace, as in existing map entries;
do not introduce aliases or manufacture unrelated outer key instantiations.

Stopping condition: actual keys and real IR nodes execute previous-value/removal,
flag/null distinctions, key retention, expired debug owners, property names and
copy flags, then enter actual declaration/symbol construction and Native scopes.
Complete source bodies and compilation probes do not end this dependency task.
Return to the named IR consumers; do not expand reflection or object-runtime
families beyond their actual consumed contracts.

## Bounded Native property metadata dependency: 2026-10-07

Recorded before source changes. IrAttribute.kt:139-142,157-158 consumes the actual
KProperty<*> name when constructing keys/delegates. Its full Native base is
runtime/src/main/kotlin/kotlin/reflect/KProperty.kt:17, inherited from
KCallable.kt:13-29 and KAnnotatedElement.kt:13. KCallable's returnType is the
actual KType.kt:12-51 contract. These properties feed IrElementBase and actual
declarations, then the recorded Native buildStateMachine/value/suspension scopes.

Translate these four consumed contracts with exact comments and source ranges.
Native KAnnotatedElement and base KProperty genuinely declare no own methods;
do not add JVM annotation methods or invent callable/property instances. Preserve
name/return-type identity, classifier nullability, type-argument list and marked
nullability. Public out-type covariance shares one real abstract property boundary,
using the existing source-supertype transport; no aliases or second property state.
The seven parameterized/mutable property families are not consumed here and remain
untranslated in raw inventories, not replaced by declarations or implementations.

Stopping condition: actual metadata/property contracts compile in the attribute
consumer and actual property objects supply names/types while real keys/declarations
execute; return to those consumers. Abstract-type checks do not prove property
lookup, attribute execution or finish the docking ring. Do not expand unrelated
reflection operations or runtime families.

Any investigation: Native Any.kt:41,46-52 calls identityHashCode and actual class
metadata. Runtime.kt:100 names Kotlin_Any_hashCode; its actual pinned body in
Natives.cpp:40-49 returns the object address converted to KInt. The low 32 address
bits are the source algorithm in this revision, including zero for null. The
earlier claim that a pointer hash could not implement that operation was incorrect.
KClassImpl.kt:42-43 and TypeInfoNames.kt:40-45 separately require real class
metadata; typeid spelling or casting a C++ compiler object to Native ObjHeader
does not provide it. Ordinary C++ objects require no Native runtime. Metadata,
actual Any and weak-reference closure remain unfinished; these interfaces supply
no fabricated Any object.

## Bounded Native identity-hash dependency: 2026-10-07

Recorded before source changes. Runtime.kt:93-100 and Natives.cpp:40-49 provide
the exact consumed operation for Any.kt:41,46-52, then unnamed IrAttribute debug
text at :108-114. Actual keys feed IrElementBase, real declarations and the
first-priority Native variable/symbol/suspension scopes. Translate the nullable
identity operation only, including source comments and signed low-32-bit result.
The C++ erased pointer boundary must neither dereference Native object headers
nor transfer ownership, and ordinary C++ use must link without Native runtime.
Do not export a duplicate Kotlin_Any_hashCode symbol into Native applications.

Stopping condition for this bounded operation: an ordinary C++ execution and
transitive link check, plus comparisons with both the strong Native runtime
function and Kotlin intrinsic on the same real rooted objects, including null
and observations before/after GC. Return to Any/class metadata, weak references
and the actual attribute/declaration consumers. This operation does not finish
Any.toString, compiler object layout, optimizer memory effects, object relocation,
shared coroutine frames or either required MLX demonstration.

## Bounded real value-access and suspension-node dependency: 2026-10-07

Recorded before source materialization/translation. Native IrToBitcode.kt:1258-1278
reads IrGetValue/IrSetValue through the actual symbol.owner declaration; :2281-2340
reads IrSuspendableExpression and IrSuspensionPoint with a real IrVariable ID
parameter and separate normal/resume results. NativeSuspendFunctionLowering.kt:
119-170 rewrites captured argument reads/writes and builds these actual nodes.
The missing generated node types prevent the first-priority source scope path.

Translate the complete consumed IrDeclarationReference/IrValueAccessExpression,
IrGetValue/IrSetValue/IrSuspendableExpression/IrSuspensionPoint classes plus their
four generated implementation classes. Materialize only their exact pinned files
from local Git, preserving existing sparse sources/configuration. Preserve actual
IrSymbol/IrValueSymbol and IrVariable identities, mutable properties/offsets/type,
nullable origin and attributeOwnerId=this. Keep child visits and mutations in
source order, including the checked transformed ID-variable cast. Nongeneric
bodies belong in .cpp; no name/integer node-ID substitute or fake visitor/type.

Stopping condition: source node constructors/properties/traversal compile through
the existing actual interfaces and integrate in compiler targets, then real
variables/keys/symbol owners execute in Native scope lookup/state-machine building.
A compiled body or declaration is not execution acceptance. Any/class metadata,
weak references/maps/empty lists/IR-based descriptors remain genuine unfinished
consumer dependencies; this node work neither bypasses nor finishes them. Do not
expand other expression families or general compiler/collection features. Return
these nodes to the recorded VariableManager/CodeContext/Native suspension scopes.
Both required standalone and direct Native/C++ MLX GPU demonstrations stay open.

## Bounded actual Native Any prerequisite: 2026-10-07

Recorded before implementation/materialization. The actual AbstractCollection.kt
contains/toString operations and Native ArrayList structural equality/hash/text
require genuine object contracts. Existing IrType.kt:31 self-equality cannot
convert its object to Any; IrAttribute.kt:87-160 also includes the missing Any.hpp.
These consumers feed real declaration/attribute identity, class child rewriting,
and Native scopes/buildStateMachine. Translate Native runtime Any.kt:18-53's
complete public class and all three default method bodies, preserving virtual
dispatch and source comments. Reuse the existing exact Native identity_hash_code
operation; do not substitute std::hash, fabricated object metadata or C++ RTTI
spelling. Source pointer identity is correct for Any's default equals, not a
replacement for subclasses' structural equality.

Any.toString:46-52 consumes Long.toString(radix) at Native
kotlin/text/StringNumberConversions.kt:55, its longToString external entry
:43-46, and actual ToString.cpp:31-66,104-106. Translate only that consumed
Long path, with Native-wasm text/Char.kt's checkRadix and its two radix constants.
Keep nongeneric/internal algorithms in .cpp, preserve the negative-domain digit
loop for LONG_MIN and source error order, and return actual UTF-16 compiler string
values without a Native runtime dependency. No general text/numeric expansion.

Source this::class lowers to KClassImpl(getObjectTypeInfo(this)) through
PostInlineLowering.visitGetClass, then KClassImpl.kt's fullName getter. Write the
actual Any text body against those genuine dependencies; an absent KClassImpl
header or metadata implementation stays a compilation/link frontier, not an
invented class name or empty metadata result. No C++ object may be reinterpreted
as a Native ObjHeader. Compiler-owned class metadata and explicit Native object
metadata need their real ownership/binding implementations before accepting this
lookup. Standalone C++ must remain independent of Kotlin tools/runtime.

Stopping condition: close and integrate the real Any/class-metadata contracts,
then return to actual collection ancestors, ArrayList/copy-on-change and IR
attribute/type/declaration identity. Do not add Any inheritance to already linked
IR consumers until the complete method/link closure is available. Compile-only
source bodies or numeric checks do not certify actual Any instances, inherited
diagnostics, class metadata, owner binding or either required MLX GPU demonstration.

## Bounded consumed Native KClass contracts: 2026-10-07

Recorded before source expansion. Any.kt:46-52 consumes the actual dynamic
class-literal construction in PostInlineLowering.kt:81-93, KClassImpl.kt:16-43
and TypeInfoHolder.kt:12-14. KClass.kt:16-49 supplies its genuine public
interface, with methodless KDeclarationContainer/KAnnotatedElement/KClassifier
ancestors. KAnnotatedElement is already translated; materialize only the absent
shared KClassifier source from the pinned revision and translate the actual
remaining contracts. This chain returns to the named Any/collection/IR consumers
and Native declaration identity/scopes/buildStateMachine; it is not a general
reflection-library project.

Missing consumed behavior: real nullable simple/qualified/full names, canonical
type-information identity equality, pointer hash through NativePtr.kt:34 and
Primitives.kt:1851-1852, null-short-circuit instance query, fullName-based text,
checked TypeInfoHolder narrowing and the actual constructor field. Preserve
KClass<T : Any> invariance and source comments. One protected abstract projection
boundary represents KClass<*> for equality/fullName; only actual typed KClass
interfaces can construct it. It supplies no concrete object, type metadata or
replacement equality. Narrow source NativePtr only to its actually consumed
borrowed TypeInfo pointer; no general pointer/array expansion. The genuine
KCLASS_IMPL constant-constructor intrinsic must remain an explicit compiler
contract until its real lowering is implemented, not a zero/default metadata body.

The actual Native object entries are Natives.cpp:110-112 and Types.cpp:16-36,49-51.
They require real ObjHeader and compiler-generated TypeInfo. Compiler-owned C++
Any objects have no such header; do not reinterpret them, import Native runtime
as a standalone dependency, or invent object/class metadata. Keep getObjectTypeInfo
and instance-query compiler-object binding unresolved until actual metadata and
object representation are supplied. Existing Native-only TypeInfoNames stays
outside standalone production linking. RTTIGenerator.kt:198-269,576-618 identifies
the genuine generated metadata/name chain; this checkpoint does not authorize
porting all RTTI/backend families or treating C++ RTTI spelling as source metadata.

Stopping condition: the consumed KClass public/source algorithms compile with
exact provenance, then actual object/type metadata and the constant constructor
are integrated and exercised with the real Any consumer. An object compile or
pointer hash execution does not finish this dependency. Return immediately to
real collection/ArrayList copy and IR binding/scopes once that closure is available.

## Bounded Clang compiler-object metadata binding: 2026-10-07

Recorded before implementation. The consumed Native Any/KClass bodies use
KClassImpl.kt:72-81, Natives.cpp:110-112 and Types.cpp:16-36,49-69. They obtain
actual class identity, subtype relationships and reflection names. Native
ObjHeader/TypeInfo is not the representation of compiler-owned C++ objects;
using the current Native-only TypeInfoNames transport on such objects would
violate the standalone requirement. This is a required C++ ABI binding, not a
second coroutine runtime or a port of the entire Native object/GC implementation.

Bind real source-translated C++ class declarations inside Clang. Source class
annotations record the exact Kotlin package, relative class name and class/interface
kind from the pinned counterpart. They describe internal translated classes and
impose no base or annotations on ordinary application classes. Emit immutable
class-identity/name/superclass/interface data from those actual declarations;
generic specializations retain the source primary class identity. Keep this
compiler-owned data explicitly distinct from actual Native TypeInfo. Do not
construct fake Native headers, infer Kotlin names by demangling C++ RTTI, create
a registration table of invented classes, or silently return base metadata for
an unsupported concrete subclass.

The source metadata chain is RTTIGenerator.kt:198-269,576-613 and TypeInfo.h:43-63,
104-151. Its consumed reflection-name and subtype data need a private compiler
ABI projection; that projection is not a complete TypeInfo layout or Native
frame ABI. A private compiler-generated virtual accessor on translated Any
objects binds the actual most-derived C++ declaration to that immutable data.
The Clang stage supplies the accessor body/overrides and canonical globals;
absence of that required stage must leave an unresolved intrinsic, not a runtime
fallback. Translate Types.cpp's subtype/name-field algorithms against this data.
Split the existing TypeInfoNames source getter algorithms from its Native-only
transport, so compiler-owned names can execute without linking Native entries,
and preserve the existing actual Native transport/rooting tests.

Stopping condition: real Any and KClassImpl C++ instances execute source identity,
names/subtyping/hash/text through Clang-emitted metadata, standalone build/link
dependencies exclude Kotlin runtime, and Native name-transport regression remains
verified. The real KCLASS_IMPL constant-constructor lowering stays a separate
required binding if not implemented by this step. Then add the actual Any ancestry
needed by IR/collection objects and return to declaration/symbol binding and
Native scopes/buildStateMachine. No broad RTTI, GC or independent reflection work.

## Compiler-owned class binding integration checkpoint: 2026-10-07

The bounded compiler-object metadata binding now executes on actual translated
Any/KClassImpl objects: canonical identity across two translation units and generic
specializations, names/subtyping/equality/hash/text. Temporary diagnostic output
is removed. KotlinxCompilerObjects builds the genuine source bodies once for
kxs-inject, KotlinxCoroutinePass, kxs_codegen_test and KotlinxSuspendPlugin; the
object test links that same library. Mandatory internal Clang class-binding flags
are applied to those production compiler targets. Compiler-owned immutable data
remains separate from actual Native TypeInfo/ObjHeader/GC/frame contracts.

The Native-OFF CMake fixture builds the plugins, compiler tools, object test,
coroutine library and ordinary C++ application. Thirty-six selected target build
records contain no kotlinc/konanc invocation or Native transport source; six root
binaries and ten binary/shared-library dependency lists exclude Kotlin runtime.
The ordinary and object-test executables link only libc++/libSystem. Five main
focused tests and three Native-OFF focused tests execute with zero failures;
ordinary C++ produces 42/43/82. The real Native name regression still executes
72 observations before/after GC on its separately linked actual runtime.
The first external-app CTest directory found no tests; the corrected coroutine-port
directory executes the three tests. An initial ordinary-executable command used
the wrong bin path and did not start; its corrected command executes. Both earlier
attempts remain visible in evidence rather than being counted as verification.

Eleven relevant files retain 71 checked pinned provenance ranges and no prohibited
labels. Both required full-root ast_distance --deep runs exit 0: compiler 672/
library 354, 586 paired units/759 physical files. Required provisional score/
normalized-logic/span zeros and generated errors remain for Any/KClassImpl/
TypeInfoNames/StringNumberConversions; their target parsing has no errors.
KClassImpl remains 4/11 functions, 1/2 types and body similarity 0.10.
The Clang ABI projection does not complete Native RTTIGenerator, constant-constructor
lowering, actual IR construction or either complete MLX demonstration.

This closes the named binding's executable/dependency stopping condition. Return
to actual Any ancestry and collection ancestors/Native ArrayList required by
TransformIfNeeded (transform.kt:126-137), then actual descriptors/one-time symbol
binding and Native suspension scopes/buildStateMachine. The separate KCLASS_IMPL
constant-constructor intrinsic remains required. Evidence:
`build/ir-recovery/ir-class-binding-integration/`.

## Consumed Native class contracts checkpoint: 2026-10-07

Historical checkpoint; the compiler-owned binding integration above supersedes
its unexecuted-object and undefined-binding frontier.

The actual KClass<T : Any> interface, methodless KDeclarationContainer/KClassifier
ancestors and TypeInfoHolder property contract are translated. The existing genuine
Native KAnnotatedElement is reused. KClassImpl.hpp contains its consumed primary
constructor/field, nullable name getters, null-short-circuit instance query,
canonical metadata equality, pointer hash and fullName-based text. KClassIdentity.cpp
and KClassNames.cpp implement the complete consumed checked-holder and name getters.
The compiler constant-constructor declaration retains its actual intrinsic
annotation; real lowering is still required and no default metadata body exists.

Strict Apple Clang compiles the three nongeneric new bodies and the actual
AnyToString.cpp consumer. Twenty-five checks of actual types and four real consumer
functions compile without fabricated classes/metadata. Any text now reaches link
dependencies rather than the former missing header. Object-to-type-information
and instance-query bindings for compiler-owned C++ objects remain undefined,
and Native-only TypeInfoNames is not added to standalone compiler linking. No
Any/KClass C++ instance, new IR inheritance or new production integration is claimed.

The consumed pointer hash agrees with actual Kotlin/Native metadata in 21
observations: ten actual class/object types before/after GC plus the null pointer.
Kotlin source getter/hash bodies are unchanged inside renamed fixture extensions;
installed Native 2.4.10 supplies metadata/intrinsics/runtime, and the C++ entry
hashes the same actual TypeInfo pointer without reading or fabricating its fields.
The executable is arm64 Mach-O with the Native runtime linked by konanc. The C++
hash object has no unresolved external symbols. This does not execute the C++
KClass/Any object model or establish either complete MLX GPU demonstration. Two
initial Native builds lacked required internal/runtime opt-ins and used the old
GC package; both diagnostics are retained, then the corrected fixture executes.

Nine C++ files have 40 checked ranges and seven exact source KDoc blocks across
seven pinned sources. Only the missing shared KClassifier source was materialized;
existing files/sparse configuration were preserved. Records are in
build/ir-recovery/ir-kclass-contract/.

Both complete-root --deep commands exit 0: compiler 672/library 354 sources,
579 paired units and 752 physical files. Raw KClassImpl functions are 4/11 and
types 1/2, body similarity 0.10. Its unconsumed unsupported-class/associated-object/
check/downcast algorithms remain required gaps; NativePtr remains reported missing.
The four consumed interface/marker groups each have 1/1 source types. All five
affected deep groups retain provisional score/logic/span zeros, generated parse
errors and no target parse errors. No measurement requirement is waived.

Continue actual compiler object/class metadata and constant-constructor binding,
then return Any to the real list/ArrayList and IR attribute/declaration consumers.
Do not substitute C++ RTTI names or Native object-header casts to close that gap.
Native binding/scopes/buildStateMachine and both complete MLX demos remain open.

## Native Any prerequisite and ordinary C++ design checkpoint: 2026-10-07

Historical checkpoint; the current consumed class checkpoint above supersedes
the header frontier and inventory counts recorded here.

The standalone design and constrained goal now explicitly distinguish internal
translated compiler classes from ordinary application classes. C++ application
classes need no Kotlin base class or object storage, including with Native
interoperability enabled. Preserve their actual declarations, values and lifetime
operations when constructing coroutine frames.

The complete consumed Any public class and three source method bodies are written
in kotlin/Any.hpp:21, AnyIdentity.cpp:12,17 and AnyToString.cpp:18. Strict Apple Clang
compiles the identity bodies and the consumed Long formatting unit. The text body
stops at actual missing native/internal/KClassImpl.hpp; no fabricated metadata,
Any instances or newly linked IR inheritance was supplied. Real metadata/link
closure remains required before returning Any to the collection/IR consumers.
The source identity-hash question is preserved with its prohibited comment label
changed to "Upstream question"; that label change adds no implementation behavior.

The consumed Long formatting path executes 393 digit/error observations against
std::to_chars and source error text under AddressSanitizer/UndefinedBehaviorSanitizer.
These include zero, signed limits, unsigned-32-bit hash values, every valid radix
and invalid radix bounds. The binary links libc++, libSystem and the sanitizer
runtime, with no Kotlin runtime. This is a C++ helper check, not a pinned Native
execution comparison, Any instance acceptance or complete standalone MLX demo.
Three inaccurate Char source ranges were corrected to checkRadix:228-233 and
constants:236,237. Records are in build/ir-recovery/ir-any-contract/.

Both complete-root --deep commands exit 0: compiler 671/library 354 source files,
571 paired units and 743 physical target files. Raw Any functions/types are 3/3
and 1/1, with body similarity 0.53; raw StringNumberConversions functions remain
0/17 and Char remains missing. Both affected deep groups retain normalized logic
and span coverage zero, generated parse errors and provisional score zero; target
parse errors are absent. No measurement criteria were waived. Return through
actual Any metadata and Native list ancestors/ArrayList to copy-on-change, then
real IR binding and Native suspension scopes/buildStateMachine. Neither complete
MLX GPU acceptance demonstration is established.

## Native ArrayList prerequisite algorithms checkpoint: 2026-10-07

Historical checkpoint; the current Any checkpoint above supersedes these counts.

The four consumed shared MutableIterable/MutableList predicate remove_all/retain_all
overloads and both complete filter_in_place bodies are translated in
kotlin/collections/MutableCollections.hpp. They retain predicate/iterator order,
the actual RandomAccess branch, source forward compaction and reverse removal;
predicate failure does not gain transactional rollback. RandomAccess.hpp is the
complete source methodless marker. Private generic bodies remain visible because
the public generic operations instantiate them. Five compiled consumers use the
real abstract interfaces, including actual IrDeclaration pointer elements; no
list, iterator, IR node or visitor implementation was fabricated. These removal
algorithms have compile evidence, not C++ list execution or production integration.

ArraysNative.hpp now contains the complete generic copy_of_range body at :23;
Arrays.hpp:16 contains actual Native terminate_collection_to_array. The latter
retains array identity and trailing slots; it does not acquire JVM null termination.
All 23 array observations agree with Native 2.4.10 executing the exact pinned
range/validation/termination bodies (only fixture names and actual modifiers
adapted). The Native fixture calls its installed stdlib's copying intrinsic.
Strict Apple Clang 21/C++20 address/undefined-sanitizer execution verifies independent
copy storage, reference identity/lifetime, error category/message order and Native
trailing-slot behavior. Additional C++ assertions verify uninitialized slots remain
unread until explicitly initialized. This is compiler-owned Array storage evidence,
not actual Native ArrayHeader/GC or shared-state-machine execution. ArraysNative.cpp
also compiles strictly. The first Native fixture compile lacked a qualified entry;
its error is retained, and the explicit kotlin.collections.main entry compiles/runs.

Four affected C++ headers contain 26 checked ranges and 15 source KDoc blocks
across four exact pinned sources. Two new upstream files were materialized without
changing existing sources or sparse configuration. No source/scorer shortcuts,
prohibited comments or proof guards were introduced. Both full-root --deep commands
exit 0: compiler 670/library 354 sources, 567 paired units/738 physical files.
Raw MutableCollections functions are 6/33, Native Arrays 3/10, generated
_ArraysNative 9/240; RandomAccess is 1/1 types and 0/0 source functions. All four
affected deep groups retain provisional normalized-logic/span zeros and generated
errors, with no target parse errors. Raw missing functions and full priority
documents remain the oracle; compilation/execution does not waive these criteria.

The actual TransformIfNeeded.cpp still stops at missing ArrayList.hpp:20. Return
these prerequisites to complete actual Native AbstractCollection/AbstractMutableCollection/
AbstractMutableList, ArrayList backing/constructor/set and real same-list/copy-first-change
execution, then parameter/variable empty-list/descriptors/one-time binding and
Native scopes/buildStateMachine. Structural Any equality/hash/text and shared-view
lifetime/metadata closure remain required; no substitute list or fake Any is allowed.
Neither complete standalone C++/MLX nor direct Native/C++/MLX GPU demonstration
is established. Receipts: build/ir-recovery/ir-array-list-ancestors/.

## Earlier actual IR class list-rewrite checkpoint: 2026-10-07

The complete consumed Native MutableList contract is translated at
kotlin/collections/MutableList.hpp:50, with indexed mutation, both add_all
overloads, narrowed mutable iterators/sub-lists and source invariance. It uses
the existing real abstract collection/iterator boundary. No backing-list instance
or fake IR node/visitor was constructed. Twenty-eight genuine type checks and
three compiled consumer functions cover primitive and actual IR declaration
element types, indexed operations, shared mutable views and both rewrite calls.

ir/util/Transform.hpp:37,51,74,84 contains the two full consumed source algorithms
and their public generic boundary. The in-place range captures size once, casts
through actual IrElementBase and checks the transformed declaration type before
set. Copy-on-change keeps the original shared list unless identity changes, then
uses the actual source ArrayList constructor and subsequent indexed set. Its
withIndex lowering retains the actual wrapper's index increment/check before
iterator.next, unsigned storage for Kotlin Int wrapping, and has_next/next order.
The original compiler cast comment is retained. CollectionFunctions.cpp:12,21
contains the full consumed Native index check and shared overflow error body;
std::overflow_error is an explicit C++ error-category adaptation, not a Native
exception box or interop exception acceptance.

TransformInPlace.cpp:22, CollectionFunctions.cpp and the real IrClassChildren.cpp
compile with strict Apple Clang 21/C++20. The latter's include path was one directory
too high as well as lacking the real header; it is corrected at :10. Both source
class traversal/rewrite bodies now compile through the real parameter/list types
and nongeneric dispatch declarations. TransformIfNeeded.cpp:20 still stops at
missing actual ArrayList.hpp. Its complete written body is not a compiled copy
operation; the forward class declaration in the generic header supplies no
implementation. None of these new bodies or the class children is production
linked yet, and no class traversal/copy, parameter construction or owner binding
execution is established. No new runtime or CTest acceptance is claimed here.

Four previously absent Kotlin source files were materialized exactly from the
pin: transform.kt, Native ArrayList.kt, Iterators.kt and Iterables.kt. Existing
sources and sparse configuration were preserved. All 735 lines of the actual
Native ArrayList, the full transform/list sources and indexing dependencies were
read. Seven affected C++ files have 47 checked ranges across ten exact pinned
sources and nine complete source KDoc blocks. No prohibited comments or source
provenance mismatches are present in these changed files.

Both complete-root ast_distance --deep commands exit 0: compiler 668/library 354
source files, 565 paired C++ units and 736 physical target files. Raw transform
functions are 1/9; eight unmatched overloads/helpers remain listed, including the
copy operation whose header is rejected for mixed namespaces. Native List is 1/2
types, reporting List missing despite the existing actual inherited contract and
compiled type evidence, and retains forced function zero for its abstract source
inventory. Native Collections is 1/11 functions; the shared Collections file is
still reported missing. The newly visible Native ArrayList has 91 missing functions
and three types, including its nested iterator/sub-list. All three affected deep
groups retain provisional normalized-logic/span zeros and generated errors, with
no target parse errors. Transform.hpp's foreign template forward and private
generic boundary produce two retained namespace/provenance mismatch diagnostics;
the earlier forward-extraction defect evidence remains available. No scorer was
changed and no required criteria or raw missing findings are waived.

Next consumer closure: translate Native ArrayList's actual collection constructor
(:81-83), set (:103-109), addAll and their backing/ancestor operations, then return
to TransformIfNeeded and execute actual same-list/copy-first-change behavior.
Parameter empty-list singleton/bottom-type covariance, IR-based descriptors,
signatures, rendering/Any/attributes and concrete one-time owner binding remain
required before Native scopes/buildStateMachine integration. Ordinary C++ must
stay independent of Kotlin tools/runtime; both complete MLX GPU demonstrations
remain unfinished. Receipts: build/ir-recovery/ir-class-list-rewrites/.

## Bounded Native ArrayList ancestor algorithms: 2026-10-07

Recorded before expanding this prerequisite. Native ArrayList.kt:61 inherits
AbstractMutableList; that source class's removeAll/retainAll implementations and
AbstractMutableCollection.kt:57,66 consume the predicate overloads in shared
libraries/stdlib/src/kotlin/collections/MutableCollections.kt. Translate only
the four consumed MutableIterable/MutableList predicate overloads and both
filterInPlace bodies. Preserve iterator removal, predicate order, exception-visible
partial mutation, the real RandomAccess type test, stable forward compaction and
reverse trailing removal. Native RandomAccess.kt:11 is the genuine methodless
marker required by that branch and ArrayList's inheritance; it is not a backing
list substitute. Materialize these two exact pinned source files without changing
the Kotlin checkout or sparse configuration.

The same ArrayList.kt:205-218,546-561 toArray operations require the generic Native
_ArraysNative.kt:1254-1257 copyOfRange and Arrays.kt:91 terminator. Translate those
complete bodies using the existing compiler-owned Array storage and array-copy
algorithms. Native termination returns the provided array unchanged, including
its trailing slots; do not import JVM null-termination. Array tests certify this
compiler-storage contract, not Native ArrayHeader/GC or shared-frame compatibility.

Consumer chain: these operations -> complete actual Native ArrayList ancestor/
backing closure -> transform.kt:126-137's ArrayList(this) and set -> IrClass child
declaration rewriting -> Native buildStateMachine/ExpressionSlicer and suspension
scope identity. Stopping condition: return these complete prerequisites to their
actual ancestor/ArrayList consumers, then close and execute the recorded copy
operation. No generic collection feature expansion, fake list/iterator test nodes,
replacement ArrayList, scorer project or change in C++ ownership is authorized.
Both standalone C++/MLX and direct Native/C++/MLX demonstrations remain required.

## Bounded IR class list-rewrite dependency: 2026-10-07

Recorded before materialization/translation. Generated IrClass.kt:74-84 and the
actual IrClassChildren.cpp:14,22 consumer require Native MutableList and two source
rewrites at compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt:31-38,
126-137. This closes part of class-child declaration identity consumed by Native
buildStateMachine/ExpressionSlicer and the first-priority suspension scopes.

Translate the complete Native MutableList interface at libraries/stdlib/native-wasm/
src/kotlin/collections/List.kt:133-243, using the existing real read-only/mutable
collection and iterator contracts. Preserve indexed set/add/remove, both addAll
overloads, invariant mutable element type, narrowed mutable iterators/sub-list,
source comments and shared backing-view ownership. No backing-list substitute is
authorized by this interface task.

Translate the two complete consumed transform algorithms, including the source
IrElementBase cast, once-evaluated in-place range, checked result type, actual
iterator order and copy on the first identity change. Preserve the existing C++
shared list handle and borrowed IR nodes. The C++ generic virtual boundary requires
the existing IrTransformerDispatch adapter; nongeneric actual-declaration adapters
belong in .cpp, with provenance for both source generic operations.

transformIfNeeded's ArrayList(this) and set consume Native ArrayList.kt:81-83,
103-109 and their real backing/ancestor algorithms. Materialize/read its exact
735-line pinned source, but do not fabricate an ArrayList definition or replace it
with std::vector. A source-class forward declaration remains an unfinished
dependency, never construction acceptance. The withIndex loop is lowered using
the actual generated _Collections.kt:1835-1837, Iterables.kt:24-26 and
Iterators.kt:38-42 behavior: check the old index before reading the next element,
increment with Kotlin Int wrapping, and preserve next/hasNext order. Its consumed
Native checkIndexOverflow at Collections.kt:108-113 and shared throwIndexOverflow
at src/kotlin/collections/Collections.kt:507 are the only new collection helpers.

Stopping condition: compile the real MutableList contracts and in-place consumers,
then finish actual ArrayList copy/set closure and execute same-list/copy-first-change
semantics on actual classes. Return to IrClassChildren, concrete parameters/variable
binding and Native scopes/buildStateMachine. EmptyList/emptyList bottom-type
covariance must retain the actual singleton identity; a separate empty singleton
per C++ element type is not authorized. Those parameter-constructor dependencies,
IR-based descriptors, rendering/Any/attributes and both complete MLX GPU demos
remain required. Do not widen this into general collection or stdlib work.

## Actual IR parameter and default-body checkpoint: 2026-10-07

The complete consumed IrTypeParameter and IrValueParameter contracts, their two
concrete generated implementations, actual leaf symbols/implementations and
IrBody/IrExpressionBody are translated. TypeParameterDescriptor.hpp:27 preserves
the actual abstract descriptor API; TypeSystemContext.hpp:34 adds only the source's
methodless TypeParameterMarker. These are prerequisites for IrClass.kt:74-84 and
NativeSuspendFunctionLowering.buildStateMachine:112-181/ExpressionSlicer:183-335,
which must use actual parameter declarations and their owner/type identities.

IrTypeParameter.hpp:19/.cpp:11 and IrValueParameter.hpp:20/.cpp:11 retain typed
symbols, mutable source fields, checked transformations and the value parameter's
initial index_in_parameters=-1. IrValueParameter.cpp:24,28 traverses/rewrites its
nullable actual default body. IrBody.hpp:15/.cpp:10 and IrExpressionBody.hpp:15/
.cpp:10,19 retain the actual body/expression hierarchy, checked result casts and
expression-child order. These six nongeneric parameter/body/leaf-symbol bodies
compile with Apple Clang 21, C++20, Wall/Wextra/Werror. Thirty-three actual-type
checks and four compiled consumer functions succeed without fabricated instances.
This is compile evidence, not execution of parameter construction or traversal.

IrTypeParameterImpl.hpp:13/.cpp:14 and IrValueParameterImpl.hpp:13/.cpp:14 contain
the complete source constructors, field access, attributeOwnerId=this, genuine
empty-list initialization, descriptor-through-symbol lookup and symbol.bind(this).
Their first compile errors are the untranslated actual empty_list operation, at
IrTypeParameterImpl.cpp:33 and IrValueParameterImpl.cpp:41. The two generated leaf
symbol implementations use the existing single IrSymbolBase state; both stop at
its actual missing IrBasedDescriptors.hpp dependency. IdSignature and rendering
remain required. No concrete parameter instance has been constructed or bound,
and none of these new bodies is linked into production yet. Declarations do not
stand in for these missing implementations.

The real IrClassChildren.cpp consumer no longer stops at missing parameter headers;
its first compiler diagnostic is now actual MutableList.hpp at :10. The distinct
transformIfNeeded and in-place helper algorithms are also missing. Complete those
named consumers, actual empty-list and descriptor/signature/rendering dependencies,
then return the real nodes to class traversal and Native scopes/buildStateMachine.
Any/class metadata/attributes and concrete variable/parameter owner binding still
need execution and production integration. Do not widen this into independent
collections, compiler or measurement work or use fake nodes to claim execution.

Six previously absent pinned files were materialized exactly: five Kotlin and one
Java; existing IrValueParameter.kt and sparse configuration were preserved. Across
22 C++ files and eleven exact pinned sources, 267 provenance ranges are checked.
Seven KDoc blocks match exactly; the eighth retains the complete nested-comment
examples and upstream isHidden question, with its prohibited comment label replaced
by "Upstream question". The actual is_hidden flag/accessors are fully translated;
the source's question about dropping that flag does not authorize omitting it.
Source opt-in/sealed/setter annotation metadata is not claimed complete.

The existing five-target compiler build exits 0 and four focused CTest checks have
zero failures. Native-OFF ordinary_cpp rebuilds its transitive library/plugins and
returns 42/43/captured-object 82. Its cache has empty konanc configuration, graph
has no TypeInfoNames unit and executable links only libc++/libSystem, with no Native
runtime unresolved symbols. This scalar/capture fixture proves neither complete
C++ resource lifetime handling nor either required MLX GPU demonstration.

Both complete-root ast_distance --deep commands exit 0: compiler 664/library 354
source files, 560 paired C++ units and 730 physical target files. Raw parameter
functions are 1/2 and 1/4, IrBody 1/1 and IrExpressionBody 1/4. Remaining accept/
child-transform operations are unmatched under the C++ virtual dispatch boundary.
The concrete parameter files retain forced function zero for their source
constructor/property-only inventories. Generated IrSymbol is 6/23 types and
generated IrSymbolImpl 3/17; TypeSystemContext is 9/32 types and 0/74 functions.
All nine affected groups retain provisional normalized-logic/span zeros and
unsupported/generated errors; target parsing reports no errors in those groups.
The previously traced foreign-template-forward namespace mismatch remains visible
in the two declaration-container findings. No tool changes, suppressed findings
or waived criteria were used for this checkpoint. Neither actual Native shared
state-machine handoffs nor real MLX GPU execution is established.

Receipts: build/ir-recovery/ir-parameter-nodes/{source-manifest,provenance,
strict-body-receipt,type-receipt,standalone-receipt,deep-receipt,implementation-receipt}.json
and the retained build/test/diagnostic logs. First priority remains real IR identity,
concrete descriptors, one-time symbol owner binding and Native suspension scopes.

## Bounded actual IR parameter and default-body dependency: 2026-10-07

Recorded before translation/materialization. IrClass.kt:74-84 visits/transforms
actual type parameters and its nullable value-parameter receiver. NativeSuspendFunctionLowering
buildStateMachine:112-181 consumes actual receiver/result parameter declarations,
and ExpressionSlicer:183-335 uses their symbol/type identities. These feed the
first-priority actual declarations/owner binding and Native suspension scopes.

Translate complete generated IrTypeParameter.kt and IrValueParameter.kt contracts
and bodies, preserving covariance/check casts, every mutable field, receiver kind,
nullable vararg/default body, initial indexInParameters=-1 and child order. Translate
the two generated concrete Impl classes with original constructor field order,
attributeOwnerId=this, actual source empty lists, descriptor-through-symbol lookup
and symbol.bind(this). Use the existing borrowed compiler objects/immutable Name
representation; do not invent owners or bypass missing collection functions.

Their leaf IrTypeParameterSymbol/IrValueParameterSymbol and generated Impl classes
must retain the existing single IrSymbolBase owner/descriptor/signature state and
actual generic owner/descriptor bounds. TypeParameterDescriptor.java is the actual
consumed abstract descriptor contract; its methodless TypeParameterMarker at
TypeSystemContext.kt:22 is the sole new type-system scope. IrValueParameter.defaultValue
requires complete generated IrBody/IrExpressionBody, including checked transform
results and actual expression-child traversal. No other declaration/body/symbol
families are authorized by this bounded chain. Preserve the upstream isHidden
question in audit evidence while excluding its prohibited source-comment tag; the
actual flag and behavior remain translated.

Stopping condition: the source parameter/body algorithms compile through genuine
interfaces, then concrete parameter constructors bind actual symbols once and are
consumed by class traversal and Native state-machine/scopes. Actual empty lists,
IR-based descriptor selection, IdSignature/rendering/Any metadata and mutable
class transformation remain dependencies until implemented. A type check or
written constructor does not establish execution or complete lowering. Return
the real nodes to IrClassChildren and buildStateMachine, without fabricated test
classes/visitors/metadata or Kotlin runtime dependencies in ordinary C++.
Both required real MLX GPU demonstrations remain unfinished.

## Actual IR class contracts checkpoint: 2026-10-07

The consumed generated IrClass contract and its five inherited declaration
interfaces are translated, with the source flags, types, receiver, metadata and
class/symbol identity. IrClass.cpp:11,13 contains the typed symbol and visitor
entry bodies; IrClassChildren.cpp:14,22 preserves type-parameters/declarations/
receiver traversal and the distinct transformIfNeeded/in-place algorithms. Its
first strict compiler diagnostic remains missing IrTypeParameter.hpp; actual
IrValueParameter, MutableList and transform helpers are also required. These
child bodies and IrClass/IrClassSymbol are not linked into production yet. No
actual class, descriptor, symbol or replacement metadata object was constructed.

ClassDescriptor and its three source parents have complete abstract source
contracts, with actual borrowed scope/type/constructor identities and source
nullability. IrClassifierSymbol/IrClassSymbol retain the existing single virtual
owner/descriptor boundary. IrType now includes the actual classifier contract.
IrClassifierSymbol.cpp and ClassKind.cpp build in all three compiler targets.
The eighth consumed type-system marker, TypeConstructorMarker, is genuinely
methodless; the other 24 types and 74 body-bearing functions remain unported.
Opaque foreign dependency declarations, sealed/opt-in metadata and value-class
representation/boxing are not completed implementations.

Eleven previously absent files were materialized exactly from the pinned local
Git objects: seven Kotlin and four Java. Sparse-checkout configuration and existing
sources were preserved. Nineteen C++ contract/body files have 142 checked source
ranges and 18 exact KDoc blocks across fourteen pinned sources. Four strict body
compiles and 26 actual-type checks/two compiled interface call sites succeed;
these checks do not execute actual class instances or establish symbol binding.
All six ClassKind values and eight properties agree in 48 observations between
unchanged pinned Kotlin source compiled by Native 2.4.10 and strict C++ sanitizer
execution. Test assertions remain in tests. Four affected CTest checks have zero
failures. The initial combined build regenerated CMake but stopped at the new
unavailable target rule; explicit configure and the final five-target build exit 0.

The Native-OFF ordinary_cpp application rebuilds its transitive C++ library and
plugins and executes 42/43/captured-object 82 with only libc++/libSystem. Its cache
has no konanc executable, its build graph has no TypeInfoNames unit, and its
unresolved symbols contain no Native runtime references. This existing fixture
contains no MLX GPU work and establishes neither complete acceptance scenario.

Both full-root ast_distance --deep commands exit 0: compiler 659/library 354 source
files, 549 paired C++ units and 709 physical target files. IrClass raw functions
remain 0/3; its generic visitor operations are unmatched against C++ dispatch.
ClassKind is 1/1 types, 0/0 source body functions/eight target bodies and forced
function score zero; its two enum-member properties remain missing in the raw
inventory. TypeSystemContext is 8/32 types and 0/74 functions. Generated IrSymbol
is 4/23 types, retaining nineteen missing symbol families. IrDeclarationContainer
and IrTypeParametersContainer are reported as missing files despite exact source
headers, range/KDoc checks and compiled inheritance/type evidence; the raw matcher
findings are preserved for investigation. All eight measured affected groups
retain provisional normalized-logic zeros, seven have zero span coverage, and
unsupported/generated parse errors remain visible. Target parsing reports no
errors in those groups. Execution does not waive required measured criteria.

The two reported-missing headers have a traced namespace-extraction defect:
imports.hpp:648-677 recognizes ordinary forward declarations but counts a
foreign template forward as a full declaration. The List/MutableList forwards
produce mixed kotlin.collections/org.jetbrains.kotlin.ir.declarations identity,
which codebase.hpp:1489-1495 rejects before source-header matching. Four exact
IDENTITY_MISMATCH diagnostics are retained in oracle-namespace-investigation.json.
The measurement tool was not changed during this source checkpoint; this finding
does not waive the missing-file criteria or justify changing the source contracts
merely to influence a score.

Return to the actual child declarations and their collection/transform consumers,
then class metadata/Any/attributes, real variable construction and one-time owner
binding in Native buildStateMachine/scopes. IrDeclarationBase rendering and actual
IR-based descriptors remain genuine link dependencies. Do not manufacture compiler
classes or reinterpret C++ objects as Native ObjHeader to bypass this closure.
Both complete standalone C++/MLX and direct Native/C++ shared-state-machine/MLX GPU
demonstrations remain required. Receipts: build/ir-recovery/ir-class-identity/.

## Bounded actual IR class identity dependency: 2026-10-07

Recorded before source materialization/translation. CodeGenerator.kt:62 reads the
actual IrClass.llvmTypeInfoPtr; IrToBitcode.kt:2340-2341 reads the actual class
symbol owner. PostInlineLowering.kt:49-57 and :81-93 constructs class literals
using actual class symbols, argument types and runtime metadata values. Native
reflection/class-name construction for Any.kt:46-52 then feeds actual IrAttribute
keys and declarations, and the existing first-priority state-machine/scope chain.
NativeSuspendFunctionLowering also builds actual coroutine class declarations.

Translate the complete generated IrClass.kt contract and source traversal bodies,
with its five actual inherited declaration contracts: IrDeclarationContainer,
IrTypeParametersContainer, IrDeclarationWithVisibility,
IrPossiblyExternalDeclaration and IrMetadataSourceOwner. Preserve source class and
descriptor/symbol identity, all flags, source element, super types, nullable receiver
and value-class representation, sealed subclasses and arbitrary metadata. Visit
type parameters, declarations and receiver in that order. Preserve the distinct
type-parameter transformIfNeeded and in-place declaration transformations, then
the nullable receiver transformation. Use actual source helper imports; no copied
container, fabricated class, surrogate symbol or metadata constant may substitute.

Complete the consumed IrClassifierSymbol/IrClassSymbol interfaces through the
existing single virtual owner/descriptor boundary and actual marker identities.
Their actual source bounds require ClassDescriptor.java and its three inherited
interfaces, ClassifierDescriptor, ClassifierDescriptorWithTypeParameters and
ClassOrPackageFragmentDescriptor. Translate those complete abstract source APIs,
with real typed scope/type/constructor references, without descriptor instances.
IrClass.kind consumes ClassKind.kt; translate its complete enum and eight property
operations, with the explicit C++ enum/free-function mapping and raw oracle gaps
retained. The consumed TypeConstructorMarker is genuinely methodless at
TypeSystemContext.kt:21. Do not expand the other type-system families.
Any required generic descriptor/owner bounds remain enforced. The actual
ClassDescriptor, child nodes, MutableList and list-transform implementations are
genuine subsequent dependencies; missing source imports must remain visible in
compiler diagnostics until translated, not be replaced by aliases or test objects.
Do not expand unconsumed declaration/symbol/reflection families.

Stopping condition: actual class contracts/bodies compile with genuine dependencies,
then real class/variable/symbol objects are constructed and consumed by metadata
generation and Native buildStateMachine/scopes in production. Surface or abstract
type checks do not finish that dependency. Ordinary C++ compiler objects remain
independent of Kotlin runtime; direct Native object lookup is not a replacement
for compiler-generated class metadata. Both MLX demonstrations remain required.

## Bounded actual Native class-name dependency: 2026-10-07

Recorded before source translation. Native `Any.kt:46-52` reads
`this::class.fullName`; `KClassImpl.kt:42-43` consumes
`TypeInfoNames.kt:40-45`. Its simple/qualified-name getters are consumed by
`KClassImpl.kt:23-27`. Actual unnamed IrAttribute diagnostic text needs the
Any operation, and those keys feed IrElementBase, real value declarations and
NativeSuspendFunctionLowering.buildStateMachine/value/suspension scopes.

Translate TypeInfoNames.kt:13-54, preserving reflection flags, null and empty
package distinctions, both last-component delimiters and lookup order. The
Native implementation must consume actual compiler-generated TypeInfo through
the strong Types.cpp:53-69 entries and their real result-root ABI. Read actual
Native String length/code units through String.kt:40-51,64-66; do not decode a
guessed ObjHeader layout. C++ optional UTF-16 results are explicit copied values,
not Native object identities. Metadata is a borrowed actual Native TypeInfo
pointer, not a replacement NativePtr class or manufactured metadata. This unit
belongs only to the explicitly enabled Native boundary; ordinary C++ applications
must acquire no Kotlin runtime dependency from it.

Stopping condition for this bounded lookup: compare all three translated getters
with the unchanged pinned Kotlin source on actual Kotlin-generated metadata,
including nested, local, anonymous, Unicode and unqualified classes, before/after
GC. Integrate the fixture with the existing optional Native CMake suite. Return
to actual Any/class-literal construction and the named attribute/declaration
consumers. This lookup does not provide C++ compiler class metadata, implicit Any
ancestry, NativePtr's other operations, object/frame layout or either complete
MLX demonstration. Do not expand general reflection or string-runtime families.

## Actual Native class-name checkpoint: 2026-10-07

TypeInfoNames.hpp/.cpp translates the complete consumed source constructor and
simpleName/qualifiedName/fullName algorithms, with strong package/relative-name
runtime bindings and actual caller roots. The source non-null requirement is
retained; proof assertions and traces are test-only. Three KDoc blocks and nineteen
source ranges resolve to the exact pinned TypeInfoNames.kt and String.kt. No
source was rewritten or materialized, and no fabricated metadata/type instance,
weak entry point, runtime substitute or prohibited source comment was added.

NativeTypeNamesContract compiles the unchanged pinned TypeInfoNames.kt and compares
all three getters with the translated C++ operations on twelve actual Kotlin-created
class identities, twice with GC between rounds: 72 observations agree. The public
Native KClass simple/qualified getters are also checked by the Kotlin fixture.
Cases cover nested/inner/deep classes, singleton, local, anonymous, lambda, native
String/boxed Int, Unicode including a surrogate pair, and a default-package class.
The actual local name is create$Local: simpleName is Local, qualifiedName is null,
and fullName retains docking.metadata.create$Local. This exercises the '$' branch
without manufacturing a class name or reflection flags. Anonymous/lambda names
are hidden through flags but remain available to fullName. Source metadata identity
and actual root-stack restoration are checked across GC and lookup calls.

The optional native_type_names_contract CMake target compiles three C++ units to
bitcode with Native 2.4.10's LLVM 21 and links that installed runtime. Its CTest and
the existing native_reference_contract have zero failures. Pinned source execution
does not establish matching pinned full-runtime execution. The first fixture
attempt used a '$' backtick class rejected by this compiler; the source-local class
provides actual '$' metadata instead. Missing global opt-in and packaged entry-point
options were corrected in the fixture command. All initial diagnostics remain in
build/ir-recovery/ir-native-type-names/; no production algorithm was changed to
accommodate the fixture.

This unit is confined to the actual Native boundary. The core source glob excludes
tools, compiler targets do not list TypeInfoNames, and Native-OFF CMake does not
include the optional fixture. The ordinary_cpp target rebuilds the full transitive
C++ port/plugins, executes 42/43/captured-object 82, and has only OS dynamic-link
dependencies and no unresolved Native symbols. The broader all-target build exits
2 at JobTest.cpp:263,298 and AsyncTest.cpp:268, where ordinary delay/yield calls are
not lowered. Do not substitute explicit continuation arguments to mask the missing
authoring integration. Ren's running JobTest card remains separate and unchanged.

Both full-root --deep commands exit 0: compiler 652/library 354 sources, 534 paired
C++ units and 690 physical files. Raw TypeInfoNames measures 1/1 source types,
0/0 body functions with seven extra target bodies, and a forced function score
zero. Documentation correspondence is 0.214423; normalized logic/span coverage
remain provisional zero. File annotations, value-class/property and external-call
emission are unsupported; source @Escapes numeric-literal grammar errors are retained
at :49/:53. Target parsing has no errors. Do not turn these limitations into a
waiver or claim the required measured criteria are satisfied.

Return to Any.kt:46-52 and KClassImpl.kt:42-43, then the genuine attribute/declaration
consumers. This boundary does not implement class-literal metadata for ordinary C++
compiler objects, full NativePtr or KClass/value-class boxing, Any inheritance,
weak/property/map/list/descriptor closure, actual variable binding or Native scopes.
The real IrType self-equality probe still rejects IrType* -> Any*. Standalone C++
objects must not acquire a Native runtime dependency to bypass that missing metadata.
Both complete MLX GPU demonstrations and actual shared-state-machine handoffs remain
required. Receipts: build/ir-recovery/ir-native-type-names/.

## Bounded actual IR type dependency: 2026-10-07

Recorded before source materialization/translation. IrVariableImpl.kt:26-40 stores
an actual IrType; IrValueDeclaration.kt:25 and IrExpression.kt:20 expose that same
object. Native IrToBitcode.kt:1258-1278 resolves these actual declarations and
:2281-2340 uses typed suspension nodes. The declaration-parent diagnostic calls
RenderIrElement.kt:37-38/:367-375, whose variable path consumes the actual type
hierarchy. Full renderer source was read (1154 lines); translating its unrelated
node families would not close the first-priority variable task by itself.

Translate the complete IrType.kt:16-99 family, its real TypeRefMarker.kt:10-17 and
only the seven complete consumed TypeSystemContext.kt marker declarations at
:19-20/:24-26/:29/:34. Preserve actual annotation containers, IrClassSymbol and
IrClassifierSymbol references, nullable original KotlinType, nullability, type
argument/projection/star distinction, and abstract symbolic equals/hash contracts.
Translate complete Variance.kt:8-44 because IrTypeProjection/IrSimpleType expose
that source enum and the renderer consumes its label. Keep default getters in
.cpp, actual required interfaces abstract and source naming/provenance/KDoc.
Do not invent concrete Any, type, classifier, annotation collection or metadata.
The source sealed-family metadata remains explicit unfinished work, not assumed
from C++ inheritance. Do not expand unrelated TypeSystemContext algorithms.

Stopping condition: actual type/default and variance bodies compile/integrate in
existing compiler targets; actual typed consumers retain the type/classifier/
argument identities. Compare the complete consumed variance/nullability operations
with unchanged pinned Kotlin source compiled by Native, without manufactured IR
instances. Return to actual variable/symbol construction, renderer inputs and
Native scopes/buildStateMachine. Concrete types/equality, collections/annotations,
object metadata and actual renderer execution remain genuine dependencies.
Neither required standalone C++/MLX nor direct Native/C++ state-machine/MLX
acceptance is established by these contracts or primitive enum observations.

## Actual IR type and enum-operation checkpoint: 2026-10-07

Previous goal turn was progress: actual get/set/suspension nodes and a verified
renderer link frontier. Full RenderIrElement.kt (1154 lines) was read; its variable
path requires actual type/origin/name objects. This turn writes the complete
explicit IrType.kt:16-99 interfaces/default bodies, complete TypeRefMarker and
seven complete consumed marker interfaces, plus all Variance.kt operations.
Six production files retain 63 source ranges/four exact KDoc blocks. Three new
pinned source materializations expand the source inventory; sparse configuration
and existing bytes are preserved. Other TypeSystemContext algorithms are unported.

Variable/expression headers now include actual IrType. The source type object,
classifier/error-class symbol, argument collection, annotation container and
projection/star/nullable distinctions remain typed; no LLVMTypeRef or manufactured
IR object replaces them. Actual self-type/default-null/origin symbol properties,
simple-type invariant variance and question-mark nullability bodies live in .cpp.
The default null is the original source getter, not missing concrete type behavior.
Variance preserves source labels, position checks, signed-factor composition,
opposite and text; its enum constructor properties are private C++ static data.
The C++ enum has free operation functions, an explicit source-to-C++ API mapping
whose raw member-function mismatch remains a required finding.

All three existing compiler targets rebuild with the real type/variance bodies.
The initial combined Make invocation built those targets but could not see the
newly generated test target in its earlier top-level rule snapshot; a subsequent
target build succeeds. Both logs remain evidence. Twenty actual-type checks
compile without concrete IR instances; all eight existing new node bodies also
compile against the genuine complete type header. The actual type bodies compile
with strict Apple Clang C++20 Wall/Wextra/Werror.

Thirty-five consumed source-operation observations in fourteen rows agree with
Native 2.4.10 machine-code execution: full unchanged pinned Variance.kt and the
exact complete IrType.kt:46-54 nullability enum. All enum combinations are covered.
The strict address/undefined sanitizer executable produces that same output with
no diagnostics. The C++ executable loads only libc++/libSystem; no Kotlin application
runtime is needed for these source operations. Three affected CTest checks have
zero failures in 7.72 seconds. Primitive enum observations do not establish actual
IR type equality, declaration identity, scopes, Native shared frames or MLX/GPU.

A direct real-interface compile probe exposes unfinished implicit Any ancestry:
IrType* cannot be supplied as Any* to its own equals contract. Actual Any/class
metadata and sealed-family metadata remain unfinished; no alternate Any definition
or replacement type was supplied. Current consumer probes still stop at actual
empty_list (IrVariableImpl), RenderIrElement (IrDeclarationBase) and IR-based
descriptors (IrVariableSymbolImpl). The bounded stopping condition remains unmet.
Return these explicit type contracts to genuine object/attribute/variable/binding
and rendering dependencies, then Native value/suspension scopes/buildStateMachine.
Do not expand independent type-system/compiler/collection projects.

Both full-root --deep commands exit 0: compiler 652/library 354 sources against
complete src 532 paired units and 687 physical files. IrType inventories 8/8 types,
0/1 body-bearing functions; Variance 1/1 types, 0/4 functions. Actual companion/
enum operations are unmatched against free C++ function spelling. TypeRefMarker
is 1/1; TypeSystemContext is 7/32 with 25 missing types and 74 unported body-bearing
functions. All four groups retain provisional normalized-logic/span zeros and
unsupported/generated parse errors, with no target parse errors. Preserve all
raw gaps and required criteria; source/compile/execution evidence waives none.
Both required standalone C++/MLX and direct Native/C++ shared-state-machine/MLX
GPU demonstrations remain unfinished. Receipts: build/ir-recovery/ir-type-contracts/.

## Real value-access and suspension-node source checkpoint: 2026-10-07

Translated ten complete generated Kotlin node classes into eighteen C++ files:
IrDeclarationReference, IrValueAccessExpression, IrGetValue, IrSetValue,
IrSuspendableExpression, IrSuspensionPoint and the four concrete implementation
classes. These are the actual types consumed by Native IrToBitcode.kt:1258-1278
and :2281-2340 and NativeSuspendFunctionLowering.kt:119-170. Reads/writes retain
the real IrValueSymbol; a suspension point retains its actual IrVariable ID,
normal result and resumed result. All offsets/type/origin/owner properties and
source-order child visits/rewrites have bodies. The checked transformed ID cast
retains the source IrVariable requirement; attributeOwnerId starts as this.
Borrowed compiler-owned identities are not cloned or replaced with names/integers.

All eight new implementation bodies compile under Apple Clang 21 C++20 with
Wall/Wextra/Werror. Twenty-seven actual-type/constructor checks and four actual
interface consumer bodies compile without constructing dummy or concrete nodes.
All 177 provenance ranges and six KDoc blocks resolve to ten exact pinned sources.
Four formerly sparse source files were materialized from the existing pin without
changing sparse configuration, expanding the compiler inventory from 645 to 649.

Actual compiler link probes add the complete node family and real compiled
IrElementBase/IrExpression/IrVariable/IrVariableSymbol bodies. Adding the already
translated ArrayUtil range body closes that independent undefined symbol. The
remaining link failure is genuine IrDeclarationBase parent/set_parent/typeinfo;
its strict body compilation stops at missing RenderIrElement.hpp. Its source
parent error requires actual IR rendering. No fabricated renderer, parent body,
RTTI, IR type, visitor, declaration or attribute key was supplied. The first probe
had an incorrect working directory; its command and correction are preserved and
that invocation error is not classified as a source defect.

These node files remain source drafts outside production CMake targets. No real
node construction, traversal, owner binding or suspension-scope execution has been
established. The bounded stopping condition remains unmet. Close the recorded
actual rendering/object/attribute/empty-list/IR-based descriptor dependencies,
then return these nodes to VariableManager/CodeContext, Native suspension scopes
and buildStateMachine. Do not expand unrelated expression families or independent
compiler/collection features. Source bodies and compilation cannot certify either
required ordinary-C++/MLX GPU or direct Native/C++ shared-state-machine/MLX demo.

Both full-root --deep commands exit 0: compiler 649/library 354 source files,
complete src 527 paired units and 680 physical files. All ten new groups match
one actual class each. The four visitor-bearing interfaces retain 0/1, 0/3, 0/3,
0/3 function matches: ten public generic operations are unmatched against the
actual nongeneric C++ virtual dispatch spelling, with corresponding target extras.
Four implementation groups retain forced function-score zeros because the source
has constructor/property declarations and the target has their explicit bodies.
All ten groups retain provisional normalized-logic/span zeros, unsupported class
emission/generated parse errors and no target parse errors. Exact inherited
public API compile checks do not waive those measured criteria. Preserve raw
findings; no scoring aliases or substitutes were added. Receipts and raw node
extract: build/ir-recovery/ir-value-suspension-nodes/.

## Frontend root build and standalone authoring checkpoint: 2026-10-07

Previous goal turn: progress, with four real abstract contracts and executed
identity-hash evidence. This turn corrects authoritative production build wiring.
The earlier "frontend unavailable" diagnosis was incorrect: the core checked for
the frontend before the root created it. The target existed and the pipeline was
not applied to the core by that branch. Root frontend registration now precedes
tests, and the existing authoring function applies both frontend and LLVM stages
to the core and coroutine executables after the targets/functions exist.
Requested authoring requires actual LLVM/Clang packages and shared registry targets;
missing packages no longer silently skip it or select static registries.

Actual generated core/test flags contain -Xclang -load/-add-plugin and
-fpass-plugin together. Debug core/suspension/compiler targets build. A separate
Release CMake application adds the actual source tree, disables Native interop and
Native compiler tests, builds both plugins and the full C++ static coroutine
library, and runs the existing suspend_lambda.cpp fixture. It emits:
"suspend lambda:42; ordinary:43; captured this:82". Its complete target closure is
the C++ port, frontend/LLVM plugins, shared Clang/LLVM and Threads; the executable
loads only libc++ and libSystem and has no unresolved Kotlin runtime entries.
The fixture proves immediate/resumed/error results, ordinary receiver use and
captured C++ object identity, not general lifetime coverage or MLX execution.

The initial Release build failed because two existing translation units omitted
Clang's inline definitions. Adding ASTContext.h to RestrictSuspensionUtils.cpp
and IrTypeUtils.cpp closes those definition dependencies without changing the
translated algorithms, optimization or shared registry linkage. Both exact pinned
Kotlin sources were read and retained; source ranges stay unchanged.

Four existing pipeline checks report zero failures in 308.63 seconds, including
sanitized ordinary/forced-include authoring, retained locals/repeated suspension,
errors/cleanup, the actual Native callback-chain fixture and ordinary IR pipeline.
The Native fixture still uses StableRef handles/callback adapters between distinct
frames; it is not the required direct shared-state-machine/MLX demonstration.
After rebuilding the final header changes, three focused analyzer/tail/core checks
report zero failures in 1.23 seconds. A requested missing-Clang package probe fails
configuration as required. Initial missing-LLVM and overly broad library-name
probes are preserved with their corrections in the receipts.

Both final full-root --deep commands exit 0: compiler 645/library 354 source files,
complete src 517 paired units / 662 physical files. Actual IR identity, descriptors,
attribute/object dependencies and scopes remain unfinished. Raw RestrictSuspension
function matches remain 1/2 and IrTypeUtils 1/47; missing APIs and provisional
normalized logic/span zeros remain required findings. No source parity criterion
is waived by build or execution evidence. Return to actual Any/class metadata,
weak references, keys/variables/symbol binding and Native suspension scopes.
Both full standalone C++/MLX and direct Native/C++ shared-state-machine/MLX GPU
acceptance remain open. Evidence: build/ir-recovery/frontend-cmake-repair/.

## Bounded frontend CMake integration repair: 2026-10-07

Recorded before build-file edits. The goal requires ordinary CMake compilation
through frontend frame construction and mandatory LLVM injection. The actual
frontend target exists in build/ir-recovery/lib/KotlinxSuspendPlugin.so and is
created by the root build after src/CMakeLists.txt already tests TARGET existence.
The prior unavailable-target diagnosis inferred a missing plugin from that early
warning; it was wrong. The compiler dependency work remains first priority, but
this source ordering prevents the core build from applying the existing frontend.

Move frontend registration before coroutine-test registration, remove the premature
library warning/flag branch, and apply the existing kxs_enable_suspend_dsl function
after both plugins and the package functions exist. When the frontend is requested,
require actual Clang/LLVM development packages and shared registry targets instead
of silently skipping or choosing a second static registry. Preserve the explicit
LLVM-only path for current Continuation-ABI callers when authoring is disabled;
requesting authoring must never silently disable it. No Kotlin compiler algorithm
is changed by this build wiring; the translated source provenance stays intact.

The separate optimized standalone build exposed two missing Clang definition
includes: RestrictSuspensionUtils.cpp uses redeclaration lazy pointers and
IrTypeUtils.cpp uses base lists and Type::getAsCXXRecordDecl. ASTContext.h defines
the lazy pointer body and includes Type.h's actual inline definitions. Debug
linking had accidentally obtained those emitted definitions from other files;
Release did not. Add the actual defining header to both consuming translation
units. Preserve the existing Kotlin functions/comments/ranges and shared registries;
do not change optimization or use static linkage to hide this dependency error.

Stopping condition: normal root configuration has no misleading missing-target
warning, actual core/test compile commands include both frontend and LLVM stages,
targets build and affected execution regressions run. Also verify that requested
frontend configuration fails on genuinely missing packages. Then return to actual
object metadata/attribute/variable construction and Native scopes. This build repair
does not establish shared Native state machines, full source parity or MLX GPU work.

## Native property contracts and identity-hash checkpoint: 2026-10-07

Four actual Native contracts are translated: KAnnotatedElement, KCallable, base
KProperty and KType. Source methodless interfaces stay methodless; the callable
name/return type share one abstract root through covariance. KType preserves
nullable classifier, actual type-argument list and marked-nullability properties.
All four headers compile under strict Apple Clang 21. Eighteen actual-type checks
and five interface consumer bodies compile without fabricated instances. Fourteen
ranges and nine KDoc blocks resolve to four exact pinned files. Intrinsic constant
evaluation is recorded as the source annotation, not implemented behavior.
Actual classifier/projection/property instances and general/star variance remain open.

The exact Native identity_hash_code operation is now a nongeneric .cpp body,
linked into three existing compiler targets and the existing name contract target.
It reads no object header, borrows its argument and requires no Kotlin runtime.
An independently built Apple Clang executable uses actual Name and std::string
objects, verifies null/repeated identity, and links only libc++ and libSystem.
This is a bounded helper dependency check; whole standalone coroutine/MLX build
and transitive dependency acceptance remain outstanding.

The real Native fixture compares the translated operation against both the actual
Kotlin_Any_hashCode code and Kotlin identityHashCode on the same rooted objects:
ten observations, including null and before/after GC. Roots are released and the
actual weak references expire. Existing ten root checks and one return-slot check
remain observed. Native 2.4.10 uses matching LLVM 21.1.6; this is not a build of the
full pinned runtime. The linked symbols are actual local text definitions, not
weak or undefined substitutes. An initial global-only symbol filter omitted the
internalized definitions; the complete symbol table resolves them. All four
affected CTest checks report zero failures. Six ranges, one KDoc block and all
seven runtime-body comment lines match two exact pinned source files.

Both final full-root --deep commands exit 0: compiler 645 and library 354 source
files, complete src 517 paired units / 662 physical files. Earlier checkpoints'
"src files" counts referred to paired units; use the explicit distinction here.
All five new groups retain provisional normalized-logic/span zeros, unsupported
emission/generated parse errors, and no target parse errors. Documentation scores
are KAnnotatedElement 0.824310, KCallable 0.232495, KProperty 0.829177, KType
0.964562 and Runtime 0.111290. Runtime identityHashCode is PRESENT in the full
symbol inventory; body-bearing inventory excludes the external declaration and
retains 0/3 whole-file matches. Other Runtime operations remain untranslated.
KProperty retains seven missing families and nine missing methods. KCallable's
inherited name/returnType and KType's qualified-list arguments still appear missing
in the raw inventory despite genuine abstract consumer compilation. Keep those
representation findings and all required criteria visible; add no scoring aliases.

Actual IrAttribute inclusion still stops first at missing Any.hpp. Class metadata,
Any/WeakReference, concrete properties/keys/maps/nodes, binding and Native scopes
remain unfinished. The early CMake warning was subsequently traced to target registration order,
not an absent plugin; the frontend build checkpoint above records that correction.
These compiler-reference builds do not prove the complete authoring pipeline. Return to the recorded attribute/variable consumers,
not unrelated reflection/runtime families. Both full C++/MLX and shared Native/C++
state-machine/MLX demonstrations remain required. Evidence:
build/ir-recovery/ir-property-contracts/ and ir-native-identity-hash/.

## Actual attribute-key/delegate source checkpoint: 2026-10-07

IrAttribute.hpp now contains the source key, both actual delegate classes, flag,
factories, extension get/set and property operations from IrAttribute.kt:42-160.
Factories and independent static nested families follow source order. Source
reference objects are retained as shared handles to the same key/flag/delegate;
the debug owner uses the genuine WeakReference contract. Diagnostic text preserves
both weak reads and the source inherited text for unnamed keys. A Boolean key can
hold false/true/absence; a flag stores true or removes the association for false.
Generic public bodies remain in the header as required by their source API.

This is an uncompiled source draft. Strict Apple Clang 21 first reports the real
missing kotlin/Any.hpp. WeakReference and KProperty implementations, full nonnullable
T-bound/star-property variance, actual key/node construction and attribute/map
execution remain unverified. No substitute object, property, owner or weak-reference
definition was provided. These are compiler-internal dependencies and may not
introduce Kotlin build/runtime dependencies into standalone application use.

26 ranges resolve to the exact pinned IrAttribute source; all seven source KDoc
blocks retain text with comment whitespace normalized. Full reading also corrected
two existing projection markers in IrElementBase.hpp: the constructor is :87-91
and copyByDefault is :90, rather than the previously inaccurate :89-93/:92.
The prior range-presence receipt checked valid bounds, not these property associations.
The new projection receipt records the correction explicitly.

Both final full-root --deep commands exit 0: compiler 645, library 354, complete
src 512 files. Raw attribute functions match 7/14. The seven flag/delegate methods
and three nested classes remain missing under independent C++ namespace spelling;
their actual target methods are also reported as extras. Normalized logic/span
coverage remain provisional zero with unsupported class emission and generated
parse errors; raw documentation correspondence is 0.085679 despite retained KDoc.
Keep all raw criteria and findings open. An earlier source-order probe mismatched
the irAttribute factory to the same-named constructor; the final report matches
the factory correctly. That observation does not prove general overload resolution.

Measurement investigation: callable_identity.hpp:22-31,42-65 builds a canonical
owner key and accepts equal or suffix owner paths; it handles companions but has
no representation for independent Kotlin static nested classes lowered into C++
namespaces. Thus IrAttribute::Flag does not equal ir_attribute_types::Flag.
transliteration_engine.cpp:541-551 filters on that owner rule, then breaks equal
score ties by source/target extraction order. The ownerless-source acceptance in
callable_identity.hpp:63 also admits the same-named C++ constructor candidate for
the free irAttribute function. These explain the observed missing nested methods
and earlier factory mismatch; no tool rule or numerical result was changed.

Receipts: build/ir-recovery/ir-attribute-keys/. Actual Any/weak/property and map
closure remains required before executing these keys against real declarations.
Then return to actual IrVariableImpl constructor/binding and the recorded Native
value/suspension scopes and buildStateMachine. No new production integration,
coroutine execution or either complete C++/Native/MLX demonstration is established.

## IR element storage and initializer checkpoint: 2026-10-07

IrElementBase.hpp:51 and .cpp:23 now contain the actual dense-slot lookup,
typed previous-value get/set, first-pair allocation, two-pair growth, swap-last
removal, and source childless visitor/transform bodies. Signed size arithmetic
retains Kotlin wrapping. IrElementBaseAttributes.cpp:26,45 contains the complete
snapshot and destination/source merge algorithms, but compilation stops at the
genuine missing Maps.hpp; IdentityHashMap and actual IrAttribute key/debug/delegate
behavior also remain untranslated. The private abstract key-property projection
is constructible only by the actual future source key; no key instance was invented.

The real generated IrExpression.hpp:18/.cpp:11 and IrVarargElement.hpp:15 are
translated, preserving mutable IrType and checked expression-returning transform.
Two exact generated sources were materialized from the existing pin without
overwriting files or changing the checkout revision. Strict Apple Clang 21
compiles IrElementBase.cpp, IrExpression.cpp, and the existing IrVariable.cpp and
IrVariableSymbol.cpp with zero diagnostics. Declaration-base rendering, the actual
empty-list singleton and IR-based descriptors still prevent the other constructor
and symbol implementation bodies from compiling. No concrete node/key, variable
construction, binding, traversal execution or production integration is established.

41 provenance ranges resolve to four exact pinned files; all four consumed KDoc
blocks retain source text with comment whitespace normalized. Both full-root
--deep commands exit 0: compiler 645, library 354, complete src 511 files.
IrElementBase matches 8/11 functions, with three generic visitor operations
unmatched against the actual virtual dispatch spelling. IrExpression matches its
one transform; the genuinely methodless IrVarargElement retains a raw function
zero. IrAttribute remains a missing file. Raw normalized logic/span coverage stay
provisional zero with unsupported class emission and generated parse errors;
documentation correspondence is 0.306041/0.412996/0.623610. The symbol inventory also
misses IrExpression.type and mispairs the base _attributes storage with its snapshot
getter. Preserve these reports; neither compilation nor provenance waives criteria.

Receipts: build/ir-recovery/ir-element-storage/. Return to actual key/map,
empty-list/render/descriptor prerequisites, then execute real variable binding
and the named Native value/suspension scopes and buildStateMachine. Standalone
ordinary C++ use remains independent of Kotlin tools/runtime; both complete
C++/MLX and Native/C++ shared-state-machine/MLX demonstrations remain unverified.

## Bounded null-array growth dependency: 2026-10-07

Recorded before source changes. `IrElementBase.initializeAttributes:100-106` and
`copyAttributesFrom:156-189` allocate arrays of nulls; `addAttributeAt:125-135`
grows their dense key/value storage through `Array.copyOf(newSize)`. These are
required by actual `IrDeclarationBase`/`IrVariableImpl`, whose declaration identity
is consumed by `NativeSuspendFunctionLowering.buildStateMachine:136-162` and
`IrToBitcode.SuspensionPointScope:2308-2317`. This is compiler-owned storage, with
no Kotlin application runtime dependency and no claim to Native ArrayHeader ABI.

Missing behavior: translate Native `ArrayIntrinsics.kt:12-20` arrayOfNulls,
native-wasm `Arrays.kt:93-106` copyOfNulls overloads and generated
`_ArraysNative.kt:1232-1243` object-array copyOf(newSize), preserving null padding,
nullable type idempotence, bounds/error order, reference identity and independent
copy storage. Retain the real copyInto/array-copy algorithm for nullable destination
types rather than substituting the uninitialized-tail copy helper. Existing C++
uninitialized slots must remain distinct from initialized null values.

Stopping condition: the consumed operations execute against the actual translated
C++ array and matching Kotlin/Native operations, including readable null growth
and reference retention, then return to IrElementBase dense attributes and its
real key/map dependencies. Do not expand other primitive arrays or collection APIs.
Array execution alone does not finish IR variables, state-machine lowering or
either required C++/Native MLX demonstration.

## Null-array growth checkpoint: 2026-10-07

The recorded IrElementBase dependency now has source-bodied null operations:
`kotlin/ArrayIntrinsics.hpp:39` array_of_nulls; `collections/Arrays.hpp:14,31`
copy_of_nulls range/size; `collections/ArraysNative.hpp:108` public object-array
copy_of(newSize). Null-tail growth differs from the existing uninitialized-tail
copy. All new slots are readable nulls; pointer/shared-handle/optional/Any nullability
is idempotent. The consumed nonnullable-to-nullable transport preserves existing
array-copy bounds order and overlap direction. An invalid int-to-float array copy
is rejected at compilation. Wider source array covariance remains open.

Strict Apple Clang 21 with address/undefined sanitizers executes 930 observations
that exactly match installed Kotlin/Native 2.4.10 with matching LLVM 21.1.6. The
68 new observations cover allocation, all selected range/bounds combinations,
truncate/grow sizes, signed overflow, nonnullable promotion, independent storage,
readable null tail and reference identity/retain/release. Compiler C++ Name objects
and Native GC-managed arrays verify their respective lifetimes; these are not
same-object runtime-layout observations. A first same-function Native lifetime
check failed; moving construction/identity checks into a helper frame resolved it.
The retained first receipt and corrected test show the change; a retained temporary
is the interpretation, not a measured runtime root dump. Production was unchanged.
Leak detection is disabled because this macOS sanitizer configuration does not
support it; no address/undefined diagnostics occurred.

Five affected CMake targets build with Homebrew Clang 23.1.2. Four focused CTest
checks have zero failures in 7.76 seconds. The ordinary C++ array executable links
only libc++ and libSystem; this bounded link check does not prove the full standalone
coroutine/MLX build and transitive-dependency requirement. 41 provenance ranges
resolve to six exact pinned source files; 16 KDoc blocks retain their source text
with comment whitespace normalized. All four newly consumed Native source blocks
match the installed source archive exactly. No proof guards were added to production.

Both final complete-root ast_distance --deep scans exit 0: compiler inventory 643,
library 354, target src inventory 507. Raw function inventories are ArrayIntrinsics
1/9, native-wasm Arrays 2/10, generated ArraysNative 8/240 and ArrayUtil 4/4
body-bearing functions. Missing genuine operations remain; ArrayUtil still falsely
matches primitive fill/copy overloads by name. Arrays' two copyOfNulls rows both
point to the size overload at line 31 despite the distinct range body at line 14;
receiver/overload correspondence remains unresolved. Generated object copy helpers
stored in ArrayUtil also remain omitted from the generated file group. Normalized
logic and span-rule coverage remain provisional zero with unsupported emission
and generated parse errors. Raw documentation correspondence is 0.216730,
0.071951, 0.020136 and 0.073890 respectively. No missing item or measured criterion
is suppressed, waived or certified by the execution trace.

Receipts: `build/ir-recovery/ir-null-array-growth/implementation-receipt.json`,
provenance-receipt.json, toolchains.json, strict/build logs, comparison-final traces,
focused-tests-final.log, negative-type-receipt.json and both deep-final logs.
The existing bounded array evidence card remains done; the broad compiler cards
remain incomplete and reserved against dispatch. Return to IrElementBase's actual
dense attribute/key/map implementation, IrVariableImpl and real symbol binding,
then VariableScope/SuspensionPointScope and Native buildStateMachine. Neither full
standalone C++/MLX nor actual shared Native/C++ coroutine-state/MLX execution is
established. The runtime target is Native machine code with real MLX/GPU work;
a separate freestanding/no-OS target is not a requirement.

## Bounded concrete variable dependency: 2026-10-07

Recorded before translation. NativeSuspendFunctionLowering.buildStateMachine
(:112-175) creates suspendResult as an actual IrVariable and rewrites get/set nodes
using their exact symbol owners (:136-162). IrToBitcode.evaluateGetValue/
evaluateSetValue (:1258-1278) sends that owner to scope lookup; VariableScope
(:549-568) declares and looks up real variables. SuspensionPointScope is at
:2308-2317 in the pinned source, not :895-939; the latter range defines continuation
blocks. Its exact IrVariable supplies the LLVM resume address when read.

The bounded source work is IrDeclarationBase's nullable parent and required-parent
error, IrVariable's typed properties and actual visit/initializer transform methods,
IrVariableImpl's property storage/defaults and constructor symbol.bind(this), and
the genuine IrVariableSymbol/IrVariableSymbolImpl inheritance. Preserve annotations
through the real emptyList dependency, actual descriptor selection and unbound/
rebinding errors through IrSymbolBase, and the attribute/visitor/type ancestors.
Do not construct reduced variables, fabricated types or replacement symbol state.

The symbol's descriptor narrows through both the value and bindable interfaces.
Use the same single root virtual property dispatch with source-named typed getters
as the owner/symbol C++ boundary, preserving the original generic descriptor bound
and source descriptor-selection algorithm. This resolves sibling virtual-return
overrides without a second descriptor or an unchecked conversion. Exact source
metadata and language-boundary execution remain required evidence.

Stopping condition: genuine concrete variable/symbol instances exercise constructor
binding, one-time binding failures, exact owner/declaration identity, nullable
initializer traversal/mutation and parent errors, then feed actual variable and
suspension scopes and the production state-machine builder. Source bodies and
header checks alone do not meet this condition. Attribute/map, empty-list, rendering,
IR-based descriptor and type closure remain actual prerequisites. Return to these
named consumers after the required dependencies; no unrelated collection expansion.

## Concrete variable source checkpoint: 2026-10-07

Five genuine source pairs are written: IrDeclarationBase.hpp:20/.cpp:14,
IrVariable.hpp:21/.cpp:13, IrVariableImpl.hpp:19/.cpp:17,
IrVariableSymbol.hpp:23/.cpp:13 and IrVariableSymbolImpl.hpp:15/.cpp:12.
The declaration base retains its initial null parent and exact render-based
required-parent error. The variable retains descriptor/symbol narrowing, all
mutable flags, nullable initializer, visitVariable dispatch, initializer-only
traversal and replacement with the transformed initializer. Its implementation
retains every source property, attributeOwnerId=this, the actual emptyList
requirement, null initializer, source factory error and symbol.bind(this) after
property initialization. The factory error is genuine source behavior. The leaf
symbol inherits the real bindable/value contracts and IrSymbolBase's existing
descriptor selection, owner field, unbound and one-time binding algorithms.

The descriptor property now uses one abstract root dispatch with checked narrowing
through actual source types, resolving sibling value/bindable interfaces. No new
descriptor, binding state, substitute type or fallback was created. Source generic
bounds remain enforced. The variable disallows copying/moving the source reference
object: its symbol must keep the original owner. Name already has an immutable C++
value representation; storing it by value retains temporary factory results.
Type/origin/annotation/symbol references borrow compiler-owned objects; that ownership
closure remains required. These choices do not establish Native object/frame ABI.

Four existing typed boundary units and fifteen actual-type checks compile with
strict Clang, including typed descriptor calls. Genuine Name used as owner or
descriptor is rejected by the source constraints. The five new body probes remain
unsuccessful: declaration/variable/variable-symbol bodies stop at IrElementBase.hpp;
the concrete symbol body stops at IrBasedDescriptors.hpp. Actual IrExpression,
attributes/maps, empty-list singleton/covariance, rendering, signatures and types
remain prerequisites. No fake ancestor or collection was supplied. New bodies are
not in production targets, and real variable/binding/traversal/scope execution is
unverified. kxs-inject, KotlinxCoroutinePass and kxs_codegen_test rebuilt; two affected
injection regressions complete with zero failures in 8.45 seconds. These tests do
not instantiate the new variables.

157 provenance ranges across fifteen touched C++ files resolve to seven exact
pinned source files. Three new consumed KDoc blocks are verbatim; existing root/
value-symbol KDoc is retained. No prohibited source comments occur in touched
files. Evidence: build/ir-recovery/ir-concrete-variable/. No Kotlin source was
materialized, changed or advanced to another revision.

Both mandatory full-root --deep commands exit 0: compiler inventory 643 Kotlin
files/library 354 against complete src. Raw type coverage is 1/1 each for declaration
base, variable and implementation, 2/23 generated symbol interfaces (21 missing),
and 1/17 implementations (16 missing). Counts do not prove binding. Property and
constructor-only source inventory forces raw function zeros; normalized logic stays
provisional zero with unsupported class/property/file-annotation emission.
IrVariable has 0/3 matched functions: inherited public IrElement wrappers and the
three source-bodied virtual *_dispatch methods represent the generic source
operations, but the current callable matcher does not relate them. Exact provenance
and raw missing/extra names remain visible. Do not add duplicate scoring aliases;
actual-node execution and supported measurement remain required acceptance work.
Raw documentation correspondence: 0.719032 declaration base, 0.411196 variable,
0.360041 variable implementation, 0.087612 generated symbols, 0.491255 generated
implementations. Consumed KDoc retention does not waive whole-file criteria. Source
construction metadata, unconsumed symbol families and transformer gaps remain open.

Next close IrElementBase/IrAttribute through the already recorded genuine array/map
and collection ancestors, source empty-list, descriptor/render/type dependencies;
return to these constructors, then VariableScope/SuspensionPointScope and Native
buildStateMachine. Both standalone C++/MLX and actual Native/C++ state-machine/MLX
demonstrations remain open. No broad or bounded binding card is Done.

## Value declaration/symbol boundary checkpoint: 2026-10-07

IrValueDeclaration.hpp:24 now retains the source ValueDescriptor, IrValueSymbol and
mutable IrType contracts. IrValueSymbol.hpp:27 retains its source ValueDescriptor
and exact IrValueDeclaration owner. All ten consumed root-symbol KDoc blocks and
the two value-declaration/value-symbol blocks are copied verbatim. The source
interfaces remain abstract; no concrete owner, descriptor or IR type was fabricated.

The initial strict Clang probe confirms C++ cannot define the two mutually narrowed
virtual return types directly: IrValueSymbol is incomplete at the declaration's
symbol override. The bounded C++ adaptation keeps typed source-named getters,
with a single abstract virtual root dispatch for owner and symbol. The typed
conversion is defined after both actual interfaces are complete. There is no
object copy, alternate binding field, name/ID lookup or runtime fallback. Root
owner/symbol and typed value getters use the same actual compiler-owned objects.
Source generic Owner/Descriptor bounds remain enforced; an actual Name owner is
rejected by strict compilation. The casts implement this C++ type boundary; they
are not temporary proof assertions or alternate source behavior. Their execution
against actual concrete objects remains required.

IrSymbolBase's source owner field, unbound error and one-time bind/rebinding body
remain unchanged. Its virtual owner lookup now supplies the root boundary, while
the public typed getter narrows that result. Dependent-base owner calls explicitly
use this->owner(). Its source descriptor/render/signature dependencies remain real;
strict direct compilation still exits 1 at IrBasedDescriptors.hpp. No placeholder
was added to make the implementation executable. Source sealed-family and opt-in
metadata, generic variance beyond the consumed bounds, concrete descriptors/types,
IrVariable/IrVariableImpl, attributes/parents and scope resolution remain open.

Four actual boundary units compile with -std=c++20 -Wall -Wextra -Werror. A genuine
consumer compiles thirteen type/inheritance/abstract/generic checks and typed owner/
symbol calls; it creates no owner instance. The units are compiled and linked into
kxs-inject, KotlinxCoroutinePass and kxs_codegen_test. Two affected injection
regressions complete with zero failures in 8.49 seconds. They verify the retained
LLVM/plugin path, not new real-node identity, one-time binding or Native/MLX behavior.
58 provenance ranges resolve to five exact pinned source files; twelve consumed
KDoc blocks are retained. No source materialization or revision change occurred.
The touched C++ files have no prohibited source comments. Exact source/compiler/
contract/test receipts are in build/ir-recovery/ir-value-identity/.

Both final mandatory project-wide --deep commands exit 0, compiler inventory 643
Kotlin files/library 354, each against complete src. IrValueDeclaration has 1/1
source types, and generated symbols/IrSymbol has 1/23 (the consumed IrValueSymbol).
The other 22 generated symbol interfaces remain genuinely untranslated. The
source abstract properties are omitted from function-body inventory while C++
defines typed boundary getters, forcing raw function scores to zero; normalized
logic stays provisional zero with unsupported interface/class/property emission.
The root symbol row still lacks UnsafeDuringIrConstructionAPI. Documentation
correspondence remains raw 0.366196 (value declaration), 0.116264 (generated symbol
file), and 0.151253 (root symbol). Exact consumed KDoc retention does not establish
whole-file documentation parity or waive measured criteria. Missing/zero/provisional
findings and the existing transformer limitations remain visible.

Next follow the same named consumers to actual IrDeclarationBase/IrElementBase,
IrVariable/IrVariableImpl and the source variable/value-parameter symbols. Their
real attributes, empty annotations, type, descriptor, rendering and constructor
binding dependencies must be translated before instance execution and source
scope integration. The first-priority plan and original stopping condition remain
in force. Standalone C++ authoring/MLX GPU execution and both-direction actual
Kotlin/Native state-machine/MLX handoff remain unverified; no broad card is Done.

## Transformer source checkpoint: 2026-10-06

The standalone C++ requirement remains governing: normal C++ functions, types,
RAII resources and MLX C++ must work without an installed Kotlin compiler, linked
Kotlin/Native runtime or JVM. Native compatibility is an explicitly linked boundary.
This checkpoint adds compiler source translation; it enables no new application
coroutine behavior and demonstrates no MLX/GPU execution.

The named consumer is NativeSuspendFunctionLowering.buildStateMachine:112-175,
including ExpressionSlicer:183-190 and the return/get/set rewriting:136-162.
IrTransformer.hpp:111 now contains all 89 actual generic visit bodies, with
node-specific reference results and all 12 child-transform/self-return bodies.
IrElementTransformerVoid.hpp:21 and .cpp:11 contain its source postfix/child helpers,
89 no-context visits, 89 final forwarding visits and the free child extension.
The no-context package visit transforms children and retains the package result;
file/external-package visits use that virtual route followed by source checked
casts. These differ from the generic transformer and are retained. Non-template
bodies stay in the .cpp; the genuine source generic bodies stay in headers.
Deprecated.hpp:19 contains the actual consumed methodless marker interface,
not an invented alias. The two unused deprecated visitor aliases and six unrelated
IrVisitors.kt extensions remain untranslated and visible in the oracle.

These are source-bodied drafts, not Wired or Plugin-backed implementation.
Strict Clang compilation of the marker header exits 0. Both transformer header
probes and the no-context body probe exit 1 at the first missing genuine node
header, expressions/IrAnnotation.hpp. The transformer includes name 88 node
headers whose real source definitions remain untranslated. No fake IR hierarchy,
unchecked node cast, callback substitute or invented traversal was supplied.
Neither draft is added to production targets before genuine type closure exists.
Concrete node traversal, symbol/declaration identity, generic star-projection views,
contravariant context conversions and integration back into buildStateMachine remain
required. The original dependency stopping condition is not satisfied.

The C++ adaptation uses references to preserve node identity, std::nullptr_t for
Kotlin Nothing? (only null is admitted), and checked reference dynamic_cast for
Kotlin as. Kotlin/C++ class-cast exception spelling is not verified. C++ uses
unnamed ignored context parameters in final forwarders; this adds no runtime
check. The oracle's regex lint counts the type token nullptr_t as an unused
parameter 89 times (porting_utils.hpp:535-566). lint-initial.log reproduces it;
the raw warnings remain visible. No production code was changed to suppress them.

457 provenance ranges resolve to three exact pinned sources, all three KDoc blocks
are retained, and the four new files contain no prohibited source comments.
The three newly materialized source files match the pinned Git objects without
changing the revision or overwriting existing files. Receipts, exact commands,
source hashes, compiler diagnostics and missing-header inventory are in
build/ir-recovery/ir-transformers/.

Both mandatory project-wide --deep commands exit 0: compiler inventory 643 Kotlin
files and library 354, each against the complete src root. IrTransformer measures
89/89 functions, 1/1 type and body similarity 0.86. IrElementTransformerVoid measures
181/181 functions, 1/1 type and similarity 0.68. Both normalized-logic scores remain
provisional zero: generic/abstract/inherited class emission is unsupported;
IrTransformer also reports source grammar/annotated-lambda errors, and the child
extension is unsupported. Documentation correspondence remains raw 0.489140 and
0.407971. Exact KDoc retention does not waive those criteria. Deprecated retains
1/3 source types; IrVisitors remains missing with six functions. Broader missing,
zero, provisional, lint and priority findings were not suppressed.

Next return to the first-priority genuine declaration/value/symbol and suspension
scope closure needed by these consumers. Translate concrete source node contracts,
attributes and owner binding before exercising return/get/set/frame rewrites.
Do not promote the transformer dependency or any broad docking-ring card to Done.
Standalone C++/MLX execution and both-direction actual Native state-machine handoff
remain required and unverified.

## Native integer-array dependency and boundary: 2026-10-06

Target remains Kotlin/Native on bare metal. The preceding turn executed object-array
utilities; this turn translates the IntArray operations and companion capacity
functions consumed by the actual Native HashMap source. Compiler identity and
suspension scopes remain first priority, not a new JVM backend task.

IntArray.hpp/.cpp now has the source zero-initialized size constructor, sequential
initializer constructor, get/set/size, and genuine private IntArrayIterator. Its
fixed-length slots have explicit C++ ownership and shared array identity. This is
compiler-owned storage, not Native ArrayHeader allocation or GC object layout.
PrimitiveIterators.hpp/.cpp translates all eight actual source specialized iterator
classes, their abstract specialized-next contracts and source next forwarding.
The specialized Int next is unboxed; only the existing generic/covariant C++ virtual
boundary boxes. All class and method KDoc is retained. No substitute collection,
map, IR node or convenience alias was introduced.

ArraysNative.hpp/.cpp contains the seven consumed IntArray algorithms: copy_into,
copy_of, resized copy_of, copy_of_range, two copy_of_uninitialized_elements overloads
and fill, with actual Kotlin default arguments mapped to overloads. Primitive copy
uses the runtime's memmove operation; resize zero padding and source bounds/error
order remain intact. Kotlin Int arithmetic wraps through unsigned C++ operations.
AbstractListFunctions.hpp/.cpp maps the five actual companion functions into the
permitted namespace: element/position/range/bounds validation and new_capacity.
The complete AbstractList class, collection equality/hash functions and ancestor
algorithms remain untranslated; the companion functions do not replace that class.

NativeArrayUtil.hpp separately declares the five actual strong Native IntArray
get/set/length/fill/copy symbols. The Native fixture hands real ObjHeader arrays to
C++ and back through those symbols without converting storage. Primitive gets
return an Int, with no object result box or final object-result root requirement.
Object-array getter roots remain unchanged.

Evidence at build/ir-recovery/native-int-array/:

- 5,367 C++/Native observations agree, including 686 exhaustive bounded same/different
  array copies, fill/range/resize, errors and their precedence, initializer order,
  typed/widened iterator identity and exhaustion, independent copies, defaults and
  wrapping subtraction. Capacity growth covers 4,225 small and 144 extreme pairs
  under the source non-negative precondition, without allocating huge arrays.
- Separate real Native/C++ execution verifies Int get/set/length, copy, both overlap
  directions and fill. Kotlin reads the mutation in the original Native objects.
- Installed Native 2.4.10 bundled sources match 13 consumed function bodies, the
  actual Native IntArrayIterator and all eight shared primitive iterator classes.
  No JVM program is used. The Native compiler's host Java process does not change
  the target. The pinned full runtime and bare-metal executable remain required.
- Strict Apple Clang emits no diagnostics. Address/undefined sanitizers execute all
  5,367 observations without diagnostics. Leak detection is unavailable on macOS;
  an initial unsupported detect_leaks setting was corrected before the final run.
- 163 source ranges validate against eight exact pinned files; 30 consumed KDoc
  blocks retain source text. Five focused CMake tests have zero failures, including
  the earlier object-array comparisons. The three compiler consumers also build.
  No new whole-library build or target acceptance is claimed.

Both mandatory full-root --deep runs exit 0: compiler inventory 640 Kotlin files,
library 354, each against the full src root. One exact PrimitiveIterators.kt file
was materialized; exposure is not port completion. Raw results remain visible:

- PrimitiveIterators: 8/8 body-bearing functions and 8/8 types; source next forwarders
  have name parity 1.00. Target generic transport adds eight next_dispatch bodies.
  Class/file-annotation emission remains unsupported, normalized logic provisional 0.
- Arrays.kt paired to IntArray: 3/24 body-bearing functions and 2/16 types, reported
  function similarity 0.04, normalized logic provisional 0. The other seven primitive
  array classes/iterators remain genuinely missing. Native generated constructor
  syntax also exceeds the current emitter; existing errors are retained.
- Generated _ArraysNative.kt: 7/240 functions, reported similarity 0.01, provisional
  logic 0. These seven IntArray functions are a consumed subset, not that file's port.
- AbstractList remains reported missing because companion functions in a separately
  named, namespaced file do not establish its actual source class. No fake class or
  renamed duplicate was introduced to conceal the gap. ArrayUtil still has incorrect
  overload-by-name pairing: seven other primitive array types and fourteen primitive
  fill/copy overloads remain unported. Int overloads now have actual bodies in
  IntArray.cpp, although that file is not concatenated into ArrayUtil's raw pair.

The execution evidence does not satisfy or replace outstanding measured criteria.
Next: port actual collection ancestor algorithms and their equality/hash/string
source dependencies; then complete Native HashMap, its iterators/views, builder and
empty-map behavior. Use that real backing map for IR attribute snapshots and copy
policy, then concrete parameter descriptors, symbol owner binding, value-declaration
identity and suspension scopes. Matching pinned Native runtime, actual shared Kotlin
coroutine frame handoffs and bare-metal validation stay open. No fallback, source
proof guard, stub or placeholder is accepted as completion.

## Native array storage and direct array boundary: 2026-10-06

The preceding goal turn made progress on source collection contracts. This turn
writes and executes the actual object-array storage dependencies of Native HashMap.
The full 917-line Native HashMap source and its map builder, collection ancestors,
array utilities and capacity algorithm were read. Seven further exact pinned files
were exposed without replacing existing source; exposure is not translation.

`kotlin/collections/ArrayUtil.hpp/.cpp` now contains source uninitialized allocation,
reset-at/reset-range, fill, overlap-safe slot copy, copy_into defaults, range copying,
resize copying and range validation. Public generic bodies remain in the header;
non-generic range validation is in .cpp. Array's actual internal size constructor
is accessible only through the compiler storage boundary. Empty slots do not
construct fake default values, copies do not read them, and resetting destroys a
stored owning handle. Reading an empty compiler slot now throws bad_optional_access
instead of dereferencing a disengaged optional; the source explicitly permits
implementation-dependent uninitialized reads. Bounds/error branches come from
source, not temporary proof guards.

Compiler-owned Array<T> remains C++ storage, not Native ArrayHeader/GC storage.
`NativeArrayUtil.hpp/.cpp` separately preserves the actual strong runtime get/set/
length/fill/copy declarations and the two source reset bodies over real ObjHeader
arrays. There is no runtime selection, reinterpretation as C++ storage, weak symbol,
GC substitute or coroutine fallback. Native object gets retain the final result slot.
The actual runtime supplies the source heap write-barrier operations.

Execution evidence:

- 862 C++/Kotlin-Native observations agree: all 686 bounded same-array/different-array
  copy cases, fill/reset ranges, slices, resizing, negative bounds, source error
  precedence/messages, defaults, signed subtraction overflow and copied references.
  Null and source-defined implementation-dependent uninitialized reads are grouped;
  runtime bounds errors without specified messages compare by exception category.
  C++ additionally executes a real translated Name without a default constructor.
  The lifetime cases use shared Name in C++ and GC-managed Array<Int> in Native;
  this is not same-object layout evidence.
- A linked Native/C++ fixture executes real Native array copies in both overlap
  directions, source resets and fill. Kotlin observes exact object identity. A
  value returned through the real Native result slot survives after its owning
  array is cleared in a separate test frame and GC runs. C++ reset removes the
  final array reference, confirmed with Native WeakReference after GC.
- Installed Native 2.4.10's bundled sources match all consumed ArrayUtil algorithm
  bodies and generated object-array copy bodies. This is a bounded host toolchain,
  not the pinned full runtime or bare-metal target.
- Three CMake targets build; three focused tests execute with zero failures. Existing
  compiler consumers also build. Strict Apple Clang and address/undefined sanitizers
  report no diagnostics for the C++ array execution. All 49 source ranges match four
  exact pinned files; all six consumed KDoc blocks are retained. Assertions are in tests.

Both full-root ast_distance --deep runs were refreshed after final source changes:
compiler 639 source files, library 354, exit 0. ArrayUtil records 4/4 body-bearing
functions, function similarity 0.19 and provisional normalized logic 0.00. The raw
inventory incorrectly pairs sixteen fill/copy overloads for eight primitive-array types to the object
helper by name; those primitive APIs and primitive array classes are NOT translated.
The generated copy routines' file pairing and private C++ storage helper diagnostics
also remain visible. No scoring rule was changed. Source/interface/algorithm criteria
remain mandatory; this checkpoint completes bounded execution evidence only.

Remaining source order:

1. Translate real IntArray/copy operations and AbstractList capacity behavior required
   by HashMap, then AbstractCollection/AbstractMutableCollection/AbstractSet/
   AbstractMutableSet view algorithms and their actual element equality/hash/string
   dependencies. Do not fabricate an Any value or a test collection to bypass them.
2. Translate Native HashMap's complete four-array storage, reverse probing, collision
   growth, compaction, rehash, remove-hole repair, modification counts, read-only build,
   EntryRef/iterators and all three live views. Preserve source type barriers for
   read-only Map.Entry membership in mutable entry views. Translate build/empty-map
   functions and verify actual backing/view lifetimes before accepting snapshots.
3. Translate IrAttribute/IrElementBase dense pairs, identity search, complemented free
   index, two-pair growth, swap-last removal, snapshots and copy flags. Then actual
   transformer/concrete declarations/one-time symbol binding and suspension scopes.

Declaration identity and suspension scopes remain first priority. Shared Native
coroutine states, spilled values, resumed results/exceptions/cleanup and bidirectional
bare-metal handoffs remain unfinished. Current receipt directory:
`build/ir-recovery/native-map-dependencies/`. No broad compiler card is complete.

## Native collection prerequisite for IR attributes: 2026-10-06

The previous goal turn made progress by translating the actual IR declaration
contracts and all visitor routes. This checkpoint closes more source contracts
required by IrElementBase's attribute snapshots and concrete parameter collections.
The target remains Kotlin/Native on bare metal; these compiler interface checks
provide no target/runtime/frame acceptance.

Native-wasm actual sources now supply Map, MutableMap, Entry, MutableEntry, Set,
MutableSet, MutableCollection, MutableIterable, MutableIterator and MutableListIterator.
The four new headers retain source KDoc, mutation/view contracts and generic types.
The existing C++ virtual-template boundary is extended for the actual abstract
operations. Kotlin's static nested Entry types use C++ namespaces to avoid capturing
an enclosing Map template's identity. Entry keys/values and Map values follow their
source covariance edges; mutable maps and sets retain invariance. Nullable results
preserve an existing pointer, shared handle, optional or Any null representation
instead of adding another nullable layer. No backing map, set or entry instance was
invented, and no source algorithm was replaced with a standard-container alias.

Strict Apple Clang consumer compilation exits 0. Thirty-seven static checks and
instantiated consumer functions cover primitive/nullable types, actual parameter
descriptors, projected put_all keys, covariant map/entry values, mutable/read-only
views and mutation signatures. This is type/surface evidence only. Real collections,
key/value equality and hashing, iterator state, mutation and view/backing lifetime
execution remain unfinished. Dynamic conversion with real entries also remains
unverified. All 140 markers resolve to five exact pinned source files.

Four genuine pinned dependencies were exposed without overwriting existing source:
Native Map.kt, Set.kt, Maps.kt and the 917-line HashMap.kt. Both required full-root
--deep runs exit 0; compiler inventory is 632 and library 354. Source exposure is
not implementation completion. The oracle recognizes Map's 4/4 and Set's 2/2 source
types, while interface/class emission and bodyless source method exclusion leave
raw function/logic scores at zero. Its Collection pairing selects the new mutable
header and omits the existing read-only Collection in the multi-source Collections.hpp;
retain that missing-type finding and repair pairing/layout against the real source.
Existing visitor namespace diagnostics and all real missing algorithms remain visible.

IrElementBase, IrAttribute, Native HashMap/build-map/empty-map algorithms and their
actual dependencies remain untranslated. No IR attribute mutation/copy execution is
claimed. Continue within priority 1: translate those complete source algorithms,
including mutable views and iterator ancestors; preserve identity, dense pair growth,
swap-last removal, copyByDefault/includeAll merging and snapshot behavior. Then close
IrTransformer/node hierarchy, concrete declarations/one-time binding, VariableManager
and suspension scopes. Shared Native coroutine states/spills/cleanup, matching pinned
runtime and bare-metal bidirectional handoffs remain required.

Files: kotlin/collections/{MutableIterator,MutableCollection,Set,Map}.hpp under
src/kotlinx/coroutines/tools/kotlinc_native_ref/. Per-class/method references and
strict checks are in build/ir-recovery/ir-attribute-dependencies/implementation-receipt.json.
The broad compiler cards remain unfinished.

## IR declaration and visitor checkpoint: 2026-10-06

The target is Kotlin/Native on bare metal. The compiler runs on the development
host; host compiler dependencies and host Native tests do not establish target
runtime or coroutine-frame acceptance.

Eight actual source contracts now exist: IrElement, IrStatement, IrSymbolOwner,
IrAnnotationContainer, IrMutableAnnotationContainer, IrDeclarationParent,
IrDeclaration and IrDeclarationWithName. Their public getters/setters retain
real descriptor, symbol, owner, Name and annotation-collection types. Source
abstract operations remain abstract. No concrete replacement node was made.
The full IrVisitor source surface is written: one abstract visit_element and
all 88 source default delegation bodies, in declaration order. For example,
visit_get_value delegates through value access, declaration reference and
expression to visit_element. C++ virtual-template transport preserves the
actual typed node/data/result; its non-template bodies are linked into the
injector, module plugin and codegen test.

Strict compilation checks all 89 actual visitor signatures, abstract contracts,
base relations and typed symbol upper bounds. A mechanical comparison verifies
all 88 delegation routes and argument forwarding. All 333 provenance markers
in the eleven new files resolve to valid pinned source ranges. Three compiler
targets rebuilt, and seven focused tests executed with zero failures in 8.38
seconds. This establishes source surface and boundary compilation, not concrete
IR traversal or full generic variance. Real node hierarchies, source attributes,
transformer behavior, owner binding and suspension scopes remain unfinished.

Six Kotlin dependencies were materialized at the same pinned revision without
overwriting existing source. Both mandatory full-root --deep scans exited 0;
compiler inventory is 628 files and library inventory 354. IrElement's raw zero
reflects excluded bodyless source methods versus four C++ generic transport
bodies. Interface/class emission remains unsupported. The scanner rejects
IrVisitor and IrAnnotationContainer identity because their genuine imported-type
forward declarations are read as file owners; dispatch companion namespace and
mixed-source provenance also conflict. Preserve the missing/zero/provisional
findings and priority documents. The route comparison does not replace them.

The actual IrSymbolImpl compile probe now first stops at
ir/descriptors/IrBasedDescriptors.hpp, rather than IrDeclaration.hpp.
IdSignature and rendering remain genuine unported direct dependencies. The
pinned Native distribution configuration probe exits 1 before compilation:
repo/kotlin-build-helpers is absent from the sparse checkout. Source documents
Xcode 27 for this host build, while installed Xcode is 26.6; the probe did not
reach an Xcode rejection. Installed Native 2.4.10 still lacks pinned RegisterGlobal.
No compatibility alias or fabricated runtime definition resolves this gap.

Next implementation order within first-priority declaration identity/scopes:

1. Translate IrElementBase's full attribute storage/search/removal/copy algorithms
   and real IrAttribute/collection dependencies; preserve identity and copy flags.
2. Translate IrTransformer and actual node hierarchy, including genuine generic
   star views/conversions, covariant returns and child mutation. Instantiate and
   execute the real visitor/transformer against source concrete nodes.
3. Translate IrDeclarationBase parent behavior, IrValueDeclaration, IrVariable
   and IrVariableImpl, actual typed symbols, origins and factory. Close descriptor,
   type, rendering/signature, collection and Native Lazy dependencies; execute
   real one-time binding and two same-name declarations with distinct owners.
4. Build/link the matching pinned Native runtime after completing its exact build
   dependency closure. Verify real heap/global roots, Native reference atomics,
   CurrentThread, reentrant Lock and Lazy without substitutes.
5. Translate VariableManager and suspension scopes against those actual objects,
   then spills/resume states/cleanup and bidirectional Kotlin/C++ shared frames.
   Execute on the actual bare-metal target before target acceptance.

Receipts, source hashes, diagnostics, route comparison, build/tests and both
full-root oracle commands are in build/ir-recovery/ir-declaration-contracts/;
the pinned Native configuration probe is in build/ir-recovery/pinned-native-build/.
The three broad compiler cards remain unfinished.

## Native reference/root boundary checkpoint: 2026-10-06

The target remains Kotlin/Native on bare metal. `KotlinGCBridge.hpp:24` now exposes
strong source runtime declarations with actual object result slots; `.hpp:65`
preserves FrameOverlay and `.hpp:74` declares the genuine ObjHolder. Its actual
constructor/root/cleanup bodies are in `KotlinGCBridge.cpp:10-31`. Forty-one source
ranges match the pinned Native C++ runtime. The previous optional no-op/weak boundary,
invented state wrapper, empty test target and standalone test fallback are removed.

A real Native executable prints `native-roots=10` and `native-return-slot=1`, exit 0:
stack roots retain objects during GC, clearing releases them, nested frames and C++
unwinding restore the real chain, compare/exchange operations preserve rooted results,
and Kotlin retains a returned object after the C++ holder exits. Atomic locations in
this fixture are shadow-stack root slots; heap/global registration, concurrent
AtomicReference and coroutine frames are not established. The macOS ARM64 host is
verification only, not bare-metal execution. Detailed scope and source behavior are in
[the Native runtime specification](../runtime-and-gc/KOTLIN_NATIVE_GC_SPECIFICATION.md).

The installed Native 2.4.10 runtime differs from the pinned compiler revision:
RegisterGlobal is absent and InitAndRegisterGlobal is present. A genuine link attempt
failed on RegisterGlobal. Keep the pinned declaration and require a matching runtime;
no alias or fabricated definition is acceptable. Receipts and exact source ranges are
in `build/ir-recovery/native-references/`. Both full-root --deep reports are refreshed;
raw zero/provisional/missing findings remain authoritative. A C++-origin Native runtime
source has no Kotlin AST pair and needs separate source/ABI evidence.

Next: establish the pinned runtime build/link and actual heap/global fields; translate
Native AtomicReference, CurrentThread's real Any identity, reentrant Lock and all Lazy
algorithms/factories; close concrete parameter dependencies; then real IR owners,
one-time symbol binding and suspension scopes. Declaration identity/scopes remain the
first priority. Shared Native coroutine frames and bare-metal execution remain open.

## Native lazy integer prerequisite: 2026-10-06

The previous goal turn made progress by writing the concrete parameter source bodies,
recording their real missing dependencies and reconciling Native target requirements.
This turn implements the Native AtomicInt dependency needed by the lazy lock. The
runtime target remains Kotlin/Native on bare metal; shared host compiler dependencies
must not be mistaken for Native runtime/object ABI completion.

`kotlin/concurrent/atomics/Atomics.hpp:32` and `.cpp:23` now contain the full consumed
Native AtomicInt class: construction, property access, load/store, exchange, strong
compare-and-set, compare-and-exchange, arithmetic, all genuine source deprecated
members, and text conversion. Six common integer extensions and the three actual
Native inline update functions retain their source KDoc and per-function provenance.
The source deprecated members are real upstream APIs, not added convenience aliases.
Public inline callback parameters use C++ callable templates, retaining captured and
move-only closures without copying an erased callable. Kotlin nonlocal-return/compiler
contract metadata and deprecation error levels are not established by that C++ mapping.

The source comparison compiles the complete pinned Native class and exact common/Native
integer extension excerpts without editing their contents, using installed Kotlin/Native
2.4.10 and its actual stdlib. All 42 observations match C++ byte for byte on macOS ARM64:
failed/successful compare operations, wraparound at both Int limits, minus-assignment of
Int.MIN_VALUE, exact old/new return values, a forced two-attempt retry, a throwing update,
deprecated operations, property/text values, captured updates, four workers accumulating
40,000 increments and exactly one compare-and-set winner. C++ additionally exercises
move-only capture and 2,000 sequential-consistency store/load litmus iterations. Strict
Clang compilation and address/undefined-behavior sanitizer execution exit 0. Native
LLVM debug metadata points to the unchanged copied class, and six entry/primitive paths
show the same seq_cst load/store/exchange/add/strong-cmpxchg instructions as C++.

The new `kxs_atomic_int_contract` is registered in CMake. The actual AtomicInt body
compiles into the three existing compiler targets. Four targets build with exit 0;
five focused tests record zero failures in 9.66 seconds. The first build regenerated
CMake before its new target was visible to that make invocation; the next build used
the generated target. Native fixture import ambiguity was corrected by importing
Worker/TransferMode explicitly. Initial diagnostics remain in the receipt directory.
Only test-fixture/configuration changes were needed; no source algorithm was altered
to make the observations agree.

Receipts and exact manifests are in `build/ir-recovery/native-lazy/`: 58 validated source
ranges, Native/C++ traces, compilation/execution logs, LLVM primitive evidence, CMake
and CTest receipts. Two newly exposed Kotlin files expand the compiler inventory from
620 to 622; Atomics.native.kt already existed and was preserved. No upstream content
or sparse-checkout configuration was replaced. Both mandatory full-root --deep scans
exit 0, with compiler 622 and library 354 units. Source exposure is not completion.

The raw Native atomic report now pairs the genuine C++ source unit: 18/62 functions
and 1/5 types are matched across the entire Native file. AtomicLong, AtomicBoolean,
AtomicReference, AtomicNativePtr and their dependent algorithms remain untranslated.
Normalized logic/emission score remains provisional zero, generated emission has
parse errors, target parsing has no errors, and class/property/callable emission is
unsupported. Documentation correspondence is measured separately, not replaced by
copied-KDoc or trace claims. Common extension provenance exists but the separate common
source unit remains unpaired by the single-file pairing model. Retain these raw gaps
and tool limitations; executable primitive evidence does not waive required criteria.

This compiler-owned C++ class has C++ atomic field storage and lacks Native ObjHeader
layout, GC roots and runtime reference operations. Its constructor initializes an
unpublished C++ atomic field, while the Native generated volatile-field initializer
uses an atomic store. No Native layout/initialization ABI equivalence is claimed.
Native reference compare/exchange uses CompareAndSetVolatileHeapRef,
CompareAndSwapVolatileHeapRef and GetAndSetVolatileHeapRef with lifetime/result-root
handling. Do not replace those operations with pointer atomics or shared_ptr atomics
and call the docking ring complete.

Next source work stays on the concrete parameter dependency path: Native reference
atomics and their reference/root operations; CurrentThread's real Any identity and
reentrant Lock plus try/finally cleanup; all source lazy initialization/publication
algorithms and factories; genuine collection singletons/map; type/substitution,
visibility and DEBUG_TEXT dependencies; real parameter construction and visitor routing;
IR declaration ownership/one-time symbol binding and suspension scopes. Bare-metal
execution, spills/cleanup and direct shared Kotlin/C++ Native frames remain required.
The three broad compiler cards remain unfinished; declaration identity and suspension
scopes remain the first priority. No new coroutine fallback, stub or proof guard was
introduced, and no tool diagnostic was suppressed.

## Concrete parameter source checkpoint: 2026-10-06

The target remains Kotlin/Native on bare metal. Shared compiler descriptor classes
written in Java are Native compiler dependencies, not authority for the target's
runtime. The parameter's lazy delegate must use Native atomic-reference and
reentrant-lock behavior; a JVM monitor implementation is not an alternative.

Source-bodied header/implementation pairs now exist for
`DeclarationDescriptorImpl`, `DeclarationDescriptorNonRootImpl`,
`VariableDescriptorImpl` and `ValueParameterDescriptorImpl`. They preserve the
source owner/original relationships, source metadata, nullable stored type during
construction, default-value eligibility by callable kind, substitution rejection,
parameter-index lookup through overridden owners, visitor routing and the actual
lazy destructuring delegate. Copying the destructuring parameter retains the same
lazy delegate; it does not evaluate the initializer prematurely. Source-defined
null/false/empty results and empty cache cleanup remain source behavior, not newly
invented substitutes. There are no manufactured descriptor instances in verification.

Strict Clang header/type checks exit 0, including the genuine inheritance, narrowed
returns and all thirteen CopyBuilder operations. The NonRoot implementation body
also compiles independently. The other three body probes stop at actual missing
headers: DescriptorRenderer, type_util/TypeUtils and DescriptorVisibilities,
respectively. No substitute headers were supplied. These classes are source-bodied
drafts with incomplete dependency closure; concrete parameter execution is unverified.

MemberDescriptor, CallableMemberDescriptor and Modality contracts are now present.
The source `Kind.isReal` and Modality flag converter compile into the three existing
compiler targets. Eight observations from the unchanged full pinned Modality.kt,
compiled and executed by Kotlin/Native 2.4.10 on macOS ARM64, match C++ byte for byte:
sealed takes precedence over abstract, then open, then final. This checks a compiler
helper; it does not establish bare-metal execution, Native runtime object layout,
parameter construction or shared coroutine frames. Four existing compiler targets
build with exit 0 and four focused tests record zero failures in 8.39 seconds.

Receipts, exact pinned source ranges and manifests are in
`build/ir-recovery/concrete-parameters/`. Provenance validation covers 161 ranges
across fifteen source files. The new source exposure adds three Kotlin files;
both mandatory project-wide `--deep` runs exit 0, with compiler inventory 620
Kotlin files and library inventory 354. Inventory growth is not completion.

Raw oracle findings remain required evidence. ValueParameterDescriptorImpl is
reported as a missing file because its header forward-declares real `kotlin::Lazy`
before declaring the descriptor namespace. The scanner rejects those two namespaces
as ambiguous, then rejects the paired body. The exact matching provenance is present
in both physical files. This is a diagnosed file-identity limitation, not evidence
that the written body is complete or semantically verified. Modality's companion
function is still missing in the symbol report after its permitted lowering to a
namespace function; normalized logic is provisional zero and enum emission remains
unsupported. Java descriptor dependencies have no Java AST parser. Keep all raw
missing-symbol, zero-score and priority findings; execution evidence does not replace
the measured acceptance requirement. No oracle diagnostics were suppressed.

Next source work, in dependency order:

1. Translate the actual Native Lazy factories, atomic-reference dependencies and
   reentrant Lock; preserve initialization/publication and cleanup rules.
2. Translate actual empty-list/set singletons and collection map/ArrayList behavior,
   retaining shared singleton identity and covariance. Resolve iterator ownership
   against those source objects rather than creating per-type empty substitutes.
3. Close KotlinType update traversal and TypeSubstitutor dependencies, then the real
   visibility/accessibility and DescriptorRenderer DEBUG_TEXT implementation.
4. Compile/link the four real bodies and exercise genuine parameter instances,
   original/copy identity, default flags, override-index mapping, visitor dispatch,
   lazy construction and failure/retry semantics. General CopyBuilder variance is
   still open despite the consumed upper-bound contract compiling.
5. Continue real IR declaration owners and one-time symbol binding, then declaration
   lookup/suspension scopes, spills, cleanup and direct Kotlin/C++ Native frame handoff.

Two compiler representation deviations remain explicit and unverified: rendering
uses C++ RTTI/address text and UTF-8/WTF-8 at the existing diagnostic boundary,
rather than exact Java simple-name/identity-hash spelling; nonempty parameter
substitution uses std::logic_error for the source UnsupportedOperationException.
These are not Native exception/object ABI acceptance. Resolve them through their
actual dependencies before claiming complete parity. Broad compiler cards remain
unfinished, with declaration identity and suspension scopes first priority.

## Dependency order and current implementation

Compiler paths below are relative to `tmp/kotlin`. C++ package paths are rooted
at `src/kotlinx/coroutines/tools/kotlinc_native_ref/`.

| Source unit | Required dependencies and behavior | Implementation / next target |
|---|---|---|
| `core/names/src/org/jetbrains/kotlin/name/Name.java:22-135` | UTF-16 values, nullable results, dynamic Object type check, JVM hashes and source errors | Source methods translated in `org/jetbrains/kotlin/name/Name.hpp:30` and `Name.cpp`; focused execution and pinned Java trace comparison. No IR declaration consumer yet. |
| `core/names/src/org/jetbrains/kotlin/name/{FqName,FqNameUnsafe}.kt` | `Name`, parent/short-name caches, root/special names and paths | Untranslated. Follow signature dependencies; plain strings cannot replace these classes. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/{DeclarationDescriptor,Named}.java` | Annotations, markers, validation, original/containing descriptors, descriptor visitors and actual `Name` | `Named.hpp:25` translates the source abstract getter. `DeclarationDescriptor.hpp:34` and `.cpp` now translate the root contract with inherited annotations/validation and the typed visitor boundary. Concrete descriptor source-bodied drafts are present at the current checkpoint; their dependency closure, annotations collections and actual node routing execution remain unfinished. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:19-49` | Fifteen typed descriptor visits and generic result/context | Source interface in `descriptors/DeclarationDescriptorVisitor.hpp:40`; C++ virtual-template boundary in `DescriptorVisitorDispatch.hpp`. All fifteen methods instantiate for four result/context configurations; actual descriptor routing remains unverified. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/annotations/{Annotations.kt,AnnotatedImpl.java}` | Source annotation getter and stored annotation reference | `annotations/Annotated.hpp:25`, `AnnotatedImpl.hpp:25` and `.cpp` compile. Full `Annotations`, filtered/composite collections and annotation descriptors remain untranslated. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/ValidateableDescriptor.java:8-10` | Source default validation method | `ValidateableDescriptor.hpp:12` and `.cpp` compile. Its empty default is present in the original source at line 9; it is not a substitute for a missing algorithm. |
| `core/compiler.common/src/org/jetbrains/kotlin/descriptors/{SourceElement,SourceFile}.java` | Genuine absent-source singletons, source-file lookup, nullable UTF-16 name | `descriptors/SourceElement.hpp:28`, `SourceFile.hpp:28` and `.cpp` compile in all three compiler targets. Pinned Java and C++ traces agree on exact singleton routing, null name and diagnostic. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorWithSource.java:21-28` | Actual source element and covariant original descriptor | Source abstract contract in `descriptors/DeclarationDescriptorWithSource.hpp:26`; strict compilation. DeclarationDescriptorNonRootImpl now has a compiling source body; concrete linking/execution remains unfinished. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorNonRoot.java:21-27` | Non-null containing declaration with the root's override-compatible return | `descriptors/DeclarationDescriptorNonRoot.hpp:25`; Clang AST confirms source nonnull return metadata. No runtime proof guard added. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorWithVisibility.java:21-24` | Actual descriptor visibility | Source abstract getter in `descriptors/DeclarationDescriptorWithVisibility.hpp:27`. The genuine DescriptorVisibility base now compiles; concrete descriptor accessibility rules and DelegatedDescriptorVisibility remain untranslated. |
| `core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:10-34`, `Visibilities.kt:8-89` | Nine actual singleton types, partial ordering, identity, display/import rules and source default behavior | `descriptors/Visibility.hpp:20`, `Visibilities.hpp:14` and `.cpp` compile in all three compiler targets. Unchanged pinned Kotlin and C++ agree on 91 observations, including all 81 singleton pair comparisons. FqName and EffectiveVisibility type consumers remain untranslated. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:23-86` | Actual base abstract accessibility contract, delegate/name/public API, comparison/text/custom-effective forwarding and source package default | `descriptors/DescriptorVisibility.hpp:28` and `.cpp` compile. Concrete descriptor routing, source obsolete-API metadata, DelegatedDescriptorVisibility normalization and DescriptorVisibilities accessibility rules remain untranslated. No fabricated concrete descriptor was used for execution. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/Substitutable.kt:21-23` | Typed substitution and NonRoot bound; source out variance | `descriptors/Substitutable.hpp:29` has the source typed abstract method and bound check. Genuine NonRoot satisfies it; genuine Name is rejected. General out-variance conversions remain unverified; no actual substitution behavior or TypeSubstitutor implementation exists yet. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/{CallableDescriptor,ValueDescriptor,VariableDescriptor,ParameterDescriptor,ValueParameterDescriptor}` | Receiver/type/parameter lists, typed user data, original/containing covariance, substitution, initializer/flags and overridden descriptors | Actual interfaces now compile in `descriptors/CallableDescriptor.hpp:51`, `ValueDescriptor.hpp:24`, `VariableDescriptor.hpp:25`, `ParameterDescriptor.hpp:23`, `ValueParameterDescriptor.hpp:24`. The genuine source `is_late_init=false` default compiles in all three compiler targets. Consumed collection covariance and typed boundaries compile; four concrete descriptor source-bodied drafts, generic nullable user-data semantics and actual descriptor execution remain unfinished. |
| `core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:8-28` | Fifteen methodless interfaces and their exact inheritance | Translated in `org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.hpp:19`. These interfaces are methodless in the source; they are not substitute declaration implementations. |
| `compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrParameterKind.kt:8-13` | Dispatch/context/extension/regular kinds in source order | Translated in `org/jetbrains/kotlin/ir/declarations/IrParameterKind.hpp:12`. |
| `compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/IrElement.kt` | Offsets, attribute-owner identity, generic visitor/transformer dispatch and traversal | Actual abstract contract and typed dispatch boundary compile in `org/jetbrains/kotlin/ir/IrElement.hpp:19`; concrete ancestor/node traversal remains open. |
| `compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/{IrElementBase,IrAttribute}.kt` | Typed keys, weak debug owners, dense attribute storage, identity lookup and copying | IrElementBase core bodies compile; complete snapshot/copy bodies stop at genuine map dependencies. IrAttribute/key/flag/delegate bodies now exist but stop at genuine Any/weak/property dependencies; no concrete key/node execution. Source childless defaults are retained. See current attribute-key and earlier element-storage checkpoints. |
| Generated `{IrSymbolOwner,IrDeclaration,IrDeclarationWithName,IrValueDeclaration}` and `IrDeclarationBase` | Element/statement inheritance, declaration parent/origin/factory/annotations, name, type, descriptor and symbol | Actual abstract root contracts compile. `IrDeclarationBase.hpp:20/.cpp:14` has source parent bodies and the genuine IrElementBase ancestor; compilation now first needs rendering. Real factory/type/annotation dependencies remain required. |
| `compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt` | Annotation/type-marker contracts and source type semantics | Explicit source interfaces/default bodies compile in three compiler targets. Actual Any ancestry, sealed metadata/concrete equality and collections remain unfinished. The actual type is not `LLVMTypeRef`; see the dated type checkpoint. |
| `compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/IdSignature.kt` | Signature variants, masks/visibility, fully-qualified names and file symbols | Untranslated. Required by symbols; empty signature substitutes are prohibited. |
| Source `IrSymbol.kt:36-138` | Typed owner/descriptor contracts, signatures, bound state and public accessibility | Abstract interfaces in `org/jetbrains/kotlin/ir/symbols/IrSymbol.hpp:58`; `is_public_api` body in `.cpp`. Compiled in injector, LLVM module plugin and codegen test. Construction opt-in/obsolescence metadata remains untranslated. |
| Source `impl/IrSymbolImpl.kt:19-85` | Descriptor hierarchy, actual owner, signatures, render/IR-based descriptor conversion; unbound access and one-time binding | Source-bodied draft in `org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.hpp:37`. Not compiled or instantiated: the current probe first stops at genuine IrBasedDescriptors.hpp; rendering/signatures remain required. Actual IrVariableSymbol/IrVariableSymbolImpl source pairs now exist with the same dependency/execution gap. |
| Generated `IrVariable`, `IrValueParameter`, `IrVariableImpl` | Declarations/symbols/types, flags, parameter kinds, annotations, initializer/default body and traversal | Actual variable/implementation source bodies exist in `declarations/IrVariable.hpp:21` and `declarations/impl/IrVariableImpl.hpp:19`, including constructor binding. Genuine ancestor closure prevents compilation/execution. IrValueParameter remains untranslated. |
| Generated get/set and suspension nodes and implementations | Expression/value-access bases, actual symbols and ID variable, type/origin/offsets and ordered transformations | Ten source classes in eighteen C++ files retain complete constructor/property/traversal bodies; eight strict compiles now include actual IrType. Concrete instance/renderer/owner binding and production Native scope integration remain unfinished. |
| `llvm/VariableManager.kt` | Declaration-identity map, source records, allocation, reference rooting/load/store and debug operations | Untranslated. Existing instruction helpers lack required object-root/result-slot dependencies. |
| `llvm/IrToBitcode.kt:123-209,264-326,546-603,1258-1278,2281-2340` | Full `CodeContext`, outer delegation, `using` ordering, ordinary and suspension-variable lookup | Untranslated scope/visitor machinery. Current callback emitters remain partial LLVM algorithms. |
| `lower/NativeSuspendFunctionLowering.kt:119-170` | Actual function/field/context/builder contracts, captured parameters and targeted return/get/set rewrites | Untranslated IR-object construction. Integrate into mandatory LLVM injection as designed. |

Backend paths in the last three rows are relative to
`kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/`.

## Representation and lifetime boundary

`Name` is a compiler value represented by UTF-16 units and a special-name flag.
It is not a declaration identity. Two declarations with equal names must still
be separate stable objects. Its value/type equality cannot serve as a variable
lookup key. Factory/module ownership and the Clang declaration-to-IR mapping
remain untranslated. They must keep declaration addresses and exact LLVM
function/block identities alive throughout lowering and generation before
production integration. Current `Name` execution proves none of those paths.

## Executable dependency evidence and measurement limits

### Native runtime correction: 2026-10-06

Sydney's target is Kotlin/Native on bare metal. Runtime algorithm and object/continuation
ABI authority is the pinned `kotlin-native/runtime` source and the Native actual stdlib
sources, not the JVM runtime. Shared compiler descriptor files written in Java remain
compiler implementation sources used by the Native backend; their JVM execution
comparisons establish only those compiler algorithms, not target runtime acceptance.

The previous array checkpoint used the wrong platform source. Its 22 JVM observations
remain historical evidence, superseded for this target. The production C++
`kotlin/jvm/internal/ArrayIterator.hpp` and JVM factory were removed. `kotlin/Array.hpp:124`
now translates the complete Native private iterator from `Array.kt:88-92`: precheck
`index < size`, then `array[index++]`, otherwise `NoSuchElementException("$index")`.
Repeated exhaustion preserves the index without a JVM catch/decrement path. Source
initialization allocates the fixed-length compiler storage before calling `init` and
writes each result in order. Collection/Iterable/List/Iterator source ranges and full
KDoc now refer to `libraries/stdlib/native-wasm/src/kotlin/collections/`.

A macOS ARM64 native executable built by installed Kotlin/Native 2.4.10 runs the entire
pinned Native iterator class copied byte for byte. Nineteen observations match C++,
including exhaustion messages `3`/`0`, typed/covariant cursor identity, mutation,
independent iterators, retained array storage, nulls and initialization/bounds errors.
The negative-size check confirms initialization did not execute. This executable uses
the installed Native stdlib for arrays; it is not a build of the full pinned runtime.
Strict C++ compilation exited 0. Four compiler targets built; four focused tests
recorded zero failures in 8.53 seconds. Receipts and hashes:
`build/ir-recovery/native-runtime-correction/`.

This is iterator-algorithm evidence, not bare-metal or direct object/frame handoff
acceptance. Current C++ storage owns compiler values with C++ lifetimes. It does not
implement Native `ObjHeader`/`ArrayHeader`, `Kotlin_Array_get` return-root updates,
`Kotlin_Array_set` heap-reference updates, Native allocation or generated frame layout.
Those dependencies remain required. The actual source operations are in `Arrays.cpp`
and `Memory.h`; do not replace them with vector layout, a raw store, or a JVM runtime.

Native lazy resolution uses `kotlin/native/concurrent/Lazy.kt` atomic references and
`Lock.kt`'s thread-owned reentrant lock with `locked` try/finally cleanup. Read the complete
source classes before implementing the parameter descriptor's delegate. The Native
`lazy(lock, initializer)` overload throws because synchronization on arbitrary Any
objects is unsupported. No JVM monitor-based lazy implementation was added.

Both mandatory full-root `--deep` scans exited 0: compiler 617 Kotlin files, library
354. Four newly materialized Native collection files expand source coverage. Earlier
concrete-parameter source exposure is recorded separately; neither expansion is port
completion. Native Array records three explicit functions matched and both source
types present in symbol inventory, but its normalized logic and emission score remain
provisional zero because class/generic/constructor/property emission is unsupported.
The inventory marks the in-class iterator declaration as declaration-only even though
its qualified template definition exists at `Array.hpp:148`; function evidence pairs
the real definition. Multiple source units sharing Collections.hpp are not all paired
by the current single file-annotation lookup. Collection/List remain missing-file
findings. Mutable interfaces and algorithms are genuinely untranslated. Retain these
raw findings; nineteen runtime observations do not waive measured acceptance.

Concrete descriptors, real IR owners/binding, suspension scopes, spills, cleanup and
shared Kotlin/C++ Native state-machine handoff remain unfinished and first priority.

### Historical callable/parameter and JVM iterator checkpoint

The actual CallableDescriptor, ValueDescriptor, VariableDescriptor,
ParameterDescriptor and ValueParameterDescriptor interfaces retain their source
inheritance, properties, overloads and narrowed original/containing/substitution
contracts. Their source KDoc and per-function ranges are present. Source
`ValueParameterDescriptor.isLateInit` is false at line 59; that exact default
is implemented in `.cpp`, not a replacement parameter implementation.

C++ cannot override virtual value-return templates covariantly. The private
virtual boundary carries the same source collection/iterator object, and typed
views recover it with checked reference casts. Owned result views share that
object's owner; they do not clone a vector or return null on a failed type cast.
The consumed descriptor edges are ValueParameter -> Variable/Parameter -> Value
-> Callable -> Declaration -> Any. Strict compilation checks those real types,
instantiates the List/Collection/Iterator access boundaries and rejects unrelated
Name collection conversion. It does not instantiate fake descriptors.

`ArrayIterator` translates the actual pinned 14-line Kotlin source, including
post-increment, bounds failure, cursor rollback and NoSuchElementException.
The source Array class is a JVM builtin with bodyless declarations. Its C++
compiler storage implements the specified fixed-size mutable operations with
shared allocation and sequential initialization. No resize operation is exposed.
This is an explicit builtin storage adaptation, not a transliteration of
nonexistent Kotlin method bodies or a claim of JVM/native runtime array ABI.
Kotlin Int cursor arithmetic wraps without C++ signed overflow. Bounds messages
remain platform-specific; existing C++ exception mappings are retained.

Unchanged pinned ArrayIterator source was compiled with cached Kotlin JVM 2.4.10.
Class origins prove the private iterator loaded from freshly compiled source;
Name loaded from the earlier pinned Java compilation. Kotlin and C++ agree on
22 trace observations: shared typed/Any iterator identity and cursor, changed
array elements, repeated normal/empty exhaustion, independent cursors, retained
array ownership, null values, real Name values and source singleton identity,
plus initializer order and specified size/index errors. No descriptor stand-ins
were used. Array bounds/exception message spelling and broad runtime ABI were
not compared. Arbitrary generic variance, List implementations/views/equality,
concrete descriptor routing, user data and substitution remain unverified.

Strict consumer compilation exited 0. Four compiler targets built; iterator,
descriptor transport and two injection regressions recorded four tests and zero
failures in 8.17 seconds. The actual IrSymbolImpl probe exits 1 at IrDeclaration;
ValueParameterDescriptor is no longer its first missing header. Real IR element,
declaration, visitor, signature, rendering and IR-based descriptor conversion
are still required before owner binding can execute. Scope integration is open.

Both mandatory full-root --deep scans exited 0: compiler 608 Kotlin files,
library 354. ArrayIterator has 3/3 explicit functions and 1/1 type, with body
similarity 0.31 and provisional normalized logic 0. ValueParameterDescriptor's
1/1 explicit body (0.99 similarity) is only its default; properties/abstract
contracts are not scored as bodies. Its target grammar reports parse errors
despite strict Clang compilation. The unsupported constructor/class/generic/
property emission and bodyless-builtin inventory zeros remain visible. Array
and Iterator receive raw scoring zeros for no source bodies versus C++ bodies.
Collections still lacks mutable/set/map types and two actual default bodies.
Java descriptor files have no AST measurement. These limitations do not waive
measured acceptance; no production behavior was removed to change a score.

The next source work is the actual DeclarationDescriptorImpl,
DeclarationDescriptorNonRootImpl, VariableDescriptorImpl and
ValueParameterDescriptorImpl dependency chain. Follow the real DescriptorRenderer,
type-update/substitutor, empty collection, mapped overridden parameter collection,
CallableMember kind/visibility and synchronized lazy-resolution dependencies.
The upstream parameter implementation returns itself only for empty substitution
and throws for nonempty substitution; preserve that source limitation. Its empty
initializer/cache and immutability results are also genuine source behavior.
The detailed five-step identity/scope plan remains the top priority.

### Visibility dependency checkpoint

`Visibility` and `Visibilities` translate the actual source base and all nine
singleton classes. Their methods retain source names/flags, unique object
identity, import rules, diagnostic strings, default normalization and nullable
comparison results. `internal` and `protected` have equal ranks but compare as
unknown; so do the two distinct private objects. Only comparing an object with
itself gives zero for that rank. Local/inherited/invisible/unknown visibility is
outside the ordered map. Inherited and unknown import checks retain the source
errors. C++ signed negation uses unsigned arithmetic plus bit conversion to
preserve Kotlin Int wrapping for custom opposite comparisons.

The full DescriptorVisibility base class is translated against genuine source
types. Its abstract accessibility/import/normalization methods remain abstract
because upstream does. Its implemented properties/comparison/text/effective
forwarding and source package default compile. The other class in that Kotlin
file, DelegatedDescriptorVisibility, depends on the real DescriptorVisibilities
mapping and concrete accessibility rules and remains untranslated. No missing
mapping or accessibility algorithm was replaced by a fabricated result.
ObsoleteDescriptorBasedAPI metadata also remains open. EffectiveVisibility,
FqName and ReceiverValue are actual forward-referenced compiler types, not
substitute classes. Visibility strings preserve UTF-16; objects have stable
compiler lifetime and borrowed references, with no native runtime ABI claim.

The unchanged pinned `Visibility.kt` and `Visibilities.kt` were compiled with
cached Kotlin JVM 2.4.10 tooling and real auxiliary compiler classes. JVM class
origins confirm both tested classes loaded from the newly compiled source
directory. Kotlin and C++ traces agree byte for byte on 91 observations: nine
singleton/property/error records, all 81 pair comparisons and the default
visibility identity. The actual package/default-effective type consumers and
concrete descriptor visibility objects are not covered by that demo. Strict
compilation exited 0; four compiler targets built; visibility, descriptor
transport and two injection regressions recorded four tests and zero failures
in 8.90 seconds. Execution and source/compiler hashes are in
`build/ir-recovery/descriptor-visibility/`.

The deep scan exposed an extraction defect for C++ reference-return methods:
the vendored grammar gives reference_declarator a sole unnamed declarator child.
The old extractor followed only named declarator fields and inventoried
Visibility::normalize and get_name as anonymous functions. The raw tree probe
and a regression reproduced this failure. The extractor now follows that exact
wrapper for names, qualifications and parameters; lvalue/rvalue references,
qualified methods and free functions are covered. All nine oracle suites record
zero failures. This corrects measurement; it adds no production coroutine path.
Unsupported constructor/inheritance/property/object emission remains visible,
as do untranslated DelegatedDescriptorVisibility/EffectiveVisibility and the
broader symbol/declaration/scope gaps. Raw scores are retained; execution does
not waive the required measured criteria.

### Source-location and descriptor-base checkpoint

`SourceElement` and `SourceFile` translate their genuine source-defined
`NO_SOURCE`/`NO_SOURCE_FILE` objects. Their C++ anonymous implementations are
private to the `.cpp` files. `NO_SOURCE` returns the exact source-file singleton,
whose name is null, not an empty string. Static object storage and borrowed
interface references preserve identity for compiler lifetime. The inherited
JVM Object methods use the previously documented C++ compiler-object adaptation;
canonical interface boxing, address hashes and RTTI spelling do not establish
Kotlin/Native runtime object compatibility. Actual source-file names use nullable
UTF-16 strings. No substitute descriptor hierarchy was added.

The descriptor's source, non-root and visibility ancestors now retain the source
abstract contracts and real inheritance. Narrowed original-descriptor references
compile as genuine covariant overrides. The root containing-declaration getter
is nullable; the non-root override retains its required pointer-compatible C++
return and carries the original nonnull contract as Clang metadata. The Clang
AST receipt contains `ReturnsNonNullAttr`. This is source metadata, not an extra
runtime assertion. Real descriptor factories/owners and visibility algorithms
are still required before these interfaces have production execution acceptance.

`Substitutable<T>` carries the typed source abstract function. Its source NonRoot
upper bound is checked after self-referential descriptor types finish definition.
Strict compilation instantiates the real NonRoot type successfully; a separate
negative compilation rejects the real `Name` value type. Source generic `out`
variance beyond descriptor return narrowing remains unverified and must be
preserved at actual consumers. TypeSubstitutor and concrete substitution remain
untranslated. Abstract source declarations are not claimed as implemented algorithms.

The source-location units rebuilt in `kxs-inject`, `KotlinxCoroutinePass` and
`kxs_codegen_test`. Strict source/base-contract compilation exited 0. The existing
transport test now executes the actual source singleton chain, plus canonical
identity/null checks. It and both injection regressions completed with zero
failures in 8.88 seconds. The pinned Java source was compiled unchanged except
for erased nullability annotations/imports. Java and C++ produced the same line:
`source=NO_SOURCE name=null same_file=true same_element=true`. Source/adaptation
hashes, traces, compilation and test receipts are in
`build/ir-recovery/descriptor-sources/`.

Both required full-root deep scans exited 0, with compiler inventory 598 Kotlin
files and library 354. Nine further Java dependencies remain outside AST language
coverage. The Substitutable row receives a raw zero: `ast_parser.hpp:1927-1930`
intentionally excludes bodyless abstract declarations from function-body scoring,
while the C++ destructor implementing the type bound is counted as an extra body.
The source's `fun substitute` is present at line 22. The emitter also leaves its
class unsupported. These findings and the earlier property/class-emission limits
remain visible; no production method was removed to change the score.

The direct symbol draft still first stops at `ValueParameterDescriptor.hpp`.
Next translate the actual callable/value/parameter contracts and dependencies.
In particular, `CallableDescriptor.java:71` returns
`Collection<? extends CallableDescriptor>`, while `ValueParameterDescriptor.kt:53`
narrows it to `Collection<ValueParameterDescriptor>`. Preserve that covariance,
source collection identity/lifetime and typed access; C++ vectors with unrelated
virtual return types are not a solution. Likewise retain the generic UserDataKey
contract, source TypeSubstitutor and DescriptorVisibility, and genuine constant
value/type dependencies. The earlier visitor bridge does not solve collection
variance or IR visitor routing. Continue the real IR element/declaration closure
and then execute symbol binding with actual owners before scope integration.

### Descriptor checkpoint: compiled dependency contracts

The root `DeclarationDescriptor` interface now has source original/containing
accessors, the generic `accept` boundary and `accept_void`. Its genuine inherited
`Annotated` and `ValidateableDescriptor` contracts are present. `AnnotatedImpl`
stores and returns the borrowed annotation collection. Compiler/module ownership
must retain that collection; no owning copy or new lifetime scheme was added.
The full collection algorithms and concrete descriptors are still required.

C++ cannot express a virtual function template. The documented internal boundary
retains all fifteen source descriptor types; each concrete translated descriptor
must select its actual visit method. Result transport retains values, references,
move-only ownership, null pointer results and void returns. Four explicit template
configurations compile every forwarding method. `kxs_descriptor_transport`
executes transport behavior only; no substitute descriptor hierarchy was created.
This test does not establish routing through actual descriptor objects.

Inherited JVM Object equality/hash/string behavior has explicit C++ compiler-object
adaptations. The default equality uses stable object identity; structural equality
calls that virtual method so the future IR-based descriptors can override it with
source owner equality. Object boxing uses the canonical descriptor reference.
Address hashes and RTTI diagnostic spelling do not establish Kotlin/Native runtime
object layout or identityHashCode compatibility. These contracts remain subject to
execution with genuine translated descriptor views.

`DeclarationDescriptor.cpp`, `ValidateableDescriptor.cpp` and `AnnotatedImpl.cpp`
compile under `-Wall -Wextra -Werror` and are linked into `kxs-inject`,
`KotlinxCoroutinePass` and `kxs_codegen_test`. The transport and two existing
injection regressions executed with zero failures in 8.44 seconds. File/function
provenance ranges resolve to pinned sources; these descriptor files contain no
prohibited source comments. Receipts are in `build/ir-recovery/descriptor-visitors/`.

A direct symbol implementation probe still exits 1. Its first missing genuine
header is now `ValueParameterDescriptor.hpp`. The remaining direct dependencies
are `IrDeclaration`, `IrBasedDescriptors`, `IdSignature` and `RenderIrElement`.
Translate the descriptor's actual variable/parameter/callable/type hierarchy,
then the real IR element/declaration/visitor hierarchy, without collapsing those
contracts to aliases. Binding actual owners, variable scopes and suspension scopes
remain unfinished. This progress is within steps 1–2 of the first-priority plan.

Both required final full-root `ast_distance --deep` runs exited 0. The compiler
inventory is 596 Kotlin files and the library 354. Three new Java dependencies
remain outside the oracle's supported languages. The new `Annotations.kt` row has
zero functions translated: the getter contract alone does not implement its 22
collection functions or remaining classes. Its raw zero is retained. The earlier
symbol property-getter inventory and unsupported class-emission limitations also
remain visible. No score or dependency count is treated as whole-priority completion.

### Earlier symbol checkpoint: compiled contracts and uncompiled implementation

The source interfaces, `is_public_api`, marker hierarchy, `Named` and parameter
kinds compile. `kxs-inject`, `KotlinxCoroutinePass` and `kxs_codegen_test` rebuilt
with the new symbol unit. `test_kxs_inject` and `test_kxs_compiler_pass` executed
with zero failures in 8.28 seconds. These existing regressions verify the retained
injection path; they do not execute binding against actual translated IR owners.
Compiler logs and the separate strict header compilation receipt are in
`build/ir-recovery/ir-symbol-contracts/`.

`IrSymbolImpl.hpp` contains the source owner binding, unbound/rebinding errors,
descriptor selection, original-descriptor recursion, signatures and rendering
branches. It is a source-bodied draft, not an accepted implementation. A direct
compilation probe at that checkpoint exits 1 at the first missing genuine dependency header,
`DeclarationDescriptor.hpp`. The remaining includes need `ValueParameterDescriptor`,
`IrDeclaration`, `IrBasedDescriptors`, `IdSignature` and `RenderIrElement`.
The generic visitor/type/annotation/parent/factory dependencies behind these types
are still required. No test-only declaration or descriptor hierarchy was supplied
to make the binding checks appear executable.

Original-descriptor checks retain Kotlin structural equality. In particular,
`IrBasedDescriptors.kt:59-63` defines equality/hash through the declaration owner,
so comparing only descriptor addresses would be wrong. The draft uses descriptor
`operator==`; the descriptor translation must provide its dynamic source equality
alongside original/containing accessors. Symbol binding itself retains exact owner
object identity.

Actual callable dependencies were located in pinned Git objects:
`IrBasedDescriptors.kt:68-82` dispatches across all real declaration kinds, and
`RenderIrElement.kt:37-38` invokes the real rendering visitor. The general descriptor
conversion cannot be replaced by just the variable case, and diagnostics cannot
substitute fabricated declaration rendering. These functions remain untranslated.

Source-defined defaults stay intact: `IrSymbolBase.signature` is null, and its
private signature starts null. Marker interfaces have no methods in upstream.
These are actual source contracts. Construction opt-in/obsolete-descriptor
metadata remains open. C++ virtual interface inheritance and abstract destructors
are documented language adaptations. Diagnostic class spelling/object identity
and assertion-enabled configuration have explicit `NOTE(port)` adaptations and
still need review when the genuine class can be instantiated. Upstream's pending
value-parameter original-descriptor correction and future JS private-signature
migration are recorded here; their existing behavior is retained in the draft.

Both project-wide deep scans completed with exit status 0. The compiler inventory
is 594 Kotlin files after the two additional materializations; the library is
354. The oracle reports 3/3 explicit functions and 2/2 types for the base-symbol
file, but zero emitted normalized logic because class/property emission remains
unsupported. That count does not certify its getters, dependencies or execution.
The interface file receives a scoring failure: no source functions found, while
C++ defines functions. Kotlin `isPublicApi` is a property getter at lines 115-116,
and interface properties become C++ methods; that source inventory distinction
must be corrected in the oracle before this criterion can be satisfied. The raw
zero and diagnostics are retained without changing C++ to manipulate scoring.

The fully qualified name cache lifetime question is also open. `FqNameUnsafe`
retains its safe-name cache, and `isSafe` depends on that cache being present.
Replacing it with an expiring weak reference would change observable behavior
for a name containing `<`. The translation must preserve the linked source
caches and explicit ownership, with no weak-cache substitute or permanent leak.

### Earlier Name execution checkpoint

`kxs_ir_name_contract` checks ordinary/special names, validation versus factory
behavior, nullable results, dynamic equality, source errors, malformed special
substring behavior, UTF-16 ordering and wrapping hashes. The reversed substring
bounds error reproduces Java's actual failure; no temporary proof guard was
added. C++ diagnostic encoding is separate from UTF-16 value behavior.

The pinned `Name.java` was compiled and executed with Java 25. Only
`NotNull`/`Nullable` annotations and imports were erased from that test copy;
all source methods stayed unchanged. Source/transformed hashes are in
`build/ir-recovery/ir-identity/java-source-receipt.json`. Six observations are
byte-for-byte identical to C++: supplementary-character order, UTF-16 hash,
overflow hash, permissive identifier factory, unclosed special-name substring
and ordinary-versus-special inequality. Traces are `name-java.trace` and
`name-cpp.trace` beside that receipt.

The exact `tools/ast_distance/ast_distance --deep` command ran against both
full compiler and library roots, with logs in the same evidence directory.
Newly visible Kotlin sources expand the compiler inventory, which is not port
progress. The tool has no Java parser, so it cannot measure this `Name.java`
translation. Pinned Java execution supplies behavioral evidence without
replacing either required project-wide scan or claiming a measured Java AST
distance. Declaration and scope gaps remain open in the generated priorities.

## Consumed collection-to-array bodies: 2026-10-07

Recorded before implementation. AbstractCollection.kt:34-54 delegates both
toArray operations to native-wasm Arrays.kt:87,89, which call the complete
common Collections.kt:514-545 algorithms. Actual Native ArrayList.kt:205-218
and its ancestors require these operations. The consumer chain remains actual
ArrayList -> transform.kt:126-137 -> real IR declaration construction/rewrite
-> Native state-machine construction and suspension scopes.

Translate both complete common loops and both Native forwarding bodies for the
consumed erased Any? array instantiation, plus Arrays.kt:85 reference-array null
allocation. Internal source generics may use concrete C++ instantiations; this
does not complete the generic API for arbitrary typed C++ arrays. Preserve
isEmpty/size/iterator/hasNext/next evaluation order, allocation only when needed,
null initialization, bounds failures, same destination storage and Native's
unchanged trailing elements. Source star-projected collections use their genuine
Collection<std::any> covariance view; do not fabricate a concrete list.

The empty branch requires actual Native ArrayIntrinsics.kt:31-35 and
Arrays.cpp:175-177 emptyArray. Translate the consumed Any? specialization as one
canonical compiler-owned empty array. Native returns a generated immutable
zero-length object; the C++ compiler storage already has fixed length and no
valid mutable slots at length zero. This projection neither links Native GC nor
claims cross-element-type canonical identity; that generic array ABI is still
required when further consumers need it.

Stopping condition: compile/link these bodies in KotlinxCompilerObjects and
return them to actual AbstractCollection/Native ArrayList. Do not substitute a
collection fixture or an independent collection project for actual consumer
execution. Record execution as unfinished until genuine source-class instances
are available. Arbitrary array-element covariance/nullability, real ancestor
equality/hash/text, ArrayList, IR construction/binding/scopes and both full MLX
paths remain open.

## Collection conversion source integration checkpoint: 2026-10-07

The recorded consumed common and Native bodies now compile in the production
KotlinxCompilerObjects archive and Native-OFF build. Shared collection loops live
in CollectionToArray.cpp/.hpp; actual Native allocation/forwarding functions
live in Arrays.cpp/.hpp. ArrayIntrinsics.cpp supplies the consumed Any? empty
entry with canonical zero-length compiler storage. No generic array alias,
replacement backing list, Native heap layout or runtime fallback is introduced.

Ten allocation/empty-array observations agree between C++ with ASan/UBSan and
Native 2.4.10, running the pinned reference-allocation body with only its actual
modifier removed and name changed to avoid an installed stdlib collision. The
installed Native intrinsic/runtime supplies allocation and empty-array entries.
These observations cover null initialization, zero length, iteration, bounds,
negative size, source-array independence and shared element handles. They do not
execute either collection loop or prove general cross-type empty-array identity.
Five production and three standalone compiler tests have zero failures; ordinary
C++ retains 42/43/82. Source checks: six files, 29 pinned ranges, no prohibited
labels. Initial and final split receipts: build/ir-recovery/ir-collection-conversion/.

Both full-root deep scans exit 0 and cover compiler 672/library 354 sources,
587 paired units and 763 physical files. Common conversion 2/54 functions;
Native Arrays 6/10. ArrayIntrinsics body table 1/9, target two functions; deep
symbol inventory explicitly records emptyArray PRESENT and arrayOf missing.
All three affected groups retain provisional normalized-logic/span zeros,
unsupported emission/generated errors, and no target parse errors. The emission
evidence identifies the conversion/Native generic function declarations as
unsupported. These results are preserved; no scorer change or waiver is made.

Strict compilation of the actual TransformIfNeeded.cpp still stops at missing
ArrayList.hpp:20. Return these available helpers to genuine AbstractCollection/
AbstractMutableCollection/AbstractMutableList and Native ArrayList, preserving
actual element equality/hash/text; then execute real copy-on-change identity and
return to parameter descriptors, symbol owner binding and Native scopes. Full
typed array ABI, constant-constructor lowering and both full MLX demonstrations
remain required. No new completion card is justified for this unfinished consumer.
