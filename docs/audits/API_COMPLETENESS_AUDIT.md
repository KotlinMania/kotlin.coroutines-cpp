# API Completeness Audit

**Date**: December 10, 2024  
**Status**: In Progress

## Overview

This document tracks the completeness of our C++ API compared to the official Kotlin kotlinx.coroutines API. Missing APIs are marked with `TODO: MISSING API` comments in the source code.

## Audit Methodology

1. **Source**: Official Kotlin API definitions from `tmp/kotlinx.coroutines/kotlinx-coroutines-core/api/kotlinx-coroutines-core.api`
2. **Comparison**: Kotlin method names converted to C++ snake_case conventions
3. **Marking**: Missing APIs marked with `TODO: MISSING API` in respective header files

## Summary

| Class | Total Methods | Implemented | Missing | Completeness |
|-------|---------------|-------------|---------|--------------|
| Job | 17 | 15 | 2 | 88% |
| Deferred | 5 | 5 | 0 | 100% |
| CoroutineScope | 1 | 1 | 0 | 100% |
| CancellableContinuation | 12 | 12 | 0 | 100% |
| AbstractCoroutine | 6 | 6* | 0* | 100%* |
| JobSupport | 35+ | 20 | 15+ | ~60% |
| CoroutineDispatcher | 12 | 11 | 1 | 92% |
| Dispatchers | 5 | 4 | 1 | 80% |
| Delay | 3 | 2 | 1 | 67% |
| CoroutineStart | 5 | 2 | 3 | 40% |
| MainCoroutineDispatcher | 4 | 2 | 2 | 50% |

*Some methods present but signature may not match exactly (suspend functions vs regular functions)

## Detailed Findings

### 1. Job Interface ✅ 88% Complete

**Location**: `include/kotlinx/coroutines/Job.hpp`

#### Missing APIs:
- `get_on_join()` - Returns SelectClause0 for select expressions
- `plus(Job)` - Combines two jobs

#### Status:
```cpp
// TODO: MISSING API markers added at line ~295
```

### 2. Deferred Interface ✅ 100% Complete

**Location**: `include/kotlinx/coroutines/Deferred.hpp`

All methods present:
- `await()` - ✓
- `get_completed()` - ✓
- `get_on_await()` - ✓
- All Job methods inherited - ✓

### 3. CoroutineScope ✅ 100% Complete

**Location**: `include/kotlinx/coroutines/CoroutineScope.hpp`

Single method present:
- `get_coroutine_context()` - ✓

### 4. CancellableContinuation ✅ 100% Complete

**Location**: `include/kotlinx/coroutines/CancellableContinuation.hpp`

All methods present with correct signatures.

### 5. AbstractCoroutine ⚠️ 100%* (Needs Review)

**Location**: `include/kotlinx/coroutines/AbstractCoroutine.hpp`

#### Present but different signatures:
- `start()` - Present but takes `std::function` instead of suspend lambda
- Methods inherited from Job/Continuation - Present

#### Status:
```cpp
// TODO: MISSING API markers added with notes at line ~112
// Flags that some methods exist but don't match Kotlin signatures exactly
```

### 6. JobSupport ⚠️ ~60% Complete

**Location**: `include/kotlinx/coroutines/JobSupport.hpp`

This is the largest gap. Many internal methods missing.

#### Key Missing APIs:
- `cancel_internal()` - Internal cancellation
- `cancel_coroutine()` - Cancel with specific cause
- `get_child_job_cancellation_cause()` - Exception for child cancellation  
- `get_completion_cause()` - Completion exception
- `get_completion_cause_handled()` - Check if exception handled
- `get_completion_exception_or_null()` - Get exception or null
- `get_on_await_internal()` - SelectClause1 for await
- `init_parent_job()` - Initialize parent-child relationship
- `to_cancellation_exception()` - Convert to CancellationException
- `await_internal()` - Suspend function to wait

#### Present APIs:
- `attach_child()` - ✓
- `cancel()` - ✓
- `is_active()` - ✓
- `is_cancelled()` - ✓
- `is_completed()` - ✓
- `join()` - ✓
- `handle_job_exception()` - ✓
- `on_cancelling()` - ✓
- `parent_cancelled()` - ✓
- `start()` - ✓

#### Status:
```cpp
// Comprehensive TODO: MISSING API section added at line ~347
// Lists all 15+ missing methods with translations
```

### 7. CoroutineDispatcher ✅ 92% Complete

**Location**: `include/kotlinx/coroutines/CoroutineDispatcher.hpp`

#### Missing API:
- `minus_key()` - Remove dispatcher from context

#### Present APIs:
- `dispatch()` - ✓
- `dispatch_yield()` - ✓  
- `is_dispatch_needed()` - ✓
- `intercept_continuation()` - ✓
- `release_intercepted_continuation()` - ✓
- `limited_parallelism()` - ✓
- `plus()` - ✓
- `to_string()` - ✓

#### Status:
```cpp
// TODO: MISSING API marker added at line ~79
```

### 8. Dispatchers ✅ 80% Complete

**Location**: `include/kotlinx/coroutines/Dispatchers.hpp`

#### Missing API:
- `shutdown()` - Shut down built-in dispatchers

#### Present APIs:
- `get_default()` - ✓
- `get_main()` - ✓
- `get_unconfined()` - ✓
- `get_io()` - ✓

