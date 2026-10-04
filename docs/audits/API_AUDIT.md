# kotlinx.coroutines C++ API Audit

This document tracks the API completeness of our C++ transliteration against the Kotlin source.

## Audit Methodology

For each public Kotlin API file, we compare:
1. **Properties** (val/var) → C++ virtual getters/setters
2. **Functions** (fun) → C++ virtual methods
3. **Suspend functions** (suspend fun) → Continuation ABI entries returning a result box or the suspension sentinel
4. **Extension functions** → C++ free functions
5. **Companion object members** → C++ static methods or free functions

Method names are converted from camelCase to snake_case per C++ conventions.

## Core Interfaces

### Job (kotlinx.coroutines.Job)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Job.kt`  
**C++ Header**: `src/kotlinx/coroutines/Job.hpp`

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
| `suspend fun join()` | `virtual void* join(Continuation<void*>*)` | ✅ | Continuation ABI suspend function |
| `val onJoin: SelectClause0` | `virtual selects::SelectClause0& on_join()` | ✅ | Select clause wired |
| `fun invokeOnCompletion(handler)` | `virtual std::shared_ptr<DisposableHandle> invoke_on_completion()` | ✅ | |
| `fun invokeOnCompletion(onCancelling, invokeImmediately, handler)` | `virtual std::shared_ptr<DisposableHandle> invoke_on_completion()` | ✅ | |

**Notes**:
- `children` returns `vector` instead of lazy `Sequence` - acceptable for C++
- `join()` uses Continuation ABI: `void* join(Continuation<void*>*)`

---

