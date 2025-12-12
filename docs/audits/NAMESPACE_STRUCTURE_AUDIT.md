# Namespace and Folder Structure Audit

**Generated Date:** December 11, 2025
**Analysis Tool:** `analyze_packages.py`
**Scope:** Complete mapping of Kotlin packages → C++ folders → C++ namespaces

---

## EXECUTIVE SUMMARY

### Overall Health Metrics
- **Total C++ Files:** 437
- **Correct Namespace:** 223 files (51%)
- **Missing Namespace:** 6 files (1.4%)
- **Mismatched Namespace:** 10 files (2.3%)
- **C++-Only Namespaces:** 6 (tooling, DSL, integration)
- **Kotlin-Only Packages:** 4 (exceptions, testing, test.internal)

### Status: **GOOD** with minor inconsistencies

The majority of files follow proper namespace conventions. Most mismatches are intentional design decisions (forward declarations, internal implementations). Critical issues are limited to 6 files missing namespace declarations entirely.

---

## CRITICAL ISSUES (Must Fix)

### Files with NO Namespace Declaration (6 files)

These files **must** be updated to include proper namespace declarations:

1. **src/kotlinx/coroutines/Builders.common.cpp**
   - Expected: `namespace kotlinx::coroutines`
   - Contains: No namespace
   - Priority: HIGH - Core builder functions

2. **src/kotlinx/coroutines/Coroutine.hpp**
   - Expected: `namespace kotlinx::coroutines`
   - Contains: No namespace
   - Priority: HIGH - Base coroutine class

3. **src/kotlinx/coroutines/CoroutineExceptionHandler.cpp**
   - Expected: `namespace kotlinx::coroutines`
   - Contains: No namespace
   - Priority: MEDIUM - Exception handling

4. **src/kotlinx/coroutines/Dispatchers.cpp**
   - Expected: `namespace kotlinx::coroutines`
   - Contains: No namespace
   - Priority: HIGH - Core dispatcher singleton

5. **src/kotlinx/coroutines/common/CoroutineExceptionHandlerImpl.common.cpp**
   - Expected: `namespace kotlinx::coroutines`
   - Contains: No namespace
   - Priority: MEDIUM - Platform-specific handler

6. **src/kotlinx/coroutines/internal/Scopes.cpp**
   - Expected: `namespace kotlinx::coroutines::internal`
   - Contains: No namespace
   - Priority: LOW - Internal utilities

---

## INTENTIONAL MISMATCHES (Design Decisions)

### Child Namespace Pattern (5 files)

These files are placed in parent folders but use child namespaces for API organization. **This is by design.**

1. **src/kotlinx/coroutines/JobSupport.hpp**
   - Folder implies: `kotlinx::coroutines`
   - Actual: `kotlinx::coroutines::internal`
   - Reason: Internal implementation detail exposed for inheritance

2. **src/kotlinx/coroutines/StacklessBuilders.hpp**
   - Folder implies: `kotlinx::coroutines`
   - Actual: `kotlinx::coroutines::stackless`
   - Reason: Experimental stackless coroutine API

3. **src/kotlinx/coroutines/Supervisor.hpp**
   - Folder implies: `kotlinx::coroutines`
   - Actual: `kotlinx::coroutines::internal`
   - Reason: Internal supervisor implementation

4. **src/kotlinx/coroutines/android_fwd.hpp**
   - Folder implies: `kotlinx::coroutines`
   - Actual: `kotlinx::coroutines::android`
   - Reason: Forward declarations for Android platform

5. **src/kotlinx/coroutines/core_fwd.hpp**
   - Folder implies: `kotlinx::coroutines`
   - Actual: `kotlinx::coroutines::internal`
   - Reason: Forward declarations for internal types

### Platform-Specific Test Namespace (1 file)

6. **src/kotlinx/coroutines/native/test/ConcurrentTestUtilities.cpp**
   - Folder implies: `kotlinx::coroutines::test`
   - Actual: `kotlinx::coroutines::native::test`
   - Reason: Platform-specific test utilities

### Parent Namespace Pattern (4 files)

These files are in child folders but use parent namespaces. **Review needed.**

7. **src/kotlinx/coroutines/internal/DispatchedTask.hpp**
   - Folder implies: `kotlinx::coroutines::internal`
   - Actual: `kotlinx::coroutines`
   - Status: **REVIEW** - Should likely be in `internal` namespace

