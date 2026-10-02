# Runtime & GC Documentation Index

This directory contains specifications and implementation guides for runtime integration and garbage collection interoperability with Kotlin/Native.

---

## 🔄 Runtime & GC Documents

### [KOTLIN_GC_BRIDGE_IMPLEMENTATION.md](KOTLIN_GC_BRIDGE_IMPLEMENTATION.md)
**Purpose:** Overview and delivery summary of zero-overhead Kotlin Native GC integration for C++ coroutine libraries.  
**Contents:**
- Core deliverables: weak-linked runtime functions, RAII guards, inline stubs for standalone mode (`include/kotlinx/coroutines/KotlinGCBridge.hpp`)
- Three-tier test coverage: standalone C++, Kotlin/Native integration, and Kotlin-side validation (`tests/gc_bridge/`)
- Build automation and platform detection (`build_and_test_gc_bridge.sh`)
- Performance metrics: zero overhead in standalone C++, <5ns state transition with Kotlin/Native

### [KOTLIN_NATIVE_GC_SPECIFICATION.md](KOTLIN_NATIVE_GC_SPECIFICATION.md)
**Purpose:** Engineering specification defining the GC coordination layer between C++ coroutines and Kotlin/Native.  
**Contents:**
- Thread State Model: `kRunnable` vs `kNative` states and transition semantics
- RAII Guard Architecture: `KotlinGCBridge::ThreadStateGuard` and scoped transitions
- Safepoint checking mechanisms: polling vs signal-based cooperative suspension
- Weak linking strategy for binary portability across standalone and embedded targets
- Performance characteristics, benchmarks, safety guarantees, and threading constraints
