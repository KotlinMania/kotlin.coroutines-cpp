# LLVM code generation contracts

## Source and implementation

`src/kotlinx/coroutines/tools/kotlinc_native_ref/` contains translated
`CodeContext`, `CodeGenerator`, `ContextUtils`, `LlvmUtils`, `Runtime`, callable,
signature, attribute and debug-binding contracts. Their ground truth is the
matching Kotlin/Native LLVM backend and `IrToBitcode.kt` under `tmp/kotlin`.

## Typed contexts and resume addresses

Inner code contexts delegate actual operations to their outer context.
Suspension-point lookup associates the actual `IrVariable` declaration with its
owning LLVM resume block address. Typed suspendable and suspension-point entries
retain source location, scope lifecycle, normal/resume evaluation and phi order.

The production marker-injection adapter uses supplied LLVM operands. The typed
entries are not yet connected to a complete normalized Clang-to-IR expression
driver. Declaration identity must not be reconstructed from names or LLVM shapes.

## LLVM values, signatures and imports

Constants and aggregate types preserve their supplied LLVM types. Shared element
lists retain metadata identity; emitted aggregate values retain the eager source
snapshot. Load/store operations retain ordering and alignment. Builder position,
terminators, conditional branches and debug location ranges are separate state.

Function signatures keep function, return and parameter attributes distinct,
including varargs and explicit object-result metadata. Calls and invokes retain
their actual provider attributes. The private raw-call operation distinguishes
nounwind, caller cleanup and local unwind destinations.

Runtime metadata reads actual supplied LLVM modules, layouts and target data.
Eager and lazy imports preserve symbol signatures, optimization-dependent names,
object-result flags, descriptor identity and retry after failed lazy resolution.
Importing a declaration does not implement or link the Kotlin/Native runtime.

## Storage and missing consumers

Static slot identities are borrowed. Dynamic parameter slots belong to their
creating lifetime, and parameter-index arrays retain their actual shared identity.
Ordinary C++ values preserve native construction, destruction and ownership;
actual Kotlin GC references require the explicitly enabled Native boundary.

Runtime bitcode loading, structurally IR-keyed caches, actual binary classification,
VariableManager consumers, complete frame allocation/root updates, public
object-result calls and exception cleanup remain unfinished. Private helpers and
metadata declarations do not establish these connected consumers.

## Build contract

The frontend and mandatory `KotlinxCoroutinePass` module plugin run inside the
selected Clang using its matching LLVM package. Production builds use
`-fpass-plugin`; they do not serialize/reparse IR through a Python compile launcher.
Standalone C++ library and plugin builds require no Kotlin compiler, JVM or linked
Native runtime. Actual Native interoperability is an explicitly linked boundary.
See [docking_ring.md](docking_ring.md) for both required complete executable paths.
