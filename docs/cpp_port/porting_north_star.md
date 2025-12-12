## kotlinx.coroutines‑cpp Porting North Star

This document is the *authoritative technical playbook* for porting Kotlin `kotlinx.coroutines` and Kotlin/Native coroutine lowering to C++.  
It is written to be precise enough for other AI agents and humans to follow without re‑discovering intent.

If anything here conflicts with a newer repo‑local rule (e.g., `AGENTS.md`, `docs/cpp_port/docking_ring.md`), treat the newer rule as source‑of‑truth and update this file.

---

## 1. Mission and invariants

### 1.1 Why this port exists
- Build a “docking ring” between Kotlin/Native and C++:
  - Kotlin/Native provides GC + coroutine runtime + suspend lowering.
  - C++ provides a high‑performance ABI target for MLX/GPU and other native ABIs.
  - Future goal: enable true Kotlin‑semantics coroutines in C++ itself and safe embedding into Python 3.14 freethreading environments.
- Apple platform is the first‑class target for this iteration (clang toolchain, MLX ABI). The bridge should remain structurally useful for later CUDA/NVIDIA ports.

### 1.2 Non‑negotiable invariants
1. **Kotlin/Native ABI fidelity for suspend functions**
   - Every suspend function *at the ABI boundary* must have the Kotlin/Native shape:
     ```cpp
     void* fn(args..., Continuation<void*>* cont);
     ```
   - Return contract:
     - `intrinsics::COROUTINE_SUSPENDED` sentinel ⇒ caller must return immediately.
     - otherwise a `void*` to a boxed heap result (or `nullptr` for Unit/void).
2. **Semantic parity is the destination, transliteration parity is the path**
   - We first reproduce Kotlin’s public API surface + structure mechanically.
   - We then converge semantics where tests or audits flag divergence.
3. **State machines must mirror Kotlin/Native lowering**
   - Labels, spill slots, resume dispatch, prompt cancellation guarantees.
   - We accept temporary switch‑based state machines; long‑term target is computed‑goto / `indirectbr` equivalence.
4. **Scope is intentionally repo‑local**
   - The DSL + compiler plugin are scoped to this project. We accept maintaining them as project infrastructure.

---

## 2. Ground truth sources and mapping rules

### 2.1 Kotlin sources are the spec
- `tmp/kotlinx.coroutines/**.kt`
  - Upstream `kotlinx.coroutines` Kotlin sources. These are the spec for public API, naming, and structure.
- `tmp/kotlin/**`
  - Kotlin compiler + Kotlin/Native runtime snapshot. This is the spec for suspend lowering and LLVM codegen.

### 2.2 Path mirroring convention
- Kotlin file:
  ```
  tmp/kotlinx.coroutines/kotlinx-coroutines-core/<src tree>/X.kt
  ```
- C++ twins:
  ```
  include/kotlinx/coroutines/<mirrored tree>/X.hpp   (public surface)
  kotlinx-coroutines-core/<mirrored tree>/X.cpp      (implementation)
  ```
- If a C++ file has no obvious `.kt` twin:
  1. Search by class/function name in `tmp/kotlinx.coroutines`.
  2. For platform‑specific logic, check Kotlin’s `common/src`, `native/src`, `darwin/src` and mirror that split.

### 2.3 What “mechanical transliteration” means
Per file:
1. Copy KDoc to C++ comments (do not reinterpret unless needed).
2. Recreate public types, methods, overloads, constants, and visibility exactly.
3. Preserve ordering and nesting of declarations where possible (helps diffing to Kotlin).
4. Move non‑public helper logic into `.cpp`.
5. Add explicit, tagged TODOs for any unresolved mismatch.

---

## 3. Naming, layout, and API rules (strict)

### 3.1 Naming
- Types/classes/structs/interfaces: `CamelCase`.
- Methods/functions/properties: **`snake_case`** (systematic camelCase → snake_case).
  - Examples: `minusKey` → `minus_key`, `dispatchYield` → `dispatch_yield`.
