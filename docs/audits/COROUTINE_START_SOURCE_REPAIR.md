# Current typed source entry and KDoc correspondence

Complete common CoroutineStart.kt and the C++ pair were reread. The source
receiver-bearing invoke now has a direct typed overload at CoroutineStart.hpp:384.
DEFAULT, ATOMIC and UNDISPATCHED call the corresponding typed start intrinsics
with the actual block, receiver and completion; LAZY does nothing. The existing
erased entry and ordinary C++ callable ownership adapter remain explicit ABI
bindings. No new dispatch helper or alternate state machine is introduced.

The complete source invoke KDoc is attached to the typed entry (:371), including
all four strategy mappings and the internal-API documentation tag. is_lazy retains
its complete source property KDoc at :363. These @suppress tags are documentation
text, not compiler-warning controls. No warning suppression is restored. Enum
KDoc examples remain in upstream Kotlin notation. An exact normalized KDoc-block
comparison is recorded in build/ir-recovery/coroutine-start-kdoc-check.json;
block presence is separate from the analyzer's documentation metric and from
correct executable behavior.

The actual test_cancellable_start.cpp was checked using the previous frontend
module with -Wall -Wextra -Wpedantic -Werror and no warning-disable flags. It
returns one on exposed unused-parameter diagnostics in consumed dependencies.
No fresh runtime execution or whole-file completion is claimed. Receipt:
build/ir-recovery/coroutine-start-typed-strict.log. Previous runtime receipts below
predate this overload and warning suppression removal. Remaining ordinary C++
callable authoring/current-continuation adaptation, broader lazy builder behavior,
compiler lowering and both MLX acceptance paths remain incomplete.

The normalized exact check finds all seven source KDoc blocks present in the
header; the eighth C++ block is file provenance. It verifies source block text,
not the complete surrounding documentation/algorithm contract. Both absolute
full-root deep scans finish with exit zero using the previous analyzer executable;
strict analyzer rebuilding remains unsuccessful. CoroutineStart keeps 1/1 source
body name and 1/1 type; measured function-body similarity is 0.15 (previously 0.13).
This remains far below complete transliteration correspondence.

Code and documentation measurements are reported separately. Whole-library body
similarity is 0.26 and documentation text similarity is 0.37, with 7238/7437
documentation lines (97% by amount), compared with 7229/7437 before the KDoc
repair. Compiler/prerequisite body similarity is 0.36 and documentation similarity
0.60. Library totals remain 824/2918 functions, 359/560 types and 123 scoring
failures; compiler totals remain 592/7657, 174/1727 and 24 failures. Text/line
measurements do not prove accurate comments or working algorithms. Receipts:
coroutine-start-typed-{library,compiler}-deep.log under build/ir-recovery.

Historical checkpoints follow.

# CoroutineStart source selection repair

Date: 2026-10-07. Read complete common CoroutineStart.kt and the existing C++
header/source, actual start intrinsics and consumed callable/context helpers.
CoroutineStart is sixth in the dependency-priority report. Its consumed Native
wrappers were translated in the preceding commit.

`CoroutineStart.cpp:7` mirrors source CoroutineStart.kt:356-362: DEFAULT calls
start_coroutine_cancellable, ATOMIC calls start_coroutine, UNDISPATCHED calls
start_coroutine_undispatched and LAZY does nothing. `CoroutineStart.cpp:24`
implements is_lazy from source :370. The public enum retains its source KDoc and
is declared in the header with provenance. Concrete selection lives in .cpp.

The old generic invoke directly queried dispatchers, enqueued a custom runnable,
checked Jobs and delivered completion itself. It bypassed ordinary continuation
interceptors and caught exceptions from successful completion, attempting an extra
failure delivery. Those substitute algorithms are removed.

`CoroutineStart.hpp:388` binds public generic C++ callables and receivers into
the existing erased suspend ABI. It invokes the actual Native wrapper argument,
returns the actual erased result or boxes an ordinary C++ result, and propagates
exceptions to the source intrinsic. The receiving typed adapter unboxes and
deletes those boxes. Moved callable/receiver owners remain retained; all
lvalue references stay explicitly borrowed, regardless of copyability. The existing legacy current-continuation binding
uses that actual wrapper. `CoroutineStart.hpp:428` delegates the public receiver
API to the concrete selection, without moving/capturing a LAZY body. The existing
compatibility extension class forwards to the same public entry, including typed
continuation arguments. This generic binding is an explicit C++ ABI adaptation;
it does not implement its own dispatcher or coroutine algorithm.

The expanded test_cancellable_start exercises all four start strategies through
the generic entry. A plain custom ContinuationInterceptor sees interception,
resumption and release for DEFAULT/ATOMIC; UNDISPATCHED enters immediately and
LAZY performs no work. The body receives an actual Native ContinuationImpl frame.
Move-only callable captures survive queued execution and cancellation, then
release. Noncopyable lvalue receiver identity remains borrowed. Typed receiver
compatibility resumes after suspension. Dispatcher failure reports the original
cause, and a throwing Undispatched completion is invoked only once.

