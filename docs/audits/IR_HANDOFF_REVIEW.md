# IR and coroutine handoff review

## Historical review and subsequent correction

The review below records the earlier marker-cleanup implementation. Its claim
that removing injection preserved the docking-ring goal was incorrect. The
current recovery restores LLVM address dispatch and persistent frame-label
stores; cleanup-only transformation and runtime marker definitions are removed.
The current contract is specified in
`docs/suspension/IR_SUSPEND_LOWERING_SPEC.md`. Historical validation below does
not establish current status or complete Kotlin/Native interoperability.

Reviewed the clean checkout at `5e9895a100cfdd7b26d2683d39650df706890a3e`,
including Iris's `cd7a14af` standalone transform and `5e9895a1` wrapper/spec
changes. The docking-ring goal is preserved: Kotlin/Native address dispatch
plus retained live state and exact immediate/suspended/resumed result handling.

## Findings repaired

1. **Native tool guessed the frame layout.** `kxs_inject.cpp` treated argument 0
   as the label field, moved entry instructions into a new block, and added
   resume edges without spill or Result lowering. A polymorphic frame's offset
   zero can hold its vtable; ordinary marked functions may have no arguments.
   The tool now verifies the module and removes direct no-op markers only.
2. **CMake compiled both source and transformed object.** The old helper added
   `.kxs.o` to the existing sources, dropped definitions/standard/include flags,
   and used basename-only paths. A compiler launcher now wraps the real command
   and retains CMake's object names, settings, generated sources and dependency
   graph. LLVM optimization/instrumentation runs after cleanup, once.
3. **Text chunking was not LLVM parsing.** Semicolon escaping and regex `define`
   boundaries could not establish a function's frame, dispatch, liveness or
   validity. Cleanup now recognizes complete canonical marker instructions,
   preserves all other bytes, and retains declarations and unsupported uses.
   Quoted/multiline strings, unwind edges, metadata and function-pointer uses
   have explicit regressions.
4. **Documentation claimed nonexistent spill generation and full ABI parity.**
   The IR spec, suspend guide, docking ring and porting north star now distinguish
   the tested cleanup/macro contracts from compiler-driven automatic spilling,
   GC/ownership interop and the experimental DSL generator. Examples route
   suspended operations back to the current frame instead of bypassing it.
5. **Installed LLVM was ignored.** CMake now prefers an installed LLVM package
   and keeps vendored sources as a fallback. The monolithic LLVM target avoids
   duplicate component-library link warnings. The no-op marker is `noexcept`.
6. **Package loading executed the standalone script without inputs.** The
   package config no longer includes the `cmake -P` entry point at import time.
   Helpers are installed beside the module and headers use their actual source
   layout. The relocated-module regression checks paths with spaces.

## Ground truth

- Kotlin compiler checkout: `tmp/kotlin`, commit
  `fee29910d8dddd2b1f7b44036c00533cee493351`.
- Compiler build metadata: Kotlin/Native `2.5.0-dev-5907`, LLVM 21.
- Separate vendored library: `tmp/kotlinx.coroutines/gradle.properties` says
  `1.10.2-SNAPSHOT`, Kotlin build version `2.1.0`. It is not a separate Git
  checkout. This sparse compiler tree does not establish the compiler
  distribution's bundled kotlinx.coroutines artifact version.
- `NativeSuspendFunctionLowering.kt:253-335`: save state/address, normal call
  value or sentinel, resume with restore plus `getOrThrow`.
- `CoroutinesVarSpillingLowering.kt:68-105`: replace spill intrinsics with fields.
- `IrToBitcode.kt:2289-2348`: per-function blockaddress/indirectbr and merge paths.
- Native runtime `ContinuationImpl.kt:21-45`: stop on suspension, propagate a
  completed value or failure to the parent, and release interception on exit.
- `SafeContinuationNative.kt` and library `CancellableContinuationImpl.kt`:
  atomic resume-before-return decisions are distinct from the saved label and
  result payload. IR cleanup leaves those paths untouched.

## Validation

The regression suites cover ten macro/runtime scenarios, including immediate
values, resumed values/failures, repeated suspension, exact completion counts
and independent frames. The cleanup build is compared with the ordinary build
under Debug and optimized AddressSanitizer settings. CMake checks include
source/transitive flags, generated inputs, duplicate basenames, existing
launchers, paths with spaces, and header/helper-triggered rebuilds. Tests account
for macOS Make's whole-second timestamps without changing dependency semantics.

LLVM parser tests also cover zero-argument functions, quoted coroutine names,
retained frame-field accesses, unwind edges, remaining symbol uses, invalid SSA,
wrong marker signatures, bitcode roundtrips and the actual runtime test module.
The locally installed LLVM 23.1.2 and Ninja 1.13.2 enable those tests; they do not
change the compiler snapshot's LLVM version.

Run `test_suspension_core`, `test_ir_pipeline` and `test_kxs_inject` through CTest.
The latter requires the optional native tool to be built with an available LLVM
package. Configure Homebrew LLVM with
`-DLLVM_DIR=/opt/homebrew/opt/llvm/lib/cmake/llvm`.

Applied-checkout validation passed all five selected CTests: suspension core,
IR pipeline, native tool, collect/reduce and combine/zip. The fresh core-library,
native-tool and test build completed without compiler/linker warnings. The
pipeline suite passed all eight checks (including Ninja); the native suite
passed all seven. The Python regression sources are explicitly allowed by
`.gitignore` so the general `test_*` executable rule cannot hide them.

## Remaining boundary

Automatic spill/frame generation, arbitrary suspend-expression/argument and
RAII lowering, full Kotlin/Native binary/GC ABI, and cancellation/dispatcher
parity still require compiler-driven work and upstream parity tests. These
repairs preserve existing state machines rather than claiming those phases are
complete. Xcode and C++ module/BMI workflows have not been validated.

## Historical standalone recovery evidence, 2026-10-05

This stage preceded the in-compiler plugin described below. Its writer and
launcher implementations have since been removed; the receipts are historical.

The native injector at `src/kotlinx/coroutines/tools/kxs_inject/kxs_inject.cpp:103`
now constructs LLVM dispatch and saved-address stores from explicit frame-field
and blockaddress operands. `src/kotlinx/coroutines/dsl/Suspend.hpp:83` supplies
those operands instead of implementing saved-label dispatch in source. The
plugin emits the same markers and rejects switch dispatch. Marker symbols have
no runtime implementations, so missing injection cannot silently link.

The original stack/null defect is repaired by using the supplied persistent
field. There is no injector-created label alloca or per-entry null store. The
regression resumes two independent frames across calls with intervening stack
activity, checks a nonzero-offset label field, and verifies that fresh-entry
work executes once. Mismatched fields, stack storage, invalid SSA, missing
entry markers and unsupported uses are rejected.

The writer at `src/kotlinx/coroutines/tools/kxs_inject/kxs_inject.cpp:197`
serializes exact APFloat bits using LLVM 21 syntax. Tests execute Apple Clang
output and inspect negative zero, infinities, ordinary values and signaling/
quiet NaN payloads. The serialization copy at line 270 also supplies LLVM 21's
lifetime size argument for LLVM 23 pointer-only lifetime intrinsics. Apple Clang
21 builds the core library and suspension test, and the optimized AddressSanitizer
pipeline executes successfully with this writer.

