# API Implementation Checklist

This document tracks the implementation progress of missing APIs marked with `TODO: MISSING`.

## How to Use

- Find a TODO in the source code: `grep -r "TODO: MISSING" include/`
- Implement the API following the Kotlin signature provided in the comment
- Test the implementation
- Update this checklist
- Remove the TODO marker from the source

## Critical Path (Must Have)

### CoroutineDispatcher Core

- [ ] `dispatch(context, block)` - Schedule runnable on dispatcher  
      **File**: `CoroutineDispatcher.hpp:54`  
      **Kotlin**: `abstract fun dispatch(context: CoroutineContext, block: Runnable)`

- [ ] `is_dispatch_needed(context)` - Check if dispatch is required  
      **File**: `CoroutineDispatcher.hpp:58`  
      **Kotlin**: `open fun isDispatchNeeded(context: CoroutineContext): Boolean = true`

- [ ] `dispatch_yield(context, block)` - Dispatch with yield  
      **File**: `CoroutineDispatcher.hpp:62`  
      **Kotlin**: `fun dispatchYield(context: CoroutineContext, block: Runnable)`

- [ ] `intercept_continuation(continuation)` - Continuation interception  
      **File**: `CoroutineDispatcher.hpp:70`  
      **Kotlin**: `override fun <T> interceptContinuation(continuation: Continuation<T>): Continuation<T>`

- [ ] `release_intercepted_continuation(continuation)` - Release continuation  
      **File**: `CoroutineDispatcher.hpp:74`  
      **Kotlin**: `override fun releaseInterceptedContinuation(continuation: Continuation<*>)`

### Suspend Functions

- [x] Convert `Job::join()` to suspend
      **File**: `Job.hpp:278`, `JobSupport.cpp:390-451`
      **Kotlin**: `suspend fun join()`
      **Implementation**: Returns `void*` (COROUTINE_SUSPENDED or nullptr)
      **Completed**: 2024-12-10 - Added `join(Continuation*)`, `join_internal()`, `join_suspend()`, `join_blocking()`, `ResumeOnCompletion` handler

- [x] Convert `Deferred::await()` to suspend
      **File**: `Deferred.hpp:72`, `JobSupport.cpp:487-550`
      **Kotlin**: `suspend fun await(): T`
      **Implementation**: Returns `void*` (COROUTINE_SUSPENDED or result)
      **Completed**: 2024-12-10 - Added `await(Continuation*)`, `await_internal()`, `await_suspend()`, `await_blocking()`, `ResumeAwaitOnCompletion` handler

- [x] Implement true suspend `delay()`
      **File**: `Delay.hpp:65`
      **Kotlin**: `suspend fun delay(timeMillis: Long)`
      **Action**: Integrate with Delay interface, schedule continuation resume via dispatcher
      **Completed**: 2024-12-10 - Implemented via `suspend_cancellable_coroutine` and `Delay` interface check

## High Priority

### Dispatchers

- [x] `Dispatchers.IO` implementation  
      **File**: `Dispatchers.hpp:34`  
      **Kotlin**: `val IO: CoroutineDispatcher`  
      **Action**: Thread pool for blocking IO
      **Completed**: 2024-12-10 - Using `ExecutorCoroutineDispatcherImpl` (native)

- [x] `Dispatchers.Unconfined` implementation  
      **File**: `Dispatchers.hpp:29`  
      **Kotlin**: `val Unconfined: CoroutineDispatcher`  
      **Action**: Immediate execution, no dispatch
      **Completed**: 2024-12-10 - Implemented inline execution

- [x] `Dispatchers.shutdown()` implementation  
      **File**: `Dispatchers.hpp:37`  
      **Kotlin**: `fun shutdown()`  
      **Action**: Clean shutdown of thread pools
      **Completed**: 2024-12-10 - Implemented with atomic exchange and cleanup

### Dispatcher Features

- [x] `limited_parallelism(parallelism, name)` - Parallelism control  
      **File**: `CoroutineDispatcher.hpp:66`  
      **Kotlin**: `fun limitedParallelism(parallelism: Int, name: String? = null): CoroutineDispatcher`
      **Completed**: 2024-12-10 - Implemented via `LimitedDispatcher` helper

