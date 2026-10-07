# Architecture Documentation Index

This directory contains the foundational architectural documents, design specifications, and strategic roadmaps for the `kotlin.coroutines-cpp` project.

---

## 🏛️ Architecture Documents

### [ir_identity_and_scopes.md](ir_identity_and_scopes.md)
**Purpose:** First implementation priority for compiler card `t_16bf1579` and
umbrella `t_1834dcec`: translate actual Kotlin IR declaration identity and
suspension scopes.
**Contents:**
- Pinned Kotlin sources, sparse-source coverage and dependency ledger
- Symbol binding, declaration identity, ordinary variable storage and scope delegation
- Suspension-point lookup, production compiler integration and captured-field rewrites
- Implementation order, executable acceptance checks, required project-wide deep reports and unfinished downstream work

### [porting_north_star.md](porting_north_star.md)
**Purpose:** Authoritative technical playbook for porting Kotlin `kotlinx.coroutines` and Kotlin/Native coroutine lowering to C++.  
**Contents:**
- Mission and invariants: Kotlin/Native ABI fidelity for suspend functions, semantic parity via transliteration parity, and state machine mirroring
- Ground truth sources and mapping rules (`tmp/kotlinx.coroutines/**.kt` and `tmp/kotlin/**`)
- Path mirroring conventions: `include/kotlinx/coroutines/<mirrored tree>/X.hpp` and `src/kotlinx/coroutines/<mirrored tree>/X.cpp`
- Kotlin/Native lowering model and pipeline stages (NativeSuspendFunctionLowering, CoroutinesVarSpillingLowering, IrToBitcode)
- Tooling contracts: required in-compiler LLVM lowering (`KotlinxCoroutinePass`, `-fpass-plugin`); standalone `kxs-inject` diagnostics

### [docking_ring.md](docking_ring.md)
**Purpose:** Architectural blueprint forming a "docking ring" between the Kotlin/Native runtime and C++.  
**Contents:**
- Technical goals: Kotlin/Native runtime (GC + coroutine state machine lowering + continuation API), C++ ABI target (MLX / GPU), freethreaded Python embedding
- Ground truth investigation in Kotlin/Native compiler IR nodes (`IrSuspensionPoint`, `IrSuspendableExpression`, `IrToBitcode.kt`)
- Suspend DSL design and Clang compiler plugin requirements
- Lowering pipeline, indirect branch / jump-table dispatch, and ABI boundary specifications
- In-compiler address lowering, continuation handoff verification, and remaining ABI/spill requirements

### [coroutines_primitives_north_star.md](coroutines_primitives_north_star.md)
**Purpose:** Fundamental primitives required to implement Kotlin Coroutines in C++, distinguishing Compiler Intrinsics from Library Code.  
**Contents:**
- Compiler Intrinsics (Kernel): `suspend` transformation, `suspendCoroutineUninterceptedOrReturn`, `COROUTINE_SUSPENDED` sentinel
- Low-Level Library Primitives (Runtime ABI): `Continuation<T>`, `ContinuationInterceptor` ("The Mode"), `startCoroutine` / `createCoroutine`
- High-Level Library Primitives (User Facing): `suspendCancellableCoroutine`, `Job`, parent-child cancellation relations
- Relationship between intrinsics and the Clang compiler plugin

### [IMPLEMENTATION_ROADMAP.md](IMPLEMENTATION_ROADMAP.md)
**Purpose:** Required `ast_distance --deep` workflow and links to generated implementation status, gaps, and repair priorities.  
**Contents:**
- CLI source/target roots and report regeneration
- Generated function/type inventories, transliteration evidence, and priority ladder
- Required implementation criteria with no TODOs or stubs
- Architectural contracts and behavioral validation evidence

### [research_notes.md](research_notes.md)
**Purpose:** Investigation into Kotlin compiler IR lowering and `Cancellable` start machinery.  
**Contents:**
- Kotlin/Native compiler lowering (`NativeSuspendFunctionLowering.kt`) and `create` hook
- Runtime intrinsics (`IntrinsicsNative.kt` / `createCoroutineUnintercepted`)
- Execution flow for `startCoroutineCancellable`
- C++ mapping using `LambdaContinuation<T>` and `create_coroutine_unintercepted`