The Apple Debug build still reports LLVM 23 reader diagnostics for function-local
variables in `DICompileUnit` global-variable lists and drops that invalid debug
metadata. Executable behavior is verified above; complete debug-information
preservation across these LLVM versions is not established.

Validation receipts in `build/ir-recovery/` record zero exits for:

- Apple Clang: suspension core, native injector and CMake pipeline CTests
  (`apple-ir-ctest-final.log`, plus rebuilt core in `apple-core-final.log`).
- LLVM Clang 23: eight focused suspension, dispatcher, flow, injector and
  pipeline CTests (`ctest-final.log`).
- Plugin analyzer liveness and generated handoffs (`plugin-ctest-final.log`).
- Configuration using an explicitly supplied external injector with the
  in-tree injector build disabled (`external-configure.log`).

The mandatory `tools/ast_distance/ast_distance --deep` run used Kotlin common
sources under `tmp/` and `src/kotlinx/coroutines`, refreshing the inventory,
priority and status documents (`ast-deep-final.log`). Its current report measures
623/1059 function matches, 155/228 type matches, average function similarity
0.23, 85 matched files below 0.60, 16 missing files and 340 lint findings. Zero
reported TODO/stub files applies to the matched scope; it is not evidence that
the whole port is complete. Deep parser warnings and unsupported constructs
remain in the evidence and require investigation.

This recovery validates the restored injection contract. Automatic frame/spill
generation, non-tail plugin argument/result/lifetime lowering and direct binary
handoffs using actual Kotlin-generated state machines remain unresolved
docking-ring requirements. They must be implemented and validated separately.

## Current in-compiler lowering, 2026-10-05

`src/kotlinx/coroutines/tools/kxs_inject/KotlinxCoroutinePass.cpp:23` is a required
LLVM module pass registered at Clang pipeline start. It consumes the actual
frontend LLVM operands before optimization and sanitizer instrumentation. The
shared lowering engine is at `CoroutineInjection.cpp:76,158`; the standalone
diagnostic driver uses that same engine and LLVM's native writer.

`kxs_enable_suspend` now adds `-fpass-plugin` to the ordinary compile command.
The Python compile launcher, APFloat text rewriter, module-clone lifetime rewrite
and production intermediate-IR stage are removed. CMake preserves existing
launchers and tracks header and plugin changes. The selected compiler and LLVM
plugin package must match; in-tree and installed-package integrations check
versions rather than silently selecting another lowering route.

The local verified toolchain is Clang/LLVM 23.1.2. The plugin is not an Apple
Clang 21 binary: that compiler requires a plugin built against its LLVM package.
This does not change the Kotlin compiler snapshot's LLVM 21 requirements or
establish direct Kotlin binary handoff compatibility.

`src/tests/ir/test_llvm_pass.py` executes Debug and optimized AddressSanitizer
handoffs, inspects actual injected IR and retained debug metadata, checks a
compiler error for stack label storage, and confirms missing injection cannot
link. `test_ir_pipeline.py` verifies ordinary compile commands without serialized
IR, paths with spaces, source/transitive settings, generated sources, duplicate
basenames, existing launchers, and header/plugin-triggered rebuilds under Make
and Ninja Multi-Config.

`build/ir-recovery/in-compiler-ctest.log` records nine CTests with zero exits,
including the new five-case compiler regression, CMake pipeline and affected
suspension/flow tests. `native-writer-ctest.log` records the diagnostic driver
regression after removing writer adaptations. The in-process core build and
debug regression do not reproduce the separate reader's debug-metadata loss.
`compiler-plugin-final-ctest.log` records the final five compiler regressions
and six pipeline regressions, including rejection of a mismatched plugin package.
The package was installed to a path with spaces, located through `find_package`,
and used to compile and execute all ten suspension-core scenarios from an external
consumer. Receipts are `installed-plugin-configure.log`,
`installed-plugin-build.log` and `installed-plugin-runtime.log`. This also repaired
the installed convenience module's relative include lookup.
`in-compiler-ast-deep.log` records the required refreshed deep analysis; generated
inventories and priority documents remain the oracle for port-wide gaps.

Automatic frame/spill generation, arbitrary suspend-expression and RAII lowering,
and direct Kotlin-generated state-machine binary handoffs remain required work.
The compiler-pass execution evidence establishes address injection and tested
C++ continuation handoffs, not those unimplemented requirements.

### Retained-local lowering evidence

`NativeSuspendLowering.cpp` now generates concrete retained argument and local
fields, ordered suspend-call arguments, separate immediate/resumed results, and
cleanup on completion or failure. Its implementation references
`tmp/kotlin/compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractSuspendFunctionsLowering.kt`,
`tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt`,
and `CoroutinesVarSpillingLowering.kt` in that same native lowering directory.

`src/tests/ir/test_suspend_plugin.py` executes the generated code using the
required LLVM plugin. The retained-local fixture runs three iterations with
immediate and suspended results and checks the final value, single completion,
object destruction and frame release. It also checks immediate and resumed
failures. The registered `kxs_plugin_handoff` test completed with exit zero on
2026-10-05, covering five existing direct/tail cases and four retained-local cases.
`build/ir-recovery/native-lowering-ast-deep.log` records the refreshed required
deep analysis against `tmp/kotlinx.coroutines` and the C++ coroutine source root.

The initial retained-local evidence used a diagnostic sidecar. The following
compiler integration and actual Kotlin regression supersede that limitation.

### Ordinary frontend compilation and actual Kotlin frames

`CompilerFrameLowering.cpp` parses a synthesized frame in memory using a copy of
the original compiler invocation. It imports AST nodes while reusing existing
header declarations, replaces the annotated entry before code generation, and
emits imported template definitions. A distinct virtual source buffer preserves
the generated nodes' source ownership. No generated file, subprocess compiler,
compile launcher or serialized LLVM module is used by production compilation.
An explicit `out-dir` argument selects diagnostic extraction only.

`kxs_enable_suspend_dsl(target)` enables this frontend and the required LLVM
plugin in ordinary CMake compilation. `kxs_plugin_handoff` compiles its original
annotated source through that helper. Its move-only call argument checks that
argument temporaries are destroyed after the callee returns while live locals
remain retained across suspension.

`test_kotlin_native_handoff.py` uses Kotlin/Native 2.4.20 and Clang/LLVM 23.1.2
locally. It compiles a real Kotlin suspend function with a string and accumulator,
retains the generated `invokeSuspend` LLVM IR, and requires saved `blockaddress`
and `indirectbr` operations. Kotlin exports a stable reference to the actual
generated continuation; C++ owns each handle until one resume and Kotlin disposes
the reference before resuming that same continuation. The test checks identity
across three suspension points. It does not reconstruct or copy a Kotlin frame.

The generated C++ caller retains its own string, calls Kotlin through the
existing cancellable-continuation decision state, and receives the final result
through its original continuation. The receiving C++ code owns and deletes its
result box. Immediate success, worker-thread success, failure and cancellation
are exercised under AddressSanitizer, with single completion, zero outstanding
interop handles and released C++ frames.

`in-compiler-frame-handoff-ctest.log` records the registered frontend and Kotlin
regressions. `compiler-frame-ast-deep.log` records the required refreshed oracle.
The library's matched-file analysis does not measure full Kotlin compiler
algorithm parity; unsupported parser cases and compiler-specific gaps remain
explicit in the evidence.

