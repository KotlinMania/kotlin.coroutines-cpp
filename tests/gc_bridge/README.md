# GC Bridge Integration Tests

This directory contains comprehensive tests for the Kotlin Native GC bridge integration.

## Test Files

### C++ Tests

- **`test_kotlin_gc_bridge.cpp`** - Standalone C++ test suite
  - Memory tracking and leak detection
  - Performance benchmarking
  - Multi-threaded scenarios
  - State switching validation

- **`test_kotlin_gc_bridge_impl.cpp`** - Implementation called from Kotlin
  - GC coordination validation
  - State transition testing
  - Callback safety
  - Long-running operation simulation

### Kotlin Tests

- **`test_kotlin_gc_bridge.kt`** - Kotlin Native integration tests
  - End-to-end GC integration
  - Mixed workload testing
  - GC statistics collection
  - Performance measurement

### Build Configuration

- **`test_gc_bridge.def`** - Kotlin Native cinterop definition
- **`build_and_test_gc_bridge.sh`** - Automated build and test script

## Running Tests

### Standalone C++ Tests

```bash
cd tests/gc_bridge
../../build_and_test_gc_bridge.sh
```

This will:
1. Compile C++ implementation
2. Create static library
3. Run standalone tests (no Kotlin required)
4. Display performance metrics and memory usage

### With Kotlin Native

Prerequisites:
```bash
brew install kotlin
```

Run full test suite:
```bash
cd tests/gc_bridge
../../build_and_test_gc_bridge.sh
```

If `kotlinc-native` is available, this will additionally:
1. Generate cinterop bindings
2. Compile Kotlin test code
3. Run integrated tests
4. Show GC coordination metrics

## Expected Results

### Standalone Mode

```
Kotlin Native runtime available: NO
Test 1 (without guard): 153 ms
Test 2 (with guard): 153 ms       ← Same performance!
Multi-threaded: 55 ms
Memory delta: 0 MB (no leaks)
```

**Key Observations**:
- Zero overhead (identical performance with/without guards)
- No memory leaks
- Thread-safe operation
- Functions optimized away by compiler

### Kotlin Native Mode

```
Kotlin Native runtime available: YES
GC pause time: 1-2 ms
C++ work in Native state: No GC waiting
All tests pass
```

**Key Observations**:
- GC coordination working
- Minimal pause times
- C++ code runs without blocking GC

## Test Coverage

| Category | Coverage |
|----------|----------|
| State transitions | ✓ |
| Memory safety | ✓ |
| Thread safety | ✓ |
| Performance | ✓ |
| GC coordination | ✓ |
| Exception safety | ✓ |
| Standalone mode | ✓ |
| Kotlin Native mode | ✓ |

## Interpreting Results

### Performance Metrics

- **Duration**: Wall-clock time for operations
- **Memory delta**: Net change in heap allocation
- **Allocation count**: Number of allocations/deallocations

### GC Metrics (Kotlin Native only)

- **GC duration**: Time spent in garbage collection
- **GC pause time**: Time application threads were paused
- **Epoch**: GC cycle number

### Success Criteria

✓ **Standalone mode**: Test 1 and Test 2 have identical performance  
✓ **Memory**: Zero leaks (delta approaches 0 over time)  
✓ **Threading**: All threads complete without deadlock  
✓ **GC coordination**: Pause times < 5ms

## Troubleshooting

### Test Fails to Compile

Check that include path is correct:
```bash
clang++ -I ../../include test_kotlin_gc_bridge.cpp
```

### kotlinc-native Not Found

Install Kotlin Native:
```bash
brew install kotlin
# OR
export PATH="$PATH:/path/to/kotlin-native/bin"
```

### Link Errors with Kotlin Native

Ensure `KOTLIN_NATIVE_RUNTIME_AVAILABLE=1` is set:
```bash
clang++ -DKOTLIN_NATIVE_RUNTIME_AVAILABLE=1 ...
```

### Memory Leaks Detected

Run with sanitizers:
```bash
clang++ -fsanitize=address test_kotlin_gc_bridge.cpp
./a.out
```

## See Also

- `../../docs/KOTLIN_NATIVE_GC_SPECIFICATION.md` - Full specification
- `../../include/kotlinx/coroutines/KotlinGCBridge.hpp` - API documentation
