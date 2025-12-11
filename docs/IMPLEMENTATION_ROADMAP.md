# kotlinx.coroutines-cpp Implementation Roadmap

**Version:** 1.0  
**Last Updated:** December 10, 2025  
**Author:** Sydney Bach  
**Related Documents:** [SUSPEND_COMPARISON.md](SUSPEND_COMPARISON.md), [COMPREHENSIVE_AUDIT_REPORT.md](../COMPREHENSIVE_AUDIT_REPORT.md), [TODO_CHECKLIST.md](TODO_CHECKLIST.md)

---

## Executive Summary

The kotlinx.coroutines-cpp implementation demonstrates **strong engineering fundamentals** with approximately **66% completion** across all modules. The core coroutine infrastructure is well-established with proper C++ architectural patterns, but critical gaps exist in suspend semantics, platform-specific implementations, and advanced synchronization.

### Key Findings

**✅ Strengths:**
- Excellent core suspend infrastructure (SuspendMacros.hpp, state machines)
- Strong Job system and cancellation propagation
- Proper memory safety with shared_ptr usage
- Brilliant manual state machine → LLVM `indirectbr` compilation

**⚠️ Critical Issues:**
- Flow operations don't actually suspend (defeats purpose of reactive streams)
- Select expressions cannot suspend (core concurrency primitive broken)
- Platform dispatchers completely missing (Darwin/nativeOther at 0%)
- Decision state machine incomplete (advanced cancellation scenarios fail)

**🎯 Strategic Focus:** Fix suspension integration before adding features. The foundational suspend infrastructure works surprisingly well - higher-level APIs just need to use it properly.

---

## Critical Implementation Analysis

### What's Actually Broken (Core Functionality)

#### 1. Flow Suspend Semantics - COMPLETELY BROKEN
**Files:** `include/kotlinx/coroutines/flow/FlowCollector.hpp`, `kotlinx-coroutines-core/common/src/flow/SharedFlow.cpp`

**Issue:** `FlowCollector::emit()` is a regular function, not suspending:
```cpp
// Current (BROKEN):
virtual void emit(T value) = 0;

// Should be (suspend function):
virtual void* emit(T value, Continuation<void*>* continuation) = 0;
```

**Impact:** Flow is essentially just a synchronous iterator chain - completely defeats the purpose of reactive streams and backpressure handling.

#### 2. Select Expression Suspension - MAJOR ISSUES
**Files:** `include/kotlinx/coroutines/selects/Select.hpp`, `kotlinx-coroutines-core/common/src/selects/SelectInstance.cpp`

**Issue:** `do_select()` throws instead of suspending:
```cpp
// Lines 191-229 in Select.hpp - throws instead of suspending
throw std::runtime_error("No clause selected");
```

**Impact:** Core concurrency primitive cannot wait for multiple clauses - makes select expressions unusable.

#### 3. Platform-Specific Dispatchers - CRITICAL GAPS
**Darwin (macOS/iOS) - 0% Complete:**
- File: `kotlinx-coroutines-core/nativeDarwin/src/Dispatchers.cpp` contains only TODO comments
- Missing: CoreFoundation integration, Grand Central Dispatch, main thread detection

**nativeOther (Linux/Windows) - 0% Complete:**
- File: `kotlinx-coroutines-core/nativeOther/src/Dispatchers.cpp` contains only Kotlin code
- Missing: Platform-specific thread pools, proper main dispatcher implementation

#### 4. Decision State Machine Integration - PARTIALLY WORKING
**File:** `include/kotlinx/coroutines/CancellableContinuationImpl.hpp`

**Issue:** `get_result()` throws instead of properly suspending:
```cpp
// Lines 291 and 558 - broken suspension mechanism
throw std::runtime_error("COROUTINE_SUSPENDED");
```

**Impact:** Basic cancellation works, but advanced suspension scenarios fail.

### What Surprisingly Works

#### 1. Core Suspend Infrastructure - EXCELLENT
**File:** `include/kotlinx/coroutines/SuspendMacros.hpp`

- Computed goto state machine generator is sophisticated and complete
- Compiles to efficient LLVM `indirectbr` instructions with -O2
- Perfectly mirrors Kotlin's suspend function compilation pattern