The focused compiler comparison initially encountered a namespace-extraction
defect: external Clang type forwards in the C++ header were counted as
implementation namespaces. `imports.hpp` now distinguishes those forwards while
preserving identity for forward-only units and rejecting foreign definitions.
`ast-namespace-ctest.log` records the identity regression. The corrected focused
comparison in `compiler-lowering-distance.log` still rejects the real package
difference between `org.jetbrains.kotlin.backend.konan.lower` and
`kotlinx::suspend`; it supplies no compiler algorithm parity score. That
adaptation needs explicit comparison support rather than weakened identity rules.
The canonical refreshed library report uses `kotlinx-coroutines-core/common/src`
as its source root. The additional whole-upstream analysis, including other
modules and build scripts, is retained separately under
`build/ir-recovery/full-upstream-analysis` and is not mixed into that scope's metrics.

Namespace entries now preserve their declaration context when building and
importing a frame. The retained-local fixture executes inside nested namespaces
with an inline namespace through ordinary CMake compilation. Both frontend and
actual Kotlin regressions completed with exit zero; the receipt is
`build/ir-recovery/namespace-frame-ctest.log`.

Constant complete array locals now use aligned retained storage. Their native
array types, multidimensional indexing, `sizeof` and reference binding are
preserved. Construction occurs at the original declaration; partial-construction
failure lets Clang destroy constructed elements and leaves the retained slot
unengaged. Completed arrays are destroyed on scope exit or failure. Reference
slots borrow concrete typed pointers; frames cannot be copied or moved.
The retained-local regression checks non-copyable object arrays, repeated
suspension, mutation through array references and partial-construction failure
under AddressSanitizer and UndefinedBehaviorSanitizer. The receipt is
`array-frame-ctest.log`; `array-frame-ast-deep.log` records the refreshed oracle.
Variable-length arrays are rejected with no output object. Macro source ranges
are recovered at their invocation so assertions can reference retained locals.

Member/template entry rewriting, suspension inside
exception handlers, additional control flow, general temporary lifetime handling,
arbitrary Kotlin object payloads and Kotlin dispatcher/Job integration still
require implementation and broader parity tests. The verified C boundary retains
actual Kotlin frames; it does not make Kotlin-GC and C++ object layouts interchangeable.

The frontend now lowers `do` loops with body-before-condition ordering and
scope cleanup on `continue` and `break`. Generated conditional branches enclose
all emitted statements, fixing unbraced branches whose cleanup and jump had
escaped the condition. The retained-local regression adds immediate and deferred
suspension in loop conditions, destruction before suspension, and early exits
from bodies containing suspension. `do-loop-ctest.log` records execution under
AddressSanitizer and UndefinedBehaviorSanitizer.

Two lowered entries in one translation unit exposed reused template declarations
without instantiated bodies. Header identity now includes the containing record
type. The importer collects referenced functions and imports missing bodies into
reused declarations, binding their parameters before notifying code generation.
The regression retains both entries in the same translation unit.
The actual Kotlin handoff regression also completed with exit zero; its receipt
is `do-loop-native-ctest.log`. `do-loop-ast-deep.log` records the refreshed required
library oracle. The compiler-specific comparison limitation described above
remains unresolved.

Structured source-switch lowering now retains initializer and condition
variables and emits case/default bodies in source order. Exit targets distinguish
switches from loops so break destroys the current switch body's locals and
continue destroys all intervening switch scopes before continuing its enclosing
loop. The retained-local fixture checks grouped cases and fallthrough after
suspension, suspension in the condition declaration, initialized object
lifetimes, immediate condition failure, and resumed case failure. Coroutine
resume dispatch continues through mandatory native LLVM address injection.
`switch-frame-ctest.log` records both the sanitizer-backed C++ regression and
actual Kotlin-generated handoff regression completing with exit zero.
`switch-frame-ast-deep.log` records the refreshed required library oracle.

Range-based loop lowering now retains implicit range and iterator declarations
and element references. Directly lifetime-extended temporary ranges construct
in aligned storage without requiring copying or moving. Iteration cleanup
occurs before continue/increment and on break; range cleanup occurs on scope
exit or failure. The regression uses mutable array elements and a non-copyable,
non-movable temporary range, with immediate/deferred results and resumed failure.
Suspending implicit iterator operations are rejected explicitly. Nested
temporary/subobject lifetimes and initializer-list backing storage still need
implementation and dedicated evidence.

The new regression exposed imported attributed types referencing storage in
previously destroyed helper ASTs. Helper AST ownership is now registered with
the main AST context and lasts for the complete compilation. The generated
frame remains imported into the owning compiler and uses mandatory native LLVM
injection. `range-frame-ctest.log` records the sanitizer-backed C++ execution.
The actual Kotlin-generated handoff regression completed with exit zero in
`range-native-ctest.log`. `range-frame-ast-deep.log` records the refreshed required
library oracle; the compiler-specific comparison limitation remains unresolved.

If condition declarations and initialization now have a distinct retained
scope enclosing the selected branch. While condition objects are reconstructed
and destroyed per iteration, including false conditions and early exits. For
condition objects survive the increment, including after continue, while body
locals are destroyed before that increment. The regression checks non-copyable
condition objects, both if branches, loop break/continue, false conditions,
reference arguments through a suspended increment and failure during that
increment. The generated continuation continues using mandatory address
injection; the condition scopes do not introduce runtime resume dispatch.
`condition-scope-ctest.log` records both the sanitizer-backed C++ regression
and actual Kotlin-generated handoff regression completing with exit zero.
`condition-scope-ast-deep.log` records the refreshed required library oracle.

Built-in comma lowering now executes discarded left operands before suspension
in right operands. Nested expressions preserve source sequencing and the final
lvalue category. Discarded non-copyable record temporaries use retained storage
and remain alive across suspension, then are destroyed in reverse order at the
enclosing full-expression boundary. Conditions first retain their evaluated
control value; returns first evaluate the outcome and then destroy temporaries
before locals. The fixture checks observed side-effect order at every external
call, retained reference identity, temporary lifetimes, condition/return
boundaries, and immediate/resumed failure. This does not establish general
temporary or overloaded-operator parity.
`comma-order-ctest.log` records the sanitizer-backed C++ regression and actual
Kotlin-generated handoff regression completing with exit zero.
`comma-order-ast-deep.log` records the refreshed required library oracle.

Built-in assignments now lower both sides, retaining the evaluated right-hand
value before a destination that suspends. Plain and compound assignments keep
their lvalue category, including reference binding to the assigned array
element. Built-in subscript lowering retains operands in source order rather
than canonical pointer/index order; reversed `index[pointer]` syntax is covered.
The regression observes side effects at each suspension and verifies retained
right-hand values, reference identity, both-sided suspension, mutation and
cleanup after destination failure. Overloaded assignment operators remain
separate work.
`assignment-order-ctest.log` records the sanitizer-backed C++ regression and
actual Kotlin-generated handoff regression completing with exit zero.
`assignment-order-ast-deep.log` records the refreshed required library oracle.

Conditional glvalue lowering now retains the selected object by reference.
Lvalues preserve identity and xvalues preserve move eligibility across
suspension. Canonical retained reference types fix standard-library aliases
whose original unqualified spelling was invalid in the generated namespace.
Both branches are verified with non-copyable tracked objects and move-only
pointers, including mutation through the selected reference, empty moved
sources, immediate/resumed failure and frame destruction. Bitfields and broader
prvalue temporary/overloaded-operator rules remain outside this evidence.
`selection-move-only-ctest.log` records the sanitizer-backed C++ regression;
`selection-ctest.log` also records the actual Kotlin-generated handoff regression.
Both completed with exit zero. `selection-ast-deep.log` records the refreshed
required library oracle.

