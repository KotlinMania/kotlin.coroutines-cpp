# Kotlin to C++ API Translation Guide

This document maps the official Kotlinx.coroutines API to our C++ implementation.

## Translation Rules

### Naming Conventions
- **Classes/Interfaces**: PascalCase (same as Kotlin)
  - Kotlin: `AbstractCoroutine` → C++: `AbstractCoroutine`
- **Methods/Functions**: snake_case
  - Kotlin: `resumeWith` → C++: `resume_with`
  - Kotlin: `afterResume` → C++: `after_resume`
  - Kotlin: `isActive` → C++: `is_active`
  - Kotlin: `getContext` → C++: `get_context`
- **Constants**: SCREAMING_SNAKE_CASE
  - Kotlin: `COROUTINE_SUSPENDED` → C++: `COROUTINE_SUSPENDED`
- **Properties**: snake_case (as methods with get_ prefix)
  - Kotlin: `val context` → C++: `get_context()`

### Type Mappings
- `Continuation<T>` → `Continuation<T>`
- `suspend fun` → State machine with `invoke_suspend(Result<void*>)`
- `Any?` → `void*` (type-erased)
- `Throwable` → `std::exception_ptr`
- `Function1<A, R>` → `std::function<R(A)>`

---

## AbstractCoroutine

**Source**: `kotlinx.coroutines.AbstractCoroutine`

### Kotlin API
```kotlin
public abstract class AbstractCoroutine : JobSupport, Continuation, CoroutineScope, Job {
    public fun <init>(context: CoroutineContext, initParentJob: Boolean, active: Boolean)
    
    // Continuation
    public final fun resumeWith(result: Result<T>)
    public final fun getContext(): CoroutineContext
    
    // CoroutineScope
    public fun getCoroutineContext(): CoroutineContext
    
    // Job
    public fun isActive(): Boolean
    
    // Lifecycle hooks
    protected fun afterResume(state: Any?)
    protected fun cancellationExceptionMessage(): String
    protected fun onCancelled(cause: Throwable, handled: Boolean)
    protected fun onCompleted(value: T)
    protected final fun onCompletionInternal(state: Any?)
    
    // Start
    public final fun start(start: CoroutineStart, receiver: R, block: suspend R.() -> T)
}
```

### C++ Translation
```cpp
template<typename T>
class AbstractCoroutine : public JobSupport, public Continuation<T>, public CoroutineScope {
public:
    AbstractCoroutine(
        std::shared_ptr<CoroutineContext> context,
        bool init_parent_job,
        bool active
    );
    
    // Continuation
    void resume_with(Result<T> result) override;
    std::shared_ptr<CoroutineContext> get_context() const override;
    
    // CoroutineScope
    std::shared_ptr<CoroutineContext> get_coroutine_context() const override;
    
    // Job
    bool is_active() const override;
    
    // Lifecycle hooks
protected:
    virtual void after_resume(void* state);
    virtual std::string cancellation_exception_message();
    virtual void on_cancelled(std::exception_ptr cause, bool handled);
    virtual void on_completed(T value);
    void on_completion_internal(void* state);
    
    // Start
public:
    template<typename R, typename Block>
    void start(CoroutineStart start, R receiver, Block block);
};
```

### Implementation Status
- ✅ Class structure
- ✅ `resume_with`
- ✅ `get_context`
- ✅ `get_coroutine_context`
- ✅ `is_active`
- ✅ `after_resume`
- ✅ `cancellation_exception_message`
- ✅ `on_cancelled`
- ✅ `on_completed`
- ✅ `on_completion_internal`
- ⚠️  `start` - partially implemented

---

## CancellableContinuation

**Source**: `kotlinx.coroutines.CancellableContinuation`

