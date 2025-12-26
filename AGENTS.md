### AGENTS playbook (transliteration-first)

#### Scope and goal
- Deliver a near 1:1 transliteration of Kotlin `kotlinx.coroutines` into C++.
- Priority is syntactic and API-surface equivalence; correctness/semantics come later.
- It is OK if code does not compile yet, as long as the translation is faithful and consistent with repo conventions.
- Use Kotlin sources under `tmp/kotlinx.coroutines/**/*.kt` as the ground truth for each C++ header/source pair under `src/kotlinx/coroutines/**` (headers are co-located with sources in this repo).

---

### What to do (high level)
- For every existing or newly created C++ file, find and open the matching Kotlin `.kt` source and mirror its public API and structure.
- Move private/internal logic into `.cpp` files; keep public interfaces, public types, and constants in `.hpp`.
- Insert new `TODO(...)` comments for any algorithmic/parameter/signature issue found; remove `TODO`s that are clearly resolved by your change.
- Prefer concrete types for non-public code; avoid templates/generics unless absolutely necessary or required by public API parity.
- Maintain naming, file layout, and ABI rules listed below.

---

### Naming, style, and file layout rules
- Classes/Structs: `CamelCase` (e.g., `JobSupport`, `TimeoutCoroutine`).
- Methods/Variables: `snake_case` (e.g., `is_active`, `invoke_on_completion`).
- Private Data Members: `snake_case_` with trailing underscore (e.g., `state_`, `parent_handle_`).
- Enums: `CamelCase` for type, `ALL_CAPS` for values (no leading `k` prefix).
- Namespaces: `snake_case` (e.g., `kotlinx::coroutines::internal`).
- Constants: `ALL_CAPS` (no leading `k` prefix).
- **Adherence**: Follow Google C++ Style Guide generally, but preserve `snake_case` for methods to match Kotlin lowering.
- Public vs private split:
    - Put public interfaces, abstract bases, public constants, and forward declarations in `.hpp`.
    - Put implementations, helper classes, and non-public functions in `.cpp`.
- Kotlin companions/extensions:
    - Companion object members → `static` methods or free functions in the same namespace.
    - Extension functions → free functions in the corresponding namespace.
- Generics:
    - Public generic Kotlin APIs may map to C++ templates, but minimize template usage internally.
    - For internal-only generics, prefer concrete specializations or type-erasure.
- Header preface block:
    - At top of each file, include a short comment with original Kotlin path and brief TODO bullets.
- Prefer explicit `override` on overridden virtuals; some legacy headers may lack it and trigger warnings.

---

### Suspend/IR/LLVM approach (dock the ring)
- Current ABI in this repo uses Kotlin/Native-style continuation:
    - Suspend functions lower to C-style entry points: `void* fn(args..., Continuation<void*>* cont)`.
    - Return either `intrinsics::COROUTINE_SUSPENDED` or a type-erased `void*` pointing to the result box.
    - Helpers exist: `suspend_cancellable_coroutine<T>(block, cont)` and `is_coroutine_suspended(...)`.
- New direction (documented in `docs/cpp_port/docking_ring.md`):
    - We are introducing a small C++ DSL for suspend and a Clang plugin (`tools/clang_suspend_plugin/`) to rewrite it into a Kotlin/Native-like state machine (labels, spilled locals, resume dispatch) at build time.
    - Continue transliteration using the current Continuation ABI; annotate code paths that will migrate to the plugin with `TODO(suspend-plugin): migrate`.
- GC/interoperability notes:
    - Keep boundaries explicit. Do not assume automatic GC; prefer `std::shared_ptr`/`std::unique_ptr` for lifetimes.
    - When returning heap-allocated results via `void*`, add `TODO(abi-ownership): document/delete policy` if ownership is not yet defined.

---

### Build and testing workflow
- Toolchain: CMake >= 3.16, Apple Clang (clang++ on macOS) — **Clang-only, no GCC/MSVC**, and Threads/pthreads.
- Prefer out-of-source builds; artifacts land under `build/` (or another build dir).
- Standard build:
    ```bash
    mkdir -p build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . -- -j4
    ```
- Useful CMake options:
    - `KOTLIN_NATIVE_RUNTIME_AVAILABLE=ON` only when Kotlin/Native runtime is present for GC bridge work.
    - `KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=ON` to build the Apple-focused suspend DSL plugin in `tools/clang_suspend_plugin/`.
    - If LLVM/Clang dev packages are missing, prefer `-DKOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=OFF` to avoid configure-time warnings.