- Enums:
  ```cpp
  enum class Name { ENUMERATOR };
  ```
- Namespaces mirror Kotlin packages:
  ```cpp
  namespace kotlinx::coroutines::channels { ... }
  ```

### 3.2 Public vs private split
- `.hpp` contains only:
  - Public interfaces, abstract bases, public constants.
  - Forward declarations.
  - Minimal inline ABI‑critical helpers.
- `.cpp` contains:
  - Implementations.
  - Internal helper classes/functions with concrete types.
- Avoid templates internally unless Kotlin public API requires generic surface.

### 3.3 Overrides and warnings
- Use explicit `override` on overridden virtuals.
- Some legacy headers may predate this rule; fix only in touched regions.

---

## 4. Suspend ABI, markers, and ownership

### 4.1 Sentinel definition
- Kotlin/Native uses `COROUTINE_SUSPENDED` (a singleton object).
- C++ port uses pointer‑identity sentinel in:
  - `include/kotlinx/coroutines/intrinsics/Intrinsics.hpp`
  - `intrinsics::get_COROUTINE_SUSPENDED()`
  - `intrinsics::is_coroutine_suspended(void*)`

### 4.2 `suspend_cancellable_coroutine<T>` contract
Implementation: `include/kotlinx/coroutines/CancellableContinuationImpl.hpp`.
- Creates a `CancellableContinuationImpl<T>` and runs a user block.
- `get_result()` decides:
  - suspend ⇒ returns sentinel,
  - completed with exception ⇒ throws immediately,
  - completed with value ⇒ returns boxed `T*` (heap‑allocated).

### 4.3 Ownership rule (current)
- Non‑void results are boxed as `T*` and returned via `void*`.
- **Caller is responsible for deleting the box.**
- If a call site cannot make ownership obvious, add:
  ```cpp
  // TODO(abi-ownership): define deleter/boxing policy here.
  ```

---

## 5. Kotlin/Native lowering model (the target)

This section is grounded in the Kotlin/Native compiler sources you vendored in `tmp/kotlin/`. See `docs/cpp_port/docking_ring.md` for the discovery trail.

### 5.1 Pipeline stages in Kotlin/Native
1. **Suspend function lowering**
   - File: `tmp/kotlin/.../NativeSuspendFunctionLowering.kt`
   - Decides whether a suspend function needs a state machine.
   - Tail‑suspend optimization: if all suspend calls are tail calls, no coroutine class/state machine is generated.
2. **Spill lowering**
   - File: `tmp/kotlin/.../CoroutinesVarSpillingLowering.kt`
   - Computes liveness at each suspend point.
   - Creates private fields for each live‑across variable.
   - Rewrites `saveCoroutineState` / `restoreCoroutineState` intrinsics into field stores/loads.
3. **IR → LLVM**
   - File: `tmp/kotlin/.../IrToBitcode.kt`
   - `evaluateSuspendableExpression`:
     - chooses fresh vs resume path,
     - resume uses `indirectbr(labelAddress, resumeBlocks...)`.
   - `evaluateSuspensionPoint`:
     - creates a resume basic block,
     - stores its block address into the coroutine label field.

### 5.2 Representation difference to close
- Kotlin/Native label is a **block address** (`void*`) used by `indirectbr`.
- Current C++ macro era label is an **int** used by a `switch`.
- `switch` is semantically correct and can lower to a jump table, but not identical in structure.

Long‑term direction: a compiler plugin that emits computed‑goto dispatch so clang generates the same IR shape as Kotlin/Native.

---

## 6. DSL + compiler plugin direction

### 6.1 Why a plugin
- Macros approximate lowering but require human‑manual spills and state numbering.
- Kotlin/Native’s correctness hinges on:
  - liveness‑based spilling,
  - consistent label/resume dispatch,
  - structured prompt cancellation.
- A Clang plugin can mechanize this and keep authoring ergonomic.

### 6.2 DSL surface (current direction)
- Suspend functions are annotated:
  ```cpp
  [[suspend]] void* foo(int x, Continuation<void*>* cont);
  ```
