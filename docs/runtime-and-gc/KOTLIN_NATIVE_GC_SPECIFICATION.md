# Kotlin/Native reference and root boundary

The production target is Kotlin/Native on bare metal. The compiler may run on a
host computer. A JVM program cannot establish the target object, continuation,
collector or coroutine-frame ABI. Host Native tests provide bounded evidence;
they do not establish execution on a bare-metal target.

## Source authority

Compiler/runtime source is pinned to
`fee29910d8dddd2b1f7b44036c00533cee493351` in `tmp/kotlin`.
The consumed runtime is already C++: its actual declarations and root-holder
algorithm are preserved in `src/kotlinx/coroutines/KotlinGCBridge.hpp` and `.cpp`,
with per-file port-lint and per-function source ranges. This is the runtime
boundary needed by Kotlin-generated object references; it is not an alternative
coroutine implementation.

| Pinned source | Required behavior |
|---|---|
| `kotlin-native/runtime/src/main/cpp/Memory.h:180-234` | Real allocation, global registration, stack/heap updates, reference atomics, result slots and frame entry/exit. |
| `Memory.h:254-264` | Actual Native/Runnable transitions and compiler safe-point entry points. |
| `Memory.h:272-311` | Actual FrameOverlay layout and ObjHolder algorithm. |
| `kotlin-native/runtime/src/main/cpp/mm/Memory.cpp:105-205` | Native runtime allocation, barriers, seq_cst reference compare/exchange, root slots and shadow-stack operations. |
| `kotlin-native/runtime/src/main/cpp/mm/ReferenceOps.hpp` and `.cpp` | Source reference accessors and collector hooks. |
| `kotlin-native/runtime/src/main/cpp/mm/ShadowStack.hpp` and `.cpp` | Per-thread frame chain and scanning of registered object slots. |
| `kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/Annotations.kt` | Actual ExportForCppRuntime and GCUnsafeCall compiler boundary. |
| `kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/CodeGenerator.kt:734-778` | Object loads retained in result/stack slots and actual source store/update operations. |

## Mandatory runtime operations

The declarations have strong linkage. A Native consumer must link the actual
runtime definitions. There are no inline no-op definitions, weak optional symbols
or availability predicates. `KOTLIN_NATIVE_RUNTIME_AVAILABLE` now enables the
optional Native host test; it does not change production reference semantics.
Disabling that test does not supply a substitute collector.

`ObjHeader` and `TypeInfo` remain opaque identities owned by Native. Neither a
C++ smart pointer nor an ordinary C++ allocation is a Native object. Native
object-returning runtime functions take the caller's final `ObjHeader**` root
slot as well as returning `ObjHeader*`. Every adapter must preserve that slot;
returning only an unrooted pointer is insufficient.

Reference compare-and-set, compare-and-exchange and exchange call the actual
Native runtime operations. Those operations use source reference accessors and
collector hooks, with sequential consistency. Pointer atomics cannot stand in
for them. Mutable object loads also require the compiler's real stack/root
handling; there is no invented exported heap-load helper.

## Stack-root lifetime

`FrameOverlay` stores the previous frame and the source parameter/count fields.
`ObjHolder` stores that overlay followed by one object-reference slot. Its default
constructor initializes the slot to null and enters a frame. Its object-taking
constructor enters a frame and calls UpdateStackRef. Its destructor calls
LeaveFrame. Clearing the holder calls ZeroStackRef. These bodies are the pinned
source algorithm, split into the repository's header/implementation layout.
They are required lifetime management, not temporary proof guards.

Object access and frame operations require a registered Runnable Native thread.
Switching into Native state is only appropriate while code does not access Native
objects. The exported state-switch functions do not encode a nesting policy.
The removed wrapper did not remember the incoming thread state; that wrapper's
claims about nested transitions were unsupported. Source ThreadStateGuard and
its old-state restoration are a separate real runtime dependency, not presently
translated here.

## Current execution evidence: 2026-10-06

The optional `native_reference_contract` CMake target builds a macOS ARM64 Native
executable with explicitly supplied Kotlin/Native and its matching LLVM Clang.
C++ bitcode is linked through Kotlin/Native's `-native-library` input. This is
verification of the runtime ABI; production coroutine compilation still uses
the mandatory in-process KotlinxCoroutinePass, without an IR text launcher.

The fixture creates real Native objects and observes WeakReference after forced
GC. Assertions exist only in the fixture. Ten contract groups cover stack-root
retention/release, nested frame links/restoration, C++ exception unwinding,
failed compare-and-set, failed compare-and-exchange return identity, successful
compare-and-set, exchange return-root retention/release, and actual thread-state
and safe-point calls. An additional Native caller receives an object through
UpdateReturnRef, forces collection after the C++ holder exits, and reads the
object's original field. Execution prints:

```text
native-roots=10
native-return-slot=1
```

The atomic-operation locations in this fixture are actual shadow-stack root
slots. This verifies operation results and root lifetime, not heap-field layout,
heap/global registration or concurrent Native AtomicReference behavior. The
fixture uses installed Native 2.4.10, not a compiler/runtime built from the pinned
revision. Its internal-annotation compiler warnings remain recorded.

A real global-registration link attempt exposed a source/runtime mismatch:
the pinned source declares RegisterGlobal; the installed runtime's mm bitcode
defines InitAndRegisterGlobal instead. The pinned declaration remains unchanged.
Global-root acceptance requires a matching pinned runtime. No alias or synthesized
runtime implementation has been introduced. Compiler source and installed runtime
versions must be checked together before broader ABI claims.

