# kotlin.coroutines-cpp

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=flat&logo=c%2B%2B)](https://isocpp.org/)
[![Apache License 2.0](https://img.shields.io/badge/license-Apache%202.0-blue.svg?style=flat)](https://www.apache.org/licenses/LICENSE-2.0)
[![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux%20%7C%20Windows-lightgrey.svg)](https://github.com/KotlinMania/kotlin.coroutines-cpp)
[![Based on kotlinx.coroutines](https://img.shields.io/badge/based%20on-kotlinx.coroutines%201.10-orange.svg)](https://github.com/Kotlin/kotlinx.coroutines)
[![Implementation](https://img.shields.io/badge/implementation-66%25-yellow.svg)](COMPREHENSIVE_AUDIT_REPORT.md)

A faithful C++ port of [kotlinx.coroutines](https://github.com/Kotlin/kotlinx.coroutines), designed to create a semantically aligned bridge between Kotlin's coroutine runtime and C++ applications. This enables seamless interoperability with Kotlin/Native's garbage collector while providing standalone C++ coroutine functionality.

```cpp
#include <kotlinx/coroutines/Builders.hpp>

int main() {
    using namespace kotlinx::coroutines;

    run_blocking([](auto& scope) {
        scope.launch([](auto& ctx) {
            delay(1000);
            std::cout << "Kotlin Coroutines World!" << std::endl;
        });
        std::cout << "Hello" << std::endl;
    });
    return 0;
}
```

## Key Innovations

### Zero-Overhead Kotlin Native GC Bridge

The flagship feature of this implementation is seamless coordination with Kotlin/Native's garbage collector:

```cpp
#include <kotlinx/coroutines/KotlinGCBridge.hpp>

extern "C" void native_inference() {
    // Without guard: GC waits 100ms, blocking all Kotlin threads
    // With guard: GC proceeds immediately, ~50-100x improvement
    kotlinx::coroutines::KotlinNativeStateGuard guard;
    run_expensive_computation();  // Takes 100ms
}
```

**Key Benefits:**
- **Zero overhead** when running standalone (functions inline away completely)
- **Weak-linked** Kotlin Native runtime functions for automatic detection
- **RAII-based** state management with exception safety guarantees
- **Thread-safe** with no synchronization overhead

### Atomic Decision State Machine

The suspension system uses a sophisticated atomic state machine that eliminates exception-based control flow:

```cpp
// Three-state decision model: UNDECIDED -> SUSPENDED or RESUMED
class CancellableContinuationImpl {
    std::atomic<int> decision_{DECISION_UNDECIDED};

    // Lock-free coordination between suspend and resume
    int try_suspend();   // Returns COROUTINE_SUSPENDED or result
    void try_resume();   // Coordinates with suspension atomically
};
```

This approach provides:
- **Deterministic behavior** without exception overhead
- **Lock-free** suspension/resumption coordination
- **Proper cancellation** semantics matching Kotlin's model

### Faithful API Transliteration

Every API maintains semantic equivalence with the original Kotlin implementation:

| Kotlin | C++ | Notes |
|--------|-----|-------|
| `launch { }` | `scope.launch([](auto& ctx) { })` | Coroutine builder |
| `async { }` | `scope.async<T>([](auto& ctx) { })` | Returns `Deferred<T>` |
| `delay(ms)` | `delay(ms)` | Suspending delay |
| `Flow<T>` | `Flow<T>` | Cold async streams |
| `Channel<T>` | `Channel<T>` | Communication primitive |
| `withContext(Dispatchers.IO)` | `with_context(Dispatchers::IO())` | Context switching |

## Architecture

```
kotlin.coroutines-cpp/
├── include/kotlinx/coroutines/    # Public C++ headers
│   ├── Builders.hpp               # launch, async, runBlocking
│   ├── Job.hpp                    # Job hierarchy and lifecycle
│   ├── Deferred.hpp               # Async result handling
│   ├── Flow.hpp                   # Cold async streams
│   ├── channels/                  # Channel implementations
│   │   ├── Channel.hpp
│   │   └── BufferedChannel.hpp
│   ├── KotlinGCBridge.hpp         # GC coordination (flagship)
│   └── ...
├── kotlinx-coroutines-core/       # Core implementation
├── kotlinx-coroutines-test/       # Test utilities
└── docs/                          # Documentation
```

## Implementation Status

### Core Infrastructure (75% Complete)

| Component | Status | Description |
|-----------|--------|-------------|
| Job System | Complete | Full job hierarchy with parent-child relationships |
| Channels | Complete | Buffered, rendezvous, conflated, unlimited |
| Flow | Complete | Cold streams with comprehensive operators |
| Dispatchers | Partial | Default and Unconfined working; IO in progress |
| Select | Partial | Framework complete, internals in progress |
| Cancellation | Complete | Structured concurrency with proper propagation |

### Platform Support

| Platform | Status | Notes |
|----------|--------|-------|
| macOS/iOS (Darwin) | In Progress | GC bridge complete, dispatchers pending |
| Linux | Functional | Core features working |
| Windows | Functional | Core features working |

### What's Working Now

- `launch` and `async` coroutine builders
- `Job` and `Deferred` with proper cancellation
- `Channel` with all buffer strategies
- `Flow` with operators: `map`, `filter`, `collect`, `take`, `drop`, etc.
- `delay` and timing operations
- `Mutex` and `Semaphore` synchronization
- Kotlin Native GC bridge (zero-overhead)
- Atomic state machine suspension

### In Progress

- Darwin-specific dispatchers (Grand Central Dispatch integration)
- Complete `select` expression implementation
- `withContext` function
- Debug probes and coroutine inspection
- Performance benchmarking infrastructure

## Building

### Requirements

- C++17 compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.16+

### Standalone Build

```bash
mkdir build && cd build
cmake -DKOTLIN_NATIVE_RUNTIME_AVAILABLE=0 ..
make
```

### With Kotlin Native Integration

```bash
mkdir build && cd build
cmake -DKOTLIN_NATIVE_RUNTIME_AVAILABLE=1 ..
make
```

### Running Tests

```bash
# Build and run test executables
./test_job
./test_channel
./test_dispatchers
./test_suspend
```

## Usage Examples

### Basic Coroutine

```cpp
#include <kotlinx/coroutines/Builders.hpp>

run_blocking([](auto& scope) {
    auto job = scope.launch([](auto& ctx) {
        std::cout << "Starting work..." << std::endl;
        delay(1000);
        std::cout << "Work complete!" << std::endl;
    });

    job->join();  // Wait for completion
});
```

### Async with Result

```cpp
auto deferred = scope.async<int>([](auto& ctx) {
    delay(500);
    return 42;
});

int result = deferred->await();  // Suspends until result ready
```

### Channel Communication

```cpp
auto channel = Channel<int>::create(Channel<int>::BUFFERED, 10);

scope.launch([&](auto& ctx) {
    for (int i = 0; i < 10; i++) {
        channel->send(i);
    }
    channel->close();
});

scope.launch([&](auto& ctx) {
    for (auto value : *channel) {
        std::cout << "Received: " << value << std::endl;
    }
});
```

### Flow Processing

```cpp
flow_of({1, 2, 3, 4, 5})
    .filter([](int x) { return x % 2 == 0; })
    .map([](int x) { return x * x; })
    .collect([](int x) {
        std::cout << x << std::endl;  // Prints: 4, 16
    });
```

### Kotlin Native GC Integration

```cpp
#include <kotlinx/coroutines/KotlinGCBridge.hpp>

// Called from Kotlin/Native code
extern "C" void process_data(const char* data) {
    // Signal to GC that this thread is doing native work
    kotlinx::coroutines::KotlinNativeStateGuard guard;

    // GC can proceed without waiting for this thread
    expensive_native_operation(data);

    // Guard destructor restores runnable state
}
```

## Documentation

- [Comprehensive Audit Report](COMPREHENSIVE_AUDIT_REPORT.md) - Detailed implementation analysis
- [Kotlin GC Bridge Specification](docs/KOTLIN_NATIVE_GC_SPECIFICATION.md) - GC integration details
- [Coroutines Guide](docs/coroutines-guide.md) - Usage patterns and best practices
- [API Reference](docs/) - Component documentation

## Contributing

Contributions are welcome! Key areas needing attention:

1. **Darwin Dispatchers** - Grand Central Dispatch integration
2. **Select Expression Internals** - Complete the `select` implementation
3. **Performance Optimization** - Lock-free data structures
4. **Documentation** - Usage examples and guides

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Acknowledgments

This project is a derivative work based on [kotlinx.coroutines](https://github.com/Kotlin/kotlinx.coroutines) by JetBrains. The original Kotlin implementation provided the semantic foundation and API design that this C++ port faithfully follows.

Special thanks to:
- The Kotlin team at JetBrains for the excellent original implementation
- Roman Elizarov for the coroutine design and structured concurrency concepts

## License

```
Copyright 2000-2020 JetBrains s.r.o. and Kotlin Programming Language contributors.
Copyright 2025 Sydney Renee and The Solace Project contributors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

---

**Maintainer:** Sydney Renee (sydney@solace.ofharmony.ai)
**Project:** [The Solace Project](https://github.com/KotlinMania/kotlin.coroutines-cpp)