### Kotlin API
```kotlin
public interface CancellableContinuation<T> : Continuation<T> {
    public fun cancel(cause: Throwable? = null): Boolean
    public fun completeResume(token: Any)
    public fun initCancellability()
    public fun invokeOnCancellation(handler: (Throwable?) -> Unit)
    public fun isActive(): Boolean
    public fun isCancelled(): Boolean
    public fun isCompleted(): Boolean
    
    public fun resume(value: T, onCancellation: ((Throwable) -> Unit)? = null)
    public fun resume(value: T, onCancellation: ((Throwable, T, CoroutineContext) -> Unit)? = null)
    
    public fun resumeUndispatched(dispatcher: CoroutineDispatcher, value: T)
    public fun resumeUndispatchedWithException(dispatcher: CoroutineDispatcher, exception: Throwable)
    
    public fun tryResume(value: T, idempotent: Any? = null): Any?
    public fun tryResume(value: T, idempotent: Any?, onCancellation: ((Throwable, T, CoroutineContext) -> Unit)? = null): Any?
    public fun tryResumeWithException(exception: Throwable): Any?
}
```

### C++ Translation
```cpp
template<typename T>
class CancellableContinuation : public Continuation<T> {
public:
    virtual ~CancellableContinuation() = default;
    
    virtual bool cancel(std::exception_ptr cause = nullptr) = 0;
    virtual void complete_resume(void* token) = 0;
    virtual void init_cancellability() = 0;
    virtual void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) = 0;
    
    virtual bool is_active() const = 0;
    virtual bool is_cancelled() const = 0;
    virtual bool is_completed() const = 0;
    
    virtual void resume(
        T value,
        std::function<void(std::exception_ptr)> on_cancellation = nullptr
    ) = 0;
    
    virtual void resume(
        T value,
        std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)> on_cancellation
    ) = 0;
    
    virtual void resume_undispatched(
        std::shared_ptr<CoroutineDispatcher> dispatcher,
        T value
    ) = 0;
    
    virtual void resume_undispatched_with_exception(
        std::shared_ptr<CoroutineDispatcher> dispatcher,
        std::exception_ptr exception
    ) = 0;
    
    virtual void* try_resume(T value, void* idempotent = nullptr) = 0;
    
    virtual void* try_resume(
        T value,
        void* idempotent,
        std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)> on_cancellation
    ) = 0;
    
    virtual void* try_resume_with_exception(std::exception_ptr exception) = 0;
};
```

### Implementation Status
- ✅ Interface structure
- ✅ `cancel`
- ⚠️  `complete_resume` - stub
- ⚠️  `init_cancellability` - stub
- ✅ `invoke_on_cancellation`
- ✅ `is_active`
- ✅ `is_cancelled`
- ✅ `is_completed`
- ⚠️  `resume` (both overloads) - partial
- ❌ `resume_undispatched` - missing
- ❌ `resume_undispatched_with_exception` - missing
- ⚠️  `try_resume` - partial
- ⚠️  `try_resume_with_exception` - partial

---

## Job

**Source**: `kotlinx.coroutines.Job`

### Kotlin API Methods (25 total)
- attachChild ✅
- cancel (multiple overloads) ✅ (partial)
- fold ✅ (inherited from CoroutineContext.Element)
- get ✅ (inherited from CoroutineContext.Element)
- getCancellationException ✅
- getChildren ✅
- **getOnJoin** ❌ MISSING - select clause support
- getParent ✅
- invokeOnCompletion (multiple overloads) ✅
- isActive ✅
- isCancelled ✅
- isCompleted ✅
- join ✅
- minusKey ✅ (inherited)
- **plus** ❌ MISSING - Job combination operator
- start ✅

### Missing Methods Analysis

#### 1. `plus` operator (Job.kt:383)
```kotlin
public operator fun Job?.plus(other: Job): Job
```
**C++ Translation:**
```cpp
// As free function (operator overloading)
std::shared_ptr<Job> operator+(
    std::shared_ptr<Job> left,
    std::shared_ptr<Job> right
);

// Or as member
virtual std::shared_ptr<Job> plus(std::shared_ptr<Job> other) const = 0;
```
**Behavior**: Returns leftmost job if it's a CompletableJob, otherwise returns rightmost job.