#### 2. Continuation and Resumption - SOLID
**Files:** `include/kotlinx/coroutines/ContinuationImpl.hpp`, `test_cancellation.cpp`

- Proper base continuation implementation with exception handling
- Working suspend/resume with state machines
- C++20 coroutine test infrastructure demonstrates functionality

#### 3. Job System and Cancellation - STRONG
**Files:** `include/kotlinx/coroutines/Job.hpp`, `include/kotlinx/coroutines/JobSupport.hpp`

- Excellent atomic operations and state management
- Proper parent-child cancellation propagation
- Structured concurrency support

---

## Strategic Implementation Phases

### Phase 1: Fix Core Suspension (CRITICAL - Weeks 1-2)

**Goal:** Restore basic coroutine functionality by making higher-level APIs actually suspend.

#### Priority 1.1: Fix Flow Suspend Semantics
**Files to Modify:**
- `include/kotlinx/coroutines/flow/FlowCollector.hpp`
- `kotlinx-coroutines-core/common/src/flow/SharedFlow.cpp`
- `kotlinx-coroutines-core/common/src/flow/AbstractSharedFlow.cpp`

**Implementation Strategy:**
```cpp
// Transform FlowCollector to use suspend functions
template<typename T>
class FlowCollector {
public:
    // Change from void emit(T value) to:
    virtual void* emit(T value, Continuation<void*>* continuation) = 0;
    
    // Add backpressure handling with condition variables
    virtual void* emit_suspend(T value, Continuation<void*>* continuation) {
        if (buffer_full()) {
            return suspend_until_space_available(continuation);
        }
        emit_internal(value);
        return nullptr; // Completed immediately
    }
};
```

**Success Criteria:**
- `emit()` actually suspends when buffer is full
- `collect()` suspends indefinitely for hot flows
- Backpressure works correctly
- Flow chain operations suspend properly

#### Priority 1.2: Fix Select Expression Suspension
**Files to Modify:**
- `include/kotlinx/coroutines/selects/Select.hpp`
- `kotlinx-coroutines-core/common/src/selects/SelectInstance.cpp`

**Implementation Strategy:**
```cpp
// Replace throwing with proper suspension
void* SelectInstance::do_select(Continuation<void*>* continuation) {
    // Use existing suspend_cancellable_coroutine infrastructure
    return suspend_cancellable_coroutine<void>([&](CancellableContinuation<void>& cont) {
        // Register clauses and wait for selection
        register_clauses(cont);
        // Don't throw - actually suspend waiting for clauses
    }, continuation);
}
```

**Success Criteria:**
- `do_select()` suspends instead of throwing
- Clause selection works properly
- Timeout clauses are implemented
- Multiple clause registration works

#### Priority 1.3: Complete Decision State Machine
**File:** `include/kotlinx/coroutines/CancellableContinuationImpl.hpp`

**Implementation Strategy:**
```cpp
// Fix get_result() to use proper suspension
T get_result() {
    if (try_suspend()) {
        // Don't throw - use the existing suspension infrastructure
        install_parent_handle();
        // Actually suspend using continuation mechanism
        return suspend_until_resumed();
    }
    
    // Handle resumed state
    State* s = state_.load(std::memory_order_acquire);
    if (auto* c = dynamic_cast<CancelledContinuation*>(s)) {
        std::rethrow_exception(c->cause);
    }
    if (auto* c = dynamic_cast<CompletedContinuation*>(s)) {
        return *static_cast<T*>(c->result.get());
    }
    throw std::runtime_error("Invalid state in get_result");
}
```

**Success Criteria:**
- `get_result()` properly suspends instead of throwing
- Decision state machine coordinates correctly
- Race conditions handled properly
- Advanced cancellation scenarios work

---

### Phase 2: Platform Support (CRITICAL - Weeks 3-4)

**Goal:** Enable full platform support for Apple and non-Apple native platforms.

#### Priority 2.1: Darwin Dispatcher Implementation
**Files to Create/Modify:**
- `kotlinx-coroutines-core/nativeDarwin/src/DarwinMainDispatcher.cpp`
- `kotlinx-coroutines-core/nativeDarwin/src/DarwinGlobalQueueDispatcher.cpp`
- `include/kotlinx/coroutines/DarwinDispatcher.hpp`

