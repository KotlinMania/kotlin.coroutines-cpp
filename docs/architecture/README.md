# Architecture Documentation Index

This directory contains the foundational architectural documents, design specifications, and strategic roadmaps for the `kotlin.coroutines-cpp` project.

---

## 🏛️ Architecture Documents

### [porting_north_star.md](porting_north_star.md)
**Purpose:** Authoritative technical playbook for porting Kotlin `kotlinx.coroutines` and Kotlin/Native coroutine lowering to C++.  
**Contents:**
- Mission and invariants: Kotlin/Native ABI fidelity for suspend functions, semantic parity via transliteration parity, and state machine mirroring
- Ground truth sources and mapping rules (`tmp/kotlinx.coroutines/**.kt` and `tmp/kotlin/**`)
- Path mirroring conventions: `include/kotlinx/coroutines/<mirrored tree>/X.hpp` and `src/kotlinx/coroutines/<mirrored tree>/X.cpp`
- Kotlin/Native lowering model and pipeline stages (NativeSuspendFunctionLowering, CoroutinesVarSpillingLowering, IrToBitcode)
- Tooling contracts: CMake compiler launcher, marker cleanup boundary (`kxs_compile.py`, `kxs_transform_ir.py`, `kxs_inject`)

### [docking_ring.md](docking_ring.md)
**Purpose:** Architectural blueprint forming a "docking ring" between the Kotlin/Native runtime and C++.  
**Contents:**
- Technical goals: Kotlin/Native runtime (GC + coroutine state machine lowering + continuation API), C++ ABI target (MLX / GPU), freethreaded Python embedding
- Ground truth investigation in Kotlin/Native compiler IR nodes (`IrSuspensionPoint`, `IrSuspendableExpression`, `IrToBitcode.kt`)
- Suspend DSL design and Clang compiler plugin requirements
- Lowering pipeline, indirect branch / jump-table dispatch, and ABI boundary specifications
- Marker cleanup verification and validation limits

### [coroutines_primitives_north_star.md](coroutines_primitives_north_star.md)
**Purpose:** Fundamental primitives required to implement Kotlin Coroutines in C++, distinguishing Compiler Intrinsics from Library Code.  
**Contents:**
- Compiler Intrinsics (Kernel): `suspend` transformation, `suspendCoroutineUninterceptedOrReturn`, `COROUTINE_SUSPENDED` sentinel
- Low-Level Library Primitives (Runtime ABI): `Continuation<T>`, `ContinuationInterceptor` ("The Mode"), `startCoroutine` / `createCoroutine`
- High-Level Library Primitives (User Facing): `suspendCancellableCoroutine`, `Job`, parent-child cancellation relations
- Relationship between intrinsics and the Clang compiler plugin

### [IMPLEMENTATION_ROADMAP.md](IMPLEMENTATION_ROADMAP.md)
**Purpose:** Comprehensive implementation status, architectural gap analysis, and phased development roadmap.  
**Contents:**
- Executive summary and completion status across core modules
- Critical broken areas: Flow suspend semantics, Select expression suspension, platform dispatchers
- Implementation priority phases: Phase 1 (Suspension Fixes), Phase 2 (Platform Integration), Phase 3 (Advanced Features)
- Testing strategy and milestone validation criteria

### [research_notes.md](research_notes.md)
**Purpose:** Investigation into Kotlin compiler IR lowering and `Cancellable` start machinery.  
**Contents:**
- Kotlin/Native compiler lowering (`NativeSuspendFunctionLowering.kt`) and `create` hook
- Runtime intrinsics (`IntrinsicsNative.kt` / `createCoroutineUnintercepted`)
- Execution flow for `startCoroutineCancellable`
- C++ mapping using `LambdaContinuation<T>` and `create_coroutine_unintercepted`
