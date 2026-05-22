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
- Define a deleter path for every `void*` you return — if you can't, port the dependency that makes ownership clear before writing the call site
- Prefer `std::shared_ptr`/`std::unique_ptr` for lifetimes

---

## No placeholders, no `TODO` comments

**Hard rule: zero `TODO` / `FIXME` / "not implemented" / "stub" comments in source.** If a function isn't ready, port it. If the dependency isn't ready, port the dependency first. If something is genuinely undecidable until upstream Clang plugin work lands, that work is itself the next ticket — open it, link it from a commit message or design doc, and ship code that is upstream-faithful *as written*. Source files are not a place to leave reminders.

This rule applies to comments and identifiers: `// TODO`, `// FIXME`, `// XXX`, `// HACK`, "stub", "placeholder text" — none of those belong in `src/`. The only exceptions are upstream-faithful translated content (e.g. Kotlin's `TODO()` intrinsic translated as `kotlin_todo()`) and domain concepts that legitimately use the word "Placeholder" as a class name (LockFreeTaskQueue's array-slot sentinel).

No taxonomy of "acceptable" TODO tags. Every match is a defect — fix it in the same pass.

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
3. No gap is "called out with a TODO" — if there's a gap, close it or port the dependency that closes it
4. Compile cleanly: `clang++ -std=c++20 -Wall -Wextra -I include your_file.cpp`
5. Tests pass: `./test_suspend`
6. Update `docs/audits/*` to reflect new API presence with file:line
7. Include `Transliterated from:` header in new files for proper matching

---

## Don'ts

- Don't refactor semantics or introduce new abstractions during transliteration
- Don't dump large implementations into headers; keep headers slim
- Don't introduce templates unless the Kotlin API requires it at the public surface
- Don't paper over semantic gaps. Don't paper over them with a `TODO` either — `TODO` comments are banned. Port the missing piece, or surface the blocker outside the source tree (commit message, audit doc, issue).
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

Transliteration first, helpers second. Keep it mechanical and reversible. When you find a mismatch, close it: port the missing piece in the same pass. Do not call it out with a `TODO` — those are banned. If a gap is genuinely blocked on the Clang suspend plugin, the blocker belongs in a commit message or audit doc, not in the source.

---

## File Matching

When a new C++ file ports a Kotlin counterpart, include this header so the file's provenance is unambiguous:

```cpp
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Channels.kt
 */
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