**Implementation Strategy:**
```cpp
class DarwinMainDispatcher : public MainCoroutineDispatcher, public Delay {
private:
    dispatch_queue_t main_queue_;
    std::atomic<bool> invoke_immediately_;
    
public:
    DarwinMainDispatcher(bool invokeImmediately) 
        : invoke_immediately_(invokeImmediately) {
        main_queue_ = dispatch_get_main_queue();
    }
    
    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) override {
        if (invoke_immediately_.load() && is_main_thread()) {
            block->run();
        } else {
            dispatch_async(main_queue_, ^{
                block->run();
            });
        }
    }
    
    std::shared_ptr<DisposableHandle> scheduleResumeAfterDelay(
        int64_t timeMillis, 
        std::shared_ptr<CancellableContinuation<Unit>> continuation
    ) override {
        auto timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, main_queue_);
        // Configure timer and cancellation handling
        return std::make_shared<DarwinTimerHandle>(timer, continuation);
    }
};
```

**Success Criteria:**
- Dispatchers work on macOS/iOS
- Main thread detection works correctly
- Timer support implemented
- CoreFoundation integration complete

#### Priority 2.2: Native Thread Pool Implementation
**Files to Create/Modify:**
- `kotlinx-coroutines-core/native/src/DefaultIoScheduler.cpp`
- `include/kotlinx/coroutines/NativeThreadPool.hpp`

**Implementation Strategy:**
```cpp
class NativeThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};
    
public:
    NativeThreadPool(size_t threads) {
        for ( reap; i < threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }
    
    template<typename F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(std::forward<F>(f));
        }
        condition_.notify_one();
    }
};
```

**Success Criteria:**
- Worker-based thread pool works correctly
- Thread-local storage for context preservation
- Platform-appropriate task scheduling
- Integration with existing dispatcher infrastructure

---

### Phase 3: Synchronization Infrastructure (HIGH - Weeks 5-6)

**Goal:** Complete advanced synchronization patterns and select expressions.

#### Priority 3.1: Complete Waiter Interface
**Files to Modify:**
- `include/kotlinx/coroutines/Waiter.hpp` (already exists)
- `include/kotlinx/coroutines/CancellableContinuationImpl.hpp`

**Implementation Strategy:**
```cpp
// Implement proper segment-based cancellation
void CancellableContinuationImpl::invoke_on_cancellation(void* segment, int index) override {
    // Create segment-based cancellation handler
    auto handler = [this, segment, index](std::exception_ptr cause) {
        // Call segment.onCancellation with proper parameters
        if (auto* seg = static_cast<SegmentBase*>(segment)) {
            seg->onCancellation(cause, index);
        }
    };
    
    // Register handler with current state
    invoke_on_cancellation(handler);
}
```

**Success Criteria:**
- Segment-based cancellation works correctly
- Proper type erasure for segment parameters
- Integration with synchronization primitives

#### Priority 3.2: Select Expression Internals
**Files to Modify:**
- `kotlinx-coroutines-core/common/src/selects/SelectInstance.cpp`
- `kotlinx-coroutines-core/common/src/selects/SelectBuilderImpl.cpp`

**Implementation Strategy:**
```cpp
class SelectInstance {
private:
    std::vector<std::shared_ptr<SelectClause>> clauses_;
    std::atomic<bool> selected_{false};
    std::condition_variable selection_cv_;
    
public:
    void* do_select(Continuation<void*>* continuation) {
        return suspend_cancellable_coroutine<void>([&](CancellableContinuation<void>& cont) {
            // Register all clauses
            for (auto& clause : clauses_) {
                clause->register_select(cont);
            }
            
            // Wait for first clause to complete
            cont.invoke_on_cancellation([&](std::exception_ptr) {
                // Clean up all clauses on cancellation
                cleanup_clauses();
            });
        }, continuation);
    }
};
```

**Success Criteria:**
- Clause registration and selection works
- Multiple clause handling implemented
- Timeout support added
- Channel select integration complete

---

### Phase 4: Production Readiness (MEDIUM - Weeks 7-8)

**Goal:** Complete context system, exception handling, and debug infrastructure.

#### Priority 4.1: Context System Completion
**Files to Create/Modify:**
- `include/kotlinx/coroutines/NonCancellable.hpp`
- `kotlinx-coroutines-core/common/src/context/NonCancellable.cpp`
- `kotlinx-coroutines-core/common/src/context/withContext.cpp`

