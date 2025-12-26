# Kotlin Coroutines C++ Port

## Project Overview

**Purpose**: Port `kotlinx.coroutines` (Kotlin's async/await framework) to C++ with near 1:1 API equivalence using a Kotlin/Native-style Continuation ABI today and a Clang-based suspend lowering plugin going forward.

**Type**: C++ library (header-only and source files)
**Size**: ~50K LOC across headers and implementation
**Languages**: C++ (C++20), Kotlin (source reference), Python (build scripts)

---

## Directory Structure

```
include/kotlinx/coroutines/          # Public API headers (.hpp)
  ├── Continuation.hpp               # Continuation interface (ABI root)
  ├── Job.hpp                        # Job/cancellation hierarchy
  ├── Deferred.hpp                   # Async results (await)
  ├── Delay.hpp                      # Delay/timeout primitive
  ├── CoroutineDispatcher.hpp        # Scheduler interface
  ├── Builders.hpp                   # launch(), async() DSL
  ├── channels/                      # Channel communication
  └── flow/                          # Flow (reactive) API

src/kotlinx/coroutines/              # Implementation (.cpp)
  ├── common/                        # Platform: common (maps to parent namespace)
  │   ├── internal/                  # → kotlinx::coroutines::internal
  │   └── flow/internal/             # → kotlinx::coroutines::flow::internal
  ├── concurrent/                    # Platform: concurrent (maps to parent namespace)
  ├── native/                        # Platform: native (maps to parent namespace)
  ├── channels/                      # → kotlinx::coroutines::channels
  ├── flow/                          # → kotlinx::coroutines::flow
  ├── internal/                      # → kotlinx::coroutines::internal
  ├── sync/                          # → kotlinx::coroutines::sync
  └── test/                          # → kotlinx::coroutines::test

tmp/kotlinx.coroutines/              # Kotlin source reference (DO NOT EDIT)
  └── **/*.kt                        # Ground truth for transliteration

docs/                                # Documentation
  ├── cpp_port/docking_ring.md       # Suspend/IR/LLVM plan and plugin design
  ├── audits/                        # Per-file audit status
  │   ├── NAMESPACE_STRUCTURE_AUDIT.md  # Folder→namespace mapping reference
  │   └── COMPREHENSIVE_AUDIT_REPORT.md # Overall implementation status
  └── topics/                        # Translated Kotlin coroutines docs

tools/clang_suspend_plugin/          # Clang plugin for suspend DSL (operational - see docs/SUSPEND_IMPLEMENTATION.md)
tests/                               # Test executables
```

**Namespace Mapping:** Platform qualifier folders (`common/`, `native/`, `concurrent/`) map to their **parent** namespace, not a child namespace. For example, `src/kotlinx/coroutines/common/Job.cpp` uses `namespace kotlinx::coroutines`, not `kotlinx::coroutines::common`. See `docs/audits/NAMESPACE_STRUCTURE_AUDIT.md` for complete mapping.

---

## Build and Test

### Prerequisites
- CMake >= 3.16
- Apple Clang (clang++ on macOS) — **Clang-only, no GCC/MSVC support**
- Threads/pthreads

### Standard Build
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j4
```

### Run Tests
```bash
# From build directory
ctest                           # Run all tests
ctest -R <regex>                # Run subset

# Build specific target
cmake --build build --target test_suspension_core
```

### CMake Options
- `KOTLIN_NATIVE_RUNTIME_AVAILABLE=ON` — Only when K/N runtime is present for GC bridge
- `KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=ON` — Build the suspend DSL plugin
- `KOTLINX_BUILD_AST_DISTANCE=ON` — Build porting analysis tool (default: ON)

---

## Transliteration Rules

### Goal
Deliver a near 1:1 transliteration of Kotlin `kotlinx.coroutines` into C++. Priority is syntactic and API-surface equivalence; correctness/semantics come later. It is OK if code does not compile yet, as long as the translation is faithful and consistent with repo conventions.

### Matching Kotlin ⇄ C++ Files
- Primary lookup: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/**/X.kt` → `include/kotlinx/coroutines/**/X.hpp` and `kotlinx-coroutines-core/**/X.cpp`
- For platform-specific code, check `common/src`, `native/src`, and `darwin/src` in Kotlin

### Naming Conventions
| Kotlin | C++ |
|--------|-----|
| Classes/structs | `CamelCase` |
| Methods/functions/properties | `snake_case` (e.g., `minusKey` → `minus_key`, `isActive` → `is_active`) |
| Enums | `enum class Name { ALL_CAPS }` |
| Packages | Namespaces: `kotlinx::coroutines::channels` |

### Public vs Private Split
- **`.hpp` (headers)**: Public interfaces, abstract bases, constants, forward declarations, minimal inline helpers
- **`.cpp` (source)**: Implementations, helper classes, algorithmic code, internal utilities with concrete types

### Kotlin Mappings
- Companion object members → `static` methods or free functions
- Extension functions → free functions in corresponding namespace
- Properties → virtual getters/setters
- Exceptions → `std::exception_ptr` with typed exception classes

---

## Suspend Function Implementation

### ⚠️ READ THIS FIRST: docs/SUSPEND_IMPLEMENTATION.md

**The complete, authoritative guide to suspend functions is in `docs/SUSPEND_IMPLEMENTATION.md`.**

### Current Approach (Clang Plugin - Operational)

Suspend functions use a **Kotlin-aligned DSL** that is processed by the Clang plugin at build time:

```cpp
#include <kotlinx/coroutines/dsl/Suspend.hpp>
using namespace kotlinx::coroutines::dsl;

[[suspend]]
void* my_function(Args..., std::shared_ptr<Continuation<void*>> completion) {
    // Regular code
    prepare_data();

    // Suspend point - plugin detects this
    suspend(async_operation(completion));

    // Code continues after resumption
    process_result();

    return nullptr;  // Unit result
}
```

The plugin (`tools/clang_suspend_plugin/`) generates a `.kx.cpp` sidecar file containing:
- Coroutine class extending `ContinuationImpl`
- State machine with computed goto dispatch (`indirectbr` + `blockaddress`)
- Automatic parameter capture
- `void* _label` storage matching Kotlin/Native's NativePtr

### ABI Convention

All suspend functions follow this signature:
```cpp
void* function_name(/* args */, std::shared_ptr<Continuation<void*>> completion)
```

**Return values:**
- `COROUTINE_SUSPENDED` - Function suspended, will resume later
- `void*` - Boxed result or `nullptr` for Unit

**Check suspension:**
```cpp
void* result = suspend_function(args, completion);
if (is_coroutine_suspended(result)) return COROUTINE_SUSPENDED;
// Process result if not suspended
```

### Ownership
- Non-void results are heap-allocated and returned as `void*`
- Callers must unbox and manage lifetime
- Tag unclear ownership with `// TODO(abi-ownership): define deleter path`
- Prefer `std::shared_ptr`/`std::unique_ptr` for lifetimes

---

## TODO Taxonomy

Use tagged TODOs to make searches precise:
- `// TODO(port): <issue>` — Direct transliteration needed or missing element
- `// TODO(semantics): <risk>` — Correctness/race/cancellation not yet mirrored
- `// TODO(suspend-plugin): <note>` — Will be handled by the Clang plugin
- `// TODO(abi-ownership): <who deletes?>` — Define ownership of `void*` return boxes
- `// TODO(perf): <issue>` — Known performance gaps

Remove TODOs you resolve; never leave contradictory TODOs in place.

### Common TODO Snippets
```cpp
// TODO(semantics): prompt cancellation guarantee — ensure cancellation between readiness and resume throws
// TODO(suspend-plugin): migrate this suspend logic to plugin-generated state machine
// TODO(abi-ownership): who deletes boxed result returned as void* from suspend?
// TODO(port): add overloads to emulate Kotlin default arguments
// TODO(semantics): replace detached thread fallback with event loop timer
```

---

## Syntax Translation Patterns

### Function Syntax
- `fun name(params): Ret` → `Ret name(params)`
- `override fun` → method declaration ending with `override`
- `suspend fun` → `void* name(args..., Continuation<void*>* cont)`
- `param: Type` → `Type param`

### Control Flow
- `when (x) { ... }` → `switch` for integral/enum, `if-else` otherwise
- `a ?: b` (Elvis) → `a != nullptr ? a : b`
- `obj?.prop` (safe call) → `if (obj) obj->prop`
- `x!!` (non-null assertion) → assert or explicit check
- `a..b` → `for (int i = a; i <= b; ++i)`
- `a until b` → `for (int i = a; i < b; ++i)`
- `for (x in collection)` → `for (auto& x : collection)`

### Inheritance
- `class Foo : Bar` → `class Foo : public Bar`
- Multiple supertypes: mark interfaces as `public` bases; ensure virtual destructor

---

## Acceptance Checks

Before submitting changes:
1. **Run `make ast-lint`** — no new lint errors in modified files
2. **Run `make ast-todos-summary`** — no untagged TODOs
3. **Run `make ast-deep`** — similarity scores for modified files are acceptable (>0.60)
4. Headers contain only the public surface and minimal ABI-critical code
5. Methods and enums follow naming rules; no camelCase methods remain in C++
6. All new gaps are called out with a specific, tagged `TODO`
7. Resolved `TODO`s are removed in edited regions
8. Compile cleanly: `clang++ -std=c++20 -Wall -Wextra -I include your_file.cpp`
9. Tests pass: `./test_suspend`
10. Update `docs/audits/*` to reflect new API presence with file:line
11. Include `Transliterated from:` header in new files for proper matching

---

## Don'ts

- Don't refactor semantics or introduce new abstractions during transliteration
- Don't dump large implementations into headers; keep headers slim
- Don't introduce templates unless the Kotlin API requires it at the public surface
- Don't paper over semantic gaps without a `TODO` — call them out explicitly
- Don't break existing tests

---

## Key References

- **Kotlin sources (ground truth)**: `tmp/kotlinx.coroutines/**/src/**/*.kt`
- **Headers**: `include/kotlinx/coroutines/*.hpp`
- **Implementations**: `kotlinx-coroutines-core/**/src/**/*.cpp`
- **Suspend intrinsics**: `kotlinx/coroutines/intrinsics/Intrinsics.hpp` and `CancellableContinuationImpl.hpp`
- **IR/LLVM plan**: `docs/cpp_port/docking_ring.md`
- **Audit docs**: `docs/audits/*.md`

---

## File-Specific Instructions

### Test Files (`test_*.cpp`, `**/test/**/*.cpp`)
- Prefer exercising Continuation ABI over legacy macro DSL
- Trust the compiler, not the IDE (suspend constructs can confuse parsers)
- Each test function validates one feature
- Never break existing tests

### Implementation Files (`kotlinx-coroutines-core/**/src/**/*.cpp`)
- Find the Kotlin source first — every `.cpp` has a corresponding `.kt`
- Use concrete types, avoid templates unless required by header ABI
- Store continuations as `std::shared_ptr<Continuation<void*>>`
- Use `std::exception_ptr` for error boxing

### Public Headers (`include/kotlinx/coroutines/**/*.hpp`)
- Match Kotlin API exactly
- Headers should be readable in under 1 minute
- Include reference to original Kotlin source file path at top
- Suspend signatures: `virtual void* await(Continuation<void*>* cont) = 0;`

---

## Final Note

Transliteration first, helpers second. Keep it mechanical and reversible. Call out every mismatch explicitly with a `TODO` so we can schedule semantic work once the Clang suspend plugin lands.

---

## Porting Quality Tools (MANDATORY)

This project includes integrated porting analysis tools that **MUST** be used to ensure transliteration quality. These tools are built as part of the main CMake build and provide CMake targets for easy access.

### ⚠️ REQUIRED: Run Before Submitting Changes

**Before any PR or significant change, you MUST run:**
```bash
make ast-lint        # Check for unused parameters, missing guards
make ast-todos       # Review outstanding TODOs
make ast-deep        # Full porting analysis
```

If any of these reveal issues in files you modified, **fix them before proceeding**.

### When to Use These Tools

| Situation | Required Action |
|-----------|-----------------|
| Compilation errors | Run `make ast-lint` to check for structural issues |
| Adding new files | Run `make ast-deep` to verify Kotlin matching |
| Modifying existing files | Run `make ast-lint` and `make ast-todos` |
| Before any commit | Run `make ast-todos-summary` to check TODO count |
| Weekly health check | Run `make porting-report` for full analysis |

---

## CMake Porting Targets

All targets are available from the build directory after running `cmake ..`:

### Quality Checks

```bash
# Lint: unused parameters, missing header guards
make ast-lint

# TODO scanning with full context
make ast-todos

# TODO scanning (summary only, no context)
make ast-todos-summary

# File statistics: line counts, stubs, issues
make ast-stats
```

### Porting Analysis

```bash
# Full analysis: AST similarity + deps + TODOs + lint + line ratios
make ast-deep
# or
make porting-report

# Find files missing from C++ port
make ast-missing
```

### Example Output

**`make ast-deep` output:**
```
=== Porting Quality Summary ===
Matched by header:    59 / 111
Matched by name:      52 / 111
Total TODOs in target: 176
Total lint errors:    136
Stub files:           33

=== Files with Issues ===
File                          Sim     Ratio   TODOs Lint  Status
----------------------------------------------------------------------
channels.Channels             0.39    0.00    1     0     STUB
flow.SharedFlow               0.54    0.00    0     2     LINT
...

=== Porting Recommendations ===
Top priority to complete:
  flow.Channels        sim=0.39 deps=14 [STUB] [1 TODOs]
  flow.Flow            sim=0.54 deps=13
```

---

## AST Distance Tool

The underlying tool powering the CMake targets. Located at `tools/ast_distance/`.

### Direct CLI Usage

```bash
# From project root (after building)
./build/bin/ast_distance --help

# Compare two files directly
./build/bin/ast_distance file1.kt kotlin file2.cpp cpp

# Dump AST structure
./build/bin/ast_distance --dump <file> <rust|kotlin|cpp>

# Scan directory for dependencies
./build/bin/ast_distance --deps <directory> <rust|kotlin|cpp>
```

### Similarity Thresholds

| Score | Status | Action |
|-------|--------|--------|
| > 0.85 | Excellent | Likely complete, verify docs match |
| 0.60–0.85 | Good | May need refinement |
| 0.40–0.60 | Partial | Significant gaps, prioritize |
| < 0.40 | Stub | Needs full implementation |

### File Matching

The tool uses two matching strategies:
1. **Header-based** (preferred): Reads `Transliterated from:` comments in C++ files
2. **Name-based** (fallback): Fuzzy matches file names with snake_case conversion

Always include this header in transliterated files:
```cpp
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt
 */
```

### Tool Location

```
tools/ast_distance/
├── include/
│   ├── ast_parser.hpp      # Tree-sitter parsing for Rust/Kotlin/C++
│   ├── codebase.hpp        # Directory scanning, dependency graphs, matching
│   ├── imports.hpp         # Import/include extraction, package detection
│   ├── porting_utils.hpp   # TODO scanning, lint checks, file stats
│   ├── node_types.hpp      # Normalized AST node type mappings
│   ├── similarity.hpp      # Cosine similarity, combined scoring
│   └── tree.hpp            # Tree data structure
└── src/
    └── main.cpp            # CLI entry point
```

---

## Interpreting Lint Errors

### Unused Parameters

```
file.hpp:42: unused_param: Unused parameter 'ctx' in function 'dispatch'
```

**Fix options:**
1. Use the parameter (preferred if it should be used)
2. Cast to void: `(void)ctx;`
3. Rename with underscore prefix: `_ctx` (if intentionally unused)

### Missing Header Guards

```
file.hpp:1: missing_guard: Missing header guard (#pragma once or #ifndef)
```

**Fix:** Add `#pragma once` at the top of the file.

---

## TODO Tag Reference

When the tool scans TODOs, it groups by tag:

| Tag | Meaning |
|-----|---------|
| `port` | Direct transliteration needed |
| `semantics` | Correctness/race condition not yet mirrored |
| `suspend-plugin` | Will be handled by Clang plugin |
| `abi-ownership` | Ownership of `void*` return unclear |
| `perf` | Known performance gap |
| `untagged` | Missing tag — should be categorized |

**Goal:** Zero untagged TODOs. Every TODO should have a category.
