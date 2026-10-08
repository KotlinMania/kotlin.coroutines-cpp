## kotlinx.coroutines‑cpp Porting North Star

This document is the *authoritative technical playbook* for porting Kotlin `kotlinx.coroutines` and Kotlin/Native coroutine lowering to C++.  
It is written to be precise enough for other AI agents and humans to follow without re‑discovering intent.

If anything here conflicts with a newer repo‑local rule (e.g., `AGENTS.md`, `docs/architecture/docking_ring.md`), treat the newer rule as source‑of‑truth and update this file.

---

## Standalone C++ use and Kotlin/Native compatibility

Sydney's requirement on 2026-10-06: this is a standalone C++ coroutine port that
must also be compatible with Kotlin/Native. Normal C++ functions, types and MLX C++
code must remain usable with the port. Standalone C++ applications must build and
run without an installed Kotlin compiler, linked Kotlin/Native runtime, or JVM.
They use the translated C++ coroutine implementation and the CMake/Clang plugins.
Upstream Kotlin is the source of the translation and compatibility contract; it is
not a required application runtime for ordinary C++ use.

Kotlin/Native interoperability is an explicitly linked boundary when requested.
That boundary requires the real matching Native runtime, continuation/frame/result
contracts, and roots for actual Kotlin GC objects. Ordinary C++ values, RAII objects
and MLX handles retain their actual C++ lifetimes; using the coroutine port does
not require converting them to Kotlin objects. This distinction does not authorize
an alternate coroutine state machine, substitute runtime or fallback.

Acceptance requires both:

1. A standalone CMake/Clang C++ executable that uses ordinary C++ functions and
   types with the coroutine authoring surface, retains C++ locals/resources across
   suspension, performs real MLX C++ GPU work, and resumes correctly. Verify its
   build and linked dependencies do not require Kotlin tools or runtime libraries.
2. The Kotlin/Native/C++ docking-ring demonstration through the actual Native
   coroutine state machine and direct unsafe MLX bindings, with both handoff
   directions, results, failure/cancellation, resource identity and cleanup.

The C++ implementation must satisfy its own executable contract even when no
Kotlin program participates. Native interoperability tests are separate evidence.
Existing array fixtures prove neither complete standalone authoring/MLX integration
nor the complete shared-state-machine boundary.

## 1. Mission and invariants

### 1.1 Why this port exists
- Build a “docking ring” between Kotlin/Native and C++:
  - C++ independently implements the translated coroutine behavior and compiler lowering, with ordinary C++ functions, types and MLX/GPU code.
  - Kotlin/Native supplies the interoperability contract and its real runtime when a Kotlin program participates.
  - Later Python embedding remains separate from the current standalone C++ and Kotlin/Native docking-ring requirements.
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
   - Mandatory LLVM injection constructs saved-address stores and `indirectbr` dispatch. Switch-based substitutes and runtime fallbacks do not meet this contract.
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
  src/kotlinx/coroutines/<mirrored tree>/X.hpp   (public surface)
  src/kotlinx/coroutines/<mirrored tree>/X.cpp   (implementation)
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
5. Implement missing behavior without stubs or placeholder bodies. Record genuine blockers in audit evidence; source TODO comments are prohibited.

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
  - `src/kotlinx/coroutines/intrinsics/Intrinsics.hpp`
  - `intrinsics::get_COROUTINE_SUSPENDED()`
  - `intrinsics::is_coroutine_suspended(void*)`

### 4.2 `suspend_cancellable_coroutine<T>` contract
Implementation: `src/kotlinx/coroutines/CancellableContinuationImpl.hpp`.
- Creates a `CancellableContinuationImpl<T>` and runs a user block.
- `get_result()` decides:
  - suspend ⇒ returns sentinel,
  - completed with exception ⇒ throws immediately,
  - completed with value ⇒ returns boxed `T*` (heap‑allocated).

