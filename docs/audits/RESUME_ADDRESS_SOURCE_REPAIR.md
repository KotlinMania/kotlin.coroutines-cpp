# Compiler resume-address source repair — 2026-10-08


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
