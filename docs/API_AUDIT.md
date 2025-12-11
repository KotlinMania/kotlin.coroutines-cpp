# kotlinx.coroutines C++ API Audit

This document tracks the API completeness of our C++ transliteration against the Kotlin source.

## Audit Methodology

For each public Kotlin API file, we compare:
1. **Properties** (val/var) → C++ virtual getters/setters
2. **Functions** (fun) → C++ virtual methods
3. **Suspend functions** (suspend fun) → C++ methods returning coroutine types
4. **Extension functions** → C++ free functions
5. **Companion object members** → C++ static methods or free functions

Method names are converted from camelCase to snake_case per C++ conventions.

## Core Interfaces

### Job (kotlinx.coroutines.Job)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Job.kt`  
**C++ Header**: `include/kotlinx/coroutines/Job.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `val parent: Job?` | `virtual std::shared_ptr<Job> get_parent()` | ✅ | |
| `val isActive: Boolean` | `virtual bool is_active()` | ✅ | |
| `val isCompleted: Boolean` | `virtual bool is_completed()` | ✅ | |
| `val isCancelled: Boolean` | `virtual bool is_cancelled()` | ✅ | |
| `fun getCancellationException()` | `virtual std::exception_ptr get_cancellation_exception()` | ✅ | |
| `fun start(): Boolean` | `virtual bool start()` | ✅ | |
| `fun cancel(cause: CancellationException?)` | `virtual void cancel(std::exception_ptr)` | ✅ | |
| `val children: Sequence<Job>` | `virtual std::vector<std::shared_ptr<Job>> get_children()` | ⚠️  | Returns vector, not lazy sequence |
| `fun attachChild(child: ChildJob)` | `virtual std::shared_ptr<ChildHandle> attach_child()` | ✅ | |
| `suspend fun join()` | `virtual void join()` | ✅ | Should be suspend |
| `val onJoin: SelectClause0` | ❌ | ❌ MISSING | Select support not implemented |
| `fun invokeOnCompletion(handler)` | `virtual std::shared_ptr<DisposableHandle> invoke_on_completion()` | ✅ | |
| `fun invokeOnCompletion(onCancelling, invokeImmediately, handler)` | `virtual std::shared_ptr<DisposableHandle> invoke_on_completion()` | ✅ | |

**Missing APIs**:
- Select clause support (`onJoin`)

**Notes**:
- `children` returns `vector` instead of lazy `Sequence` - acceptable for C++
- `join()` is not marked as suspend - needs coroutine return type

---

### CoroutineDispatcher (kotlinx.coroutines.CoroutineDispatcher)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/CoroutineDispatcher.kt`  
**C++ Header**: `include/kotlinx/coroutines/CoroutineDispatcher.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `abstract fun dispatch(context, block)` | ❌ | ❌ MISSING | Core dispatch method |
| `open fun isDispatchNeeded(context): Boolean` | ❌ | ❌ MISSING | Dispatch optimization |
| `fun limitedParallelism(parallelism, name): CoroutineDispatcher` | ❌ | ❌ MISSING | Parallelism control |
| `fun dispatchYield(context, block)` | ❌ | ❌ MISSING | Yield-aware dispatch |
| `final override fun <T> interceptContinuation(continuation): Continuation<T>` | ❌ | ❌ MISSING | Continuation interception |
| `final override fun releaseInterceptedContinuation(continuation)` | ❌ | ❌ MISSING | Continuation cleanup |
| `operator fun plus(other: CoroutineDispatcher): CoroutineDispatcher` | ❌ | ❌ MISSING | Dispatcher composition |
| `override fun toString(): String` | ❌ | ❌ MISSING | Debug string |
| `override fun minusKey(key): CoroutineContext` | ❌ | ❌ MISSING | Context manipulation |

**Status**: ⚠️ **MOSTLY STUBS** - Only basic structure exists

---

### Deferred<T> (kotlinx.coroutines.Deferred)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Deferred.kt`  
**C++ Header**: `include/kotlinx/coroutines/Deferred.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `val onAwait: SelectClause1<T>` | ❌ | ❌ MISSING | Select support |
| `suspend fun await(): T` | `virtual T await()` | ⚠️ | Should be suspend |
| `fun getCompleted(): T` | `virtual T get_completed()` | ✅ | |
| `fun getCompletionExceptionOrNull(): Throwable?` | `virtual std::exception_ptr get_completion_exception_or_null()` | ✅ | |

**Missing APIs**:
- Select clause support (`onAwait`)

**Notes**:
- `await()` is not marked as suspend - needs coroutine return type

---

### Dispatchers (kotlinx.coroutines.Dispatchers)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Dispatchers.common.kt`  
**C++ Header**: `include/kotlinx/coroutines/Dispatchers.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `val Default: CoroutineDispatcher` | `std::shared_ptr<CoroutineDispatcher> get_default()` | ✅ | |
| `val Main: MainCoroutineDispatcher` | ❌ | ❌ MISSING | UI thread dispatcher |
| `val Unconfined: CoroutineDispatcher` | ❌ | ❌ MISSING | Unconfined dispatcher |
| `val IO: CoroutineDispatcher` | ❌ | ❌ MISSING | IO dispatcher |

**Status**: ⚠️ **MINIMAL** - Only Default implemented

---

### Delay (kotlinx.coroutines.Delay)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Delay.kt`  
**C++ Header**: `include/kotlinx/coroutines/Delay.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `suspend fun delay(timeMillis: Long)` | ❌ | ❌ MISSING | Core delay function |
| `val invokeOnTimeout: (timeMillis: Long, block: Runnable, context: CoroutineContext) -> DisposableHandle` | ❌ | ❌ MISSING | Timeout callbacks |
| `fun scheduleResumeAfterDelay(timeMillis: Long, continuation: CancellableContinuation<Unit>)` | ❌ | ❌ MISSING | Low-level delay |

**Status**: ❌ **NOT IMPLEMENTED**

---

## Builders

### launch, async, runBlocking

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Builders.common.kt`  
**C++ Header**: `include/kotlinx/coroutines/CoroutineScope.hpp`, `include/kotlinx/coroutines/StacklessBuilders.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `fun CoroutineScope.launch(context, start, block): Job` | ✅ | ✅ | In StacklessBuilders.hpp |
| `fun CoroutineScope.async(context, start, block): Deferred<T>` | ✅ | ✅ | In StacklessBuilders.hpp |
| `fun <T> runBlocking(context, block): T` | ✅ | ✅ | In StacklessBuilders.hpp |
| `fun CoroutineScope.produce(context, capacity, start, onCompletion, block): ReceiveChannel<E>` | ❌ | ❌ MISSING | Channel producer |
| `fun <T> withContext(context, block): T` | ❌ | ❌ MISSING | Context switching |
| `fun <T> withTimeout(timeMillis, block): T` | ❌ | ❌ MISSING | Timeout wrapper |
| `fun <T> withTimeoutOrNull(timeMillis, block): T?` | ❌ | ❌ MISSING | Nullable timeout |
| `suspend fun <T> coroutineScope(block): T` | ❌ | ❌ MISSING | Structured concurrency scope |
| `suspend fun <T> supervisorScope(block): T` | ❌ | ❌ MISSING | Supervisor scope |

**Status**: ⚠️ **PARTIAL** - Basic builders exist, advanced patterns missing

---

## Summary Statistics

| Category | Implemented | Partial | Missing | Total |
|----------|-------------|---------|---------|-------|
| Job | 11 | 2 | 1 | 14 |
| CoroutineDispatcher | 0 | 0 | 9 | 9 |
| Deferred | 2 | 1 | 1 | 4 |
| Dispatchers | 1 | 0 | 3 | 4 |
| Delay | 0 | 0 | 3 | 3 |
| Builders | 3 | 0 | 6 | 9 |
| **TOTAL** | **17** | **3** | **23** | **43** |

**Completion**: ~40% (17/43 fully implemented)

---

## Priority Tasks

### High Priority (Core functionality)
1. ❌ `CoroutineDispatcher::dispatch()` - Cannot schedule coroutines without this
2. ❌ `CoroutineDispatcher::is_dispatch_needed()` - Performance optimization
3. ❌ `Delay::delay()` - Essential for timeouts and scheduling
4. ❌ `with_context()` - Context switching is fundamental
5. ❌ `with_timeout()` / `with_timeout_or_null()` - Common patterns

### Medium Priority (Common use cases)
6. ❌ `Dispatchers.IO` - IO-bound operations
7. ❌ `Dispatchers.Unconfined` - Immediate execution
8. ❌ `coroutine_scope()` / `supervisor_scope()` - Structured concurrency
9. ❌ `CoroutineDispatcher::limited_parallelism()` - Thread pool management
10. ⚠️ Make `join()` and `await()` properly suspend

### Low Priority (Advanced features)
11. ❌ Select support (`onJoin`, `onAwait`)
12. ❌ `produce()` builder for channels
13. ❌ `Dispatchers.Main` - UI integration

---

## Notes for Implementation

### Suspend Functions
Currently, many functions that should suspend (like `join()`, `await()`) are marked as regular virtual functions. They should return coroutine types (e.g., `Task<void>`, `Task<T>`) or use our suspend macros.

### Select Expressions
Select expressions are an advanced feature. We should implement the basic API first and add select support later.

### Thread Safety
All APIs must be thread-safe per Kotlin's guarantee. Use appropriate synchronization primitives.

### Error Handling
Use `std::exception_ptr` for exception propagation, matching Kotlin's exception model.

---

**Last Updated**: 2024-12-10  
**Next Review**: After implementing high-priority tasks
