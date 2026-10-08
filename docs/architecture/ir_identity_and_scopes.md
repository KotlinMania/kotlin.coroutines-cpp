# First priority: Kotlin IR declaration identity and suspension scopes

Date: 2026-10-06. Implementation owner: the active Codex conversation.
Compiler card: `t_16bf1579`. Umbrella: `t_1834dcec`. Audit: `t_0dd3c2ad`.

This is the first implementation priority for this project's unfinished compiler
work, ahead of further standalone LLVM helpers, convenience adapters or scorer
feature expansion. It supplies the real Kotlin IR objects and lookup rules that
state-machine construction needs. This document is a source-grounded plan;
recording it does not mean the missing classes or algorithms are implemented.

## Standalone C++ use and Kotlin/Native compatibility

Sydney's requirement on 2026-10-06: this is a standalone C++ coroutine port that
must also be compatible with Kotlin/Native. Normal C++ functions, types and MLX C++
code must remain usable with the port. Standalone C++ applications must build and
run without an installed Kotlin compiler, linked Kotlin/Native runtime, or JVM.
They use the translated C++ coroutine implementation and the CMake/Clang plugins.
Upstream Kotlin is the source of the translation and compatibility contract; it is
not a required application runtime for ordinary C++ use.

Standalone independence applies to building the C++ library and its CMake/Clang
plugins as well as to application execution. The translated compiler dependencies
must build as C++ without invoking kotlinc/konanc or linking Kotlin/Native runtime
libraries. Pinned Kotlin source remains the development reference and provenance
source. Kotlin/Native compatibility is a required capability; its runtime becomes
a dependency only for builds that explicitly enable the Native interop boundary.

Existing nonsuspending C++ functions need no Kotlin annotations or translation.
Ordinary C++ classes, standard-library values and MLX handles remain usable in
coroutine bodies and locals. The plugins must preserve their C++ ownership and
destruction across repeated suspension, completion, failure and cancellation.
Retaining a borrowed pointer/reference must not transfer ownership. This remains
an executable acceptance requirement, not a claim that all such cases work today.

The translated compiler's `Any` and IR classes are internal tooling contracts.
Ordinary C++ application classes need no Kotlin base class or object storage.
The compiler must retain their actual C++ declarations, values and lifetime
operations when constructing coroutine frames, including when Native
interoperability is enabled.

Kotlin/Native interoperability is an explicitly linked boundary when requested.
That boundary requires the real matching Native runtime, continuation/frame/result
contracts, and roots for actual Kotlin GC objects. Ordinary C++ values, RAII objects
and MLX handles retain their actual C++ lifetimes; using the coroutine port does
not require converting them to Kotlin objects. This distinction does not authorize
an alternate coroutine state machine, substitute runtime or fallback.

Acceptance requires both:

1. A standalone CMake/Clang C++ executable that uses ordinary C++ functions and
   types with the coroutine authoring surface, retains C++ locals/resources across
   suspension, performs real MLX C++ GPU work, and resumes correctly. Verify its
   build and linked dependencies do not require Kotlin tools or runtime libraries.
2. The Kotlin/Native/C++ docking-ring demonstration through the actual Native
   coroutine state machine and direct unsafe MLX bindings, with both handoff
   directions, results, failure/cancellation, resource identity and cleanup.

The C++ implementation must satisfy its own executable contract even when no
Kotlin program participates. Native interoperability tests are separate evidence.
Existing array fixtures prove neither complete standalone authoring/MLX integration
nor the complete shared-state-machine boundary.

## Consumed Native class contracts checkpoint: 2026-10-07

The genuine KClass/TypeInfoHolder contracts and consumed implementation/getter
bodies are translated. Three new nongeneric bodies and AnyToString.cpp compile;
25 actual-type checks/four real consumers compile. The consumed pointer hash has
21 agreeing observations on actual Native 2.4.10 class metadata before/after GC,
including null. No C++ Any/KClass instance or new production linking is claimed.
Real compiler-object metadata/instance bindings and the actual constant-constructor
intrinsic remain unfinished. Nine files retain 40 ranges/seven source KDocs.

