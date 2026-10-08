# Limit source transliteration — 2026-10-07

Source commits: 1bf6abd0, 050fca1f and 5570b19b. Final generated reports:
285a8a24. The complete matching upstream Limit.kt:1-140 and the existing C++
header/implementation were read before editing.

## Source changes

Limit.hpp:43,51 translates emitAbort as an annotated downstream emission followed
by the actual owned AbortFlowException. The private generic extension stays in
the source flow namespace; arbitrary public element types require its definition
in the header. Limit.hpp:62,100,154,200,252,297 translates drop, drop_while, take,
collect_while, take_while and transform_while. All handwritten Limit continuation
frames and coroutine macros are removed. The typed collectors represent source
captures; the compiler remains responsible for spilled locals and resume dispatch.

Collection and predicate entries retain their existing owners and supplied
continuations; raw collectors stay borrowed. Predicate result boxes are consumed
before subsequent emission. drop_while records its matched state before emitting.
take checks the original count diagnostic and aborts only after the final emission.
collect_while checks abort ownership against its actual FlowCollector subobject,
then checks the current context for cancellation. take_while keeps its tail call
and predicate-before-emission order. transform_while uses the safe flow builder
and permits emission before a false transform result. The callable ABI adaptation
preserves ordinary Boolean predicates, borrowed-continuation callbacks and
owning-continuation callbacks, including lvalue element argument compatibility.
Limit.cpp has canonical source provenance and the actual header include.

## Consumed compiler and CMake work

The current compiler review is recorded in MERGE_SOURCE_REPAIR.md:203. It compares
NativeSuspendFunctionLowering, CoroutinesVarSpillingLowering, local declaration
popup and IrToBitcode suspension emission with the frontend, frame importer and
mandatory LLVM module injection. Earlier source repairs preserve declared
cv-qualification, initialize saved exception state, emit used loop continuation
targets and emit grouped static declarations once. This Limit batch adds real
source suspension and exception paths; it does not modify compiler or CMake code.

The strict actual Limit consumer exposes the remaining local class/lambda
integration failure: CompilerFrameLowering requires a namespace context for
instantiated entries. LocalDeclarationPopupLowering's lexical declaration and
capture contract still needs translation. GNU label diagnostics and dependency
diagnostics also remain. Skipping those entries or suppressing warnings would
not establish a source-compatible state machine.

CMake commit 4bc60d81 connects the public installed target to the actual core and
Threads and gates actual Native handoff tests on the explicit Native boundary.
The prior Native-disabled configuration succeeds with frontend and LLVM plugins
in actual compile commands, without a Kotlin compiler or Native runtime link.
Prior fresh plugin builds fail in LLVM/Clang dependency headers. Those results
are configuration/build evidence, not complete executable acceptance.

## Verification and limits

Strict syntax checks of actual Limit.cpp and test_limit_suspension.cpp exit 1.
The final consumer recognizes annotated Limit entries but fails on instantiated
namespace context, generated code and dependencies. Commands use the selected
Clang with -std=c++20 -Wall -Wextra -Wpedantic -Werror, the existing frontend plugin
and mandatory LLVM plugin. No warnings were suppressed. Existing plugin binaries
do not validate the latest compiler source repairs. No fresh runtime result
establishes retention, destruction, resumed failure or cancellation for Limit.
Receipts under build/ir-recovery:

- limit-source-authoring-source.log
- limit-source-authoring-final-consumer.log
- limit-source-authoring-final-library-deep.log
- limit-source-authoring-final-compiler-deep.log

Both required full-root deep scans exit 0 after the final source change, without
concurrent source edits. Limit improves from 7/8 matched bodies at similarity
0.04 to 8/8 at 0.07, with 23 target bodies and four target types. Name matching
does not establish completed body translation. Library totals are 832/2918
matched bodies, 359/560 types and body similarity 0.26, with 123 scoring failures.
Compiler totals are 592/7657 bodies, 174/1727 types and similarity 0.36, with 24
scoring failures. The compiler reports remain unchanged by this library batch.

The standalone ordinary C++/MLX and actual Native shared-state-machine GPU
acceptance paths remain unestablished for these source changes. The full
transliteration and compiler state-machine goal remains active.