**Implementation Strategy:**
```cpp
// NonCancellable context element
class NonCancellable : public CoroutineContextElement {
public:
    static const Key* type_key;
    
    static std::shared_ptr<NonCancellable> instance() {
        static auto instance = std::make_shared<NonCancellable>();
        return instance;
    }
};

// withContext implementation
template<typename T, typename F>
void* with_context(std::shared_ptr<CoroutineContext> context, F&& block, Continuation<T>* continuation) {
    return suspend_cancellable_coroutine<T>([&](CancellableContinuation<T>& cont) {
        auto new_context = context->plus(continuation->get_context());
        // Execute block in new context
        auto result = block();
        cont.resume(result, nullptr);
    }, continuation);
}
```

#### Priority 4.2: Exception Handling Infrastructure
**Files to Create/Modify:**
- `kotlinx-coroutines-core/common/src/context/CoroutineExceptionHandler.cpp`
- `include/kotlinx/coroutines/CoroutineExceptionHandler.hpp`

#### Priority 4.3: Debug Infrastructure
**Files to Modify:**
- `kotlinx-coroutines-debug/src/DebugProbesImpl.cpp`
- `kotlinx-coroutines-debug/src/CoroutineInfo.cpp`

---

## Technical Architecture Strategy

### LLVM Integration Approach

#### Current Brilliance: Manual State Machines
The existing manual state machines in `SuspendMacros.hpp` are actually brilliant:
```cpp
switch (this->_label) {
case 0:
    this->_label = 1;
    void* _tmp_result = suspend_call();
    if (is_coroutine_suspended(_tmp_result)) {
        return COROUTINE_SUSPENDED;
    }
    result = _tmp_result;
case 1:
    // Continue execution
}
```

This compiles to efficient LLVM `indirectbr` instructions with -O2, exactly matching Kotlin Native's approach.

#### Phase 1: Hybrid Suspension (Immediate)
- Keep manual state machines for stability
- Add LLVM `__builtin_coro_save`/`__builtin_coro_resume` intrinsics for performance-critical paths
- Create hybrid suspension points for hot code paths

#### Phase 2: C++20 Bridge (Medium-term)
- Maintain Kotlin semantics while using C++20 coroutines
- Custom coroutine allocators for memory optimization
- Gradual migration of non-critical paths

#### Phase 3: Full LLVM Integration (Long-term)
- Custom LLVM optimization passes
- Remove manual state machine complexity
- Achieve maximum performance parity

### Memory Management Strategy

#### Current Approach: Shared_ptr Usage
- **Pros:** Excellent memory safety, prevents leaks
- **Cons:** Potential performance overhead from reference counting

#### Optimization Strategy
```cpp
// Object pooling for frequent allocations
template<typename T>
class CoroutineObjectPool {
private:
    std::queue<std::unique_ptr<T>> pool_;
    std::mutex mutex_;
    
public:
    std::unique_ptr<T> acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pool_.empty()) {
            return std::make_unique<T>();
        }
        auto obj = std::move(pool_.front());
        pool_.pop();
        return obj;
    }
    
    void release(std::unique_ptr<T> obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push(std::move(obj));
    }
};
```

#### Template Specialization
- Specialize for common types: void, int, std::string
- Reduce template instantiation overhead
- Improve compile times and runtime performance

---

## Integration with Existing Work

### Building on Current AI Implementation
The roadmap builds on excellent work already in progress:

1. **Decision State Machine** - Being actively implemented in `CancellableContinuationImpl.hpp`
2. **Waiter Interface** - Recently created with proper signature
3. **Test Infrastructure** - `test_cancellation.cpp` demonstrates working suspend/resume

### Connecting to TODO Checklist
Map roadmap phases to specific TODO items:
- **Phase 1** addresses critical suspend function TODOs
- **Phase 2** covers platform-specific dispatcher TODOs  
- **Phase 3** completes synchronization infrastructure TODOs
- **Phase 4** handles remaining context and exception handling TODOs

### Leveraging Audit Findings
Use the comprehensive audit data:
- 66% completion metrics for progress tracking
- Risk assessment for prioritization
- Function mapping statistics for verification

---

## Success Metrics & Verification

### Phase Completion Criteria

