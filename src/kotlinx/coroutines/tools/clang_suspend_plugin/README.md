# Clang Suspend DSL Plugin (kotlinx.coroutines-cpp)

This experimental Clang plugin lowers annotated free functions into retained
coroutine frames inside the compiling Clang process. It creates argument and
local fields, orders suspend-call arguments, separates immediate and resumed
results, and releases constructed locals on completion or failure. The required
LLVM plugin constructs persistent saved-address dispatch before optimization.
See `docs/suspension/IR_SUSPEND_LOWERING_SPEC.md` for the contract and limits.

The production LLVM lowering plugin is `KotlinxCoroutinePass` in the neighboring
`kxs_inject` directory. CMake loads it with `-fpass-plugin`; lowering remains
inside Clang through optimization and code generation. `KotlinxSuspendPlugin`
performs the preceding frontend operation: parse the synthesized frame in memory
with the original compiler invocation, reuse existing header declarations, import
its AST nodes, and replace the annotated function body before code generation.
The compiler performs both stages without a compile launcher or serialized IR.

`NativeSuspendLowering.cpp` keeps the compiler-derived implementation in
`org::jetbrains::kotlin::backend::konan::lower`, corresponding to its Kotlin
package. `CompilerFrameLowering.cpp` contains the Clang entry adapter and AST
integration. Frame construction calls a separate `build_state_machine` with
source-range provenance for Kotlin's `buildStateMachine`. This separation makes
the source correspondence inspectable; it does not establish full transliteration.
Use `ast_distance --compare-functions` with `--with-companions` and a separate
compiler-scoped `--deep` run to measure the compiler implementation. The library
deep report has different inputs and cannot certify this compiler source.

The current entry ABI requires a trailing by-value
`std::shared_ptr<kotlinx::coroutines::Continuation<void*>>`. The frontend resolves
that parameter through canonical Clang types, including source type aliases.
Its source name is unrestricted. Ordinary parameters named `completion` retain
their own argument fields; the continuation routes to the current frame. This
corresponds to Kotlin's trailing constructor continuation parameter in
AbstractSuspendFunctionsLowering.kt:159-185. Automatic insertion of that entry
parameter and arbitrary result-type lowering remain required compiler work.

Tail-only authoring calls with `kxs_implicit_continuation` now receive the entry's
trailing continuation directly. The frontend resolves the ABI overload and
imports the direct body in process, preserving frame-free tail forwarding.
State-machine calls continue to receive the current frame. This follows the two
cases in NativeAddContinuationToFunctionCallsLowering.kt:15-22.

`Job::join()` uses that same authoring contract. The source transform-latest
body in `flow/internal/Merge.cpp` cancels the previous Job, calls
`previous_flow->join()`, then launches its replacement. The frontend generates
the non-tail frame and supplies its continuation to the actual virtual join ABI;
LLVM injection supplies the saved address and resume dispatch. The no-argument
authoring declaration rejects an unlowered call during code generation.
Smart-pointer receivers lower their resolved `operator->` before the suspend
address is saved, preserving the receiver's actual object identity.

The Clang integration completes queued host template instantiations before
importing a frame, so ordinary standard-library deduction stays in the host
compiler. Main-file helpers parse only through the current namespace-scope
definition; later includes and definitions remain owned by the original parser.
The late-include regression checks this boundary with ordinary Result assignment
and dispatcher headers after a lowered function.

Omitted authoring arguments are materialized before that final continuation
argument. Resolved default-expression names retain their namespace identity.
Frame calls evaluate defaults into argument storage before saving the resume
address, move newly created values into by-value ABI parameters, and preserve
existing reference referents. The corresponding Kotlin default selection and
continuation contracts are cited beside the Clang adapter; complete Kotlin
default-dispatch mask generation remains untranslated.

Named functions now select their continuation base through
`get_coroutine_base_class`, following NativeSuspendFunctionLowering.kt:93-98.
`[[kotlinx::restricts_suspension]]` on a class represents Kotlin's
`@RestrictsSuspension`; `[[kotlinx::extension_receiver]]` on a parameter represents
its explicit Kotlin IR receiver role. Restrictions inherited through class bases
are found by the translated `get_all_superclasses` traversal. An ordinary
argument or C++ dispatch receiver does not acquire an extension-receiver role
from its type, position or name.