- [x] `minus_key(key)` - Context manipulation  
      **File**: `CoroutineDispatcher.hpp:78`  
      **Kotlin**: `override fun minusKey(key: CoroutineContext.Key<*>): CoroutineContext`
      **Completed**: 2024-12-10 - Implemented in `CoroutineContext` and `CoroutineDispatcher` (via inheritance)

- [ ] `to_string()` - Debug representation  
      **File**: `CoroutineDispatcher.hpp:82`  
      **Kotlin**: `override fun toString(): String`

## Medium Priority

### AbstractCoroutine Lifecycle

- [ ] `on_start()` - Called when coroutine starts  
      **File**: `AbstractCoroutine.hpp:114`  
      **Kotlin**: `protected open fun onStart()`

- [ ] `on_completed(value)` - Normal completion handler  
      **File**: `AbstractCoroutine.hpp:118`  
      **Kotlin**: `protected open fun onCompleted(value: T)`

- [ ] `on_cancelled(cause, handled)` - Cancellation handler  
      **File**: `AbstractCoroutine.hpp:122`  
      **Kotlin**: `protected open fun onCancelled(cause: Throwable, handled: Boolean)`

- [ ] `handle_on_complete_exception(exception)` - Exception in completion handler  
      **File**: `AbstractCoroutine.hpp:126`  
      **Kotlin**: `protected open fun handleOnCompletionException(exception: Throwable)`

- [ ] `name_string()` - Debug name  
      **File**: `AbstractCoroutine.hpp:130`  
      **Kotlin**: `protected open fun nameString(): String`

- [ ] `handle_job_exception(exception)` - Job exception handler  
      **File**: `AbstractCoroutine.hpp:136`  
      **Kotlin**: `override fun handleJobException(exception: Throwable): Boolean`

### JobSupport Advanced

- [ ] Advanced cancellation APIs  
      **File**: `JobSupport.hpp:348`  
      **Multiple functions**: parent cancellation, state queries  
      **Defer until core works**

## Low Priority

### Select Support

- [ ] `Job::get_on_join()` - Select clause  
      **File**: `Job.hpp:293`  
      **Kotlin**: `val onJoin: SelectClause0`  
      **Note**: Requires full select expression implementation

- [ ] `Deferred::get_on_await()` - Select clause  
      **File**: `Deferred.hpp:76` (commented out)  
      **Kotlin**: `val onAwait: SelectClause1<T>`  
      **Note**: Requires full select expression implementation

### Dispatcher Composition

- [x] `operator+(other)` - Combine dispatchers  
      **File**: `CoroutineDispatcher.hpp:74`  
      **Kotlin**: `operator fun plus(other: CoroutineDispatcher): CoroutineDispatcher`  
      **Completed**: 2024-12-10 - Implemented in `CoroutineContext` base class
      **Note**: Advanced feature, low priority

## Progress Tracking

Last updated: 2024-12-10

**Statistics**:
- Total TODOs: 23
- Implemented: 2
- In Progress: 0
- Remaining: 21

**Priority Breakdown**:
- Critical: 6 TODOs (was 8)
- High: 6 TODOs
- Medium: 7 TODOs
- Low: 2 TODOs

## Implementation Guidelines

1. **Read the Kotlin source** - Each TODO references the exact line in Kotlin source
2. **Follow C++ conventions** - snake_case, not camelCase
3. **Maintain thread safety** - All Kotlin coroutine APIs are thread-safe
4. **Add tests** - Each implemented API should have unit tests
5. **Update docs** - Update API_AUDIT.md when complete
6. **Remove TODO** - Delete the TODO marker after implementation

## Quick Reference

Find all TODOs:
```bash
grep -r "TODO: MISSING" include/kotlinx/coroutines/*.hpp
```

Find by file:
```bash
grep "TODO: MISSING" include/kotlinx/coroutines/CoroutineDispatcher.hpp
```

Count remaining:
```bash
grep -r "TODO: MISSING" include/kotlinx/coroutines/*.hpp | wc -l
```

---

**Created**: 2024-12-10  
**Maintainer**: Update this file as APIs are implemented
