# Source-authored Job extensions and unsuppressed frame integration

The complete common Job.kt and its C++ header/implementation were read before
this repair. Recovery commits precede edit batches. The user's requirement is
source-shaped coroutine calls lowered by the existing mandatory CMake compiler
pipeline, rather than another manually written suspension helper.

Job.kt:509-512 calls cancel(), then join(). Job.cpp:35 now has the same sequence:
job.cancel(); job.join(); followed by the current erased Unit ABI return. The
function is annotated for the existing Clang frontend and LLVM module plugins.
Job.hpp:441 exposes the source authoring call; :444 and :447 expose explicit
continuation boundaries. The raw entry at Job.cpp:29 uses the existing borrowed
or shared continuation binding. It does not adopt the Job receiver or implement
another state machine. The preexisting explicitly blocking C++ convenience at
Job.cpp:42 remains available to ordinary consumers, including the separately
owned JobTest work; it is not the source suspending extension.

The ordinary nonsuspending extensions also follow Job.kt:519-521,555-556,
562-564,584-586,602-604,610,627-629,644. Concrete bodies live at Job.cpp:48,53,58,
63,69,74,79,86. Defaults remain in the header. get_job preserves the source
IllegalStateException and context-bearing diagnostic. ensure_active throws the
actual cancellation exception rather than inventing a fallback exception.
The Job plus binding at Job.cpp:23 returns the other Job as upstream specifies;
the previous invented null-right fallback is removed. Existing context-prefixed
C++ API bindings forward to the source-named extensions.

CompilerFrameLowering.cpp:327 parses its deliberately partial input as Clang's
TU_Prefix, with referenced template instantiation requested while the parser
scope is live at :304. The former unused-function diagnostic pragma is removed.
The complete owning compilation performs translation-unit completion with its
original diagnostics. Declaration reuse at :48 maps earlier main-file
declarations back to their original source positions, excluding generated
frame declarations. This fixes duplicate local-function imports exposed by a
frame that calls an earlier function also used later in the original file.
The production path remains an in-process Clang AST import and LLVM pass; no
Python launcher, serialized IR repair, alternate frame or fallback is added.

The warning regression in src/tests/ir/test_suspend_plugin.py:29 uses
-Wall -Wextra -Werror with no warning-disable flag. Its used-function control
compiles with exit zero. Its genuinely unused function is rejected with
-Werror,-Wunused-function at the original source location, rather than a false
partial-parser location. Receipts: build/ir-recovery/prefix-warnings-used.log
and prefix-warnings-unused.log. This does not claim that all preexisting
project-wide warning-disable options have been removed.

The warning check exposed an unused parameter in the old inline concrete
CompletedContinuation body. Its complete Native source was read before moving
the concrete definitions into ContinuationImpl.cpp:14,20,25,30. The header at
:121 retains declarations and ranged source references. Native
ContinuationImpl.kt:118-127 supplies the singleton, completed-state error and
toString. The port now uses the existing IllegalStateException transport and
source text. The ignored input parameter has no name in its concrete throwing
definition; no diagnostic policy changes are needed.

The fresh complete core and three focused targets build with exit zero.
test_native_exception_namespace executes compiler-authored cancel_and_join
for immediate completion, real suspended completion, caller cancellation
before entry and caller cancellation while suspended. Later target completion
does not duplicate a cancelled caller's completion. It also verifies actual
context/Job identity, original cancellation cause and diagnostic, plus completed
sentinel identity, text and both source failure branches. test_suspension_core
and test_channel_as_flow_smoke execute alongside it, with zero failures.
Receipts: build/ir-recovery/job-extensions-warning-clean-build.log and
job-extensions-warning-clean-library-tests.log.

The same fixture, actual Job.cpp and actual ContinuationImpl.cpp compile and
execute with AddressSanitizer/UndefinedBehaviorSanitizer and
detect_stack_use_after_return=1, with no diagnostics. Other dependencies link
from the fresh core archive and are not instrumented by this focused command.
Receipts: job-extensions-warning-clean-sanitizer-build.log and
job-extensions-warning-clean-sanitizer-tests.log under build/ir-recovery.

Generated Job LLVM at build/ir-recovery/job-extensions-prefix.ll contains the
compiler-generated cancel_and_join frame, a blockaddress resume target,
indirectbr resume dispatch, the actual Job.join call, initial/resumed result
checks and terminal capture cleanup. There are no unresolved __kxs_ call or
invoke markers. Receipt: job-extensions-prefix-ir-check.log. Fifty-eight ranged
source references across Job and ContinuationImpl headers/implementations
resolve within their matching pinned sources; no prohibited markers occur in
those four files.

Both final absolute-root ast_distance --deep scans exit zero. Library evidence
is 825/2918 functions, 359/560 types, body similarity 0.26 and 123 scoring
failures. Job is 10/24 functions with body similarity 0.14. Compiler evidence is
592/7657 functions, 174/1727 types, body similarity 0.36 and 24 scoring failures.
ContinuationImpl is 9/9 function names, 4/4 types and body similarity 0.36;
symbol presence does not establish complete source-body equivalence.
Receipts: job-extensions-warning-clean-{library,compiler}-deep.log.
Full-library parity and the required complete standalone/Native shared-state
MLX GPU paths remain unfinished. The parent Kanban task and goal remain active.