- Outputs (as set in root `CMakeLists.txt`):
    - Binaries: `build*/bin/`
    - Libraries: `build*/lib/`
- Tests are plain C++ executables registered via `src/tests/CMakeLists.txt` using `add_coroutine_test(<name>)`.
- Run tests from a build dir with CTest; use `ctest -N` to list and `ctest -R <regex> --output-on-failure` to run subsets.
- The Kotlin/Native `tests/gc_bridge` suite is optional and requires K/N tooling; don’t enable K/N options unless that toolchain is available.
- For faster iteration, build specific targets (library or a single test), e.g. `cmake --build build --target test_suspension_core`.
- macOS note: If Homebrew paths are missing, `ld` may warn about `/opt/homebrew/opt/curl/lib` search paths; it’s typically harmless for this repo.

---

### Matching Kotlin ⇄ C++ files
- Primary lookup: mirror paths from `tmp/kotlinx.coroutines/kotlinx-coroutines-core/**/X.kt` to `src/kotlinx/coroutines/**/X.hpp` and `src/kotlinx/coroutines/**/X.cpp`.
- If a C++ file has no obvious `.kt` twin:
    - Search by class/function names in `tmp/kotlinx.coroutines`.
    - For platform-specific code, check `common/src`, `native/src`, and `darwin/src` in Kotlin; split similarly in C++ if needed.

---

### Transliteration checklist per file
1. Create/update header guard and namespaces.
2. Copy Kotlin KDoc as C++ comments; keep semantics notes but do not over-document.
3. Map public Kotlin API:
    - Properties → virtual getters/setters.
    - Functions → virtual methods or free functions (for extensions), snake_case.
    - Suspend functions → `void* name(args..., Continuation<void*>* cont)`.
4. Non-public/internal items → `.cpp` implementation with strong/concrete types.
5. Map exceptions to `std::exception_ptr` patterns where Kotlin uses `Throwable`.
6. Add missing overloads/defaults to match Kotlin signatures (note `TODO(defaults)` if default args need scaffolding).
7. Insert `TODO` for any unresolved semantic/algorithmic gap (see TODO taxonomy below).
8. Remove any stale/resolved `TODO`s in the edited region.
9. Update audit references (file:line) in `docs/audits/*` if you changed API presence.

---

### TODO taxonomy and format
- Use tagged TODOs to make searches precise. Format:
    - `// TODO(port): <short issue>` — direct transliteration needed or missing element.
    - `// TODO(semantics): <race/cancellation/algorithm risk>` — correctness not yet mirrored.
    - `// TODO(suspend-plugin): <migration note>` — will be handled by the Clang plugin.
    - `// TODO(abi-ownership): <who deletes boxed result?>` — define ownership of `void*` return boxes.
    - `// TODO(perf): <hot path issue>` — known perf gaps.
- Remove TODOs you resolve; never leave contradictory TODOs in place.

---

### Common patterns to watch and flag (insert TODOs)
- Parameter parity:
    - Default arguments present in Kotlin but not yet reflected in C++ (overloads needed). Mark `TODO(port): default args parity`.
    - Nullability vs pointers/references — mark `TODO(semantics): null handling parity` when unclear.
- Exceptions/cancellation:
    - Kotlin `CancellationException` → `std::exception_ptr` with `CancellationException` type. If not implemented, add `TODO(port): CancellationException type`.
    - Prompt cancellation guarantee (e.g., `Deferred.await`, `Job.join`, `delay`) — add `TODO(semantics): prompt cancellation race`.
- Concurrency and state machines:
    - Missing `decisionAndIndex` logic in `CancellableContinuationImpl` — ensure it’s tagged: `TODO(semantics): decision state machine parity`.
    - Reusable continuation reset paths — `TODO(semantics): reset_state_reusable`.
- Dispatcher/Delay wiring:
    - `CoroutineDispatcher` interception must recognize plugin frames — `TODO(suspend-plugin): intercept plugin frames`.
    - `Delay` fallback path captures continuation by reference (e.g., `Delay.cpp`) — add `TODO(semantics): capture shared handle, avoid dangling ref`.
- Ownership/boxing:
    - Non-void `suspend_cancellable_coroutine<T>` allocates `T` on heap and returns as `void*` — add `TODO(abi-ownership): define deleter path` near the adapter.
- Select support:
    - `on_join`, `on_await` — add `TODO(port): select clauses` where applicable.

---