Void conditional expressions now execute their selected arm without allocating
a result slot. The retained-selection regression adds both void branches,
observed branch side effects, a discarded non-copyable temporary live during
suspension, and cleanup on resumed failure in that branch. Unselected arms have
no runtime effect. The continuation still resumes through mandatory LLVM
address injection.
`void-selection-ctest.log` records the sanitizer-backed C++ regression and actual
Kotlin-generated handoff regression completing with exit zero.
`void-selection-ast-deep.log` records the refreshed required library oracle.

By-value parameters now transfer into retained fields using moves, including
const move-only source parameters. Stored fields preserve source constness;
lvalue and rvalue reference fields preserve borrowed caller-object identity.
The regression verifies ownership transfer, non-copyable borrowing, mutation
after resume and destruction on immediate/resumed failure. It now compiles
with sanitizer recovery disabled so sanitizer errors terminate execution.

The new parameter case exposed missing imported move-constructor/deleter bodies
and missing constructor member initializers. Referenced definitions are now
followed transitively, including constructor/destructor dependencies and
initializer expressions. Initializers are imported alongside constructor bodies.
The stricter regression also exposed collisions between template-local
declarations: their identity now includes the owning function signature and
class specialization. This avoids reusing locals from another instantiation.
`parameter-frame-ctest.log` records the sanitizer-backed C++ regression with
recovery disabled; `parameter-native-ctest.log` records the actual Kotlin-generated
handoff regression. Both completed with exit zero. `parameter-frame-ast-deep.log`
records the refreshed required library oracle.

Member-call receivers now evaluate before suspended arguments. Object references
retain identity; pointer receivers retain their evaluated pointer; non-copyable
temporary receivers remain owned by the frame until the full expression ends.
Lvalue/rvalue qualification is preserved. The receiver regression also suspends
inside a directly forwarding annotated member call and checks temporary cleanup
on immediate and resumed success/failure. The subsequent member-frame work below
adds non-tail member entry rewriting for definitions in the current source file.

Direct-entry recognition follows the no-state-machine tail-call distinction in
`tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:55-69`.
The C++ recognizer currently accepts the single-return tail-call shape, including
members, and keeps the original entry in production compilation.
`build/ir-recovery/receiver-frame-ctest.log` records the sanitizer-backed C++
regression; `receiver-native-ctest.log` records the actual Kotlin-generated handoff
regression; `receiver-frame-ast-deep.log` records the refreshed library oracle.
The library oracle does not measure compiler-lowering parity across the distinct
Kotlin compiler and C++ plugin packages.

Non-tail member definitions now retain their typed receiver alongside arguments,
following `AbstractSuspendFunctionsLowering.kt:91-118,169-196`. The generated frame
is a local class in the member's lexical scope, preserving private access. Its
receiver is borrowed; caller ownership is unchanged. Rewritten implicit members
and explicit `this` use that pointer after suspension. Regression cases exercise
lvalue, const-lvalue and rvalue overloads, private calls with suspended arguments,
repeated resume, a temporary receiver retained by an outer frame, and failure
cleanup. The importer now registers member definitions against existing header
prototypes and follows frame dependencies rather than all main-file functions,
preventing early installation of unrelated later definitions.

Evidence is in `build/ir-recovery/member-frame-ctest.log`,
`member-native-ctest.log` and `member-frame-ast-deep.log`. Header-defined and template
entry rewriting, exception-region suspension, broader temporary ownership,
arbitrary Kotlin payload GC boundaries and dispatcher/Job integration still need
implementation or verification; this change does not establish full parity.

Ordinary included header definitions now lower through an in-memory header
replacement in the original including context. Parsing stops the main-file
context after the relevant include, so later main-file definitions are not
installed ahead of the owning parser. Declaration offsets after the replacement
are corrected to reuse existing header declarations. Header free functions use
local frames inside the original inline function body; generated names use stable
source-file offsets across translation units. ASTImporter dropped implicit-inline
flags on generated frame methods, which produced duplicate symbols in the
two-file regression. Those source flags are now restored before code generation.

The regression executes inline free entries from both source files and const
member entries from the shared header, including suspension and cleanup after
immediate/resumed failure. `build/ir-recovery/header-frame-ctest.log` records the
sanitizer-backed regression; `header-native-ctest.log` records the actual Kotlin
handoff regression. Both completed with exit zero. `header-frame-ast-deep.log`
records the required refreshed library oracle. Macro-defined bodies and forced
includes without a concrete main-file include remain unsupported; template
entries, exception regions and the broader ownership/runtime gaps above remain
unfinished. The library oracle still does not score compiler-lowering parity.

Command-line forced includes now use their compiler preinclude context. The
include-chain traversal distinguishes Clang's built-in/command-line locations
from ordinary main-file include directives. For preincludes, the helper main
buffer contains no original main-file declarations; the cloned invocation
retains the forced headers and preprocessor settings. This closes the previous
forced-include rejection without using a source launcher or serializing LLVM IR.
The C++ regression builds and runs all sixty-seven retained-local cases in both
ordinary and forced-include variants, including shared-header free/member entries
in two translation units. `build/ir-recovery/forced-header-ctest.log` and
`forced-header-native-ctest.log` record the sanitizer-backed C++ and actual Kotlin
handoff regressions completing with exit zero. `forced-header-ast-deep.log`
records the refreshed required library oracle. Macro-defined bodies, template
entries, exception-region suspension and the broader runtime/ownership gaps
remain unfinished.

Try/catch lowering now keeps resumed failure checks inside native protected
regions, following `NativeSuspendFunctionLowering.kt:119-176`. Typed dispatch
selects the source handler and destroys retained protected locals before the
handler runs. C++ handlers execute after leaving the native catch dispatcher;
an `exception_ptr` retains the original exception across handler suspension.
Reference catch variables preserve exception-object identity. Value catch
variables copy once into retained delayed storage, with termination if catch
initialization throws, and destruction before releasing the original exception.
Direct `std::current_exception()` expressions and bare rethrows use the retained
exception after resume.

The expanded regression verifies multiple typed catches, handler suspension,
resumed handler failure, nested rethrow to a catch-all, protected-scope cleanup,
and exact copy/lifetime counts for value catches. All eighty-seven retained-local
cases execute in ordinary and forced-include builds. Evidence is recorded in
`build/ir-recovery/exception-frame-ctest.log`, `exception-native-ctest.log` and
`exception-frame-ast-deep.log`; both executable regressions completed with exit
zero and the required library oracle was refreshed. At this stage native active-catch
context was not restored for helper functions called by retained handlers.
The subsequent context-wrapper work below closes that observed gap. Template/macro entries
and broader Kotlin ownership/runtime integration also remain unfinished.

Exception-bearing frames now restore native catch context around each body
execution segment. Handler entry and exit use injected blockaddress transitions
to return to an `invoke_suspend` context wrapper; `invoke_body` retains the
mandatory LLVM begin/point markers and actual address dispatch. The wrapper
rethrows the active retained exception into a native catch before executing the
body. Nested handlers retain and restore the previous exception; handler exits
return to the wrapper before execution continues outside their context. Source
rewrites of `current_exception` and bare rethrow have been removed.