### 4.3 Ownership rule (current)
- Non‑void results are boxed as `T*` and returned via `void*`.
- **Caller is responsible for deleting the box.**
- Define ownership at each call site. If the policy depends on another upstream
  component, port that dependency. An unresolved ownership contract is an audit
  blocker, not permission to add a comment or placeholder implementation.

---

## 5. Kotlin/Native lowering model (the target)

This section is grounded in the Kotlin/Native compiler sources you vendored in `tmp/kotlin/`. See `docs/architecture/docking_ring.md` for the discovery trail.

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

### 5.2 Required representation and verified boundary

Kotlin/Native retains a block address in its frame label and resumes through
LLVM indirectbr. The C++ frontend supplies the exact persistent label-field
address and actual function-local resume block addresses. KotlinxCoroutinePass
constructs their stores and entry/resume dispatch in Clang's in-memory LLVM module.
The ID used for tooling does not replace the saved address or declaration identity.
Dispatch shape alone does not prove shared Native frame/result/GC compatibility.

---

## 6. Suspend authoring and mandatory compiler lowering

### 6.1 C++ authoring requirement

Ordinary C++ functions and values are usable inside the coroutine authoring surface.
Existing nonsuspending functions need no Kotlin annotation or translation. Ordinary
C++ classes, standard-library types and MLX handles remain C++ values with their
actual ownership and destruction rules. Retaining a borrowed pointer or reference
across suspension does not transfer ownership. Standalone application build and
transitive link dependencies must exclude Kotlin tools and runtime libraries.
The CMake-enabled Clang frontend must construct the retained coroutine frame and
save live locals without requiring the author to hand-code those operations.
The underlying authoring markers are compiler contracts; their existence is not
completion of the automatic frontend or the Native interoperability requirement.

### 6.2 Actual marker and LLVM contracts

The current source markers in src/kotlinx/coroutines/dsl/Suspend.hpp are:

```cpp
__kxs_coroutine_begin(void** label_field);
__kxs_suspend_point(int id, void** label_field, void* resume_address);
```

The macros supply these operands and the immediate/suspended/resumed result
regions. They do not implement a second entry dispatch or save the label themselves.
Marker declarations have no runtime implementation: missing mandatory injection
must fail to link. Source live-state/result accesses remain subject to compiler
lowering and execution verification.

### 6.3 Production CMake pipeline

kxs_enable_suspend_dsl(target) enables the Clang AST frontend and the mandatory
LLVM module plugin. CMake adds -fpass-plugin and loads the frontend using the
selected compiler's matching LLVM/Clang development package. The LLVM plugin
constructs saved-address stores, start/resume branching and indirectbr, consumes
the contracts and verifies the resulting module. Ordinary production compilation
does not serialize/reparse LLVM IR or use a Python compile launcher.

The standalone kxs-inject tool uses the same engine for diagnostics; it is not the
production compiler invocation. See the docking-ring and IR specification for
current verified behavior and unfinished source dependencies.

### 6.4 Independent C++ use and Native interoperability

The C++ library and plugins must work without Kotlin installed or linked. Calling
ordinary C++ code does not require the Kotlin/Native runtime. Actual Kotlin/Native
object and coroutine handoffs require the matching Native runtime at that explicit
boundary. There is no automatic choice of a substitute coroutine implementation.
The toolchain is Clang-only; GCC/MSVC coroutine fallbacks are prohibited.

---

## 7. No TODOs or stubs; required gap evidence

`TODO`, `FIXME`, `XXX`, and `HACK` comments are prohibited in source. Stubs and
placeholder implementations are prohibited. There is no temporary exception
for transliteration, semantic work, ownership, or compiler migration.

Port missing behavior from the matching Kotlin source. Remove existing
prohibited comments as the missing behavior is implemented; deleting comments
or renaming a stub does not close a gap. Record genuine blockers in
`docs/audits/` with source/target locations, upstream behavior, missing
dependencies, and `ast_distance` evidence. Such records do not make incomplete
code acceptable or establish that it meets the required criteria.