### Concrete examples to emulate
- CoroutineDispatcher (header present):
    - Ensure methods exist and are snake_case: `is_dispatch_needed`, `dispatch`, `dispatch_yield`, `limited_parallelism`, `plus`, `to_string`, `intercept_continuation`, `release_intercepted_continuation`.
    - If any are stubbed or missing behavior, add:
        - `// TODO(semantics): dispatch_yield fairness`
        - `// TODO(suspend-plugin): continuation interception for plugin frames`
- Delay:
    - Interface has `schedule_resume_after_delay` and `invoke_on_timeout`.
    - Free `delay(...)` exists in `.cpp` and uses `suspend_cancellable_coroutine`.
    - Add near fallback thread path:
        - `// TODO(semantics): capture shared continuation handle, avoid &ref in detached thread`
        - `// TODO(perf): timer wheel/heap instead of detached thread`
- Deferred/Job suspend APIs:
    - Headers should expose `void* await(Continuation<void*>*)` and `void* join(Continuation<void*>*)`.
    - Add `// TODO(semantics): prompt cancellation race parity` next to their implementations until state machine is complete.

---

### Public vs private split guide
- `.hpp` may contain:
    - Pure virtual interfaces, public enums, constants, and forward declarations.
    - Minimal inline helpers that are ABI-critical (e.g., `key()` accessors).
- `.cpp` should contain:
    - Implementations, algorithmic code, helper classes, and any heavy includes.
    - Internal utilities with concrete types (no templates unless forced by API).

---

### Audit integration (docs)
- When you add/move/rename public API, immediately update:
    - `docs/API_AUDIT.md` rows (ensure snake_case method names).
    - The relevant block in `docs/audits/*.md` with `file:line` references.
- Reclassify status accurately: `Surface` (header present), `Wired` (basic implementation), `Plugin-backed` (after migration).

---

### Don’ts
- Don’t refactor semantics or introduce new abstractions during transliteration.
- Don’t dump large implementations into headers; keep headers slim.
- Don’t introduce templates unless the Kotlin API requires it at the public surface.
- Don’t paper over semantic gaps without a `TODO` — call them out explicitly.

---

### Step-by-step workflow for each file
1. Identify the Kotlin source file(s) in `tmp/kotlinx.coroutines/**.kt` that correspond to your C++ target.
2. Ensure header type and method names match, using snake_case for methods and CamelCase for types.
3. Move non-public helpers into the `.cpp` and prefer concrete typing.
4. Reconcile parameters and defaults; add overloads or `TODO(port): defaults parity`.
5. For suspend points, use the Continuation ABI form and tag `TODO(suspend-plugin)` for migration.
6. Insert specific `TODO`s for any algorithmic gap (see taxonomy), and remove obsolete TODOs.
7. Leave a short file header linking the original Kotlin path.
8. Update the audit tables with the new status and file:line references.

---

### Immediate priority targets (triage)
- Root out residual Kotlin code in `.cpp` files that still contain `package`/`import`.
- Verify and correct public suspend signatures:
    - `Job.join`, `Deferred.await`, `Delay.delay` (free functions), `CoroutineDispatcher` interception methods.
- Delay fallback correctness and ownership TODOs.
- Dispatcher surface parity (`is_dispatch_needed`, `dispatch`, `dispatch_yield`, `limited_parallelism`).

---

### Acceptance checks per PR
- Headers contain only the public surface and minimal ABI-critical code.
- Methods and enums follow naming rules; no camelCase methods remain in C++.
- All new gaps are called out with a specific, tagged `TODO`.
- Resolved `TODO`s are removed in edited regions.
- `docs/audits/*` updated to reflect new API presence with file:line.

---

### Snippets you can paste as TODOs
- `// TODO(semantics): prompt cancellation guarantee — ensure cancellation between readiness and resume throws`.
- `// TODO(suspend-plugin): migrate this suspend logic to plugin-generated state machine`.
- `// TODO(abi-ownership): who deletes boxed result returned as void* from suspend?`.
- `// TODO(port): add overloads to emulate Kotlin default arguments`.
- `// TODO(semantics): replace detached thread fallback with event loop timer`.

---

### References
- IR/LLVM plan: `docs/cpp_port/docking_ring.md`.
- Headers: `src/kotlinx/coroutines/**/*.hpp` (headers are co-located with sources).
- Implementations: `src/kotlinx/coroutines/**/*.cpp`.
- Kotlin sources: `tmp/kotlinx.coroutines/**/src/**/*.kt`.
- Suspend intrinsics: `kotlinx/coroutines/intrinsics/Intrinsics.hpp` and `CancellableContinuationImpl.hpp`.

---

### Systematic Approach for Remaining Files (Syntax Cleanup)