Helper functions now observe the native active exception before and after handler
suspension, and helper bare rethrow reaches an outer handler. Regression cases
also verify nested handler restoration, context clearing after a handler,
`break`/`continue` exits and failure cleanup. All ninety-five retained-local cases
execute in both ordinary and forced-include builds under the sanitizers.
`build/ir-recovery/exception-context-ctest.log` and
`exception-context-native-ctest.log` record the C++ and actual Kotlin handoff
regressions completing with exit zero; `exception-context-ast-deep.log` records
the refreshed required library oracle. Broader compiler and Kotlin runtime parity
remain unfinished; this evidence closes the specific helper-context gap.

The subsequent destructor regression reproduced a nested cleanup gap: slot
cleanup ran under one native context while destroying several handler scopes.
`build/ir-recovery/nested-cleanup-baseline.log` records the local-destructor
assertion failure. After correcting scope boundaries, an exception-object
destructor exposed a second release-context issue, recorded in
`exception-object-baseline.log`.

Cleanup now transitions between handler groups, restoring the appropriate
surrounding native context before destroying its locals. Protected storage is
destroyed outside the newly selected catch; value catch variables are constructed
after protected cleanup in their restored handler context. Source return values
are retained across cleanup transitions, following the value-before-cleanup
ordering in `FinallyBlocksLowering.kt:148-205`. Unhandled failures save their
exception and enter a distinct injected cleanup path before rethrowing. Departing
exceptions remain retained until the next surrounding context is active, then
are released before body address dispatch.

Local and exception-object destructor observers now succeed for nested normal
exit, return, break, continue and resumed failure. All one hundred seven
retained-local cases run in both ordinary and forced-include builds under address
and undefined-behavior sanitizers. `build/ir-recovery/nested-cleanup-ctest.log`
and `nested-cleanup-native-ctest.log` record the C++ and actual Kotlin-generated
handoff regressions completing with exit zero. `nested-cleanup-ast-deep.log`
records the refreshed required library oracle. This closes the reproduced
destructor-context gaps; broader compiler and Kotlin runtime parity remain open.


Generic header entries now retain their type parameters in a local coroutine
frame, following `AbstractSuspendFunctionsLowering.kt:100-141`. The frontend
maps template parameters into the owning AST and leaves dependent frame methods
for normal Clang instantiation. Constructor imports preserve both initializer
expressions and their explicit source-order flags; without those flags Clang
instantiation omitted the required continuation base initialization.

Eight additional cases exercise unique and shared ownership across immediate
completion, deferred completion, resumed failure and immediate failure. All one
hundred fifteen retained-local cases execute in both ordinary and forced-include
builds with address and undefined-behavior sanitizers.
`build/ir-recovery/template-frame-ctest.log` and
`template-frame-native-ctest.log` record exit zero for the C++ regression and
actual Kotlin-generated handoff regression. `template-frame-ast-deep.log` records
the refreshed required library oracle. This evidence covers generic entries with
resolved suspension callees; dependent suspend-call resolution, dependent local
type deduction and class/member template coverage remain unfinished. The library
oracle reports zero explicitly reviewed suspension contracts and does not prove
compiler lowering parity by itself.


The generic ownership regression now uses `auto local = std::move(value)`.
`build/ir-recovery/dependent-auto-baseline.log` reproduces the previous invalid
`std::optional<auto>` field. Dependent placeholder locals now preserve deduction
through an unevaluated generic-lambda expression; the owning Clang instantiation
resolves the concrete storage type, and initialization still executes once at
its original statement. `dependent-auto-ctest.log` records exit zero for both
ordinary and forced-include sanitizer builds; `dependent-auto-native-ctest.log`
records exit zero for the actual Kotlin-generated handoff regression.
`dependent-auto-ast-deep.log` records the refreshed required library oracle.
Reference, pointer and `decltype(auto)` placeholder forms need separate runtime
coverage; dependent suspend callees and broader runtime parity remain unfinished.


Dependent placeholder reference/pointer coverage now includes `auto&`,
`const auto&`, forwarding-reference deduction, `const auto*`, parenthesized
`decltype(auto)` references, and unparenthesized `decltype(auto)` reference and
pointer declarations. A non-copyable tracked object verifies identity and
mutation across suspension. Clearing the original pointer verifies that an
unparenthesized pointer declaration retains its value rather than borrowing the
pointer variable.

`build/ir-recovery/dependent-reference-baseline.log` reproduces a nested-storage
instantiation failure: deduction referred to an enclosing frame field before
that field was instantiated. Type-only rewriting now uses declared-type
`std::declval` expressions instead of accessing frame fields. Unparenthesized
`decltype(auto)` ids preserve their declared type; parenthesized expressions
preserve their reference category. `dependent-reference-ctest.log` records exit
zero for both sanitizer build variants, `dependent-reference-native-ctest.log`
records exit zero for actual Kotlin-generated handoffs, and
`dependent-reference-ast-deep.log` records the refreshed required library oracle.
Dependent suspend calls, class/member templates and broader runtime parity remain
unfinished.


Unresolved free-function lookup now recognizes the real DSL suspension wrapper
and overload sets whose available candidates are all annotated suspend entries.
Generic frame lowering therefore runs on the template definition rather than
waiting for a concrete instantiation. Dependent argument storage uses unevaluated
concrete-type deduction, and unresolved callee lvalue arguments retain identity
until the instantiated call applies its own parameter conversion.

The generic ownership regression now calls an annotated function template with
its non-copyable tracked object by reference. That template tail-forwards the
same continuation to the external suspension entry. Both ownership forms cover
immediate/deferred completion and immediate/resumed failure without copying the
borrowed object. `build/ir-recovery/dependent-call-baseline.log` records the earlier
late-lowering compilation failure; `dependent-call-ctest.log` records exit zero
for both sanitizer build variants. `dependent-call-native-ctest.log` records exit
zero for actual Kotlin-generated handoffs, and `dependent-call-ast-deep.log`
records the refreshed required library oracle.

The analyzer regression now uses the real DSL namespace rather than a global
same-named function. `dependent-analyzer-baseline.log` records the stale fixture
failure; `dependent-analyzer-ctest.log` records exit zero for source-ordered IDs,
dependent wrapper/callee recognition and complete 300-block liveness. Dependent
member lookup, candidate sets mixing suspend and ordinary entries, class/member
templates and broader Kotlin runtime parity remain unfinished; this evidence does
not establish every dependent call form.


Class-template receiver coverage now exercises a const-qualified member and a
member function template with both unique and shared receiver ownership. The
member template additionally owns a move-only local. Sixteen new cases verify
private member access, retained construction, one completion, receiver borrowing
and local/frame cleanup on immediate/deferred completion and immediate/resumed
failure. Both in-class and out-of-class member-template definitions execute in
ordinary and forced-include builds.

`build/ir-recovery/member-template-baseline.log` reproduces deduction through an
uninstantiated receiver field. Type-only receiver rewriting now uses the declared
receiver pointer type, while executable expressions continue using the saved
receiver. `member-template-ctest.log` and
`member-template-out-of-class-ctest.log` record exit zero for the two definition
forms under address and undefined-behavior sanitizers. The latter executes all
one hundred thirty-one retained-local cases in each build variant.
`member-template-native-ctest.log` records exit zero for actual Kotlin-generated
handoffs; `member-template-ast-deep.log` records the refreshed required library
oracle. Dependent member-call lookup, mixed annotation candidate sets, broader
expression/temporary coverage and Kotlin runtime parity remain unfinished.