#### 2. `onJoin` property (Job.kt:294)
```kotlin
public val onJoin: SelectClause0
```
**Status**: Requires full select{} expression support. Deferred until select implementation.

---

## Deferred

**Source**: `kotlinx.coroutines.Deferred`

### Kotlin API Methods (10 total)
- await ✅
- cancel ✅ (inherited)
- fold ✅ (inherited)
- get ✅ (inherited)
- getCompleted ✅ (as `get_completed()`)
- getCompletionExceptionOrNull ✅
- **getOnAwait** ❌ MISSING - select clause
- minusKey ✅ (inherited)
- plus ✅ (inherited)

### Missing Methods Analysis

#### 1. `onAwait` property
```kotlin
public val onAwait: SelectClause1<T>
```
**Status**: Requires select{} support. Deferred.

---

## CoroutineDispatcher

**Source**: `kotlinx.coroutines.CoroutineDispatcher`

### Kotlin API Methods (12 total)
- dispatch ✅
- **dispatchYield** ❌ MISSING
- get ✅ (inherited)
- interceptContinuation ✅
- isDispatchNeeded ✅
- **limitedParallelism** ❌ MISSING (multiple overloads)
- minusKey ✅ (inherited)
- plus ✅ (inherited)
- releaseInterceptedContinuation ✅
- toString ✅

### Missing Methods Analysis

#### 1. `dispatchYield` (CoroutineDispatcher.kt:207)
```kotlin
public open fun dispatchYield(context: CoroutineContext, block: Runnable)
```
**C++ Translation:**
```cpp
virtual void dispatch_yield(
    std::shared_ptr<CoroutineContext> context,
    std::function<void()> block
);
```
**Behavior**: Hints the dispatcher to yield execution of the current continuation after dispatching

#### 2. `limitedParallelism` (CoroutineDispatcher.kt:236-288)
```kotlin
public open fun limitedParallelism(parallelism: Int): CoroutineDispatcher
public open fun limitedParallelism(parallelism: Int, name: String?): CoroutineDispatcher  
```
**C++ Translation:**
```cpp
virtual std::shared_ptr<CoroutineDispatcher> limited_parallelism(
    int parallelism
);

virtual std::shared_ptr<CoroutineDispatcher> limited_parallelism(
    int parallelism,
    std::optional<std::string> name
);
```
**Behavior**: Creates a view of the dispatcher with limited parallelism

---

## Summary of Missing APIs

### Critical (affects core functionality)
1. **Job.plus** - Job combination
2. **CoroutineDispatcher.dispatch_yield** - Yield hint for fairness
3. **CoroutineDispatcher.limited_parallelism** - Resource management

### Non-Critical (advanced features)
1. **Job.onJoin** - Select clause (requires full select{} support)
2. **Deferred.onAwait** - Select clause (requires full select{} support)

### Implementation Priority
1. ✅ Core continuation/job/dispatcher APIs - DONE
2. ⚠️  Job.plus, dispatch_yield, limited_parallelism - TODO
3. ❌ Select expressions - Future work

---

## Naming Convention Compliance

All method names follow C++ snake_case convention:
- Kotlin: `resumeWith` → C++: `resume_with` ✅
- Kotlin: `isActive` → C++: `is_active` ✅
- Kotlin: `invokeOnCompletion` → C++: `invoke_on_completion` ✅
- Kotlin: `getContext` → C++: `get_context` ✅
- Kotlin: `afterResume` → C++: `after_resume` ✅

Classes maintain PascalCase:
- Kotlin: `AbstractCoroutine` → C++: `AbstractCoroutine` ✅
- Kotlin: `CoroutineDispatcher` → C++: `CoroutineDispatcher` ✅