The checked Kotlin predicate contains an unlabelled return inside inline `any`.
It returns from the containing function on the first parameter; the C++ predicate
preserves that control flow. The Kotlin/Native execution receipt and C++ regression
cover this distinction. Restricted frames require the completion's context to be
the canonical empty context before executing their body, as the runtime source
requires. The compiler-generated restricted-invoke metadata is also read during
base selection. For lambda expressions, `UpgradeCallableReferences` records the
restriction before flattening receiver roles, and `NativeFunctionReferenceLowering`
sets the invoke-method flag on the actual Clang declaration. AST import preserves
that flag and reuses the original captured value/field declarations. Named-reference
wrappers, property references and Kotlin reflection metadata remain untranslated.

Annotated lambda call operators are discovered in their own function context.
Generated frame fields borrow the original closure's captured referents and keep
their constness and identity; callers retain the closure across suspension, as
they retain borrowed C++ member receivers. This does not provide Kotlin GC ownership
of arbitrary temporary closures. The front attribute syntax `[] [[suspend]] (...)`
is standard in C++23; the CMake regression requests `cxx_std_23` for that target.

Nested nongeneric lambda locals in a replaced enclosing suspend frame now use
concrete callable class construction from AbstractFunctionReferenceLowering.kt.
The generated class has capture fields, a constructor and an invoke method
(operator() in C++). The emitter uses the current invoke declaration's lowered
body, preserving its retained frame and native resume regions. It constructs
bound values in capture order and stores the callable in the enclosing frame.
An operator call's dispatch receiver is retained separately from regular arguments.
Capture fields use indexed private names (f_0_, f_1_, ...) corresponding to Kotlin's
f$0, f$1, ... fields. The invoke printer maps original captured declaration identities
to those actual fields and leaves shadowing locals and ordinary parameters intact.
Declaration initializers use the same map, including grouped declarations.
Function and parameter annotations, including the native restricted-invoke flag
and flattened receiver-origin metadata, are copied from the source declarations.
The nested CMake regression executes value-capture snapshots, reference identity,
move-only capture release, captured-owner identity and failure at both resume stages
under AddressSanitizer and UndefinedBehaviorSanitizer. Copied receiver captures
construct an owned object field and rebind invoke-body receiver expressions to
its address, preserving mutable versus const invoke access. Constant complete
array captures keep their native field shape; constructor initialization copies
each element, including nested arrays. Array reference captures retain their
referents. Generic captures, full reflection/SAM construction and arbitrary
temporary-closure ownership remain required; this is not full callable lowering.

## Features

- Detects suspend functions annotated with `[[suspend]]` or `[[kotlinx::suspend]]`
- Detects suspend points via `suspend(expr)` wrapper or `[[clang::annotate("suspend")]]`
- Installs retained frames in Clang's AST during ordinary compilation
- Emits a `.kx.cpp` diagnostic sidecar only when `out-dir` is explicitly requested
- Uses `void* _label` (Kotlin/Native NativePtr)
- Supplies `&&label` resume addresses and the persistent frame-field address
- Compiles to LLVM `indirectbr` + `blockaddress`, the same address-dispatch pattern
- CFG-based liveness analysis with complete fixed-point convergence and source-order site IDs
- Enclosing-function local, use and deferred-call analysis visits lambda capture
  initializers without treating lambda-body locals or calls as enclosing work.
- Local class declarations and member bodies also keep their own analysis
  context; ordinary enclosing calls and liveness remain visible.
- Direct/tail entries and repeated non-tail suspensions executed by regressions
- Tail-only explicit return branches and conditional return values forward the
  caller's continuation without constructing a retained frame. Non-tail calls,
  try regions and C++ storage requiring lifetime retention still use frames.
  Reference-local bindings retain frames, including bindings extending a
  temporary's lifetime; narrowing borrowed-reference eligibility remains work.
- Actual Kotlin-generated continuations resumed from C++ worker threads, with
  success, failure, cancellation and explicit interop-handle release

Target: Apple clang only.