### CoroutineDispatcher (kotlinx.coroutines.CoroutineDispatcher)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/CoroutineDispatcher.kt`  
**C++ Header**: `src/kotlinx/coroutines/CoroutineDispatcher.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `abstract fun dispatch(context, block)` | `virtual void dispatch(const CoroutineContext&, std::shared_ptr<Runnable>) const = 0` | ✅ | Core dispatch method |
| `open fun isDispatchNeeded(context): Boolean` | `virtual bool is_dispatch_needed(const CoroutineContext&) const` | ✅ | Dispatch optimization |
| `fun limitedParallelism(parallelism, name): CoroutineDispatcher` | `virtual std::shared_ptr<CoroutineDispatcher> limited_parallelism(int, const std::string&)` | ✅ | Parallelism control |
| `fun dispatchYield(context, block)` | `virtual void dispatch_yield(const CoroutineContext&, std::shared_ptr<Runnable>) const` | ✅ | Yield-aware dispatch |
| `final override fun <T> interceptContinuation(continuation): Continuation<T>` | `template <typename T> std::shared_ptr<Continuation<T>> intercept_continuation(...)` | ✅ | Continuation interception |
| `final override fun releaseInterceptedContinuation(continuation)` | `virtual void release_intercepted_continuation(...)` | ✅ | Continuation cleanup |
| `operator fun plus(other: CoroutineDispatcher): CoroutineDispatcher` | `virtual std::shared_ptr<CoroutineDispatcher> plus(...)` | ✅ | Dispatcher composition |
| `override fun toString(): String` | `virtual std::string to_string() const` | ✅ | Debug string |
| `override fun minusKey(key): CoroutineContext` | Inherited from `Element` | ✅ | Context manipulation |

**Status**: ✅ **WIRED** - Core dispatch, yield, intercept, and limited parallelism implemented

---

### Deferred<T> (kotlinx.coroutines.Deferred)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Deferred.kt`  
**C++ Header**: `src/kotlinx/coroutines/Deferred.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `val onAwait: SelectClause1<T>` | `virtual selects::SelectClause1<T>& on_await() = 0` | ✅ | Select clause wired |
| `suspend fun await(): T` | `virtual void* await(Continuation<void*>* continuation) = 0` | ✅ | Continuation ABI suspend function |
| `fun getCompleted(): T` | `virtual T get_completed() const = 0` | ✅ | |
| `fun getCompletionExceptionOrNull(): Throwable?` | `virtual std::exception_ptr get_completion_exception_or_null() const = 0` | ✅ | |

---

### Dispatchers (kotlinx.coroutines.Dispatchers)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Dispatchers.common.kt`  
**C++ Header**: `src/kotlinx/coroutines/Dispatchers.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `val Default: CoroutineDispatcher` | `static CoroutineDispatcher& get_default()` | ✅ | Shared worker thread pool |
| `val Main: MainCoroutineDispatcher` | `static MainCoroutineDispatcher& get_main()` | ✅ | Main UI thread dispatcher |
| `val Unconfined: CoroutineDispatcher` | `static CoroutineDispatcher& get_unconfined()` | ✅ | Unconfined dispatcher |
| `val IO: CoroutineDispatcher` | `static CoroutineDispatcher& get_io()` | ✅ | IO dispatcher |

**Status**: ✅ **WIRED** - Default, Main, Unconfined, and IO accessors implemented

---

### Delay (kotlinx.coroutines.Delay)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Delay.kt`  
**C++ Header**: `src/kotlinx/coroutines/Delay.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `suspend fun delay(timeMillis: Long)` | `void* delay(long long, Continuation<void*>*)` | ✅ | Continuation ABI suspend function |
| `val invokeOnTimeout: (timeMillis: Long, block: Runnable, context: CoroutineContext) -> DisposableHandle` | `virtual std::shared_ptr<DisposableHandle> invoke_on_timeout(...)` | ✅ | Timeout callbacks |
| `fun scheduleResumeAfterDelay(timeMillis: Long, continuation: CancellableContinuation<Unit>)` | `virtual void schedule_resume_after_delay(...)` | ✅ | Low-level delay scheduling |

**Status**: ✅ **WIRED** - Delay interface and delay/await_cancellation free functions implemented

---

## Continuation interception repair (2026-10-04)

| Kotlin runtime/API | C++ reference | Status | Scope |
|---|---|---|---|
| ContinuationImpl.intercepted / releaseIntercepted | ContinuationImpl.cpp:12, :24 | Wired | Cached context interceptor, release hook and completed sentinel |
| Continuation<T>.intercepted intrinsic | ContinuationImpl.cpp:53 | Wired for erased ABI | Only compiler-frame continuations are intercepted; ordinary continuations remain unchanged |
| ContinuationInterceptor default release | ContinuationInterceptor.cpp:8 | Wired | Kotlin default is a no-op |
| CoroutineDispatcher virtual erased interception | CoroutineDispatcher.hpp:137, :141; CoroutineDispatcher.cpp:41 | Wired | Exposes Continuation<void*> interception and release through the context interceptor, alongside the typed template |
| yield | Yield.cpp:43 | Wired | Dispatcher and Unconfined branches retain Kotlin ordering; queued resumption checks cancellation |
| Dispatchers.Unconfined (native) | native/Dispatchers.cpp:101 | Wired | Returns the existing canonical Unconfined object |

Detailed confirmed gaps, Kanban cards and validation scope:
[LOGIC_PARITY_REPAIR.md](LOGIC_PARITY_REPAIR.md). These entries do not certify
automatic compiler lowering or the whole Kotlin/Native binary ABI.

## Builders

### launch, async, runBlocking

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Builders.common.kt`  
**C++ Header**: `src/kotlinx/coroutines/CoroutineScope.hpp`, `src/kotlinx/coroutines/Builders.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `fun CoroutineScope.launch(context, start, block): Job` | `launch(scope, context, start, suspend_block)` | Wired | Suspend block receives a shared erased continuation: `Builders.hpp:55`, implementation `Builders.common.cpp:23`; legacy synchronous overloads remain |
| `fun CoroutineScope.async(context, start, block): Deferred<T>` | `async<T>(scope, context, start, suspend_block)` | Wired for erased suspend blocks | Builders.hpp:276; DEFAULT, LAZY and UNDISPATCHED; receiving adapter owns unboxing/deletion |
| `fun <T> runBlocking(context, block): T` | ✅ | ✅ | In Builders.hpp |
| `fun CoroutineScope.produce(context, capacity, start, onCompletion, block): ReceiveChannel<E>` | `produce(scope, context, capacity, overflow, start, suspend_block)` | Partial / Wired | `channels/Produce.hpp:187` propagates suspension; this overload does not expose `onCompletion` |
| `fun <T> withContext(context, block): T` | `with_context(...)` | Surface / divergent wiring | Builders.hpp:336; scope/dispatcher repair t_d1b9ce81 |
| `fun <T> withTimeout(timeMillis, block): T` | ❌ | ❌ MISSING | Timeout wrapper |
| `fun <T> withTimeoutOrNull(timeMillis, block): T?` | ❌ | ❌ MISSING | Nullable timeout |
| `suspend fun <T> coroutineScope(block): T` | `coroutine_scope(...)` | Surface / divergent wiring | Builders.hpp:380; structured scope repair t_d1b9ce81 |
| `suspend fun <T> supervisorScope(block): T` | `supervisor_scope(...)` | Surface / divergent wiring | Builders.hpp:410; supervisor repair t_d1b9ce81 |

Native context extensions: CoroutineScope.hpp:105 and :109 expose both
new_coroutine_context overloads; native/CoroutineContext.cpp:29 and :39 implement
Native CoroutineContext.kt:32-40. Suspend launch and async use those actuals.

**Status**: ⚠️ **PARTIAL** - Core builders exist; scope algorithms and timeouts remain incomplete

---

## Sharing continuation audit (2026-10-04)

These entries describe the repaired paths, not whole-library completion. They are
additional to the core-interface counts below.

| Kotlin API / implementation | C++ reference | Status | Verified behavior / limit |
|---|---|---|---|
| `shareIn(scope, started, replay)` and `stateIn(scope, started, initialValue)` sharing launch | `flow/Share.hpp:261` | Wired | Eager uses DEFAULT; other strategies use UNDISPATCHED; command collection uses collectLatest, including cancel/join/reset |
| `StartedWhileSubscribed.command` | `flow/SharingStarted.cpp:195` | Wired | Cancellable dispatcher delay, STOP then expiration delay then RESET; zero expiration skips STOP |
| `StartedLazily.command` | `flow/SharingStarted.cpp:146` | Wired | Collector and started flag survive suspension; emits START only once |
| `collectLatest(action)` | `flow/Collect.hpp:175`, `flow/Collect.cpp:120` | Wired | mapLatest(action).buffer(0).collect; action resumes before its Unit emission |
| `ChannelFlow.collectTo` suspend ABI | `flow/internal/ChannelFlow.hpp:327` | Wired for operator/builder/channel paths | Producer and SendingCollector remain alive through suspension; legacy concurrent-merge implementations still need translation |
| `stateIn(scope)` deferred overload | `flow/Share.hpp:282` | Surface / existing wiring | Deferred sharing path is outside this repair; no new suspension-parity claim |
| `LockFreeLinkedListNode.next`, `addLast(node, permissionsBitmask)`, `addNext`, `removeOrNext`, `close`, `toString` | `internal/LockFreeLinkedList.hpp:18`, `:22`, `:26`, `:28`, `:29`, `:31`; `internal/LockFreeLinkedList.common.cpp:63` | Wired | Atomic publication, Removed marker, predecessor repair and permission closure follow the concurrent Kotlin source; C++ marker storage owns markers, while intrusive node reclamation still requires external lifetime management |
| `JobSupport.attachChild` and completion registration | `JobSupport.cpp:721`, `:1137`, `:1194`, `:1289` | Wired | Registration respects cancellation/child/completion closure; a published single-to-list promotion survives a losing state CAS; completing flag is set under the Finishing lock before publication |
| BufferedChannel waiter unwrap and suspended send handshake | `channels/BufferedChannel.hpp:423`, `:2036`, `:2811`, `:2900`; `channels/BufferedChannel.cpp:22` | Wired for exercised paths | Plain Waiter is preserved; WaiterEB is unwrapped conditionally; a concrete Unit/erased adapter and get_result handshake preserve send resumption; sender-first and receiver-first regressions exercise suspension in both directions |

---

## Summary Statistics

These counts describe the listed surfaces and earlier assessments. They are not
a fresh AST-distance measurement or a guarantee of semantic parity.

| Category | Implemented | Partial | Missing | Total |
|----------|-------------|---------|---------|-------|
| Job | 13 | 1 | 0 | 14 |
| CoroutineDispatcher | 9 | 0 | 0 | 9 |
| Deferred | 4 | 0 | 0 | 4 |
| Dispatchers | 4 | 0 | 0 | 4 |
| Delay | 3 | 0 | 0 | 3 |
| Builders | 3 | 4 | 2 | 9 |
| **TOTAL** | **36** | **5** | **2** | **43** |

**Listed-row coverage**: ~84% (36/43 marked implemented; not whole-library parity)

---

## Priority Tasks

### In Progress / Next Up
1. `with_context()` - Context switching across dispatchers
2. `with_timeout()` / `with_timeout_or_null()` - Timeout wrappers integrated with Delay
3. `coroutine_scope()` / `supervisor_scope()` - Scope-based structured concurrency coroutine builders
4. `produce()` builder for channels
5. Compiler-driven variable spilling (Phase 2 IR lowering)

### Completed Core Tasks
- ✅ `CoroutineDispatcher::dispatch()` / `is_dispatch_needed()` / `dispatch_yield()` / `limited_parallelism()`
- ✅ `Delay::delay()` / `schedule_resume_after_delay()` / `invoke_on_timeout()`
- ✅ `Dispatchers.Default`, `Dispatchers.Main`, `Dispatchers.Unconfined`, `Dispatchers.IO`
- ✅ Suspend `join()` and `await()` using Continuation ABI
- ✅ Select clause foundations (`on_join`, `on_await`)
- ✅ Clang computed-goto state machine macros and IR marker cleanup pipeline

---

## Notes for Implementation

### Suspend Functions
Suspend functions are lowered to Kotlin/Native Continuation ABI: `void* fn(args..., Continuation<void*>* cont)` returning either the result or `intrinsics::COROUTINE_SUSPENDED`. State machine lowering uses computed goto (`Suspend.hpp`) with IR cleanup via `kxs_compile.py` / `kxs_transform_ir.py` or `kxs-inject`.

### Thread Safety
Thread safety and dispatcher behavior require implementation-specific verification. API presence and mutex/atomic use alone do not establish Kotlin parity.

### Error Handling
Uses `std::exception_ptr` and `kotlinx::coroutines::Result<T>` matching Kotlin Native's exception model.

---

**Last Updated**: October 2026  
**Next Review**: After Phase 2 compiler-driven automatic spilling and scope builder completion