The former test directory contained an empty interface target and a script that
changed to standalone C++ after Native compile/toolchain failures. Those paths
are replaced by the real Native executable. Missing tools, compilation failures
and link failures produce nonzero exits; none executes a substitute fixture.
The test README records the exact command and host scope.

Receipts, 41 pinned source ranges, build/test logs and runtime symbol diagnostics
are under `build/ir-recovery/native-references/`. Current project-wide parity and
repair order come from [both mandatory deep reports](../audits/project-wide/README.md).
The Kotlin-only scanner does not measure this C++-origin runtime unit as a Kotlin
pair; source-range checks and execution do not waive other oracle criteria.

## Native array reference boundary: 2026-10-06

The source array boundary now also exists in
`tools/kotlinc_native_ref/kotlin/collections/NativeArrayUtil.hpp/.cpp` under the
coroutine source root. Runtime get/set/length/fill/copy declarations have strong
linkage and preserve the actual ObjHeader pointers. Object get has the final
caller result slot. The two source reset bodies call Kotlin_Array_set and
Kotlin_Array_fillImpl with null. They do not inspect guessed object layouts,
convert to C++ arrays, or replace the source heap write barrier.

The linked `native_array_contract` fixture executes copies between real Native
arrays and within one array in both overlap directions. Kotlin observes object
identity after C++ writes, resets and fill. C++ get updates the Native result slot;
its caller retains the result after a separate creation frame clears the array and
exits, followed by GC. C++ reset clears the last array reference, verified with a
Native WeakReference. This adds bounded array heap-write/reference evidence;
non-array heap fields, global registration and concurrent reference objects are
still unverified. Assertion/observation code belongs only in test fixtures.

The same fixture compares 862 array utility observations with compiler-owned C++
storage. Compiler arrays use C++ ownership, not ObjHeader/ArrayHeader or GC layouts.
Implementation-dependent uninitialized reads are explicitly grouped with null in
that comparison; no equality of the two storage layouts is claimed. Installed
Native 2.4.10 source bodies match the consumed pinned utility bodies. Its runtime
version, macOS host and internal annotation warnings remain explicit limits.

Receipts are under `build/ir-recovery/native-map-dependencies/`. Both full-root deep
reports retain low similarity, provisional zero logic and incorrect primitive
function matches. A matching pinned runtime and actual bare-metal shared coroutine
frames remain required. Production coroutine compilation continues to use the
in-process LLVM pass; this test's native-library inputs do not change that pipeline.

## Pinned Native build discovery: 2026-10-06

The offline :kotlin-native:dist --dry-run probe with Native enabled and Xcode
validation requested exited 1 in settings configuration: the exact source included
build repo/kotlin-build-helpers is absent from the sparse checkout. No compiler or
runtime compilation started. Pinned source documents Xcode 27 for the macOS host
build; this host has 26.6. The probe stopped before checking that requirement.
Receipts and the unchanged relevant source build definitions are in
build/ir-recovery/pinned-native-build/.

Complete the pinned build dependency/toolchain closure, then compile and link that
actual runtime. Installed Native 2.4.10 and its LLVM 21 remain the bounded host-fixture
toolchain only. No symbol alias, older ABI assertion, toolchain check suppression or
host success substitutes for matching source and actual bare-metal execution.
The real Native Kotlin fixture is now explicitly included by .gitignore's narrow
exception; the global *.kt ignore rule had hidden this required test source.

## Remaining requirements

- Build and link a runtime from the pinned compiler revision; verify global roots,
  source heap-field layout, barriers and initialization on actual Native storage.
- Translate Native AtomicReference, real CurrentThread Any identity, reentrant Lock
  and Lazy algorithms and factories before accepting the parameter lazy delegate.
- Complete real IR declaration ownership/binding and suspension scopes, preserving
  object result slots in VariableManager and code generation.
- Translate and execute spills, cleanup, resumed results and direct Kotlin/C++
  use of the same Kotlin-generated coroutine frame/state machine.
- Establish the actual bare-metal toolchain/runtime target and execute there.
  The macOS fixture supplies no OS-free runtime, scheduler or allocator claim.

These requirements remain open on the existing compiler cards. No whole-runtime,
whole-project, performance, leak-free or complete docking-ring claim follows from
the bounded reference fixture.


## Native integer-array boundary: 2026-10-06

NativeArrayUtil.hpp declares the actual strong Kotlin_IntArray get/set/getArrayLength/
fillImpl/copyImpl symbols from pinned runtime Arrays.cpp:519-590. Primitive get
returns KInt directly; it does not use the object getter's caller result slot.
The host fixture retains actual Native arrays through the translated ObjHolder
while calling the actual runtime operations. Kotlin observes mutation of its
original arrays, including both overlap directions. No Native array is converted
to compiler-owned C++ IntArray storage, and no substitute runtime is selected.

Compiler-owned IntArray has explicit fixed-length C++ slot ownership and is not a
Native ArrayHeader layout claim. The installed Native 2.4.10 fixture verifies the
bounded declared ABI on macOS ARM64. Matching pinned runtime construction, allocator/
GC target integration, Kotlin/C++ shared continuation frames and bare-metal execution
remain required. Source/algorithm/ABI evidence and limits are recorded in
build/ir-recovery/native-int-array/ and the IR identity dependency ledger.
