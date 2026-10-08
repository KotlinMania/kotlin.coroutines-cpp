# IR Suspend Lowering Specification

The docking-ring target is Kotlin/Native's continuation protocol: saved resume
addresses, persistent live values, and the correct immediate/suspended/resumed
result handoffs. Matching two LLVM instructions alone does not establish ABI,
GC, ownership, cancellation, or complete compiler parity.

## Standalone C++ use is required

The translated coroutine library and CMake/Clang compiler pipeline must work with
ordinary C++ applications without an installed Kotlin compiler, a linked
Kotlin/Native runtime, or a JVM. Existing nonsuspending C++ functions, classes,
standard-library types and MLX C++ calls remain usable without Kotlin annotations
or translation. Kotlin supplies the source and compatibility contracts; an
ordinary C++ application does not need a Kotlin program to participate.

Coroutine lowering must retain the actual C++ values and resources needed after
suspension, preserve their ownership, and destroy them according to their C++
lifetime rules on completion, failure or cancellation. Keeping a borrowed pointer
or reference in a frame does not transfer ownership. Authors must not manually
save their locals or build resume dispatch.

An application that enables Kotlin/Native interoperability explicitly links the
matching Native runtime and uses its actual continuation, state-machine, result
and GC contracts at that boundary. Only actual Kotlin GC objects require Kotlin
roots; ordinary C++ objects and MLX handles retain their C++ representation and
ownership. Standalone use must follow the translated lowering contracts without
introducing an alternate coroutine state machine or fallback.

