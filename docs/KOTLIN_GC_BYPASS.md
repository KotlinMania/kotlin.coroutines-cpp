# Bypassing Kotlin GC Thread State for Long-Running C++ Operations

## The Problem

When Kotlin calls C++ via cinterop:
1. Thread switches to `kNative` state (tells GC it won't touch managed objects)
2. C++ code runs
3. Thread switches back to `kRunnable` state

For long-running C++ operations (like ML inference), other Kotlin threads hit safepoints and can experience latency.

## Solution: Direct Thread State Control

Kotlin Native exposes functions to manually control thread state:

```cpp
// Declared in Memory.h:
extern "C" {
    RUNTIME_NOTHROW void Kotlin_mm_switchThreadStateNative();
    RUNTIME_NOTHROW void Kotlin_mm_switchThreadStateRunnable();
}
```

## Implementation Strategy

### Option 1: Use @GCUnsafeCall Annotation

```kotlin
// In your Kotlin code:
@GCUnsafeCall("my_cpp_function")
external fun myCppFunction(): Int
```

This tells the Kotlin compiler:
- **Don't** automatically insert thread state switches
- The C++ function (`my_cpp_function`) is responsible for GC safety
- No safepoint checks on entry/exit

From `Annotations.kt` line 132-142:
> This annotation is unsafe and should be used with care: [callee] is
> responsible for correct interaction with the garbage collector, like
> placing safe points and switching thread state when using blocking APIs.

### Option 2: Manual Thread State Control in C++

```cpp
// Your C++ bridge function
extern "C" void my_cpp_function() {
    // We're in kRunnable state (if called from Kotlin)
    
    // Switch to Native before long operation
    Kotlin_mm_switchThreadStateNative();
    
    // Do your long-running work here
    // GC won't wait for this thread
    perform_ml_inference();
    
    // Switch back to Runnable
    Kotlin_mm_switchThreadStateRunnable();
    
    // Can safely access Kotlin objects now
}
```

### Option 3: Pointer-Based Control (Your Idea!)

Expose a function that gives you direct control:

```kotlin
// Kotlin side
@OptIn(InternalForKotlinNative::class)
object ThreadStateControl {
    @GCUnsafeCall("ThreadStateControl_switchToNative")
    external fun switchToNative()
    
    @GCUnsafeCall("ThreadStateControl_switchToRunnable")
    external fun switchToRunnable()
    
    @GCUnsafeCall("ThreadStateControl_getSwitchPointer")
    external fun getSwitchPointer(): Long  // Returns function pointer
}
```

```cpp
// C++ implementation
extern "C" {
    RUNTIME_NOTHROW void ThreadStateControl_switchToNative() {
        Kotlin_mm_switchThreadStateNative();
    }
    
    RUNTIME_NOTHROW void ThreadStateControl_switchToRunnable() {
        Kotlin_mm_switchThreadStateRunnable();
    }
    
    // Return function pointer that Kotlin can call
    RUNTIME_NOTHROW void* ThreadStateControl_getSwitchPointer() {
        return (void*)&Kotlin_mm_switchThreadStateNative;
    }
}
```

Now you can do:

```kotlin
fun runLongOperation() {
    ThreadStateControl.switchToNative()
    try {
        // Long C++ operation
        myCppLibrary.doWork()
    } finally {
        ThreadStateControl.switchToRunnable()
    }
}
```

## Advanced: Callback-Based Approach

If your C++ code needs to call back into Kotlin during a long operation:

```cpp
struct ThreadStateGuard {
    ThreadStateGuard() {
        Kotlin_mm_switchThreadStateNative();
    }
    
    ~ThreadStateGuard() {
        Kotlin_mm_switchThreadStateRunnable();
    }
    
    // Temporarily switch back to Runnable for Kotlin callback
    struct RunableScope {
        RunableScope() { Kotlin_mm_switchThreadStateRunnable(); }
        ~RunableScope() { Kotlin_mm_switchThreadStateNative(); }
    };
};

extern "C" void long_operation_with_callbacks(void (*callback)(int)) {
    ThreadStateGuard native_guard;  // Switch to Native
    
    for (int i = 0; i < 1000; i++) {
        // Do heavy computation in Native state
        do_heavy_work();
        
        // Need to call Kotlin? Temporarily switch back
        {
            ThreadStateGuard::RunableScope runnable;
            callback(i);  // Safe to call Kotlin
        }
        // Back to Native automatically
    }
    // Switches back to Runnable on destruction
}
```

## Safety Considerations

**CRITICAL**: When in `kNative` state:
- **Don't** access Kotlin objects
- **Don't** call Kotlin functions
- **Don't** allocate Kotlin objects
- **Do** check for suspension points if the operation is interruptible

**From IrToBitcode.kt line 2640**:
```kotlin
check(!annotations.hasAnnotation(KonanFqNames.gcUnsafeCall))
```

The compiler **enforces** that `@GCUnsafeCall` functions:
1. Are `external` (implemented in C++)
2. Don't have automatic thread state management
3. You are responsible for GC safety

## Example: MLX Integration

```kotlin
@OptIn(InternalForKotlinNative::class)
object MLXBridge {
    @GCUnsafeCall("mlx_inference_native")
    external fun runInference(model: Long, input: Long): Long
}
```

```cpp
// In your C++ bridge
extern "C" int64_t mlx_inference_native(int64_t model_ptr, int64_t input_ptr) {
    // Already in Native state (due to @GCUnsafeCall)
    // No automatic switching happens
    
    auto* model = reinterpret_cast<mlx::core::nn::Module*>(model_ptr);
    auto* input = reinterpret_cast<mlx::core::array*>(input_ptr);
    
    // Run inference - takes as long as needed
    // GC won't wait for this thread
    auto result = model->forward(*input);
    
    // Return result (still in Native state)
    return reinterpret_cast<int64_t>(new mlx::core::array(result));
}
```

Then in Kotlin:
```kotlin
fun runMLXInference() {
    // Thread is in Runnable
    val result = MLXBridge.runInference(modelPtr, inputPtr)
    // Thread switches to Native, runs inference, stays Native, returns
    // We're back in Runnable now (automatic on return from @GCUnsafeCall)
}
```

**Wait, that's wrong!** `@GCUnsafeCall` means **no automatic switching**. Let me correct:

```cpp
extern "C" int64_t mlx_inference_native(int64_t model_ptr, int64_t input_ptr) {
    // With @GCUnsafeCall, we enter in whatever state the caller was (Runnable)
    // We must manually switch to Native
    Kotlin_mm_switchThreadStateNative();
    
    auto* model = reinterpret_cast<mlx::core::nn::Module*>(model_ptr);
    auto* input = reinterpret_cast<mlx::core::array*>(input_ptr);
    
    auto result = model->forward(*input);
    
    // Switch back before returning
    Kotlin_mm_switchThreadStateRunnable();
    
    return reinterpret_cast<int64_t>(new mlx::core::array(result));
}
```

## Testing Thread State

From `Debugging.kt`:
```kotlin
import kotlin.native.runtime.Debugging

if (!Debugging.isThreadStateRunnable) {
    error("Bug: should be in Runnable state!")
}
```

## References

- `Annotations.kt` line 132-142: `@GCUnsafeCall` documentation
- `IrToBitcode.kt` line 2634-2694: Thread state switching logic
- `Memory.h` line 257-263: Thread state functions
- `Memory.cpp` line 393-409: Implementation
- `SafePoint.cpp`: Safepoint checking
- `ThreadSuspension.cpp`: GC suspension mechanism

## Summary

You have **three approaches**:

1. **@GCUnsafeCall**: Mark function as GC-unsafe, manually manage state in C++
2. **Direct calls**: Call `Kotlin_mm_switchThreadState*` from C++
3. **Kotlin wrapper**: Expose thread state control to Kotlin, manage from there

For MLX or other long-running C++ ops, **Option 1 is best**:
- Clear contract: function is GC-unsafe
- Compiler enforces it's external
- You control exactly when to be in Native vs Runnable state
- Zero overhead when staying in Native for long periods