Classify audit gaps by API/transliteration, semantics, compiler lowering,
ownership, or performance. Keep generated `ast_distance` inventories and
priority reports current rather than embedding work lists in source comments.

---

## 8. Build and test workflow

### 8.1 Toolchain expectations
- CMake ≥ 3.16, Clang C++20 and its matching LLVM/Clang development package, Threads/pthreads.
- Out‑of‑source builds; artifacts land under `build/` (or another build dir).

### 8.2 Standard build
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j4
```

### 8.3 Key CMake options
- `KOTLIN_NATIVE_RUNTIME_AVAILABLE=ON`
  - enables the actual Native ABI host fixtures when the explicit Native toolchain is supplied; standalone C++ use must not require this option.
- `KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=ON`
  - builds the suspend DSL plugin in `tools/clang_suspend_plugin/`.

### 8.4 Tests
- Tests are plain C++ executables registered via `src/tests/CMakeLists.txt` using:
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
- `src/tests/src/gc_bridge` suite is optional and requires Kotlin/Native tooling; do not enable K/N options unless that toolchain is installed.

---

## 9. Repo hygiene and safety norms

- **Read before you cut metal.**  
  This project is intentionally delicate; inspect existing machinery and docs before changing core paths.
- Persist non‑trivial analysis as markdown in `docs/` (not scattered summary files).
- Avoid repo litter (no random “SUPER_SUMMARY.md” files).
- Use git commits as checkpoints for risky changes; keep commit messages descriptive of the step.

---

## 10. Current priority queue (triage)

### Required ASTDistance evidence and priorities

`ast_distance` is the oracle for current porting status and repair priorities,
and a required acceptance check for transliteration fidelity. Ports
must meet the applicable criteria and record their measured results and
unresolved gaps. The tool is still being refined: investigate findings against
the source and target, and explicitly record demonstrated parser, matching, or
rule limitations. These limitations do not make the check optional. Runtime
parity still requires the continuation contracts and behavioral tests.

Use `ast_distance`'s generated gap inventories and priority documents to select
and sequence work; refresh them after relevant source or tool changes. The
static queue below supplies architectural context and must be reconciled with
those reports. See [the tooling contract](../../tools/ast_distance/README.md#required-porting-criteria).
Numeric thresholds and the authoritative scoring model must be specified in
the applicable acceptance criteria rather than inferred from historical scores.

### Architectural triage context

1. Clean residual Kotlin syntax in C++ `.cpp` files (`package`, `import`, `fun`, `when`, etc.).
2. Public suspend signature parity:
   - `Job::join`, `Deferred::await`, `Delay::delay` free functions, dispatcher interception.
3. Delay fallback correctness:
   - avoid capturing continuations by reference in detached threads.
4. Ownership / boxing policy formalization, with explicit call-site contracts.
5. Plugin phase‑2/3:
   - computed‑goto labels for exact `indirectbr`,
   - spill inference parity with `CoroutinesVarSpillingLowering.kt`,
   - golden IR diff tests vs Kotlin/Native.

---

### End state definition
We consider this port “docked” when:
- Relevant public API and algorithms match the Kotlin sources and meet the required measured ast_distance criteria.
- All suspend functions lower through the plugin to K/N‑parity state machines.
- LLVM IR from clang matches Kotlin/Native patterns (blockaddress + indirectbr + spill fields).
- Prompt cancellation, dispatcher fairness and select behavior have source-faithful execution evidence.
- Standalone CMake/Clang C++ code uses normal C++ functions/types and real MLX GPU work without Kotlin tools or runtime linked.
- Actual Kotlin/Native and C++ coroutine state machines hand execution across the direct unsafe MLX boundary with verified results, cancellation/failure and cleanup.