## Building (Apple/Clang)

Requires a Clang/LLVM installation with CMake package configs.

```bash
mkdir build && cd build
cmake -DKOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=ON ..
make KotlinxSuspendPlugin
```

The plugin dylib/so is emitted into `build/lib/` with platform suffix.

## Usage

### Ordinary compilation

Load both plugins built against the selected Clang development package:

```bash
clang++ -std=c++20 -I src \
  -Xclang -load -Xclang build/lib/KotlinxSuspendPlugin.so \
  -Xclang -add-plugin -Xclang kotlinx-suspend \
  -fpass-plugin=build/lib/KotlinxCoroutinePass.so \
  -c path/to/file.cpp
```

In CMake, call `kxs_enable_suspend_dsl(target)` after adding the target's sources.
It enables the frontend and mandatory LLVM stage and tracks both plugin files as
object dependencies. `kxs_enable_suspend(target)` remains the entry point for
existing explicit frames that already supply their LLVM authoring markers.

`kxs_add_executable` and `kxs_add_library` enable both stages automatically.
Inside a lowered suspend body, the real `yield()` authoring intrinsic supplies
the current frame implicitly:

```cpp
#include <kotlinx/coroutines/dsl/Coroutines.hpp>
#include <kotlinx/coroutines/Yield.hpp>
using namespace kotlinx::coroutines;

[[suspend]] void* yield_twice(std::shared_ptr<Continuation<void*>> completion) {
    int value = 40;
    yield();
    ++value;
    yield();
    return new int(value + 1);
}
```

The caller owns and deletes the returned integer box. The frontend selects the
`yield(current_frame)` continuation-ABI overload and retains `value` across both
suspensions. A declaration annotation identifies this implicit-continuation
construction; the compiler does not guess from the function name. The no-argument
declaration is a compiler intrinsic, with no runtime implementation. Clang rejects
its unlowered calls, including ordinary functions compiled with the plugin. The
old OS-thread/event-loop compatibility implementation has been removed. Explicit
`yield(completion)` remains available at an ABI boundary. Other suspend APIs still
need their authoring declarations and verified implicit-argument lowering.
The DSL header supplies the frame runtime types and LLVM marker declarations
required by authored suspend definitions.

Including `Delay.hpp` enables Kotlin's two delay forms:
`delay(time_millis)` and `delay(kotlin::time::Duration)` inside lowered bodies.
The compiler adds the current frame and selects the shared-continuation ABI
entry. These calls use cancellable timer scheduling. Nonpositive durations
complete immediately; positive infinity suspends until cancellation without
scheduling an expiring timer. Chrono delay entries and the duplicate DSL
namespace delay/yield wrappers have been removed. The master header includes the
actual translated Delay/Yield APIs.

The duration dependency translates Kotlin's tagged nanosecond/millisecond
representation, precision normalization, Long construction, negation, addition,
and infinities. Its raw factory retains the upstream invariant checks enabled by
Kotlin/Native. Native unit conversion follows the original wrapped multiplication,
overflow test and saturation branches, and includes its separate overflow
conversion function. Per-function source ranges identify these implementations.
All eleven upstream DurationToMillisTest cases execute; a Kotlin/Native comparison
checks units and range boundaries. Duration's remaining arithmetic, parsing,
formatting, and other public APIs remain required in the compiler-source deep
report. These behavioral checks do not establish complete line-for-line parity.

Concrete retained value fields use Clang's fully qualified type names. Resolved
auto types are desugared before qualification, preserving namespaces outside
the coroutine's declaration context. The timer regression covers the actual
millisecond-count API and Duration values constructed from milliseconds,
nanoseconds, and a retained auto local. It checks repeated suspension, cleanup,
and prompt cancellation after timer completion but before queued dispatch.
Unlowered authoring calls are rejected.

The frontend currently supports `void*` entries with a trailing continuation
parameter identified by canonical type, including free functions in nested and
inline namespaces and member definitions. Constant complete array locals retain their native types and delayed
construction, including multidimensional arrays and non-copyable elements.
Reference fields borrow concrete typed pointers. Free and member definitions in
ordinary included headers also lower during compilation. Template definitions,
and additional control
flow still require implementation and verification. These limits prevent a
claim of complete Kotlin compiler parity.