Dependent member receivers now evaluate before suspended arguments. The runtime
baseline in `build/ir-recovery/dependent-member-baseline.log` reproduces argument
execution before receiver selection. Storage now resolves the receiver category
at instantiation: reference results borrow the original object; value results
construct owned delayed storage directly, preserving non-copyable/non-movable
prvalue construction. Forwarding preserves the resolved reference category, and
receiver storage remains alive through the enclosing full expression, including
suspension inside the member callee. Type-only variable references use
`add_lvalue_reference_t` to avoid turning an existing `T&` spelling into `T&&`.

Sixteen new cases check receiver-before-argument ordering, two suspension sites,
lvalue/rvalue-qualified member selection, non-copyable borrowed and owned
receivers, and immediate/resumed failures at either site. Direct annotated member
template calls additionally verify unresolved member candidate recognition
without a DSL wrapper. Available overload candidates must all carry the suspend
annotation; completely dependent member names use the explicit suspension marker.

`dependent-member-ctest.log` records exit zero for all one hundred forty-seven
retained-local cases in both ordinary and forced-include sanitizer builds.
`dependent-member-analyzer-ctest.log` records exit zero for unresolved annotated
member candidate recognition and the existing order/liveness checks.
`dependent-member-native-ctest.log` records exit zero for actual Kotlin-generated
handoffs; `dependent-member-ast-deep.log` records the refreshed required library
oracle. Mixed annotation candidate sets, unmarked fully dependent member names,
broader expression/temporary support and Kotlin runtime parity remain unfinished.


Free template entries that reach instantiation without earlier frame lowering can
now lower their selected suspend overload after overload resolution. The frontend
parses a separate concrete typed helper in memory, imports its body into the
existing specialization, and leaves the primary template and ordinary overload
specializations unchanged. The original consumer already supplies instantiated
bodies before code generation; `mixed-overload-baseline.log` reproduces the former
helper lookup failure at that stage.

Non-type integral arguments use Clang's actual substitution expressions, including
uses inside type expressions. `mixed-constant-baseline.log` records the invalid
capture of a function-local constant, and `mixed-constant-link-baseline.log`
records the subsequent unwanted runtime constant symbol. The implementation
retains the original AST expression before stripping implicit nodes and does not
substitute template parameters with new constexpr variables. The regression
checks both value and `decltype` behavior.

`mixed-instantiation-identity-baseline.log` reproduces one constant instantiation
using the other's frame body. Independently lowered concrete frame names now
include a digest of Clang's mangled function identity, including template
arguments. This separates concrete frames while keeping their names stable
across translation units. Two source files instantiate both integral constants
and both selected overloads, checking immediate/deferred completion, failure,
one completion and cleanup. The ordinary overload completes directly without
registering an external continuation in all modes.

`build/ir-recovery/mixed-overload-ctest.log` records exit zero for all one hundred
seventy-nine retained-local cases in each ordinary/forced-include sanitizer
variant. `mixed-overload-native-ctest.log` records exit zero for actual
Kotlin-generated handoffs, and `mixed-overload-ast-deep.log` records the refreshed
required library oracle. Already-lowered primary templates containing a later
mixed/unknown call, late-resolved member bodies, template packs and non-integral
non-type arguments require further integration. Broader expression/temporary
support and Kotlin runtime parity remain unfinished.

Free primary templates containing mixed annotated/ordinary overload candidates
now defer the whole body until Clang selects the overload. An earlier known
suspension no longer causes premature lowering of the primary body. The baseline
`build/ir-recovery/partial-mixed-baseline.log` reproduces an invalid deletion of
the suspension marker. Sixteen added cases exercise the earlier suspension and
later selected overload, immediate/deferred success, failure at either site,
one completion and local cleanup. `partial-mixed-ctest.log` records exit zero
for 195 retained-local cases per ordinary/forced-include sanitizer variant
(52.59 seconds). The analyzer regression checks both mixed-candidate deferral
and continued recognition of fully annotated dependent candidates.
`partial-mixed-native-ctest.log` records exit zero for real Kotlin-generated
handoffs (11.90 seconds), and `partial-mixed-ast-deep.log` records the refreshed
required deep oracle. Its syntax evidence does not establish inferred call-target
resolution or full compiler parity. Completely unknown dependent calls and
late-resolved member bodies remain unfinished.

Compiler references read in full for this work are
`tmp/kotlin/compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractSuspendFunctionsLowering.kt`,
and the native `NativeSuspendFunctionLowering.kt`, `CoroutinesLivenessAnalysis.kt`
and `CoroutinesVarSpillingLowering.kt` under
`tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/`.
They establish argument fields, native resume addresses, expression slicing,
live-variable fields and restore-before-resumed-result behavior. Reading them
does not establish that the C++ implementation covers all of those behaviors.

Free primary templates now also defer type-dependent calls with no known
candidate, including an unmarked `receiver.await(completion)` after a known
suspension. `build/ir-recovery/unknown-call-baseline.log` reproduces invalid
deletion of the suspension marker in that path. Sixteen additional runtime cases
select a suspend receiver or an ordinary receiver, exercise immediate/deferred
completion and failure at either call, and check exactly one completion and
local cleanup. The required overload distinction comes from Clang's instantiated
callee, not a runtime dispatch fallback.

The concrete import exposed a missing template definition at link time. The
importer follows the installed owning AST's transitive function references,
instantiates absent template bodies through Clang Sema, and submits concrete
referenced template definitions for code generation. Helper-AST references alone
missed a specialization called through an instantiated member.
`unknown-call-ctest.log` records exit zero for all 211 retained-local cases per
ordinary/forced-include sanitizer variant (47.23 seconds).
`unknown-call-analyzer-ctest.log` records the unknown-candidate deferral check;
`unknown-call-native-ctest.log` records exit zero for actual Kotlin-generated
handoffs (11.99 seconds). `unknown-call-ast-deep.log` records a refreshed required
deep oracle after the final compiler change. Late-resolved member definitions,
additional template argument kinds, broader expression/lifetime behavior and
Kotlin runtime parity remain unfinished.

Calls requiring argument-dependent lookup now defer free primary template
lowering even if all currently visible candidates have the same annotation.
The visible candidate set is incomplete until the argument types are known.
`build/ir-recovery/adl-baseline.log` reproduces invalid deletion of the suspension
marker when an initially ordinary candidate is superseded by a suspend overload
from the argument's namespace. Sixteen additional cases select suspend and
ordinary namespace overloads, exercise success and failure at either call,
and verify completion and cleanup.

`adl-ctest.log` records exit zero for all 227 retained-local cases per
ordinary/forced-include sanitizer variant (47.28 seconds). The analyzer test
requires deferral for pending argument-dependent lookup, including initially
fully annotated candidates. `adl-analyzer-ctest.log` records exit zero;
`adl-native-ctest.log` records actual Kotlin-generated handoffs (12.10 seconds).
`adl-ast-deep.log` records the refreshed required deep oracle. Late-resolved
member definitions and the previously recorded broader compiler/runtime gaps
remain unfinished.

Late-resolved member templates and ordinary methods of class templates now
defer lowering until their concrete calls are known. The in-memory helper parses
an explicit concrete member specialization with the owning class's lexical
context, preserving private access and const/ref qualification. The importer
attaches its body to the original instantiated method without rewriting the
primary body for other specializations. Function-template aliases are local to
that specialized body. Concrete class-method frames use canonical types and
the mangled identity for their frame names as function specializations do.