Use this as the canonical checklist when cleaning residual Kotlin syntax from `.cpp` files. Cross‑reference `docs/SYNTAX_CLEANUP_STATUS.md` for live counts and priority files.

1) Files with Kotlin `fun` keyword
- Replace `fun name(params): Ret` with C++ `Ret name(params)`.
- Replace `override fun` with C++ method declarations that end with `override`.
- Replace `suspend fun` with the Continuation ABI now, then migrate via the plugin:
  - `void* name(args..., Continuation<void*>* cont)`
  - Add `// TODO(suspend-plugin): migrate to plugin-generated state machine`.
- Parameters: `param: Type` → `Type param`. If Kotlin used default args, add C++ overloads or tag `// TODO(port): defaults parity`.

2) Files with `when` expressions
- Use `switch` for integral/enum subjects; otherwise use `if-else` chains.
- Assignment form: `auto x = when (y) { ... }` → `auto x = /*init*/;` then assign in each branch.
- Subjectless form: `when { c1 -> ..., c2 -> ..., else -> ... }` → ordered `if/else if/else`.
- Type checks: `is T` → `if (auto* p = dynamic_cast<T*>(expr)) { ... }`.
- Membership/range: `in range`/`!in` → explicit bound checks or helper; add `// TODO(port): range helper` if a utility is desired.
- If Kotlin was exhaustive (sealed types), ensure coverage or add `// TODO(semantics): exhaustiveness parity`.

3) Kotlin operators to replace
- Elvis `a ?: b` → `a != nullptr ? a : b` (or explicit null checks for non-pointer types).
- Safe call `obj?.prop` / `obj?.call()` → `if (obj) obj->prop/call();`.
- Non-null assertion `x!!` → assert or explicit check; add `// TODO(semantics): non-null assertion parity` if behavior matters.
- Ranges:
  - `a..b` (inclusive) → `for (int i = a; i <= b; ++i)`.
  - `a until b` → `i < b` upper bound.
  - `a downTo b step s` → `for (int i = a; i >= b; i -= s)`.
- Equality vs identity: Kotlin `==` (structural) vs `===` (reference). Use `operator==` vs pointer identity accordingly; add `// TODO(semantics): equality vs identity` when unclear.

4) Control-flow and statement syntax
- Kotlin `if condition { ... }` (no parentheses) → C++ `if (condition) { ... }`.
- Kotlin `for (x in collection)` → C++ `for (auto& x : collection)` or index loop as needed.
- Labeled returns `return@label` → restructure with flags/helper functions; add `// TODO(port): labeled return rewrite` if non-trivial.

5) Inheritance and type declarations
- Kotlin `class Foo : Bar` → C++ `class Foo : public Bar`.
- Multiple supertypes: mark interfaces as `public` bases; ensure virtual destructor on bases. Add `// TODO(port): multiple inheritance parity` if ambiguous.

6) Grep helpers
```
# Kotlin function syntax
grep -R "\bfun\s\+[a-zA-Z_]" --include='*.cpp' kotlinx-coroutines-core/

# Kotlin parameter syntax
grep -R ":\s\+[A-Z][a-zA-Z_]*\)" --include='*.cpp' kotlinx-coroutines-core/

# when expressions
grep -R "\bwhen\s*(" --include='*.cpp' kotlinx-coroutines-core/

# if without parentheses (heuristic)
grep -R "^\s*if\s+[^ (]" --include='*.cpp' kotlinx-coroutines-core/

# Missing public inheritance
grep -R "\bclass\s\+[A-Za-z_][A-Za-z0-9_]*\s*:\s*[A-Z]" --include='*.cpp' kotlinx-coroutines-core/

# Elvis and safe-call operators
grep -R "\?:\|\?\." --include='*.cpp' kotlinx-coroutines-core/
```

7) Priority order
- Core coroutine files (Job, Continuation, Dispatcher, Delay, etc.).
- Flow implementation files (SharedFlow, StateFlow).
- Channel implementation files.
- Tests (lower priority).
- Build/configuration translation (Gradle → CMake notes).

8) Cross-references and rules
- Methods in C++ are `snake_case`; classes remain `CamelCase`.
- Keep public interfaces in headers; move implementations/private helpers to `.cpp`.
- Suspend signatures must use the Continuation ABI today; tag with `TODO(suspend-plugin)` for migration per `docs/cpp_port/docking_ring.md`.
- Insert specific, categorized TODOs for any gap; remove resolved TODOs in edited regions.

---

### Final note
Transliteration first, helpers second. Keep it mechanical and reversible. Call out every mismatch explicitly with a `TODO` so we can schedule semantic work once the Clang suspend plugin lands.