### Basic (Phase 1 defaults)
```bash
clang++ -fsyntax-only \
  -Xclang -load -Xclang build/lib/KotlinxSuspendPlugin.dylib \
  -Xclang -plugin -Xclang kotlinx-suspend \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang out-dir=build/kxs_generated \
  path/to/file.cpp
```

### With Computed Gotos (Phase 3 - Kotlin/Native parity)
```bash
clang++ -fsyntax-only \
  -Xclang -load -Xclang build/lib/KotlinxSuspendPlugin.dylib \
  -Xclang -plugin -Xclang kotlinx-suspend \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang out-dir=build/kxs_generated \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang dispatch=goto \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang spill=all \
  path/to/file.cpp
```

## Plugin Arguments

| Argument | Values | Default | Description |
|----------|--------|---------|-------------|
| `out-dir=<path>` | directory path | omitted | Explicit diagnostic extraction mode; ordinary compilation imports the frame in memory |
| `dispatch=<mode>` | `goto` | `goto` | State machine dispatch (computed goto) |
| `spill=<mode>` | `all` | `all` | Retain concrete local fields; liveness-guided field selection is rejected until implemented |

## Example

Input:
```cpp
using namespace kotlinx::coroutines::dsl;

[[suspend]]
void* my_suspend_fn(int x, std::shared_ptr<Continuation<void*>> completion) {
    int y = x + 1;
    suspend(delay(100, completion));  // suspension point
    return reinterpret_cast<void*>(y);
}
```

Direct and sole-tail functions retain their original continuation entry. This
matches the native lowering decision to allocate a state machine only for non-tail
suspensions. Parameter declarations preserve array/reference declarators and
`noexcept`; the incoming continuation is forwarded to the tail callee unchanged.
The generated-code regression executes immediate/delayed success and failure,
checks continuation release, and checks a borrowed array reference.

Tail classification now uses the translated `collect_tail_suspend_calls` visitor
from `TailSuspendCallsCollector.kt`, with separate try-region and tail-expression
state. Suspend calls nested in arguments remain non-tail. Catch results inherit
their surrounding tail state in the collector; C++ exception ownership still
requires retained storage during handler suspension. Lambda capture initializers
belong to the enclosing body, while nested lambda bodies have their own returns.
The Clang adapter makes parentheses, cleanup and forwarding-reference
materialization transparent without treating explicit source casts as tail calls.

For an erased Unit return, `clang::annotate("kotlin.ir.UnitReturn")` supplies the
return-type metadata lost by `void*`. The current ABI represents the Unit result
as nullptr. A Unit tail statement followed by `return nullptr` is rewritten to
return the actual suspend result; `returnIfSuspended` DSL wrappers are removed.
This explicit metadata adapter does not supply Kotlin IR returnable-block symbols
or automatically import Kotlin return-type metadata. Those compiler boundaries
remain incomplete.

Non-tail lowering now supplies value-result consumption, the current frame as
callee continuation, local/reference rewriting and tested lifetime handling.
Arguments that are temporary objects are moved into calls and released after the
callee returns, including a suspended result. This preserves the distinction
between a local retained across suspension and a completed call's temporary.

## Verified plugin and handoff tests

Load the plugin with the Clang version matching its LLVM development packages.
The macOS test build uses Homebrew LLVM/Clang 23 and shared `clang-cpp`/`LLVM` so
plugin loading shares the compiler runtime registries. The analyzer is compiled
and linked into the plugin. CTest checks source-order suspension IDs and a
300-block fixed-point liveness regression; a reconstructed 100-iteration cap
fails that regression.

With the plugin enabled, `kxs_plugin_handoff` also compiles the original annotated
retained-local fixture through CMake. It checks repeated suspensions, move-only
argument lifetimes, failures and frame destruction. `kxs_kotlin_native_handoff`
uses the locally installed `konanc` selected by `KXS_KONANC_EXECUTABLE`, compiles
a real Kotlin coroutine, and resumes the same generated continuation from C++.
It retains Kotlin's emitted IR and requires `blockaddress` and `indirectbr` in
the generated resume function. The generated C++ caller completes through its
original continuation under AddressSanitizer. Stable references are disposed
exactly once before resuming; result boxes belong to the receiving C++ caller.
This verifies the tested C interop boundary, not interchangeable Kotlin-GC/C++
object layouts or arbitrary Kotlin object payloads.