`build/ir-recovery/late-member-baseline.log` reproduces a generated constructor
parameter collision with a user parameter named `receiver`; the synthesized
receiver parameter now uses its compiler name. `late-class-baseline.log`
reproduces invalid deletion of the suspension marker in a class-template method
whose later receiver call resolves to a suspend method. Thirty-two additional
runtime cases cover member templates and class-template methods with private
state, `const &` receivers, suspend/ordinary selection, immediate/deferred
completion, failure at either site, exactly one completion and cleanup.

`late-class-ctest.log` records exit zero for all 259 retained-local cases per
ordinary/forced-include sanitizer variant (43.65 seconds), including the earlier
in-class and out-of-class member-template regressions. Actual Kotlin-generated
handoff evidence is in `late-class-native-ctest.log`, and the required refreshed
deep oracle is in `late-class-ast-deep.log`. Nested class contexts, further
exception specifications, packs and additional non-type template arguments
remain unfinished alongside the recorded broader expression/runtime gaps.

Nested member lowering now walks enclosing record scopes to reach the owning
namespaces and emits specialization prefixes for the instantiated class chain.
`build/ir-recovery/nested-member-baseline.log` reproduces rejection of a nested
class as a namespace context. Sixteen additional cases use a nested class of
an enclosing template, retaining its private type alias and integral argument
across both suspend/ordinary selections and failures at either call.

Concrete helper signatures now print the instantiated exception specification
from Clang's function type. `member-noexcept-baseline.log` reproduces the missing
`noexcept(true)` declaration mismatch. Sixteen additional cases exercise boolean
exception specifications. Immediate throwing checks use `noexcept(false)`;
deferred failures also exercise `noexcept(true)`. Separate ordinary/forced-include
processes require the terminate handler to observe exactly one external call and
exit with its expected code when an immediate exception escapes `noexcept(true)`.
An initial runtime test incorrectly expected that exception to escape without
termination; it was corrected to verify the actual C++ contract.

`member-noexcept-ctest.log` records exit zero for all 291 retained-local cases per
ordinary/forced-include sanitizer variant plus both termination checks (52.46
seconds). `member-noexcept-native-ctest.log` records actual Kotlin-generated
handoffs, and `member-noexcept-ast-deep.log` records the required refreshed deep
oracle after the compiler change. Explicitly specialized nested owners, broader
dependent exception expressions, packs and additional non-type template
arguments remain unfinished alongside the recorded expression/runtime gaps.

Pointer, reference and null non-type template arguments are now accepted by the
concrete helper. The replacement expression retains the resolved declaration's
qualified scope. Substitution casts use Clang's substituted parameter type rather
than the expression type, which loses a reference parameter's reference type.
`build/ir-recovery/symbol-argument-baseline.log` records the former rejection.
Forty-eight additional runtime cases exercise two globals, pointer/reference/null
arguments, suspend/ordinary overload selection, completion and failure, exact
`decltype`, address identity and cleanup. The referent changes while suspended;
the resumed body must observe the new value while its earlier local retains the
original value. No synthesized replacement variable holds the template argument.

`symbol-argument-ctest.log` records exit zero for all 355 retained-local cases per
ordinary/forced-include sanitizer variant and the termination checks (65.20
seconds). `symbol-argument-native-ctest.log` records actual Kotlin-generated
handoffs; `symbol-argument-ast-deep.log` records the refreshed required deep
oracle. Packs, structural values and additional declaration argument forms
remain unfinished alongside the recorded exception/expression/runtime gaps.

An explicitly specialized nested owner now ends the helper's enclosing class
specialization prefix chain. The member function template still contributes its
own prefix. `build/ir-recovery/explicit-nested-baseline.log` records Clang's
rejection of the prior extra template head. Sixteen additional runtime cases
exercise a member template inside a fully specialized nested class, private
state, suspend/ordinary receiver selection, immediate/deferred success,
failure at either call, exactly one completion and local cleanup.

`explicit-nested-ctest.log` records exit zero for 307 retained-local cases per
ordinary/forced-include sanitizer variant plus the existing termination checks
(56.04 seconds). `explicit-nested-native-ctest.log` records actual Kotlin-generated
handoffs, and `explicit-nested-ast-deep.log` records the refreshed required deep
oracle. Broader dependent exception expressions, packs and additional non-type
arguments remain unfinished alongside the recorded expression/runtime gaps.

Function-pointer and function-reference non-type arguments now recover their
deduced `auto` parameter type from Clang's actual function specialization argument.
`build/ir-recovery/function-argument-baseline.log` records the invalid
`static_cast<auto>` previously emitted in both a call and its `decltype` check.
Clang already resolves these concrete calls to the underlying annotated function;
the analyzer regression verifies one suspension site for the suspend target and
none for the ordinary target. No separate annotation resolver was needed.

Sixteen additional runtime cases exercise function pointers and references,
exact `decltype`, suspend/ordinary target selection, immediate/deferred completion
and failure, retained local ownership, exactly one completion and cleanup.
`function-argument-ctest.log` records exit zero for all 371 retained-local cases per
ordinary/forced-include sanitizer variant and termination checks (69.41 seconds).
`function-argument-analyzer-ctest.log` records the concrete target analysis;
`function-argument-native-ctest.log` records real Kotlin-generated handoffs
(12.17 seconds). `function-argument-ast-deep.log` records the refreshed required
deep oracle. Packs, structural values and remaining declaration argument forms
remain unfinished alongside the recorded exception/expression/runtime gaps.

Provenance review: `NativeSuspendLowering.cpp` now declares its primary compiler
source with `port-lint: source` before the ranged `Transliterated from` comments.
The provenance reader in `tools/ast_distance/include/porting_utils.hpp` recognizes
both forms and returns the first annotation. Function/class references now use
full paths relative to `tmp/kotlin`, including references to the native lowering
and variable-spilling sources. Ten attribution paths and range bounds were
checked against the local compiler checkout. This establishes traceability;
it does not establish algorithmic or comment parity by itself.

Removed misleading Kotlin function attributions from Clang AST integration,
implicit C++ iterator AST printing, native catch-context transitions, and C++
destructor cleanup. Their `NOTE(port)` explanations identify C++ mechanics;
these bodies must not be counted as direct Kotlin function transliterations.
`CompilerFrameLowering.cpp` is compiler integration infrastructure. The required
library deep analysis is recorded in `build/ir-recovery/provenance-ast-deep.log`;
its Kotlin input is the coroutine library, not the compiler checkout, so it
does not certify compiler-function parity. A complete function/comment parity
review and compiler-specific pairing remain required.

Class-owned `auto` non-type function arguments now recover their concrete type
from the actual enclosing Clang class specialization. Recovery requires matching
the parameter declaration and index, avoiding accidental use of another nested
template's argument. `class-auto-baseline.log` reproduces the former diagnostic
for both pointer and reference targets. Sixteen added runtime cases exercise
class-owned targets, exact `decltype`, suspend/ordinary selection, immediate and
resumed results/failures, single completion and retained-local cleanup.
`class-auto-ctest.log` records exit zero for 387 retained-local cases per ordinary
and forced-include sanitizer variant plus termination checks.
`class-auto-analyzer-ctest.log` records the analyzer checks;
`class-auto-native-ctest.log` records the real Kotlin-generated handoff checks
(12.11 seconds). `class-auto-ast-deep.log` records the required refreshed library
analysis. Packs, structural values, additional declaration forms and the other
recorded compiler/runtime gaps remain unfinished.

