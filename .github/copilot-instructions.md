# Kotlin Coroutines C++ Port - Copilot Instructions

## Project Overview

**Purpose**: Port `kotlinx.coroutines` (Kotlin's async/await framework) to C++ with near 1:1 API equivalence using a Kotlin/Native-style Continuation ABI today and a Clang-based suspend lowering plugin going forward. A legacy macro-based state machine DSL exists but is being deprecated.

**Type**: C++ library (header-only and source files)  
**Size**: ~50K LOC across headers and implementation  
**Languages**: C++ (C++17 standard), Kotlin (source reference), Python (build scripts)  
**Key Frameworks**: 
- Kotlin coroutines (source reference in `tmp/kotlinx.coroutines/`)
- C++ Continuation ABI (custom, type-erased `void*` result + `Continuation<void*>*`)
- Clang plugin for suspend lowering (in development; replaces macro DSL)

## High-Level Architecture

### Directory Structure
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

kotlinx-coroutines-core/common/src/  # Implementation (.cpp)
  ├── Job.cpp, Delay.cpp, etc.      # State machines and schedulers
  └── ...

tmp/kotlinx.coroutines/              # Kotlin source reference (DO NOT EDIT)
  └── **/*.kt                        # Ground truth for transliteration

docs/                                # Documentation
  ├── SUSPEND_COMPARISON.md          # Kotlin vs C++ comparison
  ├── cpp_port/docking_ring.md       # Suspend/IR/LLVM plan and plugin design
  ├── API_AUDIT.md                   # API completeness tracker
  ├── API_TRANSLATION.md             # Translation patterns
  └── audits/                        # Per-file audit status

tools/clang_suspend_plugin/          # Clang plugin for suspend DSL
  └── (in development)
```

### Build System
- **CMake** (`CMakeLists.txt` in root and `kotlinx-coroutines-core/`)
- **GCC/Clang** (C++17, `-std=c++17`)
- **Test framework**: Custom test harness (no external test framework yet)

## Build and Test Instructions

### Prerequisites
```bash
# macOS (tested):
# - GCC 11+ or Clang 13+
# - CMake 3.15+
# - Python 3.8+ (for build scripts)

# Verify compiler:
g++ --version
```

### Bootstrap and Build
```bash
cd /Volumes/stuff/Projects/kotlin.coroutines-cpp

# Configure CMake (creates build/ directory)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build all targets
cd build
make -j4
cd ..

# Or compile individual tests directly:
g++ -std=c++17 -Wall -Wextra -I include test_suspend.cpp -o test_suspend
```

### Run Tests
```bash
# Main suspend test (validates state machine)
./test_suspend

# Output should show:
# === Suspend Function Tests ===
# Test 1: Simple suspend function...
#   Result: 42
#   PASSED
# [... more tests ...]
# === All tests passed ===

# Compile-time validation (no execution):
g++ -std=c++17 -Wall -Wextra -I include -fsyntax-only test_suspend.cpp
```

### Lint and Validation
```bash
# Check for real compiler warnings (ignore IDE false positives):
g++ -std=c++17 -Wall -Wextra -I include *.cpp -o /tmp/test 2>&1 | grep -v "false positive"

# Verify header includes work:
for f in include/kotlinx/coroutines/*.hpp; do
  echo '#include "'$f'"' | g++ -std=c++17 -I include -x c++ -c - -o /dev/null || echo "FAIL: $f"
done
```

### Known Issues and Workarounds

| Issue | Symptom | Workaround |
|-------|---------|-----------|
| IntelliJ IDE false positives | Parser confusion around legacy `SUSPEND_*` macros and future suspend annotations | Trust compiler; see `docs/cpp_port/docking_ring.md` (IDE notes) and `docs/SUSPEND_COMPARISON.md` |
| Incomplete transliterations | "never used" warnings in JobSupport.cpp, etc. | Intentional; marked with `TODO(port)` |
| No GC integration | Memory leaks if continuations aren't released | Use `std::shared_ptr`/`std::unique_ptr` explicitly; see `ContinuationImpl.hpp` |
| Delay fallback | Threaded fallback resume used when no dispatcher `Delay` is present | Marked `TODO(semantics)`/`TODO(perf)` in `Delay.cpp`; will integrate timer heap/wheel |

## Project Layout and Architecture

### Key Files to Know

**Continuation ABI** (foundation):
- `include/kotlinx/coroutines/Continuation.hpp` — Resume interface
- `include/kotlinx/coroutines/ContinuationImpl.hpp` — Base continuation impl
- `include/kotlinx/coroutines/Result.hpp` — Success/failure boxing
- `include/kotlinx/coroutines/intrinsics/Intrinsics.hpp` — `COROUTINE_SUSPENDED` + helpers

**Legacy Suspend Macros (deprecated)**:
- `include/kotlinx/coroutines/SuspendMacros.hpp` — `SUSPEND_BEGIN`, `SUSPEND_CALL`, etc.
- Present in some tests; do not introduce new usages. Prefer Continuation ABI and future plugin-based suspend.
- See `docs/SUSPEND_COMPARISON.md` for historical comparison

**Job and Cancellation**:
- `include/kotlinx/coroutines/Job.hpp` — Job interface
- `kotlinx-coroutines-core/common/src/JobSupport.cpp` — Implementation
- `include/kotlinx/coroutines/CancellableContinuationImpl.hpp` — Cancellable continuation

**Builders and DSL**:
- `include/kotlinx/coroutines/Builders.hpp` — `launch()`, `async()`
- `include/kotlinx/coroutines/StacklessBuilders.hpp` — Lightweight coroutine builders

**Dispatcher**:
- `include/kotlinx/coroutines/CoroutineDispatcher.hpp` — Scheduler interface
- `kotlinx-coroutines-core/common/src/Dispatchers.cpp` — Built-in dispatchers

**Delay and suspend helpers**:
- `include/kotlinx/coroutines/Delay.hpp` — Delay capability; `delay(...)` free functions
- `kotlinx-coroutines-core/common/src/Delay.cpp` — Implementation using `suspend_cancellable_coroutine`

### Transliteration Matching

Every C++ file corresponds to a Kotlin source:
- C++ header: `include/kotlinx/coroutines/Foo.hpp`
- Kotlin source: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Foo.kt`
- C++ impl: `kotlinx-coroutines-core/common/src/Foo.cpp`

Use this mapping to locate the ground truth when implementing or fixing APIs.

## Key Validation Steps

### Before Committing Changes

1. **Compile cleanly** (no real errors, only IDE false positives are acceptable):
   ```bash
   g++ -std=c++17 -Wall -Wextra -I include your_modified_file.cpp -o /tmp/test
   ```

2. **Update audit docs** if you add/rename public API:
   - Edit `docs/API_AUDIT.md` (add row or update status)
   - Update `docs/audits/*.md` with file:line reference

3. **Add TODO comments** for any unresolved semantic gap:
   - Format: `// TODO(tag): description`
   - Tags: `port`, `semantics`, `suspend-plugin`, `abi-ownership`, `perf`
   - See `.github/AGENTS.md` "TODO taxonomy" for full list

4. **Verify test still passes**:
   ```bash
   ./test_suspend
   ```

## Critical Context for Agents

### Naming Conventions
- Methods: `snake_case` (Kotlin `isActive` → C++ `is_active`)
- Types: `CamelCase`
- Enums: `enum class Name { kPascalCaseEnumerators }`
- Namespaces: `kotlinx::coroutines::*`

### Public vs Private Split
- **Headers (.hpp)**: Public interfaces, abstract bases, constants, forward declarations
- **Source (.cpp)**: Implementations, helper classes, internal logic (prefer concrete types)

### Suspend Function ABI
- Entry point: `void* fn(args..., Continuation<void*>* cont)`
- Return: Either `intrinsics::COROUTINE_SUSPENDED` or boxed result as `void*`
- Helper: `is_coroutine_suspended(result)` to check if suspended

### Type Erasure and Ownership
- Non-void results are typically boxed as heap-allocated values (e.g., `new T(...)`) and returned as `void*` via adapters.
- Callers must unbox and manage lifetime according to the surrounding API contract. If unclear, add `// TODO(abi-ownership): define deleter path`.
- For `void`/`Unit`, return `nullptr` on success; use `intrinsics::COROUTINE_SUSPENDED` to indicate suspension.

## Documentation References

- **Ground truth**: Kotlin sources in `tmp/kotlinx.coroutines/`
- **API status**: `docs/API_AUDIT.md`
- **Suspend plan (plugin)**: `docs/cpp_port/docking_ring.md`
- **Suspend macros**: `docs/SUSPEND_COMPARISON.md` (legacy reference)
- **Translation patterns**: `docs/API_TRANSLATION.md`
- **Agent playbook**: `.github/AGENTS.md` (comprehensive transliteration guide)

## Trust These Instructions

Only search or ask for clarification if:
1. Build/test commands fail in a new way
2. A file path or naming convention contradicts the instructions
3. You need to understand Kotlin semantics not covered here

Otherwise, follow the checklist above and refer to the docs listed in "Documentation References."