The retained-local CMake fixture runs under AddressSanitizer and
UndefinedBehaviorSanitizer. It checks array indexing and `sizeof`, mutation through
an array reference, object arrays across suspension, and cleanup when a later
array element's constructor throws. Variable-length arrays are rejected; they
are not a standard C++ array type supported by this lowering.

`do` loops lower their body before their condition. `continue` reaches the
condition, and `break` exits after destroying body locals. Conditional branches
always enclose their generated statements, including cleanup and jumps. The
fixture checks immediate and delayed suspension in both loop bodies and loop
conditions, early exits, and destruction before a condition suspends.

Source `switch` statements retain their initializer and condition variable.
Case bodies can suspend and fall through. Break targets the nearest loop or
switch, while continue finds the enclosing loop and destroys intervening
switch locals. The regression checks suspension while evaluating the condition
and inside cases, grouped labels, fallthrough, loop continuation, and cleanup
after immediate condition failure or resumed case failure. Coroutine resume
dispatch still uses the mandatory LLVM address-injection plugin.

Range-based loops retain the compiler's range, begin/end iterators and element
variable. Element references continue to refer to retained array/range storage.
Directly lifetime-extended temporary ranges construct in aligned retained
storage without a copy or move. Continue destroys iteration locals before
advancing; break and failure also destroy the range when leaving its scope.
The sanitizer fixture checks mutable array references and a non-copyable,
non-movable temporary range with immediate/deferred results and resumed failure.
Suspending implicit iterator operations are explicitly rejected. Nested
temporary/subobject lifetime rules and initializer-list backing storage still
need separate implementation and verification.

The in-memory helper AST remains owned by the main compilation's AST context
until that context is destroyed. This preserves attribute storage referenced
by imported attributed types across multiple lowered entries.

Condition declarations retain their own lifetime scopes. If initializers and
condition variables are destroyed after the selected branch. While conditions
are reconstructed each iteration and destroyed on continue, break or a false
condition. For conditions remain alive during the increment, including an
increment that suspends after continue, and are destroyed before the next
condition evaluation. Body locals are destroyed before the increment.
The sanitizer fixture checks both if branches, false loop conditions, early
exits, retained reference arguments across a suspended increment, and failure
while that increment is suspended.

Built-in comma expressions execute the discarded left operand before lowering
the right operand. The right operand keeps its value category, including an
lvalue used to bind a retained reference. Discarded record temporaries construct
in retained storage without copying and remain alive across right-operand
suspension. They are destroyed in reverse construction order at the enclosing
full-expression boundary. Conditions retain their evaluated control value
before destroying these temporaries; return expressions compute the result
before destroying temporaries and then locals. The fixture checks nested comma
sequencing, reference identity, non-copyable temporary lifetime, condition and
return cleanup, and immediate/resumed failure. General expression temporary
and overloaded-operator rules remain separate work.

Argument slicing uses the translated `is_pure` predicate from
NativeSuspendFunctionLowering.kt:400-408. Constants and immutable declaration
reads through non-checking implicit conversions remain inline; mutable reads,
calls and observable volatile loads retain their order before later suspension.
Explicit/checked casts remain impure. Ordinary mutable C++ parameters retain
conservative storage because Kotlin's value parameters are immutable. Constructed
arguments keep the existing C++ ownership and lifetime handling. The slicing
regression checks a mutable snapshot before a later argument changes its value,
single side-effect execution, immutable reads, a volatile load and resumed failure.
The argument slicer also computes Kotlin's suffix table of suspension calls
(:215-250), including its first-child/only-suspension decision. Scalar trailing
operands without another suspension remain in the final call when their order
permits it. Earlier impure operands retain Kotlin's evaluation order before later
impure operands because native C++ argument order differs. Constructed/dependent
values retain their C++ lifetime storage, and suspend-call operands finish before
the saved resume address. The regression checks trailing effects in order after
resumption and their absence after resumed failure. The remaining ExpressionSlicer
transformations still require translation.