The real custom-interceptor regression compiles against both the preceding header
and the new implementation. The preceding program exits one; the new one exits
zero. `build/ir-recovery/coroutine-start-probe-results.json` records both results.
Nine focused executables finish with zero failures in 2.05 seconds, recorded in
`coroutine-start-final-tests.log`, with builds in `coroutine-start-regression-build.log`.
The actual CoroutineStart, start intrinsic and continuation sources compile and
run directly with Clang AddressSanitizer/UndefinedBehaviorSanitizer and the core
archive, exit zero and no diagnostics (`coroutine-start-sanitizer-build.log`,
`coroutine-start-sanitizer-tests.log`). This component command invokes no Kotlin
compiler and links no Native runtime. It does not establish either complete
required standalone/Native MLX application path.

All new source ranges resolve to valid upstream bounds; the two changed library
files contain no prohibited markers. Broader lazy builder, result and compiler
lowering contracts remain incomplete. The full source-first goal remains active.

Both final full-root deep scans exit zero and refresh the inventories, measured
criteria and dependency priorities after the library and tool changes. Library
function names are 783/2918, types 341/560, average body similarity 0.26 and
scoring failures 122. CoroutineStart records 1/1 function names, 1/1 types and
0.13 body similarity. Its method/getter presence is corrected by the reproduced
[enum receiver report repair](AST_DISTANCE_ENUM_RECEIVER_REPAIR.md), rather than
by changing the actual source API to satisfy a false owner-matching report.
Public binding differences remain visible in body/parameter evidence; name
presence and focused tests do not establish whole-library completion. The formerly reported consumed Native namespace-projection gaps are repaired
in STDLIB_COROUTINE_NAMESPACES.md; other source-body gaps remain visible.

Copyable-borrow follow-up (2026-10-07): CoroutineStart.hpp:375 no longer
copies a copyable lvalue callable or receiver. It borrows the original object
with std::addressof, while rvalues transfer into owned storage. Kotlin's
CoroutineStart.kt:356-362 invokes the actual supplied block and receiver;
cloning an ordinary referenced C++ object changes that identity and its mutable
state. C++ borrowed inputs must remain alive until the computation finishes;
retaining their address does not transfer ownership or extend their lifetime.

The regression at test_cancellable_start.cpp:515 fails against the preceding
binding at line 560, before the source fix (the later include shifts that line
by one). It checks copy counts, actual callable and receiver addresses, mutable
callable state, delayed reads of the receiver's fields and suspension/resumed
success or failure across all four start modes. DEFAULT cancellation prevents
entry; ATOMIC and UNDISPATCHED enter despite cancellation; LAZY performs no
captures or entry. The caller keeps the borrowed objects alive through actual
frame termination and checks that the frame is then released.

AbstractCoroutine.hpp:231,238 owns its by-value parameters. Those two start
bindings now move their callable and forward their receiver into the wrapper,
matching AbstractCoroutine.kt:133-135. This avoids retaining pointers to expired
local parameter objects after the general lvalue borrowing repair. The new
regression at test_cancellable_start.cpp:591 destroys the external receiver and
capture handles before queued execution, then verifies pointee identity, actual
completion result and resource release for both ordinary-result and erased-entry
start overloads. Existing move-only rvalue ownership cases remain exercised.

A build-only control shadows AbstractCoroutine.hpp with the preceding two
parameter-forwarding expressions while retaining the repaired borrow binding.
No production file is reverted. The instrumented control exits one at the
receiver/capture lifetime assertion (:625) before queued entry, proving that
both source caller changes are required. The repaired program exits zero.
Receipts: coroutine-start-borrow-control-{build,tests}.log; isolated control:
build/ir-recovery/coroutine-start-borrow-control/kotlinx/coroutines/AbstractCoroutine.hpp.

The core and ten focused executables build after the behavior changes; all ten
CTests finish with zero failures (2.80 seconds). The final regression fixture
instantiates and instruments the changed header bindings with AddressSanitizer
and UndefinedBehaviorSanitizer, the existing frontend and mandatory LLVM plugin.
Execution exits zero with detect_stack_use_after_return=1 and no diagnostics.
Receipts: coroutine-start-borrow-focused-{build,tests}.log,
coroutine-start-borrow-final-build.log and
coroutine-start-borrow-sanitizer-{build,tests}.log. Fifteen ranged source
references across the changed library headers resolve to valid Kotlin bounds;
neither header contains prohibited markers. Receipt:
coroutine-start-borrow-provenance.log.

Both full-root ast_distance --deep scans are refreshed after the final source
changes; each exits zero. Library totals remain 811/2918 body names, 359/560
types, average body similarity 0.26 and 123 scoring failures. The compiler scan
is refreshed and unchanged. These bounded identity/lifetime cases establish
neither full source-body parity nor the required standalone/Native MLX product
acceptance paths. The earlier broad frontend retained-locals compilation failure
is not repaired by this source binding change. Deep receipts:
coroutine-start-borrow-{library,compiler}-deep.log.