Added function-level full-path attribution for frame construction, typed field
creation, argument extraction and suspension emission. Corresponding Kotlin
comments are retained for argument fields, evaluation-order temporaries and late
state saving. `NOTE(port)` explains direct persistent C++ fields and lifetime
differences instead of attributing C++ ownership mechanics to Kotlin.
`build/ir-recovery/compiler-provenance-evidence.json` records all fifteen current
attribution paths, verified range bounds and local source SHA-256 fingerprints;
these are traceability evidence, not equivalence scores.

An actual `ast_distance --compare-functions` attempt against
`NativeSuspendFunctionLowering.kt` is recorded in
`build/ir-recovery/compiler-provenance-functions.log` (exit one). It reports
Kotlin parse errors at lines 60, 119 and 225 and rejects the declared package
`org.jetbrains.kotlin.backend.konan.lower` against namespace `kotlinx.suspend`.
The compiler comparison has therefore not certified parity. This tool limitation
must be resolved without falsifying namespace identity or treating the mandatory
library analysis as compiler proof.

Function-for-function measurement now supports the real C++ header/source split:
`ast_distance --compare-functions <source.kt> kotlin <target.cpp> cpp --with-companions`.
The option extracts each same-stem companion separately, retains physical file
paths and local line numbers, and uses the existing similarity and ordered-logic
metrics. It does not relax provenance, namespaces or callable-owner matching.
The CLI regression verifies a body in a header and a body in its source are both
measured, declarations are not counted as implementations, and a conflicting
header namespace is rejected. Report capture, logic-drift and identity regression
results are recorded in `build/ir-recovery/function-companions-tests.log`.

The measured common-source snapshots are recorded in
`build/ir-recovery/CancellableContinuationImpl-function-companions.log`,
`Delay-function-companions.log` and `JobSupport-function-companions.log`.
They respectively match 37/50, 5/6 and 54/91 source bodies, with source-inclusive
combined body scores 0.225, 0.253 and 0.128. Matched ordered-logic averages are
0.426, 0.487 and 0.354. These are diagnostic measurements of the current bodies,
not completion figures or evidence that acceptance criteria have been met.
The reports retain every matched function's AST/identifier/logic measurements,
line ratio/gap and ordered operation sequences, plus unmatched functions.

Physical-file-only comparison previously measured only three bodies for
`CancellableContinuationImpl.cpp`; its header contains 126 extracted bodies,
including template specializations and supporting types. That earlier scope
cannot substantiate a claim that the other Kotlin functions are absent.
Companion-aware matching still reports owner ambiguity and unmapped constructs.
In particular, `Delay`'s `to_delay_millis` body exists in its header, but the
Kotlin `Duration` extension receiver does not match the C++ chrono parameter
under current identity rules. It remains reported as unmatched, not waived.
The required regenerated deep analysis is recorded in
`build/ir-recovery/function-companions-ast-deep.log`. Compiler lowering parity
remains unverified for the previously recorded identity/parser limitations.

The compiler-derived class now retains its real Kotlin package namespace,
`org::jetbrains::kotlin::backend::konan::lower`, and the Kotlin class name
`NativeSuspendFunctionsLowering`. Its primary `port-lint` source is
`NativeSuspendFunctionLowering.kt`. The Clang entry adapter remains in
`CompilerFrameLowering.cpp`; its declaration belongs to the integration header.
This changes actual implementation identity without relaxing oracle checks.
Frame construction and `build_state_machine` are separate functions, with the
native source range and the original comment about extracting suspend calls.
The generated normal/resume and C++ exception/lifetime regions remain the same.

`build/ir-recovery/compiler-function-correspondence.log` records the first
non-rejected compiler function comparison. It matches only `buildStateMachine`
(1/33 extracted Kotlin bodies); that pair has 67 Kotlin declaration lines and
40 C++ lines, combined similarity 0.007 and ordered-logic similarity 0.056.
The source-inclusive combined score rounds to 0.000. These measurements are
provisional because the Kotlin parser reports errors at lines 60, 119 and 225
and the C++ parser reports missing type identifiers at two braced defaults.
The namespace rejection is resolved; function/algorithm parity is not.

A separate compiler-scoped required deep run is recorded in
`build/ir-recovery/compiler-function-deep.log`, with generated inventories,
ordered evidence and priorities under `build/ir-recovery/compiler-oracle/`.
It reports the missing `ExpressionSlicer` type and 32 unmatched source functions.
This is actionable evidence of incomplete compiler transliteration, not a waiver
based on parser limitations or successful runtime examples. The library oracle
was separately refreshed in `compiler-function-library-deep.log`.

`compiler-function-build.log` records the rebuilt plugin.
`compiler-function-ctest.log` records exit zero for the existing 387 retained-local
cases per sanitizer variant and termination checks (72.38 seconds).
`compiler-function-native-ctest.log` records real Kotlin-generated handoffs
(11.95 seconds). The traceability JSON was refreshed with sixteen source/range
records. Full function/comment/algorithm parity and the recorded broader
compiler/runtime requirements remain unfinished.


## Compiler correspondence: coroutine class names (2026-10-05)

The class-name construction formerly embedded in `build_coroutine` is now
`NativeSuspendFunctionsLowering::name_for_coroutine_class`, corresponding to
`NativeSuspendFunctionLowering.kt:100-101`. The existing source-offset identity
and mangled concrete-template identity are preserved. Kotlin's fileLowerState
unique-name allocator and Clang's naming inputs differ; this is recorded as a
platform adaptation, not identical compiler IR behavior. The function has an
explicit source-range provenance comment, and both project-wide deep inventories
are refreshed after this compiler change.

Build receipt: `build/ir-recovery/compiler-naming-build.log`. Runtime/compiler
receipt: `build/ir-recovery/compiler-naming-tests.log`, covering the retained-local
plugin suite and actual Kotlin/Native handoff suite. This extraction restores a
function boundary for measured correspondence; it does not close the remaining
ExpressionSlicer, spilling, GC, or unrestricted interoperability gaps.

## Required LLVM package and compiled result merges (2026-10-06)

The earlier historical finding about an optional vendored LLVM path no longer
describes the current build. kxs_inject/CMakeLists.txt requires the selected
compiler's LLVM package and shared LLVM target, with no vendored or component
library substitute. llvm-package-required.log records configuration rejecting a
disabled LLVM package.

IrToBitcode_coroutines.hpp's reference pseudocode and local void-pointer aliases
have been replaced by compiled .hpp/.cpp LLVM helpers. They implement real start,
dispatch and resume blocks, block-address registration, and normal/resumed value
merging through a phi; Unit uses the supplied Unit instance without a value phi.
The injector uses the translated SuspendableExpressionScope, while frontend
expression evaluation and general Kotlin IR scope resolution remain separate
missing dependencies. The LLVM module adapter no longer claims full
evaluateSuspensionPoint provenance merely because it emits address dispatch.

llvm-real-codegen-tests.log records four suites with zero failures in 38.98
seconds, including execution of distinct normal/resumed values from generated
LLVM and nonsequential-ID two-point frame resumes. Both full project-wide deep
inventories were refreshed. IrToBitcode pairing is now present, but its measured
logic remains zero/provisional and the compiler file is overwhelmingly unported.