Constructor declarations and expressions use the same suspension suffix table, with
Kotlin's constructor exception: none of the explicit arguments is the first
child, because Kotlin lowers the allocated instance ahead of them. Impure
arguments are retained before a later suspension; earlier effects keep source
order. The constructor executes only after its suspended arguments complete.
The regression checks a mutable snapshot, one execution of the earlier effect,
and absence of construction on resumed failure. The shared operand translation
also handles a temporary constructed directly as a call argument. Omitted
constructor arguments resolve to their Clang default expressions and evaluate
in parameter order into retained storage, following explicit arguments. Both
declarations and temporary expressions receive the resolved values explicitly.
The regression checks default effects in order after successful resumption and
their absence after resumed failure. Kotlin allocation, mask-based default
dispatcher lowering and full ExpressionSlicer parity remain incomplete.

Suspension while evaluating a parameter default is rejected, following
FirSuspendCallChecker.kt:67-73 and :149-162. Constructing a suspend callable as
the default is valid: its body is visited in its own callable context. The
regression checks rejection for functions and constructors and acceptance of
a suspend lambda used as a default. This is a source-language restriction,
not an unimplemented default-expression lowering mode.

The restriction lives in `FirSuspendCallChecker`, in the matching Kotlin checker
namespace, with public declarations in its header and the scope walk in its
implementation file. The Clang visitor supplies the actual containing-declaration
stack. The reverse walk checks parameter membership and default presence, and
stops at a function boundary. Lambda capture initializers execute in the enclosing
default scope; the lambda method body has its own declaration context. Kotlin
inline status uses an explicit IR annotation rather than C++ inline linkage.
The enclosing function is selected by Kotlin's reverse search for an actual
suspend function or suspend callable. This lets an inline default expression
reach the outer suspend function's parameter scope. Defaults in an ordinary
function or constructor receive the illegal-suspend-call diagnostic rather than
being treated as suspend owners. The regression checks the same diagnostic
distinction as Kotlin/Native.
The translated non-local return-usage walk permits local variables and parameters,
and inline lambdas with return permission, before reaching that suspend function.
Ordinary lambdas and local-class declaration boundaries reject suspension rather
than inheriting it from the enclosing body. The regression checks an ordinary
lambda, a local-class initializer and a local-class method against Kotlin/Native.
Generated coroutine records retain Kotlin's `COROUTINE_IMPL` declaration origin.
Source checks run before lowering; imported records carrying that origin already
contain lowered bodies and are not source-local suspension scopes. This preserves
the phase distinction when a nested callable's lowered frame is imported into its
enclosing body.
The other FIR checker diagnostics and full inline-context lowering remain
untranslated.

Suspension-containing built-in assignments lower and retain the destination
before the value, following Kotlin's receiver-before-value child order. The
retained destination is a C++ reference; plain and compound assignments keep
their lvalue result and assigned-element identity. Temporary record receivers
for reference members retain owning storage through the full expression,
including suspension, and are destroyed on completion or failure. Built-in
subscripts retain their operands in source order, including reversed `index[ptr]`
syntax. The fixture checks receiver side effects, both-sided suspension,
assignment-result identity and temporary-receiver cleanup. Complete Kotlin
IrSetField/type lowering remains untranslated.

Conditional expressions with glvalue results retain a reference to the selected
object. Lvalues remain lvalues and xvalues remain eligible for moving after
resumption. Retained reference types use canonical Clang types so standard
library alias spellings remain valid in the generated frame's namespace.
The fixture verifies both branches with non-copyable objects, address identity,
mutation, move-only ownership transfer and immediate/resumed failure.

Void conditional expressions execute only the selected branch without creating
a result slot. Their branch temporaries retain the enclosing full-expression
lifetime across suspension. The fixture checks both branches, observed side
effects, temporary destruction after completion, and resumed branch failure.

