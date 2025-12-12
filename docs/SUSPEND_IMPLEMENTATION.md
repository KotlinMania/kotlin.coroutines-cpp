# Suspend Function Implementation in kotlinx.coroutines-cpp

**Version:** 2.0
**Last Updated:** December 11, 2025
**Status:** Active Implementation Guide
**Supersedes:** SUSPEND_COMPARISON.md (legacy macro documentation)

---

## Executive Summary

This document describes **how suspend functions work** in kotlinx.coroutines-cpp and provides the **authoritative guide** for implementing suspend code. It combines information from the Clang plugin implementation, the Kotlin/Native compiler internals, and the runtime ABI.

### Current Implementation Status

**✅ Production Approach (2025):**
- **Clang Plugin** (`tools/clang_suspend_plugin/`) - Generates Kotlin-style state machines
- **DSL Syntax** - Clean `[[suspend]]` + `suspend(...)` spelling
- **Sidecar Generation** - Plugin emits `.kx.cpp` files with state machines
- **ContinuationImpl Runtime** - Full resume loop and suspension propagation

**⚠️ Legacy Approach (Deprecated):**
- **SuspendMacros.hpp** - Manual macro-based state machines (still supported for compatibility)
- **SUSPEND_BEGIN/CALL/END** - Old explicit state machine macros
- Documentation in old SUSPEND_COMPARISON.md is outdated

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Suspend Function ABI](#suspend-function-abi)
3. [Using the Clang Plugin](#using-the-clang-plugin)
4. [Writing Suspend Functions](#writing-suspend-functions)
5. [How It Works Internally](#how-it-works-internally)
6. [Kotlin/Native Equivalence](#kotlinnative-equivalence)
7. [Migration Guide](#migration-guide)
8. [Troubleshooting](#troubleshooting)

---

## Architecture Overview

### The Three Layers

```
┌─────────────────────────────────────────────────────────────┐
│  1. User Code (DSL)                                          │
│     [[suspend]] void* foo() { suspend(bar()); }             │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│  2. Clang Plugin (tools/clang_suspend_plugin/)               │
│     Detects [[suspend]] + suspend() → Generates .kx.cpp      │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│  3. Runtime (ContinuationImpl, COROUTINE_SUSPENDED)          │
│     State machine execution + suspension propagation         │
└─────────────────────────────────────────────────────────────┘
```

### Design Goals

1. **Kotlin/Native Parity** - Generate identical state machines to kotlinc
2. **Clean Syntax** - Kotlin-like DSL with minimal C++ noise
3. **Zero Runtime Cost** - `suspend(...)` is compile-time only
4. **LLVM Optimization** - State machines compile to `indirectbr` instructions
5. **Interop Ready** - ABI matches Kotlin/Native for future bridging

---

## Suspend Function ABI

### Signature Convention

All suspend functions follow this pattern:

```cpp
void* function_name(/* args */, std::shared_ptr<Continuation<void*>> completion)
```

**Return Value:**
- `COROUTINE_SUSPENDED` - Function suspended, will resume later
- `void*` - Boxed result (for non-Unit types), function completed immediately
- `nullptr` - Unit result, function completed immediately

### Suspension Marker

```cpp
// Defined in include/kotlinx/coroutines/intrinsics/Intrinsics.hpp
namespace kotlinx::coroutines::intrinsics {
    void* get_COROUTINE_SUSPENDED();  // Returns singleton marker
    bool is_coroutine_suspended(void* result);  // Checks if result == marker
}
```

**Critical:** Never create your own suspended marker - always use `get_COROUTINE_SUSPENDED()`.

### State Machine Pattern

Every suspend function compiles to this structure:

```cpp
struct FunctionNameCoroutine : public ContinuationImpl {
    int _label = 0;              // Current suspension point
    // ... captured parameters ...

    void* invoke_suspend(Result<void*> result) override {
        switch (_label) {
        case 0:                  // Fresh start
            // ... code before first suspend point ...
            _label = 1;
            {
                void* _tmp = suspending_call(completion);
                if (is_coroutine_suspended(_tmp)) return COROUTINE_SUSPENDED;
            }
            [[fallthrough]];
        case 1:                  // Resume after first suspension
            // ... code after first suspend point ...
            break;
        }
        return nullptr;          // Completed
    }
};

void* function_name(/* args */, std::shared_ptr<Continuation<void*>> completion) {
    auto coro = std::make_shared<FunctionNameCoroutine>(completion, /* args */);
    return coro->invoke_suspend(Result<void*>::success(nullptr));
}
```

---

## Using the Clang Plugin

### Building the Plugin

```bash
# Enable plugin build in CMake
mkdir build && cd build
cmake -DKOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=ON ..
make KotlinxSuspendPlugin

# Plugin output: build/lib/KotlinxSuspendPlugin.dylib (macOS)
#                build/lib/KotlinxSuspendPlugin.so (Linux)
```

### Invoking the Plugin

```bash
clang++ -fsyntax-only \
  -Xclang -load -Xclang build/lib/KotlinxSuspendPlugin.dylib \
  -Xclang -plugin -Xclang kotlinx-suspend \
  -Xclang -plugin-arg-kotlinx-suspend -Xclang out-dir=build/kxs_generated \
  -I include \
  your_file.cpp
```

**Plugin Arguments:**
- `out-dir=<path>` - Directory for generated `.kx.cpp` files (default: `kxs_generated`)

### Generated Output

For input file `example.cpp`, the plugin generates `build/kxs_generated/example.kx.cpp` containing:
- Coroutine class definitions extending `ContinuationImpl`
- `invoke_suspend()` state machines with proper label dispatch
- Wrapper functions matching original signatures

**Build Integration:**
Add generated `.kx.cpp` files to your CMake targets or compile them separately.

---

## Writing Suspend Functions

### Modern DSL Syntax (Recommended)

```cpp
#include <kotlinx/coroutines/ContinuationImpl.hpp>
#include <kotlinx/coroutines/dsl/Suspend.hpp>

using namespace kotlinx::coroutines::dsl;

// Declare a suspend function
[[suspend]]
void* fetch_data(int id, std::shared_ptr<Continuation<void*>> completion) {
    // Regular code
    validate_id(id);

    // Suspend point - calls another suspend function
    suspend(async_load(id, completion));

    // More code after resumption
    process_data();

    return nullptr;  // Unit result
}

// Non-Unit result
[[suspend]]
void* compute(int x, std::shared_ptr<Continuation<void*>> completion) {
    suspend(delay(100, completion));

    int* result = new int(x * 2);
    return reinterpret_cast<void*>(result);  // Boxed result
}
```

**Key Elements:**
1. **`[[suspend]]` attribute** - Marks function for plugin processing
2. **`suspend(expr)` wrapper** - Marks suspension points (zero-cost at runtime)
3. **`completion` parameter** - Always last parameter, type `std::shared_ptr<Continuation<void*>>`
4. **Return `void*`** - Either marker, nullptr, or boxed pointer

### Alternative Syntax

```cpp
// Namespaced attribute for functions (equivalent to [[suspend]])
[[kotlinx::suspend]]
void* foo(std::shared_ptr<Continuation<void*>> completion);

// Statement attribute for suspend points (alternative to suspend(...) wrapper)
[[clang::annotate("suspend")]]
bar(completion);
```

### What Gets Captured

The plugin automatically captures all function parameters **except `completion`** into the coroutine class:

```cpp
[[suspend]]
void* process(int x, std::string name, std::shared_ptr<Continuation<void*>> completion) {
    // Generated coroutine struct has:
    // int x_;
    // std::string name_;
    // (completion is passed to ContinuationImpl base)
}
```

### Returning Results

**Unit (void) result:**
```cpp
return nullptr;
```

**Non-void result:**
```cpp
// Heap-allocate the result
int* result = new int(42);
return reinterpret_cast<void*>(result);

// Caller must unbox and manage lifetime
// TODO(abi-ownership): Define deleter conventions
```

---

## How It Works Internally

### Plugin Phase (Compile-Time)

**File:** `tools/clang_suspend_plugin/KotlinxSuspendPlugin.cpp`

1. **Detection** - `KotlinxSuspendVisitor` traverses AST
   - Finds `FunctionDecl` with `[[suspend]]` or `[[clang::annotate("kotlinx_suspend")]]`
   - Finds suspend points via `suspend(...)` calls or statement attributes

2. **Sidecar Generation** - `KotlinxSuspendConsumer::emitSidecar()`
   - Creates coroutine struct extending `ContinuationImpl`
   - Generates `invoke_suspend()` with switch-based state machine
   - Captures parameters (except `completion`)
   - Inserts label assignments and suspension checks at each suspend point

3. **State Machine Structure:**
   ```cpp
   switch (_label) {
   case 0:
       // ... code before first suspend point ...
       _label = 1;
       {
           void* _tmp = suspend(other_suspend_fn(completion));
           if (is_coroutine_suspended(_tmp)) return COROUTINE_SUSPENDED;
       }
       [[fallthrough]];
   case 1:
       // ... code after first suspend point ...
       break;
   }
   ```

### Runtime Phase (Execution)

**File:** `include/kotlinx/coroutines/ContinuationImpl.hpp`

1. **Initial Call** - User calls wrapper function
   - Wrapper creates coroutine object
   - Calls `invoke_suspend(Result<void*>::success(nullptr))`

2. **Suspension** - When suspend point is hit
   - Label is set to next resume point
   - Callee returns `COROUTINE_SUSPENDED`
   - Caller propagates marker up the call stack
   - Control returns to event loop

3. **Resumption** - When async operation completes
   - `BaseContinuationImpl::resume_with()` is called
   - Runs loop to unroll continuation chain
   - Calls `invoke_suspend(result)` with result from callee
   - Switch dispatches to saved label
   - Execution continues after suspend point

### LLVM Optimization

With `-O2`, Clang optimizes the switch statement into:

1. **Bounds check** on `_label`
2. **Jump table** - Array of block addresses
3. **Indirect branch** - `indirectbr` instruction (same as Kotlin/Native)

```llvm
; Generated LLVM IR (simplified)
%target = getelementptr [3 x i8*], [3 x i8*]* @switch.table, i64 %_label
indirectbr i8* %target, [label %case0, label %case1, label %case2]
```

**Performance:** Single indirect jump, no loop overhead, identical to Kotlin/Native.

---

## Kotlin/Native Equivalence

### Source Comparison

**Kotlin:**
```kotlin
suspend fun example() {
    dummy()
    println(1)
    dummy()
    println(2)
}
```

**C++ (DSL):**
```cpp
[[suspend]]
void* example(std::shared_ptr<Continuation<void*>> completion) {
    suspend(dummy(completion));
    std::cout << 1 << std::endl;
    suspend(dummy(completion));
    std::cout << 2 << std::endl;
    return nullptr;
}
```

### Compiled Output Comparison

**Kotlin (kotlinc-native output):**
```kotlin
class ExampleCoroutine : ContinuationImpl(...) {
    var label: Int = 0

    override fun invokeSuspend(result: Any?): Any? {
        when (label) {
            0 -> {
                label = 1
                val tmp = dummy(this)
                if (tmp === COROUTINE_SUSPENDED) return COROUTINE_SUSPENDED
                // fallthrough
            }
            1 -> {
                println(1)
                label = 2
                val tmp = dummy(this)
                if (tmp === COROUTINE_SUSPENDED) return COROUTINE_SUSPENDED
                // fallthrough
            }
            2 -> {
                println(2)
                return Unit
            }
        }
    }
}
```

**C++ (plugin-generated `.kx.cpp`):**
```cpp
struct __kxs_coroutine_example : public ContinuationImpl {
    int _label = 0;

    void* invoke_suspend(Result<void*> result) override {
        switch (_label) {
        case 0:
            _label = 1;
            {
                void* _tmp = dummy(completion);
                if (is_coroutine_suspended(_tmp)) return COROUTINE_SUSPENDED;
            }
            [[fallthrough]];
        case 1:
            std::cout << 1 << std::endl;
            _label = 2;
            {
                void* _tmp = dummy(completion);
                if (is_coroutine_suspended(_tmp)) return COROUTINE_SUSPENDED;
            }
            [[fallthrough]];
        case 2:
            std::cout << 2 << std::endl;
            break;
        }
        return nullptr;
    }
};
```

### Key Differences (Intentional)

| Aspect | Kotlin/Native | C++ Port | Notes |
|--------|---------------|----------|-------|
| **Label Storage** | `var label: Int` | `int _label` | Same semantics |
| **Marker Type** | `Any?` | `void*` | Both use singleton marker |
| **Type Erasure** | `Any?` | `void*` | Equivalent erasure |
| **Base Class** | `ContinuationImpl` | `ContinuationImpl` | Same inheritance |
| **Ownership** | GC | `shared_ptr` | Different memory model |
| **Return Type** | `Any?` | `void*` | Same type erasure |

### Runtime Intrinsics Mapping

| Kotlin/Native | C++ Port | Location |
|---------------|----------|----------|
| `COROUTINE_SUSPENDED` | `get_COROUTINE_SUSPENDED()` | `Intrinsics.hpp` |
| `suspendCoroutineUninterceptedOrReturn` | `suspend_coroutine_unintercepted_or_return<T>()` | `Intrinsics.hpp` |
| `ContinuationImpl` | `ContinuationImpl` | `ContinuationImpl.hpp` |
| `BaseContinuationImpl` | `BaseContinuationImpl` | `ContinuationImpl.hpp` |
| `Continuation<T>` | `Continuation<T>` | `Continuation.hpp` |

---

## Migration Guide

### From Macros (Pre-2025) to Plugin (2025+)

**Old Code (SuspendMacros.hpp):**
```cpp
class MySuspendFn : public SuspendLambda<int> {
public:
    void* invoke_suspend(Result<void*> result) override {
        void* tmp;

        SUSPEND_BEGIN(2)

        SUSPEND_CALL(1, foo(this), tmp)
        std::cout << "After foo" << std::endl;

        SUSPEND_RETURN(42);

        SUSPEND_END
    }
};
```

**New Code (Plugin DSL):**
```cpp
[[suspend]]
void* my_suspend_fn(std::shared_ptr<Continuation<void*>> completion) {
    suspend(foo(completion));
    std::cout << "After foo" << std::endl;

    int* result = new int(42);
    return reinterpret_cast<void*>(result);
}

// No manual state machine required - plugin generates it!
```

### Migration Checklist

- [ ] Replace `class XxxFn : public SuspendLambda<T>` with standalone function
- [ ] Add `[[suspend]]` attribute to function declaration
- [ ] Replace `SUSPEND_CALL(n, expr, tmp)` with `suspend(expr)`
- [ ] Remove `SUSPEND_BEGIN`, `SUSPEND_END`, `SUSPEND_RETURN` macros
- [ ] Add `completion` parameter as last parameter
- [ ] Remove manual `_label` management
- [ ] Update build to invoke plugin and compile `.kx.cpp` files

### Backward Compatibility

**SuspendMacros.hpp is still supported** for:
- Legacy code that hasn't migrated yet
- Platforms without Clang plugin support
- Hand-tuned state machines needing fine control

However, **new code should use the plugin DSL**.

---

## Troubleshooting

### Plugin Not Detecting Suspend Functions

**Symptom:** No `.kx.cpp` files generated, no remarks from plugin.

**Solutions:**
1. Verify plugin is loaded: `clang++ -Xclang -load -Xclang <path-to-plugin.dylib>`
2. Check function has `[[suspend]]` or `[[kotlinx::suspend]]` attribute
3. Ensure function has a body (declarations are skipped)
4. Run with `-fsyntax-only` to see plugin remarks

### Suspend Points Not Detected

**Symptom:** Plugin generates coroutine but doesn't create case labels for suspend points.

**Solutions:**
1. Verify using `suspend(expr)` wrapper from `kotlinx/coroutines/dsl/Suspend.hpp`
2. Or use `[[clang::annotate("suspend")]]` statement attribute
3. Check `using namespace kotlinx::coroutines::dsl;` is present

### Generated Code Doesn't Compile

**Symptom:** `.kx.cpp` files have syntax errors or missing includes.

**Solutions:**
1. Ensure original code includes `<kotlinx/coroutines/ContinuationImpl.hpp>`
2. Check that suspend point expressions are complete statements
3. Verify parameter types are copyable/movable for capture
4. Look at generated code for missing `std::` qualifiers or includes

### Linker Errors: Multiple Definitions

**Symptom:** Linker complains about duplicate symbols for suspend functions.

**Solutions:**
1. Don't compile original `.cpp` file if using plugin-generated `.kx.cpp`
2. Or use plugin to generate declarations only and implement manually
3. Check CMakeLists.txt isn't including both original and generated files

### Runtime: Crashes or Hangs

**Symptom:** Program crashes or hangs when suspend function is called.

**Solutions:**
1. Verify `completion` parameter is valid `shared_ptr`, not null
2. Check return value is tested with `is_coroutine_suspended()` at call sites
3. Ensure caller is a suspend function or runs in a coroutine scope
4. Use debugger to check `_label` field and switch dispatch

### IDE Errors (False Positives)

**Symptom:** IntelliJ/CLion shows red squiggles on valid `[[suspend]]` code.

**Solution:** This is cosmetic - IDE doesn't run plugin during static analysis.
```cpp
// Suppress IDE warnings
// NOLINT - IDE cannot parse suspend DSL
[[suspend]]
void* foo(std::shared_ptr<Continuation<void*>> completion) {
    suspend(bar(completion)); // NOLINT
    return nullptr;
}
```

Trust the compiler, not the IDE. If `clang++` compiles it, it's correct.

---

## Advanced Topics

### Variable Spilling

**Current Status:** Manual spilling required (Phase 1).

When locals must survive across suspension:
```cpp
struct MyCoroutine : public ContinuationImpl {
    int _label = 0;
    int x_spill;        // Spilled local
    std::string s_spill; // Spilled local

    void* invoke_suspend(Result<void*> result) override {
        int x;
        std::string s;

        switch (_label) {
        case 0:
            x = 42;
            s = "hello";

            // Spill before suspension
            x_spill = x;
            s_spill = s;
            _label = 1;
            {
                void* _tmp = suspend(other_suspend_fn(completion));
                if (is_coroutine_suspended(_tmp)) return COROUTINE_SUSPENDED;
            }
            [[fallthrough]];
        case 1:
            // Restore after resumption
            x = x_spill;
            s = s_spill;

            use(x, s);
            break;
        }
        return nullptr;
    }
};
```

**Future:** Phase 2 will add CFG liveness analysis for automatic spilling.

### Tail-Suspend Optimization

**Current Status:** Not implemented (Phase 4).

When all suspend calls are in tail position, Kotlin skips state machine generation:
```cpp
[[suspend]]
void* tail_example(std::shared_ptr<Continuation<void*>> completion) {
    // Direct delegation - no state machine needed
    return other_suspend_fn(completion);
}
```

**Future:** Plugin will detect tail calls and generate simpler code.

### Exception Handling Across Suspend

**Current Status:** Basic exception propagation works via `Result<T>`.

```cpp
[[suspend]]
void* may_throw(std::shared_ptr<Continuation<void*>> completion) {
    try {
        suspend(risky_operation(completion));
        return nullptr;
    } catch (const std::exception& e) {
        // Exception after suspension - propagate via Result
        return reinterpret_cast<void*>(new std::exception_ptr(std::current_exception()));
    }
}
```

**Caveat:** RAII destructors may not run correctly across suspend points yet.

---

## References

### Source Files

**Plugin Implementation:**
- `tools/clang_suspend_plugin/KotlinxSuspendPlugin.cpp` - Main plugin logic
- `tools/clang_suspend_plugin/README.md` - Plugin usage guide

**Runtime:**
- `include/kotlinx/coroutines/ContinuationImpl.hpp` - Base continuation classes
- `include/kotlinx/coroutines/intrinsics/Intrinsics.hpp` - Suspension markers
- `include/kotlinx/coroutines/dsl/Suspend.hpp` - DSL helpers
- `include/kotlinx/coroutines/Result.hpp` - Result wrapper for exceptions

**Legacy:**
- `include/kotlinx/coroutines/SuspendMacros.hpp` - Old macro approach (deprecated)

**Kotlin/Native Sources (Reference):**
- `tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt`
- `tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt`

### Documentation

- `docs/cpp_port/docking_ring.md` - Compiler internals and LLVM lowering
- `AGENTS.md` - Transliteration rules and TODO taxonomy
- `CLAUDE.md` - Project conventions and structure

### Tests

- `tests/test_suspend.cpp` - Demonstrates plugin-friendly state machines
- `tests/test_suspension_core.cpp` - Basic suspension infrastructure
- `tools/clang_suspend_plugin/examples/simple_suspend.cpp` - Minimal DSL example

---

## Changelog

**Version 2.0 (December 2025):**
- Complete rewrite reflecting plugin-based implementation
- Deprecated macro-based approach documentation
- Added plugin usage instructions
- Documented Kotlin/Native equivalence
- Added migration guide from macros to plugin

**Version 1.0 (Pre-2025):**
- Original SUSPEND_COMPARISON.md documenting macro approach
- Now superseded by this document

---

**Maintainer:** Update this document when plugin phases advance (Phase 2: auto-spilling, Phase 3: tail optimization).

**Next Review:** January 2026 or when Phase 2 is implemented.

---

*This is the authoritative suspend implementation guide for kotlinx.coroutines-cpp. All other suspend documentation is historical reference only.*
