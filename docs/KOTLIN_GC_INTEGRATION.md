# Using kotlinx.coroutines C++ Library with Kotlin Native GC

## The Problem Solved

When you call our C++ coroutine library from Kotlin Native:
- Kotlin has a GC that needs to know thread states
- Our C++ code might run for a long time
- We need to tell the GC: "don't wait for me during long operations"

## The Solution: Already Done!

Kotlin Native **already exposes** these functions in its runtime:

```cpp
extern "C" {
    void Kotlin_mm_switchThreadStateNative();    // Tell GC: "I'm in native code"
    void Kotlin_mm_switchThreadStateRunnable();  // Tell GC: "I'm back in Kotlin"
    void Kotlin_mm_safePointWhileLoopBody();     // Check if GC wants to pause
}
```

**Key insight**: If your C++ code is called FROM Kotlin Native, these functions are **already linked** - you just call them!

## Usage Pattern

### Option 1: Automatic with RAII Guard

```cpp
#include "kotlinx/coroutines/KotlinGCBridge.hpp"

extern "C" void long_running_cpp_operation() {
    // Create guard - switches to Native state
    kotlinx::coroutines::KotlinNativeStateGuard guard;
    
    // Do your work - GC won't wait for this thread
    for (int i = 0; i < 1000000; i++) {
        do_heavy_computation();
        
        // Optional: allow GC to pause if needed
        if (i % 1000 == 0) {
            kotlinx::coroutines::check_safepoint();
        }
    }
    
    // Guard destructor automatically switches back to Runnable
}
```

### Option 2: Manual Control

```cpp
#include "kotlinx/coroutines/KotlinGCBridge.hpp"

extern "C" void mixed_kotlin_cpp_operation(void (*kotlin_callback)(int)) {
    // Switch to Native for heavy work
    Kotlin_mm_switchThreadStateNative();
    
    do_heavy_computation();
    
    // Need to call Kotlin? Switch back first
    Kotlin_mm_switchThreadStateRunnable();
    kotlin_callback(42);
    
    // Back to Native for more work
    Kotlin_mm_switchThreadStateNative();
    do_more_work();
    
    // Done - switch back to Runnable
    Kotlin_mm_switchThreadStateRunnable();
}
```

### Option 3: Using with @GCUnsafeCall

```kotlin
// Kotlin side
@OptIn(InternalForKotlinNative::class)
@GCUnsafeCall("my_cpp_coroutine")
external fun launchCoroutine(callback: (Int) -> Unit)
```

```cpp
// C++ side - #include "kotlinx/coroutines/KotlinGCBridge.hpp"
extern "C" void my_cpp_coroutine(void (*callback)(int)) {
    kotlinx::coroutines::KotlinNativeStateGuard guard;
    
    // Your coroutine code here - GC-aware automatically
    // The guard handles thread state switches
    
    // ...
}
```

## Building

### Standalone C++ (No Kotlin)

```bash
cmake -DKOTLIN_NATIVE_RUNTIME_AVAILABLE=0 ..
make
```

The GC bridge functions become no-ops. Your code runs without any GC overhead.

### With Kotlin Native

```bash
cmake -DKOTLIN_NATIVE_RUNTIME_AVAILABLE=1 ..
make
```

The GC bridge functions are declared `weak` - they'll be resolved by the Kotlin Native runtime when you link.

From Kotlin:
```kotlin
// Just call the C++ function - the Kotlin runtime is already loaded
external fun longOperation()

fun main() {
    longOperation()  // C++ sees Kotlin_mm_* functions
}
```

## How It Works

1. **When called from Kotlin Native**:
   - Kotlin Native runtime is already loaded in the process
   - Our C++ code makes direct calls to `Kotlin_mm_switchThreadState*`
   - These functions are already in memory (weak linking finds them)
   - Zero overhead - direct function calls

2. **When standalone C++**:
   - Functions are inline stubs that do nothing
   - Compiler optimizes them away entirely
   - No runtime overhead at all

## Example: MLX Integration

```cpp
// mlx_bridge.cpp
#include "kotlinx/coroutines/KotlinGCBridge.hpp"
#include <mlx/mlx.h>

extern "C" int64_t mlx_run_inference(int64_t model_ptr, int64_t input_ptr) {
    // Tell Kotlin GC: "Don't wait for me"
    kotlinx::coroutines::KotlinNativeStateGuard guard;
    
    auto* model = reinterpret_cast<mlx::core::nn::Module*>(model_ptr);
    auto* input = reinterpret_cast<mlx::core::array*>(input_ptr);
    
    // Run inference - takes as long as needed
    // Other Kotlin threads can continue without being blocked
    auto result = model->forward(*input);
    
    return reinterpret_cast<int64_t>(new mlx::core::array(result));
    // Guard destructor switches back to Runnable
}
```

From Kotlin:
```kotlin
@GCUnsafeCall("mlx_run_inference")
external fun runInference(model: Long, input: Long): Long

fun predict() {
    val result = runInference(modelPtr, inputPtr)
    // C++ ran in Native state, no GC latency
}
```

## Safety Rules

When in **Native state** (after `Kotlin_mm_switchThreadStateNative()`):
- ✅ Can call C/C++ functions
- ✅ Can do heavy computation
- ✅ Can call `check_safepoint()` periodically
- ❌ Cannot access Kotlin objects
- ❌ Cannot call Kotlin functions
- ❌ Cannot allocate Kotlin objects

When in **Runnable state** (after `Kotlin_mm_switchThreadStateRunnable()`):
- ✅ Can access Kotlin objects
- ✅ Can call Kotlin functions
- ✅ Can allocate Kotlin objects
- ⚠️  GC may pause you at safepoints

## Verification

```cpp
#include "kotlinx/coroutines/KotlinGCBridge.hpp"
#include <iostream>

extern "C" void test_gc_bridge() {
    if (kotlinx::coroutines::is_kotlin_native_runtime_available()) {
        std::cout << "Running with Kotlin Native GC\n";
    } else {
        std::cout << "Running standalone C++\n";
    }
}
```

## Summary

**You don't need to vendor the GC code!**

- Kotlin Native runtime is already running when your code is called
- Just call the exposed functions: `Kotlin_mm_switchThreadState*`
- Use weak linking so it works both with and without Kotlin
- RAII guard makes it automatic and safe
- Zero overhead when standalone C++

This is exactly what you wanted: a simple bridge that "just works" whether called from Kotlin or used standalone.