By-value entry parameters transfer into retained frame fields using moves;
stored fields preserve source constness. Lvalue and rvalue reference parameters
remain borrowed references to the caller's objects. Caller-owned referenced
objects must remain alive through suspension. The regression verifies a const
move-only value, non-copyable lvalue identity, rvalue-reference identity and
cleanup after immediate/resumed failure, with sanitizer recovery disabled.

The AST importer follows constructor/destructor calls and their transitive
definitions, including constructor member initializers. Reused function-local
declarations include the owning function signature and class specialization in
their identity to prevent collisions between template instantiations.

## LLVM IR Output

Non-tail member definitions lower inside the original
member scope, retaining a borrowed typed receiver pointer and access to private
members. Receiver constness and entry ref-qualification are preserved. Explicit
`this` and implicit member references use the retained pointer after suspension.
The regression exercises lvalue, const-lvalue and rvalue overloads, private calls
with suspended arguments, repeated suspension and nested member frames on an
owned temporary receiver. Template entries remain unverified.
The importer reuses header prototypes before importing their member definitions,
preventing unrelated later definitions from being installed ahead of the parser.

Ordinary included header definitions are parsed using an in-memory replacement
for that header and the including context through its include directive. No
header file is rewritten on disk. Header free functions use a frame local to the
original function body, preserving inline linkage across translation units.
Declaration offsets after the edited body are mapped back to the original file
so existing declarations are reused. Imported frame methods retain the source
implicit-inline flag before code generation. The two-file regression exercises
inline free and const member entries, cross-file calls, suspension and failure
cleanup under address and undefined-behavior sanitizers. Command-line forced
includes also use the cloned invocation's preinclude context, without parsing
later main-file definitions. The same regression builds and executes both normal
and `-include` variants. Macro-defined function bodies remain unsupported.

Try regions preserve native typed exception selection around suspension points.
Resumed failures are raised inside the protected region. Retained protected
locals are destroyed before the selected handler executes. Catch dispatchers
retain an `exception_ptr` and leave the native catch before running a resumable
handler body. Reference catch variables borrow the retained exception; value
variables are copied once into delayed storage and destroyed at handler exit.
Copy-constructor failure terminates, matching native catch initialization.
Frames with exception regions execute their generated body inside a native catch
context restored from the active retained exception. Handler entry and exit return
to this context wrapper using injected blockaddress transitions. The wrapper
re-enters native catch context before calling the body; LLVM address dispatch
selects the continuation region. Nested handlers retain and restore the previous
active exception. `std::current_exception()` and bare rethrow therefore retain
native behavior in both direct expressions and called helpers, without rewriting
those expressions. The regression checks helper rethrow, nested context restoration,
context clearing after handlers, and exits through `break` and `continue`.

Nested cleanup restores the surrounding native context between handler groups.
Protected fields are destroyed after leaving the newly selected catch and before
constructing its value-catch variable. Return values are retained across cleanup
transitions. Unhandled failures use a separate address-dispatched cleanup path
before rethrowing the saved failure. A departing exception remains retained until
the next surrounding context is active, then is released before body dispatch.
Destructor observers verify local and exception-object context on nested normal
exit, return, break, continue and resumed failure in both build variants.

After native injection, the dispatch pattern is:
```llvm
entry:
  %label = load ptr, ptr %_label
  %is_null = icmp eq ptr %label, null
  br i1 %is_null, label %start, label %dispatch

dispatch:
  indirectbr ptr %label, [label %resume0, label %resume1, ...]

start:
  ; ... normal execution ...
  store ptr blockaddress(@invoke_suspend, %resume0), ptr %_label
  ; ... suspend call ...

resume0:
  ; ... resume execution ...
```

This expresses the address-dispatch pattern. Full Kotlin/Native parity needs separate frame, result and lifetime validation.

For exception-bearing frames, the blockaddresses belong to `invoke_body`; the
`invoke_suspend` context wrapper contains no resume-location dispatch.

## Architecture

- `KotlinxSuspendPlugin.cpp` - Main plugin: attribute registration, AST visitor, code generation
- `SuspendFunctionAnalyzer.hpp/cpp` - CFG construction and backward dataflow liveness analysis
- Generated code inherits from `ContinuationImpl` and uses the Kotlin/Native continuation ABI