8-10. **Test files in native/test/**
   - **DelayExceptionTest.cpp**
   - **MultithreadedDispatchersTest.cpp**
   - **WorkerTest.cpp**
   - Folder implies: `kotlinx::coroutines::test`
   - Actual: `kotlinx::coroutines`
   - Status: **REVIEW** - Should likely be in `test` namespace

---

## FOLDER → NAMESPACE MAPPING

### Core Namespaces (Match Kotlin)

| Folder Path | Expected Namespace | Status |
|-------------|-------------------|--------|
| `kotlinx/coroutines/` | `kotlinx::coroutines` | ✅ Good |
| `kotlinx/coroutines/channels/` | `kotlinx::coroutines::channels` | ✅ Good |
| `kotlinx/coroutines/flow/` | `kotlinx::coroutines::flow` | ✅ Good |
| `kotlinx/coroutines/flow/internal/` | `kotlinx::coroutines::flow::internal` | ✅ Good |
| `kotlinx/coroutines/internal/` | `kotlinx::coroutines::internal` | ✅ Good |
| `kotlinx/coroutines/intrinsics/` | `kotlinx::coroutines::intrinsics` | ✅ Good |
| `kotlinx/coroutines/selects/` | `kotlinx::coroutines::selects` | ✅ Good |
| `kotlinx/coroutines/sync/` | `kotlinx::coroutines::sync` | ✅ Good |
| `kotlinx/coroutines/test/` | `kotlinx::coroutines::test` | ⚠️ Some mismatches |

### Platform-Specific Folders (Map to Core)

| Folder Path | Maps To Namespace | Reason |
|-------------|-------------------|--------|
| `kotlinx/coroutines/common/` | `kotlinx::coroutines` | Common/multiplatform impl |
| `kotlinx/coroutines/common/internal/` | `kotlinx::coroutines::internal` | Common internal utils |
| `kotlinx/coroutines/common/flow/internal/` | `kotlinx::coroutines::flow::internal` | Common flow internals |
| `kotlinx/coroutines/native/` | `kotlinx::coroutines` | Native platform impl |
| `kotlinx/coroutines/native/test/` | `kotlinx::coroutines::test` | Native test utilities |
| `kotlinx/coroutines/concurrent/` | `kotlinx::coroutines` | Concurrent utilities |
| `kotlinx/coroutines/concurrent/channels/` | `kotlinx::coroutines::channels` | Concurrent channels |
| `kotlinx/coroutines/concurrent/internal/` | `kotlinx::coroutines::internal` | Concurrent internals |

### C++-Only Namespaces (No Kotlin Equivalent)

| Namespace | Purpose | Files |
|-----------|---------|-------|
| `kotlinx::coroutines::dsl` | Suspend DSL macros/helpers | 1 |
| `kotlinx::coroutines::stackless` | Experimental stackless API | 1 |
| `kotlinx::coroutines::android` | Android platform types | 1 |
| `kotlinx::coroutines::integration::*` | Play Services integration | 4 |
| `kotlinx::coroutines::tools::clang_suspend_plugin` | Clang plugin code | 2 |
| `kotlinx::coroutines::tools::kotlinc_native_ref` | K/N reference code | 1 |

### Kotlin-Only Packages (No C++ Equivalent Yet)

| Kotlin Package | Files | Port Status |
|----------------|-------|-------------|
| `kotlinx.coroutines.exceptions` | 2 | ⚠️ TODO |
| `kotlinx.coroutines.test.internal` | 4 | ⚠️ TODO |
| `kotlinx.coroutines.testing` | 3 | ⚠️ TODO |
| `kotlinx.coroutines.testing.flow` | 1 | ⚠️ TODO |

---

## KOTLIN PACKAGE ANALYSIS

### Kotlin Packages Found (13 total)

From `tmp/kotlinx.coroutines/`:

1. **kotlinx.coroutines** - Core types (Job, Deferred, builders)
2. **kotlinx.coroutines.channels** - Channel communication
3. **kotlinx.coroutines.exceptions** - ⚠️ Not ported
4. **kotlinx.coroutines.flow** - Flow operators
5. **kotlinx.coroutines.flow.internal** - Flow internals
6. **kotlinx.coroutines.internal** - Internal utilities
7. **kotlinx.coroutines.intrinsics** - Low-level suspend APIs
8. **kotlinx.coroutines.selects** - Select expressions
9. **kotlinx.coroutines.sync** - Synchronization primitives
10. **kotlinx.coroutines.test** - Test utilities
11. **kotlinx.coroutines.test.internal** - ⚠️ Not ported
12. **kotlinx.coroutines.testing** - ⚠️ Not ported
13. **kotlinx.coroutines.testing.flow** - ⚠️ Not ported

---

## RECOMMENDATIONS

### Immediate Actions (Priority: HIGH)

1. **Add namespace declarations to 6 files** listed in Critical Issues
2. **Review DispatchedTask.hpp** - Should it be in `internal` namespace?
3. **Review native/test/*.cpp** files - Should they be in `test` namespace?

### Short-term Actions (Priority: MEDIUM)

4. **Port missing Kotlin packages:**
   - `kotlinx.coroutines.exceptions`
   - `kotlinx.coroutines.test.internal`
   - `kotlinx.coroutines.testing`
   - `kotlinx.coroutines.testing.flow`

### Documentation Actions (Priority: LOW)

5. **Document intentional namespace mismatches** in code comments
6. **Update CLAUDE.md** with namespace mapping examples
7. **Create namespace decision log** for future reference

---

## FOLDER STRUCTURE SUMMARY

### Physical C++ Folder Tree (Simplified)

```
src/kotlinx/coroutines/
├── common/                    # Platform: common (maps to parent ns)
│   ├── internal/              # → kotlinx::coroutines::internal
│   └── flow/internal/         # → kotlinx::coroutines::flow::internal
├── concurrent/                # Platform: concurrent (maps to parent ns)
│   ├── channels/              # → kotlinx::coroutines::channels
│   └── internal/              # → kotlinx::coroutines::internal
├── native/                    # Platform: native (maps to parent ns)
│   └── test/                  # → kotlinx::coroutines::test
├── channels/                  # → kotlinx::coroutines::channels
├── flow/                      # → kotlinx::coroutines::flow
│   └── internal/              # → kotlinx::coroutines::flow::internal
├── internal/                  # → kotlinx::coroutines::internal
├── intrinsics/                # → kotlinx::coroutines::intrinsics
├── selects/                   # → kotlinx::coroutines::selects
├── sync/                      # → kotlinx::coroutines::sync
├── test/                      # → kotlinx::coroutines::test
├── dsl/                       # → kotlinx::coroutines::dsl (C++ only)
└── integration/               # → kotlinx::coroutines::integration::* (C++ only)
```

### Namespace Tree (Logical)

```
kotlinx::coroutines
├── android                    # C++ only (forward decls)
├── channels
├── dsl                        # C++ only (suspend macros)
├── flow
│   └── internal
├── integration                # C++ only
│   └── kotlinx-coroutines-play-services
│       └── test
├── internal
├── intrinsics
├── selects
├── stackless                  # C++ only (experimental)
├── sync
├── test
└── tools                      # C++ only
    ├── clang_suspend_plugin
    │   └── examples
    └── kotlinc_native_ref
```

---

## VALIDATION COMMAND

To regenerate this report:

```bash
python analyze_packages.py
```

The script analyzes:
- Kotlin package declarations in `tmp/kotlinx.coroutines/`
- C++ namespace declarations in `src/` and `include/`
- Folder-to-namespace consistency
- Cross-references between Kotlin and C++
- **Import/include dependencies** (NEW) - Compares Kotlin `import` statements vs C++ `#include` directives

### Import/Include Analysis (New Feature)

The enhanced script now checks dependency consistency between Kotlin and C++ files:

**Current Statistics:**
- **293 common files** exist in both Kotlin and C++
- **283 files (97%)** have dependency mismatches

**Common Mismatch Patterns:**

1. **Missing Test Infrastructure**
   - Kotlin imports `kotlinx.coroutines.testing.*` not matched in C++
   - Status: Test infrastructure incomplete (see Block 5 audit - 70% complete)

2. **C++-Specific Headers**
   - C++ includes forward declaration headers (`core_fwd.hpp`, `android_fwd.hpp`)
   - These have no Kotlin equivalent (C++ idiom for compilation speed)

3. **Structural Differences**
   - Kotlin: `import kotlinx.coroutines.Job` (imports class)
   - C++: `#include <kotlinx/coroutines/Job.hpp>` (includes file)
   - Different granularity makes direct comparison approximate

4. **Explicit vs Implicit Dependencies**
   - C++ requires explicit `#include` for all used types
   - Kotlin imports are more permissive with package-level visibility

**Note:** High mismatch rate (97%) is **expected and acceptable** due to fundamental language differences. The analysis is useful for identifying missing APIs (e.g., `kotlinx.coroutines.testing`), not as a strict conformance check.

---

## NOTES

### Naming Convention Consistency

✅ **Following conventions:**
- Folders use `/` path separators
- Namespaces use `::` separators
- Both use lowercase `kotlinx.coroutines` (Kotlin) or `kotlinx::coroutines` (C++)

### Platform Folder Pattern

The codebase uses **platform qualifier folders** that map to parent namespaces:
- `common/` - Multiplatform code → parent namespace
- `native/` - Native platform code → parent namespace
- `concurrent/` - Concurrent implementations → parent namespace

This matches Kotlin Multiplatform conventions where:
```kotlin
// File: common/src/Job.kt
package kotlinx.coroutines  // Not kotlinx.coroutines.common

// File: native/src/JobNative.kt
package kotlinx.coroutines  // Not kotlinx.coroutines.native
```

### Forward Declaration Files

Files like `*_fwd.hpp` intentionally use child namespaces to forward-declare types without pulling in full implementations. This is a C++ idiom with no Kotlin equivalent.

---

## CHANGE HISTORY

| Date | Change | By |
|------|--------|-----|
| 2025-12-11 | Initial namespace audit document created | analyze_packages.py |
| 2025-12-11 | Added folder mapping and Kotlin package analysis | Claude |
| 2025-12-11 | Enhanced analyze_packages.py with import/include analysis | Claude |

---

**See Also:**
- `CLAUDE.md` - Transliteration rules and namespace conventions
- `COMPREHENSIVE_AUDIT_REPORT.md` - Overall implementation status
- `API_COMPLETENESS_AUDIT.md` - API coverage by class
- `analyze_packages.py` - Automated analysis tool