#### Phase 1 Success Criteria
- [ ] Flow `emit()` suspends when buffer is full
- [ ] Flow `collect()` suspends indefinitely for hot flows
- [ ] Select `do_select()` suspends instead of throwing
- [ ] Decision state machine handles race conditions correctly
- [ ] All basic coroutine scenarios from Kotlin documentation work

#### Phase 2 Success Criteria
- [ ] Darwin dispatchers work on macOS/iOS
- [ ] nativeOther dispatchers work on Linux/Windows
- [ ] Timer support implemented across all platforms
- [ ] Thread-local storage preserves context correctly
- [ ] Platform-specific optimizations functional

#### Phase 3 Success Criteria
- [ ] Waiter interface supports segment-based cancellation
- [ ] Select expressions handle multiple clauses correctly
- [ ] Channel select integration works
- [ ] Timeout clauses implemented properly
- [ ] Advanced synchronization scenarios functional

#### Phase 4 Success Criteria
- [ ] NonCancellable context prevents cancellation
- [ ] withContext switches context correctly
- [ ] Exception handling preserves coroutine semantics
- [ ] Debug infrastructure provides runtime inspection
- [ ] Test coverage exceeds 80% of implementation

### Quality Assurance Process

#### Build and Test Verification
```bash
# Build the project
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run existing tests
./test_cancellation

# Run comprehensive test suite
ctest --output-on-failure
```

#### Performance Benchmarks
- Suspend/resume latency measurements
- Memory usage profiling
- Comparison with Kotlin Native benchmarks
- Regression detection for performance changes

#### Thread Safety Validation
- Concurrent stress testing
- Race condition detection
- Atomic operation verification
- Memory ordering validation

---

## Implementation Guidelines

### Code Quality Standards
1. **Follow existing C++ conventions** - snake_case functions, CamelCase classes
2. **Maintain thread safety** - All coroutine APIs must be thread-safe
3. **Use RAII patterns** - Proper resource management and cleanup
4. **Add comprehensive tests** - Each implementation needs unit tests
5. **Document thoroughly** - API documentation and implementation comments

### Integration Best Practices
1. **Build on existing infrastructure** - Use SuspendMacros.hpp, ContinuationImpl, etc.
2. **Maintain semantic equivalence** - Follow Kotlin behavior exactly
3. **Preserve architectural decisions** - Don't break existing patterns
4. **Update documentation** - Keep API_AUDIT.md and TODO_CHECKLIST.md current
5. **Verify with benchmarks** - Ensure performance doesn't regress

### Development Workflow
1. **Implement phase by phase** - Complete each phase before starting next
2. **Continuous testing** - Run tests after each major change
3. **Documentation updates** - Update roadmap as implementation progresses
4. **Performance monitoring** - Benchmark at each milestone
5. **Code reviews** - Maintain quality standards throughout

---

## Conclusion

The kotlinx.coroutines-cpp implementation has **excellent foundational work** completed. The core suspend infrastructure, Job system, and architectural patterns are all high-quality. The critical gaps are in **integration** - making higher-level APIs actually use the suspend infrastructure that already works.

This roadmap provides a **phased approach** that:
1. **Fixes core functionality first** (suspension integration)
2. **Adds platform support** (Darwin/nativeOther dispatchers)  
3. **Completes synchronization** (select expressions, advanced patterns)
4. **Achieves production readiness** (context system, debugging)

With focused execution of this roadmap, the implementation can achieve **production-ready status** with full functional parity to Kotlin coroutines within 2-3 months.

### Success Metrics
- **Functional Parity:** 95%+ of Kotlin coroutine API available in C++
- **Platform Coverage:** 90%+ of Kotlin Native platforms supported  
- **Performance Parity:** Within 20% of Kotlin Native performance
- **Test Coverage:** 80%+ of implementation covered by automated tests

The strategic focus on **fixing suspension integration first** ensures that the excellent foundation already established can be fully leveraged, making the remaining implementation work much more straightforward.

---

**Next Review Date:** January 10, 2026  
**Update Frequency:** Weekly during Phase 1-2, bi-weekly during Phase 3-4  
**Maintainer:** Update this document as implementation phases are completed

---

*This roadmap serves as the master strategic document for completing kotlinx.coroutines-cpp implementation, integrating all existing analysis and providing clear direction for achieving production readiness.*