Both full-root scans cover compiler 672/library 354, 579 paired units/752 physical
files. KClassImpl raw functions/types are 4/11 and 1/2 with body similarity 0.10;
NativePtr remains missing. All five affected deep groups retain provisional
score/logic/span zeros and generated errors. Return through real metadata binding
to Any/list/IR consumers and Native scopes/buildStateMachine; both complete MLX
GPU demonstrations remain required. See [the precise checkpoint](../audits/IR_IDENTITY_DEPENDENCIES.md#consumed-native-class-contracts-checkpoint-2026-10-07).

## Native Any prerequisite checkpoint: 2026-10-07

Historical checkpoint; the current consumed class checkpoint supersedes its
header frontier and inventory counts.

Any's complete public source class and three method bodies are written. Its
identity bodies and consumed Long formatter compile; 393 C++ digit/error checks
execute under sanitizers with no Kotlin runtime. The text body still stops at
actual KClassImpl.hpp. No metadata or Any instance was fabricated and no new IR
inheritance was linked. Return through real metadata/list ancestor closure to
ArrayList/copy-on-change and actual IR binding/Native scopes/buildStateMachine.

Both full-root scans cover compiler 671/library 354 sources, 571 paired units/
743 physical files. Raw Any functions/types are 3/3 and 1/1, body similarity 0.53;
StringNumberConversions remains 0/17, Char missing, and both affected deep groups
retain provisional score/logic/span zeros and generated errors. These checks
establish neither complete MLX GPU demonstration. See [the exact checkpoint](../audits/IR_IDENTITY_DEPENDENCIES.md#native-any-prerequisite-and-ordinary-c-design-checkpoint-2026-10-07).

## Native ArrayList prerequisite algorithms checkpoint: 2026-10-07

Historical checkpoint; the current Any checkpoint above supersedes these counts.

The consumed ancestor predicate-removal bodies and actual RandomAccess marker
are translated. Five real-interface consumers compile; they include actual IR
declarations without fabricated backing lists/iterators. Complete Native range
copying and array-return helpers execute on compiler-owned storage; all 23
observations agree with exact pinned source bodies running Native 2.4.10 and
C++ sanitizer execution. Actual list-removal/copy and owner binding remain
unexecuted. No new compiler production integration or Native heap/frame claim
follows from those array checks.

Full-root --deep inventories are compiler 670/library 354, 567 paired units/738
physical files. Raw MutableCollections functions 6/33, Native Arrays 3/10,
generated arrays 9/240 and RandomAccess 1/1 types retain all provisional deep
logic/span zeros and generated errors. Return to actual Native ancestors/ArrayList
constructor/backing/set; TransformIfNeeded still stops at missing ArrayList.hpp.
Then resume concrete IR declaration/descriptors/binding and Native scopes/
buildStateMachine. Both complete MLX GPU demonstrations remain required. See
[the precise checkpoint](../audits/IR_IDENTITY_DEPENDENCIES.md#native-arraylist-prerequisite-algorithms-checkpoint-2026-10-07).

## Earlier actual IR class list-rewrite checkpoint: 2026-10-07

The actual Native mutable-list interface and two consumed compiler rewrite
algorithms are written. Strict Clang compiles the in-place declaration rewrite,
its index-check dependency and the real IrClassChildren source. Twenty-eight
actual-type checks and three consumer functions compile without fabricated nodes,
visitors or backing lists. Class traversal and mutation retain their actual
parameter/declaration identities and original order.

The type-parameter copy-on-change body still requires actual Native ArrayList;
its nongeneric body stops at missing ArrayList.hpp. Its written algorithm retains
the original list until the first identity change, the source collection copy,
index check before next element and subsequent indexed writes. Class children
and these new bodies are not linked into production yet. No actual list traversal,
copy operation, parameter construction or binding was executed. Return through
the actual ArrayList constructor/set/backing closure to this consumer, then real
empty lists/descriptors/owners and Native scopes/buildStateMachine.

47 ranges and nine source comment blocks are checked against ten exact pinned
sources. Both full-root scans cover compiler 668/library 354 sources, 565 paired
C++ units and 736 physical files. Raw transform functions are 1/9; Native List
reports 1/2 types and forced function zero; Native ArrayList remains missing with
91 functions/three types. Provisional logic/span zeros, generated errors and the
transform-header namespace rejection remain required findings. Neither complete
standalone C++/MLX nor direct Native/C++ state-machine/MLX GPU demo is established.
See [the exact consumer checkpoint](../audits/IR_IDENTITY_DEPENDENCIES.md#actual-ir-class-list-rewrite-checkpoint-2026-10-07).

## Earlier actual IR parameter and default-body checkpoint: 2026-10-07

Actual type/value parameters, their complete concrete source constructors, typed
leaf symbols and real default-expression bodies are written. Their six interface/
visitor bodies compile; 33 real-type checks and four consumer functions compile
without fabricated objects. Both concrete parameter constructors retain actual
empty lists and symbol.bind(this), but compile stops at missing empty_list. Both
leaf-symbol implementation bodies stop at the actual IR-based descriptor header.
There is no constructed/bound parameter execution or production linking yet.

The class-child consumer now reaches the real missing MutableList.hpp, beyond
its former missing parameter headers. Finish the consumed mutable-list transforms,
empty-list and descriptor/signature/rendering closure, then return the actual
nodes to class traversal and Native scopes/buildStateMachine. Any/class metadata,
attributes and real variable construction/binding remain required. A type check
does not replace those algorithms or establish complete coroutine lowering.

267 source ranges and eight full source comment blocks are checked; the nested
isHidden examples/question are retained with only its prohibited label changed.
Five existing compiler targets build and four focused CTest checks have zero
failures. Native-OFF ordinary_cpp executes 42/43/captured-object 82 with OS libraries
only and no Kotlin tools/runtime references in its configured graph. It remains
the scalar/capture example, without complete lifetime or MLX GPU acceptance.

Both full-root deep scans cover compiler 664/library 354 sources against 560 paired
units/730 physical files. Parameter functions are 1/2 and 1/4, body functions 1/1
and 1/4; generated symbols 6/23 types and implementations 3/17. TypeSystemContext
is 9/32 and 0/74 functions. Forced constructor/property zeros, provisional logic/
span zeros, generated errors and reported-missing container files remain required.
Both full MLX GPU demonstrations stay open. See [the source/compiler/oracle
checkpoint](../audits/IR_IDENTITY_DEPENDENCIES.md#actual-ir-parameter-and-default-body-checkpoint-2026-10-07).

## Earlier actual IR class contracts checkpoint: 2026-10-07

The actual IrClass and five source declaration contracts are written; its source
child traversal/transform bodies still stop at missing IrTypeParameter.hpp and
need actual value parameters, mutable lists and transform helpers. IrClass and
IrClassSymbol remain outside production linking until those dependencies and the
base renderer are available. Twenty-six actual-type checks and two interface
call sites compile without fabricated instances. No real class or symbol owner
was constructed or bound.

The actual classifier contract is included by IrType; its descriptor body and the
complete ClassKind operations compile in all three LLVM compiler targets. Forty-eight
property observations agree with unchanged pinned Kotlin source running as Native
machine code and C++ sanitizer execution. Four affected CTest checks have zero
failures. Native-OFF ordinary_cpp rebuilds/executes with OS libraries only, empty
konanc configuration and no Native unit in its build graph. This is the existing
scalar/capture fixture, without MLX GPU execution.

Both full-root deep commands exit 0: compiler 659/library 354, 549 paired C++ units/
709 physical files. Raw IrClass visitor functions stay 0/3; ClassKind retains forced
function zero, and the two declaration-container headers remain reported missing.
The missing-container findings trace to foreign template forwards being treated
as full declaration namespaces; the exact diagnostics and source trace remain in
build/ir-recovery/ir-class-identity/oracle-namespace-investigation.json.

TypeSystemContext is 8/32 types and 0/74 functions. Provisional normalized-logic
zeros, unsupported/generated errors and other missing APIs remain required; no
criteria are waived. The next source consumer is actual class child declarations/
transformations, followed by compiler metadata/Any/attributes, variable owner
binding and Native scopes/buildStateMachine. Both MLX GPU demonstrations remain
unfinished. See [the source and execution checkpoint](../audits/IR_IDENTITY_DEPENDENCIES.md#actual-ir-class-contracts-checkpoint-2026-10-07).

## Actual Native class-name prerequisite checkpoint: 2026-10-07

The complete consumed TypeInfoNames source getters now use real Native metadata
and strong runtime/root calls. Seventy-two comparisons with unchanged pinned source
on twelve actual Kotlin class identities agree before/after GC, including local '$'
names, hidden reflection names, Unicode and empty packages. Two optional Native
CMake checks have zero failures. The Native-OFF ordinary C++ application rebuilds
and runs without Native libraries or the new unit in its build graph. A broader
all-target build exposes unfinished delay/yield authoring in JobTest/AsyncTest.

This is the Native portion of the class-name prerequisite. Actual Any ancestry and
compiler-generated metadata for C++ compiler objects remain missing; the real
IrType self-equality probe still rejects that conversion. Do not root or reinterpret
C++ objects as Native objects to bypass it. Return to actual Any/KClass class-literal
construction, then attributes, declarations, symbol binding and suspension scopes.
Deep reports retain forced function/logic/span zeros and unsupported source evidence.
Both complete standalone and Native/C++ MLX demos remain required. See
[the source/execution checkpoint](../audits/IR_IDENTITY_DEPENDENCIES.md#actual-native-class-name-checkpoint-2026-10-07).

## Replacement goal: constrained docking-ring implementation

Written at Sydney's request on 2026-10-06. This replaces the broad instruction to
work through priorities and cards as the written objective. Recording this wording
does not activate or complete an app goal.

Complete the docking ring in kotlin.coroutines-cpp: deliver a standalone C++ coroutine port usable with ordinary C++ code, with no Kotlin installation or runtime dependency for standalone applications, and compatible with Kotlin/Native. Make Kotlin-style coroutine authoring and the faithfully translated coroutine APIs available in C++ through the CMake-enabled Clang frontend and mandatory LLVM injection, and enable Kotlin/Native to call MLX C++ and GPU-driver operations through direct unsafe native bindings. Use the actual Kotlin/Native IR, continuation and coroutine state-machine contracts. The target is Native machine-code execution and MLX/GPU interoperability.

Keep IR value-declaration identity, concrete parameter descriptors, symbol owner binding and suspension scopes as the first implementation priority. Continue the existing source dependency work only where a named Kotlin compiler/runtime function requires it to implement this production path. Before expanding a dependency, record its upstream path and function, its consuming function, the chain to a docking-ring requirement, the exact missing behavior, and the condition that ends that dependency task. Follow that chain back to its consumer after the prerequisite is available. Do not turn collection, standard-library, compiler or measurement work into independent projects. A genuinely required source dependency must be translated faithfully; it may not be bypassed with a fake implementation.

Preserve function-for-function and, where possible, line-for-line translation of the relevant pinned sources under tmp/kotlin and tmp/kotlinx.coroutines, including comments, public signatures and algorithms. Provide per-file port-lint provenance and per-function Transliterated from source ranges. No stubs, placeholders, cheap aliases, prohibited source comments or coroutine fallbacks. Keep proof assertions and temporary instrumentation in tests; retain only checks required by the upstream behavior in production.

Integrate the translated identity, scope, state-machine and spilling algorithms into ordinary CMake/Clang compilation. C++ authors must not manually construct the coroutine state machine or save local variables. The plugins must preserve actual IR declarations and LLVM values, saved resume addresses, live variables, immediate/suspended/resumed results, exceptions, cancellation and cleanup. Equivalent-looking C++ coroutine behavior or isolated dispatch instructions do not establish Kotlin/Native compatibility.

Implement the unsafe Kotlin/Native-to-MLX C++ boundary with explicit resource handles, ownership and release contracts. Preserve the actual binding's caller-controlled lifetime policy and the required roots for GC-managed Kotlin objects. Do not invent an automatic safe-wrapper policy, simulated GPU backend or substitute coroutine runtime.

Completion requires a reproducible CMake-built demonstration in which Kotlin/Native and plugin-compiled C++ hand execution through an actual Kotlin/Native coroutine state machine, call real MLX C++ GPU operations through the native bindings, suspend and resume with the correct retained variables and resource identities, and return the correct result. Verify both handoff directions, immediate and suspended outcomes, repeated suspension, failure, cancellation and cleanup. Identify the Native toolchain/runtime, the GPU operation and the evidence that it executed. Array tests, compilation and host-only helper tests cannot certify that end-to-end requirement.

Run tools/ast_distance/ast_distance --deep project-wide over both source roots against the complete src root after relevant source/tool changes. Use its inventories, required measured criteria and priorities as the oracle. Investigate documented measurement defects explicitly; do not suppress gaps, waive the requirement or equate successful execution with measured translation parity.

Keep the existing docking-ring cards and written designs synchronized. Report progress in plain language: the docking-ring behavior enabled, the dependency completed and its named consumer, the execution evidence, the remaining required oracle gaps, and the next consumer to integrate. Demonstrate progress periodically. Mark only the bounded work actually verified as done; do not mark this goal complete until the end-to-end docking-ring requirements and their relevant required criteria are satisfied.

## Governing deliverable: Kotlin-style authoring and unsafe MLX C++ bindings

Sydney's clarification on 2026-10-06: the docking ring is the deliverable. C++ must
expose the Kotlin-style coroutine authoring surface, including suspend and yield
and the translated coroutine operations, through the CMake-enabled Clang frontend
and mandatory LLVM injection. Lowering must follow actual Kotlin/Native IR values,
state-machine construction, saved live values and continuation/result behavior.
Generic C++ coroutine behavior or matching dispatch shape alone does not establish
that requirement.

Kotlin/Native must also be able to call MLX C++ and GPU-driver operations through
explicit unsafe native bindings. Preserve the actual binding's raw handles and
caller-controlled lifetimes; document the actual ownership contract rather than
inventing an automatic safe wrapper or changing the resource lifetime policy.
Native GC roots remain required for actual GC-managed Kotlin objects. Resource
handles are not automatically Kotlin GC objects.

The end-to-end acceptance scenario is a CMake-built C++ coroutine using the authoring
surface, joined to an actual Kotlin/Native coroutine chain, calling real MLX C++ GPU
work through the native binding, suspending and resuming with live state retained,
and returning the correct result or source failure/cancellation outcome. Verify
which GPU work executed and the continuation/frame/resource identities at the
boundary. The matching pinned runtime and bare-metal target acceptance remain
required. No MLX/GPU end-to-end execution has been established by the integer-array
demo.

A "Native map" in prior progress reports means a lookup table from Kotlin's Native
standard library. It is an internal source dependency, not the coroutine authoring
surface, GPU integration, or docking-ring delivery. Array/table tests certify only
those dependency operations. Each further dependency must be tied to its named
consuming Kotlin compiler/runtime function and the production plugin path; report
its effect on the end-to-end deliverable separately from its narrow test evidence.
Source-faithful dependencies and project-wide ast_distance --deep criteria remain
mandatory. Do not replace missing source behavior with a stub to reach a demo.

## Runtime target and acceptance boundary

The target is Native machine-code execution with real MLX/GPU interoperability.
The user's bare-metal requirement distinguishes Native hardware execution from
JVM execution; it does not introduce a separate freestanding or no-OS target.
Use pinned Native runtime and Native actual stdlib implementations for
platform-dependent runtime behavior. Compiler
classes shared with other backends, including classes written in Java, are real
Native compiler dependencies; their host execution does not establish runtime ABI.

The docking ring must use the real Native continuation/state machine and object
reference rules. Trace Native object allocation, ObjHeader/ArrayHeader, stack and
heap roots, UpdateHeapRef/UpdateReturnRef, frame enter/leave and exception cleanup
through their source dependencies. Compiler-owned C++ containers are not Native
object layouts. Complete the actual reference operations before accepting a SlotRecord
or shared runtime object; an ordinary C++ pointer store is insufficient.

The parameter descriptor's lazy dependency follows the complete Native atomic and
reentrant-lock algorithms in kotlin/native/concurrent/Lazy.kt and Lock.kt, plus the
Native lazy factories. Do not implement arbitrary-object JVM monitor semantics.
Collection interfaces and iterator algorithms use Native actual source and comments.

Acceptance must include Native-generated frames handed between Kotlin and C++ in
both directions, preserving declaration identity, spilled values, resume state,
results/exceptions, roots and cleanup. Host C++ tests, JVM compiler-helper comparisons
and an array-only Native executable on macOS are bounded evidence. Actual MLX/GPU
execution and direct shared coroutine frames remain required; no full acceptance is inferred
from a successful host build or a narrow trace.

## Current actual type and node dependency checkpoint

Actual get/set/suspension nodes now carry real value symbols, actual ID-variable
declarations and separate normal/resumed children, preserving source traversal
order. The explicit IrType family/default getters and consumed type markers now
compile in the existing compiler targets; variable/expression headers include that
actual type. Scalar variance/nullability operations agree with Native execution.
These bodies do not establish concrete type/variable/owner or scope execution.

The next actual object prerequisite is explicit: IrType self-equality cannot yet
convert its own type object to the genuine Any root. Close actual Any/class metadata
and the required key/map/empty-list/IR-based descriptor/rendering dependencies,
then construct real variables, bind owners exactly once and enter Native scopes/
buildStateMachine. Preserve compiler object identity independently of Native runtime
GC layout. Do not invent a type, key, visitor, renderer or base object to reach a
fixture. Keep the declaration/type stopping conditions open until those consumers
execute. The source enum/free-function mapping also retains mandatory measured
function gaps; compile and scalar observations do not waive them.

Both project-wide deep reports are refreshed against complete src, with all raw
missing/zero/provisional findings retained. See
[the actual type checkpoint](../audits/IR_IDENTITY_DEPENDENCIES.md#actual-ir-type-and-enum-operation-checkpoint-2026-10-07)
and [the real node checkpoint](../audits/IR_IDENTITY_DEPENDENCIES.md#real-value-access-and-suspension-node-source-checkpoint-2026-10-07).
Ordinary C++ independence and both complete standalone/direct Native C++ MLX/GPU
acceptance requirements remain unchanged and unfinished.

## Current actual attribute-key/delegate source work

The source key/flag/delegate family is now written, including nullable previous
values, false-as-removal flags, property names, copy flags and weak debug owners.
It remains an uncompiled draft: strict Clang first stops at the genuine Any dependency;
actual weak/property behavior and real-key/node execution are still required.
26 ranges and seven KDoc blocks resolve to the pinned source. Full-root oracle
reports retain 7/14 matches, seven unmatched namespace-represented nested methods,
missing nested types and provisional logic/span zeros. No criterion is waived.
See [the current source and diagnostic receipt](../audits/IR_IDENTITY_DEPENDENCIES.md#actual-attribute-keydelegate-source-checkpoint-2026-10-07).
Return through those actual dependencies to declaration/symbol construction and
Native scopes/buildStateMachine. Standalone C++ must remain independent of Kotlin
tools/runtime; both full MLX demonstrations remain unfinished.

## Earlier IR element storage and initializer work

The actual IrElementBase dense attribute algorithms and the generated initializer
expression hierarchy are now written. The core element, expression, variable and
variable-symbol bodies compile with strict Clang checks. Complete attribute snapshot
and copy bodies still require genuine maps and keys; declaration construction and
binding still require source empty-list, rendering and descriptor dependencies.
No actual node/key execution or production state-machine integration is established.
See [the current source/compiler/oracle receipt](../audits/IR_IDENTITY_DEPENDENCIES.md#ir-element-storage-and-initializer-checkpoint-2026-10-07).
Continue those recorded prerequisites back to real variables, the Native value and
suspension scopes, and buildStateMachine. Both required MLX demonstrations remain open.

## Earlier null-array growth dependency work

The array operations required by IrElementBase's dense attributes are now
translated: arrayOfNulls, both copyOfNulls overloads and object-array copyOf(newSize).
New tail slots are initialized null values, distinct from implementation-dependent
uninitialized storage. Nullable C++ types do not acquire a second nullable layer.
Copy transport retains the existing source bounds/overlap algorithm and permits
the consumed nonnullable-to-nullable type promotion; arbitrary numeric conversion
is rejected. Wider source array covariance remains required, not claimed here.

930 C++/Kotlin/Native observations agree, with strict Apple Clang and address/undefined
sanitizer execution. Source errors, readable null padding, copy independence,
reference identity and retain/release are exercised. The four consumed Native
source blocks match installed Native 2.4.10. Four affected CTest checks have zero
failures. Compiler-owned array storage does not implement Native ArrayHeader/GC ABI.
See [the source and execution receipt](../audits/IR_IDENTITY_DEPENDENCIES.md#null-array-growth-checkpoint-2026-10-07).

Both complete-root deep scans retain low/provisional-zero criteria and missing
source operations. This completes execution of the consumed array operations,
not IrElementBase, attributes, real variables, scope resolution or either required
MLX coroutine demonstration. Return to those actual compiler consumers.

## Earlier concrete variable source checkpoint

IrDeclarationBase, IrVariable, IrVariableImpl and genuine variable symbol/implementation
pairs now have source bodies: parent errors, initializer traversal/mutation, all
properties/defaults and constructor binding to the same declaration. Descriptor
access joins value/bindable paths through one abstract root dispatch, retaining
existing source binding state and generic bounds. This is source translation;
strict new-body compilation stops at real IrElementBase and IR-based descriptor
dependencies. No replacement node or collection was supplied.

Four existing boundary units compile in three compiler targets and fifteen genuine
contract checks; invalid owner/descriptor types are rejected. Two injection
regressions have zero failures; they do not instantiate these variables. Both
full-root --deep reports retain 21 missing generated symbol interfaces, 16 missing
implementations, three unmatched variable operations and provisional logic zeros.
See [the current receipt](../audits/IR_IDENTITY_DEPENDENCIES.md#concrete-variable-source-checkpoint-2026-10-07).
Close the recorded genuine ancestor/attribute/collection/descriptor dependencies,
then execute the constructors and return to variable/suspension scopes and Native
buildStateMachine. Both complete C++/Native/MLX demonstrations remain open.

## Earlier typed value declaration/symbol work

The actual IrValueDeclaration and IrValueSymbol contracts now compile together,
including source descriptor/type contracts and the typed declaration-to-symbol-to-
owner relationship. C++ cannot validate mutually narrowed virtual returns while
the opposite class is incomplete. The documented C++ boundary retains the typed
source getters through one abstract virtual dispatch on the actual root object;
conversion occurs after both genuine interfaces are defined. No replacement owner,
symbol or binding state is created. Source generic bounds remain enforced.

These units compile in the existing injector/LLVM plugin/codegen targets. Thirteen
actual-type contract checks compile; an invalid Name owner is rejected. Concrete
variable objects, one-time binding and suspension scope execution remain unverified:
the actual symbol implementation still requires IR-based descriptor/render/signature
closure. Read [the current source/measurement receipt](../audits/IR_IDENTITY_DEPENDENCIES.md#value-declarationsymbol-boundary-checkpoint-2026-10-07)
for exact provenance, C++ adaptation limits and raw property-scoring zeros. Return
to genuine declarations/attributes/variables and their named Native builder/scope
consumers. This checkpoint does not complete either required C++/Native/MLX demo.

## Current coroutine transformer dependency work

The real source transformer bodies used by Native buildStateMachine are now written:
IrTransformer's 89 generic visit methods and IrElementTransformerVoid's 181 source
methods/helpers. They retain child-first rewrites, typed node results and the actual
package/file route and checked casts. They remain source-bodied drafts: strict
compilation stops at missing real node definitions, and they do not yet execute
against real declarations or the production coroutine builder. Their provenance,
compiler diagnostics and raw oracle criteria are recorded in
[the dependency ledger](../audits/IR_IDENTITY_DEPENDENCIES.md#transformer-source-checkpoint-2026-10-06).
This does not change the first-priority declaration/value/symbol/scope sequence or
the two required standalone C++ and actual Native/MLX acceptance demonstrations.

## Current Native integer-array dependency work

The Native HashMap's integer-array prerequisite is now translated: zero-initialized
IntArray, its real specialized iterator, primitive overlap-safe copy/fill, generated
range/resize/default algorithms and the actual five AbstractList companion helpers.
5,367 C++/Native observations agree. A separate fixture directly mutates real Native
IntArray objects from C++ through mandatory runtime symbols. Compiler-owned array
storage remains separate from Native GC-managed layout.

The [integer-array dependency ledger](../audits/IR_IDENTITY_DEPENDENCIES.md#native-integer-array-dependency-and-boundary-2026-10-06)
records source functions/comments, exact provenance, execution and mandatory raw
--deep findings. All eight primitive iterator contracts and forwarders are present;
the other seven primitive arrays, most generated operations, full collection ancestor
algorithms and the complete map remain unfinished. Oracle normalized logic remains
provisional zero; bounded execution does not waive those required criteria.

Continue with the genuine collection ancestor equality/hash/string dependencies,
then Native HashMap and its iterator/view/builder/empty-map behavior, then IR
attribute storage/copy. Concrete descriptors, symbol ownership, value-declaration
identity and suspension scopes remain the top-priority destination of this sequence.
Pinned runtime, shared Kotlin/Native frames and bare-metal execution remain required.

## Current Native array dependency work

The actual object-array utility algorithms needed by HashMap are now written and
executed: uninitialized allocation, copying/resizing without reading empty slots,
overlap-safe copies, resets and source bounds handling. A separate source Native
boundary operates on real ObjHeader arrays with strong runtime symbols and result
slots. 862 Native/C++ observations agree; a real linked Native fixture verifies
object identity, both overlap directions, C++ reset/fill and returned-root behavior.
This does not turn compiler-owned C++ arrays into Native objects.

The [array dependency ledger](../audits/IR_IDENTITY_DEPENDENCIES.md#native-array-storage-and-direct-array-boundary-2026-10-06)
records provenance, test scope and the mandatory raw oracle result: ArrayUtil body
similarity 0.19, provisional normalized logic zero, and incorrect primitive-overload
pairing. Primitive array classes/overloads, complete map/view/ancestor algorithms,
IR attributes, real owner binding and suspension scopes remain unfinished. Continue
that exact source dependency order; no further convenience adapter is a substitute.
Pinned runtime and bare-metal shared-frame acceptance remain required.

## Current Native collection dependency work

Native map/set and mutable collection/iterator contracts now exist with source KDoc
and provenance. Strict checks compile typed views and mutation calls with primitive
and actual parameter descriptor types. This closes an interface prerequisite for
attribute snapshots; IrElementBase/IrAttribute and the Native backing map algorithms
remain untranslated. There is no attribute mutation/copy execution acceptance yet.

Translate the complete Native HashMap, builder/empty-map functions and view/iterator
ancestors next, then attribute dense-pair storage and copy policy. Keep declaration
identity/scopes as the first priority. The [current dependency checkpoint](../audits/IR_IDENTITY_DEPENDENCIES.md#native-collection-prerequisite-for-ir-attributes-2026-10-06)
records the exact checks, raw oracle zero/missing findings and remaining source order.
No compiler interface result establishes Native object layout or bare-metal handoffs.

## Current IR declaration and visitor work

The eight actual root/declaration contracts and all 89 IrVisitor methods are now
written with pinned source ranges. Strict checks verify the actual abstract types
and signatures; all 88 default routes retain source order and forwarding. The
C++ virtual-template dispatch body builds in three compiler targets. Seven focused
tests recorded zero failures. Concrete IR traversal, attributes, transformer
behavior, star-projection variance, real owner binding and scopes remain required.

The [dependency ledger](../audits/IR_IDENTITY_DEPENDENCIES.md#ir-declaration-and-visitor-checkpoint-2026-10-06)
records the detailed next implementation order, raw oracle identity/emission gaps,
and exact pinned Native build diagnostic. Host Native evidence does not complete
bare-metal acceptance. Shared state machines, spill fields and direct Native
handoffs remain the goal; the broad cards are unfinished.

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

## Current Native lazy dependency progress

The actual Native AtomicInt class and integer extension/update functions now have
source bodies and executable comparison evidence. Forty-two unchanged-source
Kotlin/Native observations agree with C++; the source integer ordering also agrees
in generated LLVM. This supplies primitive compiler storage for the upcoming lock,
not Native object/reference ABI. Reference operations, actual thread identity,
reentrant locking and all Lazy algorithms remain required before parameter delegate
execution. The [dependency ledger](../audits/IR_IDENTITY_DEPENDENCIES.md) preserves raw
whole-file deep gaps, source ranges, representation limits and the next source order.

## Current concrete parameter work

Four actual descriptor implementation pairs now have source bodies. Strict header
checks and the non-root body compile; root rendering, variable type updates and
parameter visibility first stop on untranslated dependencies. These are unfinished
implementations until their genuine dependency closure and real instance execution
are complete. The detailed sequence is in the [dependency ledger](../audits/IR_IDENTITY_DEPENDENCIES.md):
Native Lazy/Lock, genuine collection singletons/map, types/substitution, visibility
and full DEBUG_TEXT rendering, then real parameter execution and IR owner binding.
Eight Kotlin/Native Modality observations agree with C++; this is bounded compiler
helper evidence. Source symbol ownership, suspension scopes and shared Native frames
remain the first priority. No JVM runtime algorithm is a substitute for a Native actual.

## What this must accomplish

A read of a variable must identify the exact declaration that introduced it.
Two variables called `result` in different scopes must remain different objects.
A suspension scope must recognize its own ID variable and return that point's
actual LLVM block address. It must send every other read to the enclosing scope.
Neither a string name nor an integer ID can substitute for those declarations.

The intended path is:

`IrGetValue -> symbol.owner -> current CodeContext -> matching declaration or outer context -> actual LLVM value`

State-machine construction must then use those same declarations, symbols,
types and fields when capturing arguments and rewriting reads/writes. The goal
is the source algorithm operating on translated IR, not a new class with a
Kotlin name around the current callback-based emitter.

There are two connected dependency paths. Declaration/symbol identity feeds
state-machine construction and its captured-field rewrites. The same identity
also feeds `VariableManager`, `CodeContext` and suspension scopes during
IR-to-LLVM generation. `CodeContext` is a code-generation scope, not a
replacement for Clang's parsing scopes or Kotlin's lowering context.

## Current evidence and source identity

The C++ checkout is `solace/sharing-transliteration`, HEAD
`727c198a1fa4229360f74660660e91852b2c028c`, with extensive additional dirty work.
The current `IrToBitcode_coroutines` helper has a resume-point vector, but no
translated `CodeContext`, `InnerScopeImpl` or `SuspensionPointScope` chain. It
passes a block address directly to an emitter callback and discards the numeric
resume-point ID. The source also uses a block address for the actual read; the
missing scope/declaration structure is the gap, not evidence that this numeric
ID must become a runtime state number.

Kotlin source authority is commit
`fee29910d8dddd2b1f7b44036c00533cee493351` in `tmp/kotlin`. Several IR tree files
are tracked in that repository but absent from its sparse working tree. They
were read from that exact commit with `git show HEAD:<path>`. The planning
receipt at `build/ir-recovery/kanban-ir-plan/kotlin-ir-source-manifest.json`
records paths, line counts, content hashes and materialization state for the
14 investigated IR files. No fetched revision or invented declaration is used.

Before implementation, inventory dirty upstream files and materialize only
required missing tracked files from this same revision, preserving existing
files and sparse-checkout configuration. Record the exact source closure and
hashes. Deep-report denominators must disclose any expanded source inventory;
a newly visible source file is not a porting improvement.

Implementation dependency findings and materialization receipts are tracked in
[the IR identity dependency ledger](../audits/IR_IDENTITY_DEPENDENCIES.md).
The initial `Name` prerequisite has executable evidence; declarations, symbol
binding and scopes remain unfinished there.

## Kotlin functions and dependencies to translate

The paths in the first four rows are relative to
`compiler/ir/ir.tree/` in the Kotlin checkout. The remaining backend paths are
relative to `kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/`.

| Source | Required translation |
|---|---|
| `src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt` and `symbols/impl/IrSymbolImpl.kt` | Bound/unbound symbols, owner access, one-time binding, signatures and descriptor behavior. Preserve the actual source failure conditions. |
| `gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:138-168` and `gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:66-73` | `IrValueSymbol`, `IrValueParameterSymbol`, `IrVariableSymbol` and concrete implementations with their real inheritance. |
| `gen/org/jetbrains/kotlin/ir/declarations/{IrValueDeclaration,IrVariable,IrValueParameter}.kt`, `declarations/impl/IrVariableImpl.kt` | Declaration/symbol relationship, types, variable flags, initializer/default bodies, constructor binding and visitor/transform behavior. |
| `gen/org/jetbrains/kotlin/ir/expressions/{IrGetValue,IrSetValue,IrSuspendableExpression,IrSuspensionPoint}.kt` and corresponding `impl` files | Value-access symbols, the suspension ID declaration, normal/resume expressions, offsets, types and source child traversal order. |
| `llvm/IrToBitcode.kt:123-209,264-326` | `CodeContext`, top-level context, `InnerScope`, `InnerScopeImpl`, `using` and their delegation/lifecycle behavior. |
| `llvm/VariableManager.kt:20-148` | Records, declaration-to-index map, variable/parameter allocation, immutable values, loads/stores, addresses and clearing. Required debug helpers are in the same source file. |
| `llvm/IrToBitcode.kt:546-603,1258-1278` | Variable/parameter scope lookup and `evaluateGetValue`/`evaluateSetValue`. |
| `llvm/IrToBitcode.kt:895-939,2281-2340` | Continuation block (:895-939), resume registration, suspendable expression, `SuspensionPointScope` (:2308-2317) and suspension-point evaluation. |
| `lower/NativeSuspendFunctionLowering.kt:119-170` | Real state-machine parameters, captured-argument field map, return-target rewrite and read/write rewrite by declaration identity. |
| `NativeLoweringContext.kt` | Required real context/symbol-table contracts and their upstream dependencies; do not alias it to a Clang context. |

Read complete functions/classes and their comments before each edit. Follow
imports and base classes: `IrElement`, declaration/parent/origin/name interfaces,
`IrType`, descriptors/signatures, visitors/transformers, parameter kinds,
`IrValueAccessExpression`, constructors and factory dependencies. This table is
an entry list, not permission to discard members whose dependencies are larger.

## Representation decisions

- Preserve Kotlin namespaces and type names. Methods lower to `snake_case`.
  A C++ keyword such as Kotlin's `using` needs an explicitly documented escape
  (`using_`), not a second convenience API.
- Declaration identity is stable object identity. The compiler's translated
  factory/module ownership must keep declarations alive throughout lowering
  and code generation. Use stable heap allocation; relocating a declaration
  must not silently change its symbol owner or lookup identity. Symbol owners
  and parent links refer back to the actual objects. Document C++ ownership at
  these boundaries and preserve existing unrelated ownership contracts.
- Keep real symbol objects separate from declarations. Unbound owner access and
  rebinding retain the source errors. Source-defined errors are behavior;
  temporary proof checks and unsupported-operation substitutes are prohibited.
- Preserve type, offsets, origin, parameter kind, initializer/default body,
  attribute owner and source visitor behavior. `IrVariable` cannot be an alias
  for a Clang variable or for `LLVMValueRef`. Public/source generic contracts can
  require templates; do not invent unrelated internal generic frameworks.
- Clang's boundary maps its resolved declarations to these actual IR objects.
  Use declaration identity, including canonical identity for genuine
  redeclarations; never match locals by spelling, source line or field index.
  Captured-parameter maps use the actual `IrValueParameter` as their key.
- LLVM functions/basic blocks remain compiler-owned LLVM objects. IR objects
  refer to them only at code-generation boundaries. Runtime frame ownership and
  Kotlin/Native GC compatibility remain explicit downstream requirements.

## Implementation sequence and acceptance

### 1. Establish the complete source dependency ledger

Identify every prerequisite of the listed classes and operations, map it to a
C++ target and mark it present, untranslated or source-blocked. Open existing
files before changing them. Use the existing compiler reference root
`src/kotlinx/coroutines/tools/kotlinc_native_ref/`; new source-backed IR files
should mirror the Kotlin package tree below it, with co-located `.hpp`/`.cpp`.
Search for each counterpart before creating it. Record final paths in the ledger.

Translate prerequisite classes rather than creating empty classes or missing
method bodies to satisfy compilation. Source abstract interfaces may remain
abstract; concrete methods must implement their source behavior. Missing
upstream files are retrieved from the pinned local repository; unresolved
source dependencies remain explicit audit findings, never fake implementations.

Exit evidence: exact Kotlin path/ranges, dependency edges and intended target
files for each complete unit. There is no dependency-closure size estimate yet;
the ledger determines it, without making the entire Kotlin compiler the scope.

### 2. Port symbols, declarations and IR expression nodes

Translate symbols and their bases, then declarations/constructors and expression
nodes/visitors in dependency order. Preserve constructor binding and source
transform child order. Port required descriptor/signature/type dependencies;
null or empty-return substitutes for these APIs are not acceptable.

Acceptance uses real translated objects: distinct same-name declarations remain
distinct; a bound symbol resolves its exact owner; unbound reads and rebinding
follow Kotlin's errors; child transformations preserve or deliberately replace
the correct symbols; parameter/default/initializer metadata survives. These
checks must not rely on a test-only stand-in declaration hierarchy.

### 3. Port ordinary value storage and scope delegation

Translate `VariableManager` records and identity-index map, then the required
`FunctionGenerationContext` allocation/load/store/reference and debug operations.
`SlotRecord` uses source `loadSlot`/`storeAny`; object references need the source
root/reference-update dependencies. A plain LLVM load/store is insufficient for
an object-reference record. Translate those dependencies before accepting it.
`ValueRecord`'s immutable-write/no-address errors are actual source behavior.

Port the full `CodeContext` contract and enclosing scopes needed by the consumed
operations. `InnerScope` delegates methods it does not override to its stored
outer context; `InnerScopeImpl` captures the visitor's current context. This
source delegation is required, not an excuse for unrelated forwarding aliases.
Implement `using_` in the same order: replace context, enter, execute, wrap a
body exception, exit, restore. Kotlin calls `onEnter` before its `try`, and
`onExit` before restoration in `finally`; preserve those details rather than
invent stronger cleanup guarantees if a hook throws. Reproduce source exception
representation through `std::exception_ptr` where required.

Translate VariableScope/ParameterScope and expression read/write visitors.
A missing local index delegates outward; a found record loads with the supplied
result slot. `evaluateSetValue` evaluates its expression, resolves the declared
index, stores and produces Unit in source order. No name search or synthetic
catch-all LLVM value is allowed.

Acceptance: same-name locals and parameters, mutable and immutable records,
object-reference/result-slot behavior, nested lookup and context restoration
on normal/body-exception paths; source hook-failure ordering; no added mutex,
new lifetime protocol or simplified GC claim.

Keep non-public scope implementations in `.cpp`. If the source generic `using`
helper requires a C++ template, provide explicit instantiations for every actual
result type or place the definition where its consumers can instantiate it;
do not leave a template declaration whose body is unavailable at use sites.

### 4. Port suspension scopes through the real visitor

`SuspendableExpressionScope` owns the ordered resume list and returns the
pre-append index. `SuspensionPointScope` retains all three source members:
its `IrVariable`, LLVM resume block and numeric resume index. Its read override
returns `block_address(bb_resume)` only for that exact declaration. All other
reads delegate outward, including reads of an enclosing suspension point.

Move the translated source algorithms into the actual `CodeGeneratorVisitor`
structure and implement their real expression dispatch dependencies. Construct
resume blocks, enter scopes with `using_`, evaluate normal/resume expressions,
merge with the continuation block and preserve `IrType`, Unit, result slots and
locations. Replace current callback-supplied identity with actual declaration
resolution at production call sites. Do not keep a renamed callback facade as
proof that the visitor was translated. Preserve mandatory LLVM address injection
by making the LLVM plugin the production owner of saved-address stores and
entry/resume dispatch. Integrate the corresponding translated visitor routines
into that owner; do not add a second frontend dispatch or a production route
that bypasses injection. The standalone diagnostic driver uses the same engine.
The frontend-to-codegen binding must retain the exact IR declarations and actual
LLVM function/block identities for the owning compilation. Its ownership and
lifetime mapping belong in the dependency ledger before integration edits;
names, serialized IR or unowned opaque pointers cannot replace that contract.

Acceptance: two same-name ID declarations select distinct real LLVM addresses;
a nested point can resolve the enclosing point through delegation; ordinary
locals still resolve correctly; two functions can reuse tooling IDs without
sharing destinations. Verify LLVM modules, resume-list order, normal/resume
merges and Unit/failure behavior. Execute both points with independent immediate
and suspended outcomes. A completed frame's retained address remains distinct
from completion state, as the IR specification now explains.

### 5. Connect identity to state-machine construction

Feed the same IR declarations into the source `buildStateMachine` mapping and
its return/get/set visitors. Captured parameters become their corresponding
fields; shadowing locals do not. Return targets change only when they target
the transformed source function. Preserve parent repair, initial failure check,
Unit return and `IrSuspendableExpression` construction. Translate required
function/field/context/builder dependencies rather than approximate them with
source text substitutions. Clang integration must consume this machinery during
ordinary compilation; a test-only visitor is not production acceptance.

Acceptance: original Kotlin-derived IR and the translated IR agree on declaration
bindings, captured fields, targeted returns and suspension points. CMake/plugin
execution must exercise this path. Retain receiver-before-value and existing
retained-local/exception regressions while removing replaced custom behavior.

## Verification and required oracle evidence

Extend the existing `kxs_codegen_test` and plugin/IR suites where they cover the
new consumers. Tests must detect wrong declaration selection, incorrect scope
restoration or wrong block routing; compile-only tests are insufficient for
those behaviors. Keep checks/traces in tests. Verify source-derived failure
paths without injecting temporary production guards.

Build relevant existing targets in `build/ir-recovery`: `KotlinxSuspendPlugin`,
`KotlinxCoroutinePass`, `kxs-inject`, `kxs_codegen_test`. Run the affected
`test_kxs_inject`, `test_kxs_compiler_pass`, `test_ir_pipeline`,
`kxs_analyzer_liveness`, `kxs_tail_collector`, `kxs_plugin_handoff` and
`kxs_kotlin_native_handoff` tests according to the consumers changed. Record
exact terminal results and source/tool/compiler identities. Broaden to the
configured Debug/optimized ASan runtime gates when integrated runtime changes
justify it. Existing bridge execution does not certify shared frame layout.

After relevant source/tool changes, run the exact required CLI over both full
available source universes from separate report directories:

```bash
# From the repository root, with these directories already present:
root="$PWD"
(cd docs/audits/project-wide/compiler && "$root/tools/ast_distance/ast_distance" --deep "$root/tmp/kotlin" kotlin "$root/src" cpp)
(cd docs/audits/project-wide/library && "$root/tools/ast_distance/ast_distance" --deep "$root/tmp/kotlinx.coroutines" kotlin "$root/src" cpp)
```

Retain generated inventories, criteria, priorities and transliteration evidence.
Record materialized source coverage and exact source commit. Inspect unsupported
emission, parser and owner-matching limitations explicitly; do not hide them,
waive the tool, or change production code to flatter its score. Tool work is
limited to demonstrated measurement defects affecting this source translation.
A better score without translated source functions is not this milestone's work.

## What completion of this priority means

The required real IR types, symbol binding, ordinary variable lookup, scope
chain and suspension-declaration lookup are translated and used by the
production compiler path. Captured-parameter and return rewrites consume those
identities. Every translated unit has file/function `Transliterated from:`
ranges, `port-lint` provenance, translated comments and updated audit references.
There are no stubs, placeholders, cheap aliases or production fallbacks.

Liveness/spill transformations, complete native cleanup and actual shared
Kotlin/C++ frame handoffs follow this priority. They stay open on the compiler
card until their own source translation and execution evidence exists. Finishing
identity/scopes does not finish the compiler, docking ring or project.