#### Status:
```cpp
// TODO: MISSING API marker added at line ~37
```

### 9. Delay Interface ⚠️ 67% Complete

**Location**: `include/kotlinx/coroutines/Delay.hpp`

#### Missing API:
- `delay(timeMillis)` as suspend function - Current implementation blocks

#### Present APIs:
- `schedule_resume_after_delay()` - ✓
- `invoke_on_timeout()` - ✓
- Non-suspend `delay()` functions - ✓ (but not true suspend functions)

#### Status:
```cpp
// TODO: MISSING API marker added at line ~65
// Notes that delay should be suspending, not blocking
```

### 10. CoroutineStart Enum ⚠️ 40% Complete

**Location**: `include/kotlinx/coroutines/CoroutineStart.hpp`

#### Missing APIs:
- `get_entries()` - Get all enum values
- `invoke()` - Operator to start coroutine
- `is_lazy()` - Check if lazy start
- `value_of()` - Get enum from string
- `values()` - Get all values

#### Present APIs:
- Enum values (DEFAULT, LAZY, ATOMIC, UNDISPATCHED) - ✓

### 11. MainCoroutineDispatcher ⚠️ 50% Complete

**Location**: `include/kotlinx/coroutines/MainCoroutineDispatcher.hpp`

#### Missing APIs:
- `limited_parallelism()` - Override from CoroutineDispatcher
- `to_string_internal_impl()` - Internal string representation

#### Present APIs:
- `get_immediate()` - ✓
- Core dispatcher methods inherited - ✓

### 12. CancellableContinuationImpl ⚠️ Implementation Class

**Location**: `include/kotlinx/coroutines/CancellableContinuationImpl.hpp`

This is an implementation class with many internal methods. Not audited in detail as it's not part of public API surface.

## Priority for Implementation

### High Priority (Public API)
1. **Job.get_on_join()** - Needed for select expressions
2. **Job.plus()** - Job composition
3. **Dispatchers.shutdown()** - Resource cleanup
4. **Delay.delay() as suspend** - Core suspending function

### Medium Priority (Advanced Features)
5. **JobSupport internal methods** - Many are internal/protected
6. **CoroutineDispatcher.minus_key()** - Context manipulation
7. **CoroutineStart methods** - Enum utilities

### Low Priority (Rarely Used)
8. **MainCoroutineDispatcher.limited_parallelism()** - Override
9. **toString implementations** - Debugging

## Implementation Notes

### Suspend Functions Challenge

Many missing APIs are **suspend functions** in Kotlin:
- `delay(timeMillis)`
- `await_internal()`
- `join()` (implemented but not as suspend)

**Current Status**: We have a suspending mechanism (Protothreads-based) but haven't integrated it fully with the API.

**Action Items**:
- Complete `kotlin/coroutines/SuspendMacros.hpp` integration
- Update builder functions to use suspend mechanism
- Retrofit existing blocking APIs to use suspend patterns

### Select Expressions

Several missing APIs involve `SelectClause0` and `SelectClause1`:
- `Job.get_on_join()`
- `Deferred.get_on_await()`
- `JobSupport.get_on_await_internal()`

**Current Status**: Select API exists in `include/kotlinx/coroutines/selects/` but needs integration.

**Action Items**:
- Review select implementation completeness
- Add missing SelectClause getters to Job/Deferred
- Test select expressions with coroutine builders

### Context Operations

Missing context manipulation methods:
- `CoroutineDispatcher.minus_key()`
- `CoroutineContext.plus()` overloads

**Current Status**: Basic context operations work, advanced operations incomplete.

**Action Items**:
- Implement minus_key for removing elements
- Add operator overloads for context combination

## Files Modified

All files with `TODO: MISSING API` markers:

1. `include/kotlinx/coroutines/Job.hpp` - 2 TODOs
2. `include/kotlinx/coroutines/Dispatchers.hpp` - 1 TODO
3. `include/kotlinx/coroutines/Delay.hpp` - 1 TODO  
4. `include/kotlinx/coroutines/AbstractCoroutine.hpp` - 6 TODOs (documentation)
5. `include/kotlinx/coroutines/JobSupport.hpp` - 15+ TODOs
6. `include/kotlinx/coroutines/CoroutineDispatcher.hpp` - 1 TODO

## Search Instructions

To find all missing APIs:
```bash
grep -r "TODO: MISSING" include/kotlinx/coroutines/
```

To find specific class TODOs:
```bash
grep -A 5 "TODO: MISSING API - kotlinx.coroutines.Job" include/
```

## Next Steps

1. ✅ Mark missing APIs with TODO comments
2. ⏳ Prioritize critical public APIs
3. ⏳ Implement suspend function integration
4. ⏳ Complete select expression support
5. ⏳ Add context manipulation operations
6. ⏳ Review and implement JobSupport internals
7. ⏳ Add comprehensive API tests

## Conclusion

Our C++ port has **~80% completeness** for high-level public APIs and **~60% completeness** for internal/implementation APIs. Most critical user-facing methods are present. The main gaps are:

1. **Suspend functions** not fully integrated
2. **Select expressions** need completion
3. **JobSupport internals** partially implemented
4. **Context operations** need enhancement

The codebase is functional for basic coroutine usage but needs work for advanced scenarios like select expressions and fine-grained job control.

---

**Document Status**: Complete  
**Last Updated**: December 10, 2024  
**Maintainer**: Review quarterly
