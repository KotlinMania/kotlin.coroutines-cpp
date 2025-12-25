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
- C++20 compiler (clang++ on macOS)
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
- State machine with switch-based dispatch
- Automatic parameter capture
- Label management and suspension checks

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
1. Headers contain only the public surface and minimal ABI-critical code
2. Methods and enums follow naming rules; no camelCase methods remain in C++
3. All new gaps are called out with a specific, tagged `TODO`
4. Resolved `TODO`s are removed in edited regions
5. Compile cleanly: `g++ -std=c++20 -Wall -Wextra -I include your_file.cpp`
6. Tests pass: `./test_suspend`
7. Update `docs/audits/*` to reflect new API presence with file:line

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

## AST Distance Tool

A vendored cross-language AST comparison tool for analyzing port progress and identifying priority files.

### Location
```
tools/ast_distance/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── ast_parser.hpp      # Tree-sitter parsing for Rust/Kotlin/C++
│   ├── codebase.hpp        # Directory scanning, dependency graphs, matching
│   ├── imports.hpp         # Import/include extraction, package detection
│   ├── node_types.hpp      # Normalized AST node type mappings
│   ├── similarity.hpp      # Cosine similarity, combined scoring
│   └── tree.hpp            # Tree data structure
└── src/
    ├── main.cpp            # CLI entry point
    ├── ast_parser.cpp
    ├── ast_normalizer.cpp
    └── similarity.cpp
```

### Build
```bash
cd tools/ast_distance
mkdir -p build && cd build
cmake .. && make -j8
```

### Commands

**Analyze this project (Kotlin → C++):**
```bash
./ast_distance --deep \
    /path/to/kotlinx.coroutines/kotlinx-coroutines-core/common/src kotlin \
    ../../../src/kotlinx/coroutines cpp
```

**Check what's missing:**
```bash
./ast_distance --missing <src_dir> <src_lang> <tgt_dir> <tgt_lang>
```

**Scan a codebase (dependency graph):**
```bash
./ast_distance --deps <directory> <rust|kotlin|cpp>
```

**Compare two files directly:**
```bash
./ast_distance file1.kt file2.cpp
```

**Dump AST structure:**
```bash
./ast_distance --dump <file> <rust|kotlin|cpp>
```

### Output Interpretation

The `--deep` command outputs:
- **Matched files** with similarity scores (0.0–1.0)
- **Priority score** = dependents × (1 - similarity) — high priority = many dependents + low similarity
- **Incomplete ports** (similarity < 60%)
- **Missing files** not yet ported

Similarity thresholds:
- `> 0.85` — Excellent port, likely complete
- `0.60–0.85` — Good port, may need refinement
- `0.40–0.60` — Partial port, significant gaps
- `< 0.40` — Stub or very different implementation

### Extending the Tool

**Add a new language:**
1. Add tree-sitter grammar to `CMakeLists.txt` (FetchContent)
2. Add `extern "C" { const TSLanguage* tree_sitter_<lang>(); }` to `ast_parser.hpp` and `imports.hpp`
3. Add `<LANG>` to `enum class Language` in `ast_parser.hpp`
4. Add `<lang>_node_to_type()` mapping in `node_types.hpp`
5. Add import/package extraction in `imports.hpp`
6. Update file extension checks in `codebase.hpp`

**Improve matching:**
- Edit `name_match_score()` in `codebase.hpp` for better fuzzy matching
- Edit `PackageDecl::similarity_to()` in `imports.hpp` for package-aware matching

**Add new metrics:**
- Add to `ASTSimilarity` class in `similarity.hpp`
- Expose in `combined_similarity()` function

### Using for Porting Decisions

Run periodically to track progress:
```bash
# Save baseline
./ast_distance --deep ... > baseline.txt

# After porting work
./ast_distance --deep ... > current.txt
diff baseline.txt current.txt
```

Focus porting effort on files with:
1. High dependent count (core infrastructure)
2. Low similarity score (incomplete)
3. Listed in "Top priority to complete" output