- Suspend points use a Kotlin‑aligned marker:
  ```cpp
  auto y = suspend(bar(x, cont));
  ```
  Use `suspend(expr)` as the single canonical suspension marker in C++ source.

### 6.3 Plugin responsibilities
For each `[[suspend]]` function:
1. Discover suspend points (`suspend(expr)`).
2. Build CFG and run liveness to compute spill sets per suspend point.
3. Synthesize a coroutine frame/class:
   - `_label` field (prefer `void*` label‑address mode; fallback to int).
   - spill fields for live locals.
4. Rewrite into a Kotlin/Native‑equivalent state machine:
   - save spills → set label → call → propagate sentinel → resume label → restore spills.
5. (Later) apply tail‑suspend optimization as in `NativeSuspendFunctionLowering.kt`.

Implementation scaffold lives in `tools/clang_suspend_plugin/`.

---

## 7. TODO taxonomy (mandatory)

Use explicit tags so audits and AI can classify gaps:
- `TODO(port):` missing transliteration or API surface mismatch.
- `TODO(semantics):` correctness gap vs Kotlin (races, cancellation, algorithms).
- `TODO(suspend-plugin):` this suspend logic should migrate to plugin‑generated state machine.
- `TODO(abi-ownership):` boxed result ownership unclear.
- `TODO(perf):` known performance gap.

Remove TODOs you resolve in the region you edit.

---

## 8. Build and test workflow

### 8.1 Toolchain expectations
- CMake ≥ 3.16, C++20 compiler, Threads/pthreads.
- Out‑of‑source builds; artifacts land under `build/` (or another build dir).

### 8.2 Standard build
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j4
```

### 8.3 Key CMake options
- `KOTLIN_NATIVE_RUNTIME_AVAILABLE=ON`
  - only enable when Kotlin/Native runtime is present and GC bridge is required.
- `KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=ON`
  - builds the suspend DSL plugin in `tools/clang_suspend_plugin/`.

### 8.4 Tests
- Tests are plain C++ executables registered via `tests/CMakeLists.txt` using:
  ```cmake
  add_coroutine_test(test_name)
  ```
- Run from a build dir:
  ```bash
  ctest -N
  ctest --output-on-failure
  ctest -R <regex> --output-on-failure
  ```
- Use `-R` subsets to keep suspend progress moving when unrelated targets fail.
- `tests/gc_bridge` suite is optional and requires Kotlin/Native tooling; do not enable K/N options unless that toolchain is installed.

---

## 9. Repo hygiene and safety norms

- **Read before you cut metal.**  
  This project is intentionally delicate; inspect existing machinery and docs before changing core paths.
- Persist non‑trivial analysis as markdown in `docs/` (not scattered summary files).
- Avoid repo litter (no random “SUPER_SUMMARY.md” files).
- Use git commits as checkpoints for risky changes; keep commit messages descriptive of the step.

---

## 10. Current priority queue (triage)

1. Clean residual Kotlin syntax in C++ `.cpp` files (`package`, `import`, `fun`, `when`, etc.).
2. Public suspend signature parity:
   - `Job::join`, `Deferred::await`, `Delay::delay` free functions, dispatcher interception.
3. Delay fallback correctness:
   - avoid capturing continuations by reference in detached threads.
4. Ownership / boxing policy formalization (`abi-ownership` TODOs).
5. Plugin phase‑2/3:
   - computed‑goto labels for exact `indirectbr`,
   - spill inference parity with `CoroutinesVarSpillingLowering.kt`,
   - golden IR diff tests vs Kotlin/Native.

---

### End state definition
We consider this port “docked” when:
- Public API surface matches `kotlinx.coroutines` (audits green).
- All suspend functions lower through the plugin to K/N‑parity state machines.
- LLVM IR from clang matches Kotlin/Native patterns (blockaddress + indirectbr + spill fields).
- Prompt cancellation, dispatcher fairness, and select semantics pass parity tests.