Acceptance requires both executable paths in the
[docking-ring design](../architecture/docking_ring.md#standalone-c-use-and-kotlinnative-compatibility):
standalone C++ coroutine authoring with real MLX GPU work and verified absence of
Kotlin build and transitive link dependencies, and direct Kotlin/Native/C++
shared-state-machine handoffs with real MLX GPU work. Verify retained resource
identity and cleanup across repeated suspension, completion, failure and
cancellation in each path. The existing bounded compiler and runtime tests do
not establish either complete acceptance scenario.

## Resume addresses and tooling IDs

A switch state machine stores an integer selecting a resume case. Kotlin/Native
instead stores a `NativePtr` identifying a block in the generated `invokeSuspend`
function. The frontend supplies `void* _label` field storage and ordinary C++
labelled branches. `__kxs_suspend_site(id, &field)` pairs with the conditional
branch `if (__kxs_resume_point(id)) goto resume;` in that function. Mandatory LLVM
injection creates the true successor's block address, stores it in the supplied
field, replaces the marker condition with false and erases both marker calls.
The integer pairs compile-time regions; it is never stored as coroutine state.
The earlier explicit-blockaddress marker remains accepted for existing LLVM
consumers. There is no runtime marker implementation or dispatch substitute.

This branch adaptation is committed in a4e0c319 and is not yet executable
validation: strict C++ frontend emission succeeds, but fresh plugin builds stop
in dependency diagnostics. The older installed pass leaves the new markers
unresolved. See [the source checkpoint](../audits/RESUME_ADDRESS_SOURCE_REPAIR.md).

The resulting `blockaddress(@function, %resume)` and `indirectbr` destinations
belong to that function. The integer passed to the site/resume markers, or
to the earlier `__kxs_suspend_point` marker, is a tooling ID, not the saved address and not the runtime suspension sentinel.
Two coroutine functions may use the same tooling ID without sharing a label.
Neither a code address nor a C++ object's layout can be inferred from that ID.
In a polymorphic C++ frame the first field may be its vtable, not `_label`.

## The state machine's actual states and transitions

Three separate pieces of state participate in suspension:

- The generated frame's `label` records **where execution will resume**.
- A suspend operation's atomic decision records **whether its result is returned
  immediately or delivered by resuming the continuation**.
- The operation's completion state records **its value, failure or cancellation**.

These are different fields with different transitions. `COROUTINE_SUSPENDED`
is the returned suspension marker; it is not a resume address or a failure.
The names below explain execution stages; they do not introduce a new runtime
enum or replace Kotlin's fields.

### Generated frame: start and one resume address per suspension point

Kotlin/Native's generated `label` has type `NativePtr`. Its initial null value
selects the start block. Every `IrSuspensionPoint` has its own resume block inside
the same generated `invokeSuspend` function. The corresponding C++ field is
`void* _label`, initially `nullptr`.

| Stored label | Entry destination | Work performed there |
|---|---|---|
| Null | `start` | Check whether the incoming `Result` contains a failure, then begin the original function body. |
| `blockaddress(invokeSuspend, resume_i)` | `resume_i` | Restore the locals saved for point `i`, then consume the incoming `Result` with `getOrThrow`. Continue after the suspended call. |

The non-null entry path goes through `dispatch`, whose `indirectbr` lists the
actual resume blocks belonging to this function. There is no fixed universal
set of numbered states: each generated function has the points required by its
own body. A point inside a loop can be reached repeatedly with the same address
and different saved locals. Functions with only eligible tail suspend calls do
not acquire a state machine merely to forward a continuation.

For each suspension point, Kotlin constructs two paths and a common result
block:

1. **Calling the operation:** evaluate its inputs in source order. Save the
   required locals and store this point's resume address as late as possible,
   before the operation is called. Give the operation the current frame's
   continuation, so its eventual completion returns to this frame.
2. **Immediate result:** if the operation returns an ordinary result, use that
   result and continue through the common result block. Do not execute the
   resume-only restore/result code.
3. **Suspended result:** if the operation returns `COROUTINE_SUSPENDED`, return
   that marker from the frame immediately. The frame, its saved address and
   its required live values must survive; the following body statements have
   not executed yet.
4. **Resumed result:** a later `resumeWith(Result)` enters the same frame.
   Dispatch jumps to the saved resume block, restores the saved locals, and
   gets the successful value or throws the supplied failure. It then reaches
   the same common result block as the immediate path. It does not call the
   suspended operation again. A Unit result still checks for failure before
   discarding a successful value.
5. **Next point or completion:** following body statements can prepare another
   suspension point, return a final result, or throw. The next point stores its
   own address before its operation, even if that operation completes immediately.

Saving an address happens before knowing whether the operation will suspend.
Therefore a non-null label does **not** by itself mean the frame is waiting: the
body may already be running past that point on an immediate result. This lowering
does not define a special `RUNNING`, `DONE`, `FAILED` or `CANCELLED` label, nor
reset the label to null after every immediate result or on termination.

### Example: two suspension points

```kotlin
suspend fun total(): Int {
    val first = readFirst()
    val second = readSecond(first)
    return first + second
}
```

For this body the frame can enter at `start`, `resume_first` or `resume_second`.
The names here describe generated blocks, not integer state numbers.

| Event | Saved address and execution |
|---|---|
| First entry | Null selects `start`. Prepare `readFirst`, save `resume_first`, and call it. |
| `readFirst` completes immediately | Assign its returned value to `first` and proceed to `readSecond`; do not enter `resume_first`. |
| `readFirst` suspends | Return the marker with `resume_first` saved. On resume, obtain `first` from the incoming successful `Result`. |
| Reach `readSecond` | Preserve `first`, which is needed for the final sum. Save `resume_second` before calling `readSecond(first)`. |
| `readSecond` suspends | Return the marker. On resume, restore `first` and obtain `second` from the incoming successful `Result`. |
| `readSecond` completes immediately | Use its direct result as `second`; do not run the resume-only restoration. |
| Both results available | Return `first + second`. The last saved address may still be present; it is not a completion flag. |

Either operation can complete immediately or suspend independently. A resumed
failure is thrown at that point rather than used as `first` or `second`; ordinary
source exception handling and cleanup then apply. A handler can itself suspend,
in which case its suspension point becomes the next saved resume address.

### Operation decision: completion before or after suspension

`CancellableContinuationImpl` packs its decision and a cancellation-segment index
into `_decisionAndIndex`. The source decision constants are `UNDECIDED = 0`,
`SUSPENDED = 1` and `RESUMED = 2`. They are not the frame's resume labels.
The decision occupies the high bits (`decision << 29`); the low 29 bits hold
the segment index, with `NO_INDEX = (1 << 29) - 1`. Decision updates preserve
that index. These numbers encode the operation decision, never a code address.

| Decision | Event | Transition and action |
|---|---|---|
| `UNDECIDED` | `getResult` wins `trySuspend` | Change to `SUSPENDED`; return the suspension marker after the required parent/reusable-continuation setup. |
| `UNDECIDED` | Completion wins `tryResume` | Change to `RESUMED`; keep the outcome for `getResult` to return or throw without separately dispatching the delegate. |
| `RESUMED` | `getResult` calls `trySuspend` | No decision change; consume the already published outcome, including the source cancellation check. |
| `SUSPENDED` | Completion calls `tryResume` | No decision change; dispatch the completed outcome to the delegate. |

In particular, this decision machine does **not** change `SUSPENDED` to `RESUMED`
when later completion is dispatched. The separate completion state prevents
ordinary repeated completion; the decision selects the delivery route. The
source reusable reset path can initialize the decision to `UNDECIDED` for a new
operation, subject to its reset conditions.

`SafeContinuationNative` uses a different atomic result slot. Its transitions are
`UNDECIDED` to a stored outcome when completion wins first, or `UNDECIDED` to
`COROUTINE_SUSPENDED` when `getOrThrow` wins first. Completion after suspension
changes that slot from `COROUTINE_SUSPENDED` to `RESUMED` and calls the delegate.
`getOrThrow` seeing `RESUMED` still returns the suspension marker, because the
delegate has already received the outcome. Do not copy that slot transition
into `_decisionAndIndex` or into the frame label.

### Completion, failure and cancellation

`CancellableContinuationImpl._state` starts as `Active`. A cancellation handler
or registered segment can be stored while it remains `NotCompleted`. Successful
completion publishes the value, sometimes wrapped in `CompletedContinuation`
to retain handler/cancellation/idempotence information. Failure publishes
`CompletedExceptionally`; cancellation publishes `CancelledContinuation`.
The decision above determines how that outcome reaches the generated frame.
Prompt cancellation can replace delivery of a successful value with the Job's
cancellation exception according to the source `getResult`/dispatch rules.
Cancellation does not select a separate generated resume label.

On resumed entry, `BaseContinuationImpl.resumeWith` calls `invokeSuspend`:
another suspension marker stops its loop without releasing interception. A final
value becomes `Result.success`; an escaping exception becomes `Result.failure`.
It then releases interception and sends the outcome to the parent continuation,
unrolling parent frames in the same loop where possible. The C++ direct entry
returns its immediate result to its caller; `start` releases interception on
termination without also resuming the parent. The native runtime's
`CompletedContinuation` object marks released interception; it is distinct from
the library's `CompletedContinuation` outcome wrapper and from the saved label.
A retained label is not permission to resume a terminated frame again.

Source references under the local checkouts:

- `tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:55-90,119-170,253-335`
  — frame creation, start-body failure check, save, suspension test and resumed result.
- `tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/CoroutinesVarSpillingLowering.kt:46-105`
  — fields and per-point save/restore of the source-selected variables.
- `tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2281-2340`
  — start/dispatch, declaration-to-block-address lookup and normal/resume merge.
- `tmp/kotlin/kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:21-49,109-127`
  — continuation loop, termination and completed interception.
- `tmp/kotlin/kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/SafeContinuationNative.kt:24-57`
  — atomic result-slot transitions.
- `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:9-20,52-80,146-160,201-217,269-337,467-472,474-524`
  — decision/index, completion state, reusable reset, result and dispatch rules.

The label and result paths described above are required contracts. Current
injection and explicit C++ frames implement tested portions; complete translation
of the Kotlin IR scopes, spilling and shared Kotlin/C++ frame/runtime layout
remains unfinished. This section does not claim those missing pieces are implemented.

## Kotlin source contracts

Ground truth is the local source, rather than the text format of Clang's IR:

1. `tmp/kotlin/.../lower/NativeSuspendFunctionLowering.kt:55-90,119-170,253-335`
   decides whether non-tail suspension needs a frame and adds a `NativePtr`
   label. Before calling the suspended operation it saves state and the resume
   address, after evaluating side-effecting arguments. The normal path returns
   `COROUTINE_SUSPENDED` only if the call actually suspended; otherwise it uses
   that call's value. The resume path restores state and uses
   `getOrThrow(resultArgument)`. A Unit coercion may discard the value but must
   still propagate a failure.
2. `tmp/kotlin/.../lower/CoroutinesVarSpillingLowering.kt:68-105` replaces
   `saveCoroutineState` and `restoreCoroutineState` with frame-field stores and
   loads for variables live at each suspension point.
3. `tmp/kotlin/.../llvm/IrToBitcode.kt:2289-2348` creates function-local resume
   blocks and the start/dispatch/result merge. Reading a suspension-point ID
   emits its `blockaddress`; earlier suspend lowering supplied the label store.
4. `tmp/kotlin/kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:21-45`
   runs the continuation loop. A suspension marker stops the loop. A value or
   caught exception becomes the next frame's `Result`; interceptor release
   happens when the frame terminates.
5. `SafeContinuationNative.kt` coordinates resume-before-return with suspension.
   The library's `CancellableContinuationImpl.kt:269-328,467-472` similarly
   chooses immediate completion or dispatch using its atomic decision state.
   The runtime sentinel, decision state, resumed payload/failure, and dispatch
   mode are distinct parts of the handoff. LLVM injection must preserve them.

The vendored `tmp/kotlinx.coroutines/gradle.properties` identifies that library
snapshot as `1.10.2-SNAPSHOT`, with `kotlin_version=2.1.0`. The independently
checked-out Kotlin compiler is `fee29910d8dddd2b1f7b44036c00533cee493351` and its
build defaults name Kotlin/Native `2.5.0-dev-5907` and LLVM 21. This sparse
compiler checkout does not establish which kotlinx.coroutines artifact is
bundled with a compiler distribution.

## Source lowering and continuation routing

The Clang frontend identifies the trailing shared `Continuation<void*>`
parameter by canonical type and position. It does not require the source name
`completion`. This follows the constructor continuation appended by Kotlin's
AbstractSuspendFunctionsLowering.kt:159-185; ordinary named arguments remain
separate frame fields. The current C++ entry still explicitly declares that
shared parameter and returns `void*`. Automatic entry-signature construction
and broader Kotlin suspend-lambda lowering are incomplete.

For an eligible direct tail entry, authoring calls marked
`kxs_implicit_continuation` receive that trailing continuation as their final ABI
argument. The frontend imports the rewritten direct body inside Clang; it does
not allocate a frame solely to supply the argument. For state-machine entries,
the existing call lowering supplies the current frame. Source contracts:
AddContinuationToFunctionCallsLowering.kt:74-99 and
NativeAddContinuationToFunctionCallsLowering.kt:15-22.

`src/kotlinx/coroutines/dsl/Suspend.hpp` supplies the frame label address and
normal/resume result regions. `KotlinxCoroutinePass` generates the dispatch and saved-address
stores using LLVM values and basic blocks. Live state must already be in a retained frame. `coroutine_yield_value`
uses the immediate call value on the normal path and `result.get_or_throw()` on
resume. `coroutine_yield` discards a successful Unit value but still checks for
failure. The compiler markers are declared `noexcept` and have no runtime definitions.

A suspended operation must receive the current frame's continuation so its
completion re-enters that frame. Passing only the frame's parent completion
would skip the frame's remaining work. For example:

```cpp
class ExampleCoroutine : public ContinuationImpl {
public:
    void* _label = nullptr;
    int accumulator = 0;

    explicit ExampleCoroutine(std::shared_ptr<Continuation<void*>> parent)
        : ContinuationImpl(std::move(parent)) {}

    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        accumulator = 10;
        coroutine_yield(this, delay(100, this));
        accumulator += 20;
        return &accumulator;  // Example only: caller must retain this frame.
    }
};
```

C++ locals and objects whose lifetimes cross suspension require explicit frame
storage and valid lifetimes. The macros do not implement automatic spilling,
RAII lifetime lowering, or the compiler's argument-evaluation rewrite.

## Mandatory LLVM injection pipeline

`__kxs_coroutine_begin(ptr label_field)` supplies the exact persistent frame field.
`__kxs_suspend_point(i32 id, ptr label_field, ptr blockaddress)` supplies the function-local resume
block. The integer ID is tooling information, not a substitute for an LLVM value.

The LLVM module pass runs at Clang’s pipeline-start extension point, before
optimization and sanitizer instrumentation:

1. Verifies Clang’s in-memory module and validates marker signatures and direct uses.
2. Rejects missing entry information, stack/null label storage, foreign resume
   addresses, duplicate destinations and unsupported marker uses.
3. Splits the entry-marker block after the frontend's argument/result prologue.
4. Loads the saved address from the supplied field and emits null-to-start versus
   non-null-to-dispatch branches and `indirectbr` with actual resume destinations.
5. Replaces each suspension marker with its actual `blockaddress` store to that
   field, preserving immediate/suspended/resumed result and failure regions.
6. Verifies the transformed module and returns it to Clang’s optimizer. Invalid
   dominance or unsupported markers produce compiler errors.

No stack label slot is allocated or initialized by the injector. Initial null
belongs to frame construction; the field persists across suspended returns.
No frame offset or continuation argument index is guessed. Frame layout, live
state storage, ownership and result regions remain explicit frontend inputs.
Automatic spill generation and direct Kotlin/Native binary handoffs still need
separate compiler/runtime integration evidence; injected address dispatch alone
is not evidence of completion of those docking-ring requirements.

`kxs_enable_coroutine_transform(target)` adds `-fpass-plugin` to the ordinary
Clang compile command and makes the plugin a build dependency. Clang performs
frontend lowering, coroutine injection, optimization and code generation in one
process using its own LLVM values. CMake retains target/source settings, object
paths, header dependencies and existing compiler launchers. Plugin changes also
rebuild affected objects. No Python compile launcher or intermediate IR files
are used by the production build.

Build and load the plugin with the same LLVM development package as the selected
Clang. The in-tree and installed-package integrations check the version. Installed
callers can select `KXS_LLVM_PASS_PLUGIN` explicitly. The tested local toolchain
is Homebrew Clang/LLVM 23.1.2. Loading that plugin into Apple Clang 21 is unsupported;
build it against the selected compiler’s LLVM package instead. The Kotlin compiler
snapshot’s LLVM 21 requirements remain separate cross-language integration work.

`kxs-inject` remains a standalone diagnostic driver of the same lowering engine.
Its text and bitcode output use that LLVM package’s native format. The former
cross-version floating-point writer and lifetime-intrinsic rewrite are removed.
Production compilation neither serializes nor reparses the module. Missing the
plugin leaves undefined compiler markers; no runtime marker bodies rescue it.

```cmake
include(KotlinxCoroutines)
add_executable(my_target my_source.cpp)
target_link_libraries(my_target PRIVATE kotlinx::coroutines)
kxs_enable_suspend(my_target)
```

## Verification

```bash
cmake -S . -B build -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang -DKOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=OFF -DKOTLINX_BUILD_KXS_INJECT=ON -DLLVM_DIR=/opt/homebrew/opt/llvm/lib/cmake/llvm
cmake --build build --target KotlinxCoroutinePass kxs-inject test_suspension_core
ctest --test-dir build -R '^(test_suspension_core|test_kxs_compiler_pass|test_ir_pipeline|test_kxs_inject)$' --output-on-failure
```

`test_ir_pipeline` builds and executes the actual macro tests through the native
in-process compiler plugin. It covers immediate and resumed values, failure propagation,
repeated suspension, independent frames, transitive C++20/include/definition
settings, source-specific options, generated sources, duplicate basenames,
paths with spaces, header-triggered rebuilds, and an optimized AddressSanitizer
build. `test_kxs_inject` checks persistent nonzero-offset frame fields, independent
frames, stack-label rejection, malformed inputs and unsupported marker uses.
`test_kxs_compiler_pass` verifies actual Debug and optimized AddressSanitizer
handoffs, retained debug metadata and compile-time rejection of stack label
storage. Ninja
Multi-Config is exercised when Ninja is installed.

`KotlinxSuspendPlugin` now constructs retained frames before Clang code generation
for the tested free-function entries, including nested and inline namespaces. `kxs_enable_suspend_dsl`
enables it together with mandatory LLVM injection. Parsing and AST import happen
inside the compiling process using the original invocation; the diagnostic
sidecar is not used for ordinary compilation. Both plugin files are object
dependencies. Constant complete array locals retain their native types in aligned
storage, with construction at the original declaration and destruction on scope
exit or failure. Reference slots borrow typed pointers. Frames cannot be copied
or moved, preserving references into retained storage. Template entry rewriting,
and additional control flow remain compiler work.

With the frontend enabled, `kxs_plugin_handoff` compiles original annotated source
through CMake, exercising repeated suspension, moved argument temporaries,
retained locals, failure and destruction. `kxs_kotlin_native_handoff` requires a
local `konanc` and runs actual Kotlin-generated frames in a generated C++ caller
under AddressSanitizer. It checks Kotlin continuation identity at repeated
suspension points, worker-thread resumes, failure, cancellation, single completion
and disposal of each stable-reference handle. Kotlin's emitted resume IR is kept
and checked for native saved-address dispatch. The local compiler is Kotlin/Native
2.4.20; the C++ plugins use Clang/LLVM 23.1.2. Their C interop boundary does not
depend on reparsing one compiler's LLVM output with the other compiler.

These checks establish the tested frame and C interop contracts. Interchangeable
object layouts, arbitrary Kotlin object payloads, full temporary/exception
lifetime lowering and upstream dispatcher/Job parity require further work.

Structured `do` loops preserve body-before-condition ordering. A lowered
`continue` destroys body locals and reaches the condition; `break` destroys them
and exits. Each generated conditional branch encloses all its emitted cleanup
and control-flow statements. The retained-local fixture checks suspension in
both the body and condition with immediate and deferred results.

Source switches preserve initializer and condition-variable lifetimes across
suspension. Case labels and defaults retain source ordering and fallthrough.
Break exits the nearest loop or switch; continue finds the enclosing loop and
clears all exited scopes, including switch initialization. Tests exercise
condition and case suspension, grouped cases, fallthrough, both exit forms,
and failures in the condition and a resumed case. Resume dispatch remains
the injected persistent-label/blockaddress/indirectbr contract.

Range-based loops retain their implicit range and begin/end declarations,
comparison, increment and element variable. References borrow the retained
element storage. A directly lifetime-extended temporary constructs in aligned
storage at the original declaration and is destroyed when the range scope exits,
including failure. This supports the verified non-copyable, non-movable range.
Continue destroys iteration locals before incrementing; break also clears the
range scope. Implicit iterator operations containing suspension are rejected;
nested temporary/subobject and initializer-list backing-storage lifetime
lowering still require implementation and validation.

Helper AST ownership lasts for the owning compilation's AST-context lifetime.
Imported attributed types can reference attributes stored in that helper AST;
destroying it at the end of one frame import invalidated subsequent compilation.

If initializers and condition declarations now have a retained scope enclosing
both branches. While condition declarations have an iteration scope and are
destroyed on continue, break, false evaluation or normal iteration completion.
For condition declarations survive the increment and are destroyed before the
next condition evaluation. Continue destroys body scopes while retaining the
condition through that increment; break destroys both. A regression verifies
non-copyable condition objects and reference arguments across a suspended
increment, false loop conditions, and cleanup on resumed increment failure.

Built-in comma expressions now sequence the left operand before any suspension
in the right operand and preserve the right operand's lvalue category. Discarded
record temporaries use retained delayed storage, including non-copyable values,
and are destroyed at the enclosing full-expression boundary. Condition values
are retained before this cleanup; returned outcomes are evaluated before
temporary cleanup and then local cleanup. Tests cover nested sequencing,
reference binding, suspension with live discarded objects, condition/return
boundaries, and immediate/resumed failures. General temporary lifetime and
overloaded-operator lowering still need implementation and verification.

When a built-in assignment contains a suspend call, lowering evaluates and
retains the destination before calculating what will be stored there. This
follows Kotlin's receiver-before-value order in
`NativeSuspendFunctionLowering.kt:212-250`, where `IrSetField` children are
processed as receiver, then value. For example:

```cpp
find_person().score = calculate_score();
```

First, `find_person()` selects the person whose score will change. Next,
`calculate_score()` produces the number. Finally, that number is stored in the
selected person's score field. If calculating the score suspends, the frame
retains the selected destination so resumption updates that same person. A
failure before the store leaves the field unchanged.

The C++ lowering retains a reference to the destination; Kotlin's `IrSetField`
retains the receiver. Temporary C++ receivers have owning storage until the full
expression completes or fails. Plain and compound assignments preserve their
lvalue result. Subscript operands are retained in syntactic left-to-right order,
including reversed `index[pointer]` syntax. Existing regression fixtures cover
receiver side effects, suspension in both sides, reference identity,
plain/compound mutation and temporary-receiver cleanup. These checks do not
establish complete Kotlin IR or frame-layout parity.

Conditional glvalue results now retain references rather than copying selected
objects. Lvalue identity and xvalue move eligibility survive suspension.
Canonical reference types avoid context-dependent standard-library aliases in
generated frame declarations. Tests check both branches using non-copyable
objects and move-only pointers, selected-object mutation, empty moved sources,
single completion and cleanup on immediate/resumed failure. Bitfields and
broader prvalue temporary/overloaded-operator rules need separate validation.

Void conditional expressions now lower to branch execution without a result
slot. Only the selected arm executes; discarded comma temporaries remain alive
across suspension until the full expression completes. The regression checks
both branch side effects and temporary/frame cleanup on success and resumed
branch failure under the same mandatory LLVM address injection.

By-value parameters transfer ownership into the frame through moves. Stored
parameter fields preserve source constness while construction uses an unqualified
transfer value. Lvalue and rvalue reference parameters retain caller-object
identity and remain borrowed; their owners must retain them across suspension.
The regression checks const move-only ownership, non-copyable borrowing,
rvalue-reference identity, mutation after resume, and destruction on success
or immediate/resumed failure with sanitizer recovery disabled.

Referenced constructor/destructor definitions are imported transitively.
Constructor member initializers are imported together with bodies. Header-local
identity includes the owning function signature and class specialization so
template instantiations cannot reuse each other's locals.

Member receivers evaluate before suspended arguments. Borrowed object receivers
preserve identity, pointer receivers preserve their evaluated value, and owned
temporary receivers survive through completion of the full expression, including
suspension inside the member callee. Calls preserve lvalue/rvalue qualification.
Single-return annotated tail member entries forward the existing continuation;
non-tail member definitions now generate a frame
inside the original member scope. Its typed receiver pointer preserves constness
and private-member access, and remains borrowed from the caller. Implicit member
references and explicit `this` expressions use that saved receiver after resume.
The regression covers repeated suspension, const/lvalue/rvalue-qualified overloads,
private calls with suspended arguments, nested member frames on a retained
temporary receiver, and cleanup on immediate/resumed failure. Generic header
entries retain type parameters inside a local frame; template-parameter identity,
constructor initializer source flags and dependent frame method instantiation are
preserved during AST import. The ownership regression covers typed and deduced
`auto` locals with unique and shared ownership. Dependent placeholder storage
uses unevaluated deduction resolved by the owning Clang instantiation, with
stand-ins based on declared types so nested storage does not depend on unfinished
frame-field instantiation. Reference and pointer regressions verify identity,
mutation, const borrowing, forwarding-reference deduction, and the distinction
between parenthesized and unparenthesized `decltype(auto)` declarations. Unresolved free-function lookup recognizes DSL wrappers and candidate sets
whose available declarations are all annotated suspend entries. Dependent
argument storage retains deduced types and lvalue identity until the instantiated
callee applies its own conversions. The generic regression calls an annotated
function template with a non-copyable borrowed object through immediate/deferred
completion and failure. Class-template const members and member function
templates now retain the enclosing class and function type-parameter contexts,
private member access and borrowed receiver identity. Type-only receiver
expressions use the declared pointer type instead of referring to frame fields
before instantiation. Regressions cover both in-class and out-of-class member
function template definitions with unique/shared receiver ownership and move-only
locals. Unresolved annotated member-template candidates now participate in
suspension recognition. Explicitly marked calls through completely dependent
member names retain their receivers before evaluating arguments. Receiver
storage resolves reference/value ownership at instantiation and preserves
lvalue/rvalue qualification; owned non-copyable prvalues use direct delayed
construction. Regressions exercise borrowed and owned generic receivers across
suspended arguments and suspended member callees, with failure at either site.
Free template bodies that have not yet been lowered can resolve a mixed
annotation candidate set at instantiation. A separate in-memory concrete helper
supplies the selected specialization's frame body without replacing the primary
template. Canonical types and actual integral substitution expressions preserve
template argument values and `decltype` behavior. Concrete frame identities
include the mangled function identity so distinct constants cannot share a frame
body and the same specialization stays stable across translation units. The
regression checks both selected overloads and two integral constants from two
source files. Primary function templates and dependent class methods with mixed annotated/ordinary overload
candidates, type-dependent calls without known candidates, or calls requiring
argument-dependent lookup defer their entire body until instantiation, including when an earlier
call is already known to suspend. This prevents that earlier lowering from hiding
a later selected suspension. Regressions cover success and failure at either
site, retained local ownership, and selection of the ordinary overload. A visible
candidate set is not final when argument-dependent lookup is required: an
argument's namespace can supply a different suspend or ordinary overload.
Referenced template specializations are instantiated in the owning Clang Sema
when their bodies are absent and submitted for code generation when concrete.
Late-resolved member function templates and methods of class templates use a
concrete member specialization in the in-memory helper. Its lexical class context
preserves private access and cv/ref qualifiers; imported bodies remain attached
to the original owning method. The primary body stays unchanged for other
specializations. Member-template type aliases live inside that specialized body.
Generated receiver constructor parameters use a compiler name so a source
parameter named `receiver` remains valid. Regressions cover private state and
`const &` methods with either suspend or ordinary selected receiver methods.
Nested class contexts retain their enclosing record chain while the helper opens
the enclosing namespaces. Instantiated class scopes contribute specialization
prefixes; the nested regression retains an enclosing private type alias and
integral template argument. Concrete helpers print Clang's instantiated exception
specification. Boolean `noexcept` arguments exercise both declarations, and a
separate process verifies termination when an immediate exception escapes a
`noexcept(true)` entry. Resumed failures still travel through the continuation.
An explicitly specialized nested owner ends the enclosing class specialization
prefix chain. Its member function template retains its own prefix; the regression
checks both selected receiver methods with private state inside that owner.
Pointer, reference and null non-type arguments are retained using Clang's actual
substitution expression and substituted parameter type. Symbol expressions use
their resolved declaration's qualified scope. Reference parameters retain their
reference type in `decltype` and address identity; no replacement runtime variable
is created. Regressions mutate the referent during suspension and verify the
resumed read, initial local value and exact parameter type. For undeduced `auto`
non-type parameters belonging to the function specialization, the helper recovers
the concrete type from Clang's actual specialization argument. Class-owned
parameters use the corresponding enclosing class specialization. Recovery checks
both the parameter index and its declaration identity, preserving distinctions
between nested template owners. Function-pointer
and function-reference targets retain their resolved suspend annotations;
regressions check exact `decltype`, ordinary target selection and retained locals.
Broader dependent
exception expressions, packs, structural values and other declaration argument
forms still need integration and validation.

Ordinary included header definitions use an in-memory replacement of their
source header while parsing its original including context. Header free entries
retain frames inside the original function body, preserving inline linkage.
The importer corrects subsequent declaration offsets and preserves implicit
inline on generated frame methods before code generation. The regression compiles
the same header in two translation units and exercises an inline free function
from both files and a const member function through immediate/resumed success
and failure. Command-line forced includes use the cloned invocation's preinclude
context before parsing any main-file declarations. The regression also compiles
and executes the same two-file program with `-include`, verifying header lowering
and retained-local cleanup in both compilation paths. Macro-defined function
bodies still require support.

Try/catch regions now preserve typed native exception selection for immediate
and resumed failures. The failure check at a resume label remains inside its
protected region. Retained protected locals are reset before handler execution.
Native catch dispatchers save an `exception_ptr`, bind reference catch variables
or copy value catch variables once into retained storage, then leave the catch
before running the resumable handler body. Catch-variable copy failure invokes
`std::terminate`; variable destruction precedes releasing the retained exception.
Exception-bearing frames now execute their body through a native catch-context
wrapper. Handler entry/exit stores an injected blockaddress and returns to the
wrapper, which leaves the previous catch and restores the active retained
exception before calling the body again. Actual resume dispatch remains in the
body's LLVM `indirectbr`. Nested handlers retain the previous active exception;
handler exits restore it before executing the following region. Direct and helper
uses of `std::current_exception()` and bare rethrow retain native behavior, so
source-expression substitutions for these operations have been removed.

Regression cases verify protected cleanup, matching among multiple typed catches,
reference identity, handler suspension/failure, nested rethrow to a catch-all, and
exact value-catch copy/lifetime counts. They execute in ordinary and forced-include
builds with address and undefined-behavior sanitizers. Additional cases verify
helper rethrow, restoration from nested handlers, context clearing after handlers,
and `break`/`continue` exits. The context wrapper does not choose resume labels;
all transitions use the mandatory injected field/blockaddress mechanism.

Cleanup now restores native context between nested handler groups. Protected
storage is destroyed after leaving the selected catch dispatcher, before entering
its handler and constructing any value-catch variable. Source return values are
retained while scope cleanup crosses context boundaries. Unhandled failures save
their exception and enter a separate injected cleanup region outside the body's
catch before propagating the saved failure. Departing exceptions remain retained
until the surrounding context has been restored; their release precedes body
address dispatch in that context.

Destructor observers reproduced failures for both nested locals and exception
objects before these corrections. Regression cases now verify their observed
`current_exception` on normal exit, return, break, continue and resumed failure.
The expanded cases run in ordinary and forced-include builds under the sanitizers.
This establishes the tested destruction/context behavior, not full Kotlin runtime
or compiler parity.
