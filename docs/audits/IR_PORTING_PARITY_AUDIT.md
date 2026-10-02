# IR Porting Parity Audit: Kotlin Lowering vs C++ Coroutines

**Date:** October 2026  
**Auditor:** Sydney Renee  
**Branch:** `session/2026-10-01-ir-handoffs`  
**Related Documents:** [IR_HANDOFF_REVIEW.md](IR_HANDOFF_REVIEW.md), [IR_SUSPEND_LOWERING_SPEC.md](../suspension/IR_SUSPEND_LOWERING_SPEC.md), [SUSPEND_IMPLEMENTATION.md](../suspension/SUSPEND_IMPLEMENTATION.md), [docking_ring.md](../architecture/docking_ring.md), [API_AUDIT.md](API_AUDIT.md)

---

## 1. Executive Summary & Purpose

The recent fixes to LLVM IR lowering in `kotlinx.coroutines-cpp` resolved critical flaws in compiler settings, text cleanup, and frame assumptions:
- **Compiler launcher (`kxs_compile.py`)**: Intercepts Clang compilation to emit IR, run conservative marker cleanup (`kxs_transform_ir.py`), and assemble the final object without losing compiler definitions, include paths, or dependency tracking.
- **Native tool hardening (`kxs_inject.cpp`)**: Verified module parsing that erases only direct `call void @__kxs_suspend_point(i32 N)` no-op calls, eliminating erroneous assumptions about coroutine frame layout (e.g. treating argument 0 as label pointer on polymorphic classes).
- **Macro state machine (`src/kotlinx/coroutines/dsl/Suspend.hpp`)**: Established Clang computed gotos (`void* _label`, `&&label`, `goto *_label`) compiling directly to LLVM `indirectbr` + `blockaddress`, matching Kotlin/Native's address-dispatch lowering.

This audit evaluates the entire C++ codebase—and establishes rules for all future porting—to guarantee that **C++ transliterations faithfully mirror Kotlin's compiler lowering semantics**. Specifically, it investigates how suspending functions, flow operators, channel operations, and coroutine builders interact with the Continuation ABI and state machine boundaries.

---

## 2. Ground Truth: Kotlin/Native Compiler Lowering

The Kotlin/Native compiler (ground truth in `tmp/kotlin`) lowers coroutines through four coordinated phases:

### 2.1 NativeSuspendFunctionLowering (`NativeSuspendFunctionLowering.kt:55-335`)
1. **Continuation Parameter Injection**: Every `suspend fun foo(args): T` is transformed into an entry point `foo(args, completion: Continuation<T>): Any?`.
2. **State Machine Class Generation**: Non-tail suspend functions generate a local subclass of `ContinuationImpl`.
3. **Continuation Routing (Critical Invariant)**:
   - When a coroutine calls a suspending sub-operation, it passes **its own state machine instance (`this`)** as the continuation parameter.
   - It **never passes the parent `completion`** to an intermediate suspension point. Passing the parent completion would bypass the remaining execution of the caller upon resumption.
4. **Result & Sentinel Handling**:
   - The operation is called. If it returns the sentinel `COROUTINE_SUSPENDED`, the state machine immediately returns `COROUTINE_SUSPENDED`.
   - If it returns an ordinary value, execution continues synchronously.
   - On resumption, `result.getOrThrow()` is invoked to unpack the value or rethrow failures before executing subsequent code.
5. **Tail-Call Optimization**: If the *only* suspension point is in the tail position, no state machine is generated; the caller's `completion` is forwarded directly.

### 2.2 CoroutinesVarSpillingLowering (`CoroutinesVarSpillingLowering.kt:68-105`)
- Computes variable liveness across each suspension point.
- Any variable whose lifetime crosses a suspension point is spilled into a dedicated field in the coroutine state machine class.
- Stack locals cannot survive across suspension because the function frame returns to its caller when suspended.

### 2.3 IrToBitcode (`IrToBitcode.kt:2289-2348`)
- Emits function-local block addresses: `blockaddress(@func, %resume_point)`.
- Stores the resume block address into the coroutine frame's `_label` field (a `NativePtr`).
- Emits an `indirectbr` instruction at the function's entry block to dispatch resumption directly to the saved block address.

### 2.4 ContinuationImpl Loop (`ContinuationImpl.kt:21-45`)
- `resumeWith(result)` executes an iterative loop unrolling continuation recursion.
- Calls `invokeSuspend(result)`.
- If `invokeSuspend` returns `COROUTINE_SUSPENDED`, the loop breaks and waits for asynchronous completion.
- Once the coroutine terminates (normal value or thrown exception), the outcome is passed to the parent `completion.resumeWith(outcome)`.

---

## 3. The 5 Golden Rules for C++ Coroutine Transliteration

To match Kotlin's lowering exactly, all C++ coroutine code must strictly follow these five invariants:

```
┌────────────────────────────────────────────────────────────────────────────────────────┐
│                        THE 5 INVARIANTS OF C++ COROUTINE LOWERING                      │
├────────────────────────────────────────────────────────────────────────────────────────┤
│ 1. Continuation ABI Signature:                                                         │
│    void* func(args..., Continuation<void*>* completion)                                │
│                                                                                        │
│ 2. Address-Dispatch State Machine:                                                     │
│    coroutine_begin(this) -> goto *(this)->_label -> coroutine_yield -> coroutine_end      │
│                                                                                        │
│ 3. Strict Continuation Routing (No Parent Bypassing):                                  │
│    Sub-operations MUST receive the current frame (this / shared_from_this()),          │
│    NEVER the parent completion, unless it is a pure tail-call delegation.              │
│                                                                                        │
│ 4. Heap Frame Retention & No Stack Allocations Across Suspension:                      │
│    Stack objects (collectors, lambdas, locals) are destroyed upon returning            │
│    COROUTINE_SUSPENDED. All state crossing suspension must be in heap-retained fields. │
│                                                                                        │
│ 5. Result Protocol & Failure Propagation:                                              │
│    Resumed code MUST consume (void)result.get_or_throw() before executing downstream.  │
└────────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 4. Subsystem Audit & Defect Findings

A comprehensive audit of the C++ codebase revealed significant variations in adherence to these rules. While the core suspension infrastructure and recent reduction operators are solid, several flow operators and channel operations suffer from critical architectural defects.

### 4.1 Flow Terminal Operators

#### The Reference Standard: `Reduce.hpp` / `Reduce.cpp` ✅
- **Pattern**: `ReduceFrame` and `FoldFrame` extend `FlowCollector<T>`, `Continuation<void*>`, and `std::enable_shared_from_this`.
- **Why It Matches Kotlin**:
  1. Spilled state (`accumulator`, `has_value`, `failure`) lives in the heap frame, not on the C++ stack.
  2. Suspending reduction operations receive a heap-retained `FunctionalContinuation` capturing `self = shared_from_this()`.
  3. `retain_self()` and `release_self()` prevent premature deallocation during async collection.
  4. Resumed results are coordinated under mutex, check failure, and resume `completion` only upon terminal finish.

#### Critical Defect 1: Stack-Allocated Collectors in `Collect.hpp` ❌
- **Affected File**: `src/kotlinx/coroutines/flow/Collect.hpp:34-58`
- **The Code**:
  ```cpp
  template <typename T>
  inline void* collect(std::shared_ptr<Flow<T>> flow,
                       std::function<void*(T, Continuation<void*>*)> action,
                       Continuation<void*>* continuation) {
      class ActionCollector : public FlowCollector<T> { ... };
      ActionCollector collector(std::move(action)); // STACK ALLOCATION
      return flow->collect(&collector, continuation);
  }
  ```
- **The Violation (Rule 4)**:
  `ActionCollector` is allocated on the C++ stack of `collect()`. If `flow->collect` suspends and returns `COROUTINE_SUSPENDED`, `collect()` returns immediately, unwinding its stack frame and **destroying `collector`**.
  When the upstream flow later resumes asynchronously and emits a value, `&collector` is a **dangling pointer**, causing memory corruption or segmentation faults.
- **Remedy**: `ActionCollector` must be allocated on the heap via `std::make_shared` and retained until the collection terminates.

#### Critical Defect 2: Stack Allocations in `Collection.hpp` ❌
- **Affected File**: `src/kotlinx/coroutines/flow/Collection.hpp:59-73`
- **The Code**:
  ```cpp
  template <typename T, typename Container>
  inline void* to_collection(Flow<T>* flow, Container* destination,
                             std::shared_ptr<Continuation<void*>> completion) {
      detail::CollectingFlowCollector<T, Container> collector(destination); // STACK
      void* collect_result = dsl::suspend(flow->collect(&collector, completion.get()));
      if (intrinsics::is_coroutine_suspended(collect_result)) {
          return intrinsics::get_COROUTINE_SUSPENDED();
      }
      return static_cast<void*>(destination); // NEVER RUNS ON ASYNC RESUME
  }
  ```
- **The Violations (Rules 3, 4, 5)**:
  1. `collector` is destroyed when `collect` suspends.
  2. `completion.get()` is passed directly to `flow->collect`. When collection finishes, it resumes `completion` with `Unit` / `nullptr`.
  3. The post-suspension code `return static_cast<void*>(destination)` is **never re-entered**. The caller expecting a pointer to `destination` receives `nullptr` instead.
- **Remedy**: Convert `to_collection` to use a dedicated heap frame (`ToCollectionFrame`) extending `FlowCollector` and `Continuation`.

#### Critical Defect 3: Lost Increments and Dangling Pointers in `Count.hpp` ❌
- **Affected File**: `src/kotlinx/coroutines/flow/Count.hpp:42-62, 87-124`
- **The Code**:
  ```cpp
  template <typename T>
  inline void* count(Flow<T>* flow, std::shared_ptr<Continuation<void*>> completion) {
      int i = 0; // STACK
      detail::CountCollector<T> collector(&i); // STACK
      void* collect_result = dsl::suspend(flow->collect(&collector, completion.get()));
      if (intrinsics::is_coroutine_suspended(collect_result)) {
          return intrinsics::get_COROUTINE_SUSPENDED();
      }
      return new int(i); // NEVER RUNS ON RESUME
  }
  ```
- **The Violations (Rules 3, 4, 5)**:
  1. Both `i` and `collector` are stack locals destroyed upon suspension.
  2. In `CountPredicateCollector::emit`:
     ```cpp
     void* predicate_result = dsl::suspend(predicate_(std::move(value), continuation));
     if (intrinsics::is_coroutine_suspended(predicate_result)) return COROUTINE_SUSPENDED;
     if (matched) ++(*counter_); // NEVER RUNS IF PREDICATE SUSPENDS!
     ```
     Passing `continuation` directly to `predicate_` causes `continuation` to be resumed with the predicate boolean, completely skipping the subsequent `++(*counter_)`!
- **Remedy**: Port `CountFrame` following the `ReduceFrame` architecture with an internal predicate continuation bridge.

#### Critical Defect 4: Swallowed Suspension & Invalid Return in `Logic.hpp` ❌
- **Affected File**: `src/kotlinx/coroutines/flow/Logic.hpp:71-92`
- **The Code**:
  ```cpp
  template <typename T>
  [[suspend]] inline bool any(Flow<T>* flow,
                              std::function<void*(T, Continuation<void*>*)> predicate,
                              std::shared_ptr<Continuation<void*>> completion) {
      bool found = false;
      auto sink = detail::CollectWhileCollector<T>(
          [&found, predicate = std::move(predicate)](T value, Continuation<void*>* cont) {
              void* result = dsl::suspend(predicate(std::move(value), cont));
              if (intrinsics::is_coroutine_suspended(result)) return true; // SWALLOWED!
              bool satisfies = result && *static_cast<bool*>(result);
              if (satisfies) found = true;
              return !satisfies;
          });
      dsl::suspend(flow->collect(&sink, completion.get()));
      return found; // CANNOT RETURN COROUTINE_SUSPENDED!
  }
  ```
- **The Violations (Rules 1, 4, 5)**:
  1. Return type is `bool` instead of `void*`. It is physically incapable of returning `COROUTINE_SUSPENDED`.
  2. If `predicate` suspends, it executes `return true; // keep going on suspend`, completely swallowing the suspension and breaking coroutine semantics.
  3. `sink` is allocated on the stack.
- **Remedy**: Rewrite `any`, `all`, `none` to return `void*` using a heap-allocated `LogicFrame<T>`.

---

### 4.2 Flow Intermediate Operators

#### Critical Defect 5: Lost Downstream Emission in `Transform.hpp` ❌
- **Affected File**: `src/kotlinx/coroutines/flow/Transform.hpp:121-144`
- **The Code**:
  ```cpp
  template <typename T>
  inline std::shared_ptr<Flow<T>> filter(
      std::shared_ptr<Flow<T>> upstream,
      std::function<void*(const T&, Continuation<void*>*)> predicate) {
      return internal::unsafe_transform<T, T>(
          std::move(upstream),
          [predicate = std::move(predicate)](FlowCollector<T>* collector, T value, Continuation<void*>* cont) -> void* {
              void* res = predicate(value, cont); // PASSES CALLER'S CONT DIRECTLY
              if (intrinsics::is_coroutine_suspended(res)) {
                  return intrinsics::get_COROUTINE_SUSPENDED();
              }
              // Code below runs ONLY if predicate was synchronous:
              bool matches = res && *static_cast<bool*>(res);
              if (matches) return collector->emit(std::move(value), cont);
              return nullptr;
          });
  }
  ```
- **The Violation (Rule 3 & State Machine Lowering)**:
  In Kotlin, `filter` is an inline function whose lambda is lowered into a 2-state coroutine state machine:
  - State 0: invoke `predicate(value, this)`.
  - State 1: unpack boolean. If true, invoke `collector.emit(value, this)`.
  - State 2: complete.
  
  In `Transform.hpp`, because there is no state machine:
  1. If `predicate` suspends, it receives `cont` (the caller's continuation).
  2. When `predicate` finishes in the background, it resumes `cont` directly.
  3. The caller (`upstream.collect`) wakes up thinking `collector.emit` has finished!
  4. The code `if (matches) collector->emit(value)` is **never executed**! The value is silently lost.
- **Remedy**: Intermediate transform operators must bridge suspending callbacks through a `TransformFrame` or `FunctionalContinuation` that intercepts the predicate result and triggers downstream `emit`.

---

### 4.3 Channels

#### Critical Defect 6: Missing Suspend Implementation in `BufferedChannel` ❌
- **Affected File**: `src/kotlinx/coroutines/channels/BufferedChannel.hpp:1155-1178`
- **The Code**:
  ```cpp
  void* receive(Continuation<void*>* continuation) override {
      auto result = try_receive();
      if (result.is_success()) return new E(result.get_or_throw());
      if (result.is_closed()) ...
      // Need to suspend - full state machine implementation required
      (void)continuation;
      throw std::logic_error("BufferedChannel::receive suspend path requires full state machine implementation");
  }
  ```
- **The Violation**:
  While `send()` implements `send_on_no_waiter_suspend` using `CancellableContinuationImpl`, `receive()` and `receive_catching()` throw a `logic_error` whenever the channel buffer is empty.
- **Remedy**: Port `receive_on_no_waiter_suspend` using `CancellableContinuationImpl<E>` matching Kotlin's `receiveOnNoWaiterSuspend`.

---

### 4.4 Coroutine Builders & Select Expressions

#### Builder Suspension Status: `Builders.hpp` ⚠️
- `launch` and `async` currently accept synchronous blocks `std::function<void(CoroutineScope*)>` or `std::function<T(CoroutineScope*)>`.
- While `CoroutineStart::invoke` in `CoroutineStart.hpp` has stubs for `is_invocable_v<Block, R, Continuation<void*>*>`, full suspend lambda builders are not yet exposed in user-facing overloads.
- `with_context`, `coroutine_scope`, and `supervisor_scope` need full `suspend_cancellable_coroutine` implementations.

#### Select Suspension Status: `Select.hpp` ❌
- `SelectInstance::do_select` currently throws `std::runtime_error("No clause selected")`.
- Must be wired to `suspend_cancellable_coroutine` to suspend until a registered clause completes.

---

## 5. Canonical Implementation Templates for Future Porting

Any future file porting Kotlin coroutines to C++ must follow one of these three canonical patterns:

### Pattern A: Terminal Operator Frame (Derived from `ReduceFrame`)

Use this pattern for terminal operators (`collect`, `count`, `toList`, `any`, `all`):

```cpp
template <typename T, typename R>
class MyTerminalFrame : public FlowCollector<T>,
                        public Continuation<void*>,
                        public std::enable_shared_from_this<MyTerminalFrame<T, R>> {
public:
    R result_accumulator{};
    Continuation<void*>* parent_completion = nullptr;
    std::atomic<bool> is_completed{false};
    std::shared_ptr<MyTerminalFrame<T, R>> self_retention;

    MyTerminalFrame(Continuation<void*>* completion)
        : parent_completion(completion) {}

    void retain_self() {
        if (!is_completed.load()) self_retention = this->shared_from_this();
    }

    void* emit(T value, Continuation<void*>* cont) override {
        // Process value; if child operation suspends, bridge via FunctionalContinuation
        return nullptr;
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return parent_completion ? parent_completion->get_context() : nullptr;
    }

    void resume_with(Result<void*> outcome) override {
        if (is_completed.exchange(true)) return;
        auto guard = std::move(self_retention);
        if (parent_completion) {
            if (outcome.is_success()) {
                parent_completion->resume_with(Result<void*>::success(new R(std::move(result_accumulator))));
            } else {
                parent_completion->resume_with(outcome);
            }
        }
    }
};
```

### Pattern B: Intermediate Operator Continuation Bridge

Use this pattern when an operator must suspend on a user lambda before emitting downstream:

```cpp
// Within intermediate collector's emit(value, cont):
auto self = this->shared_from_this();
auto bridge_cont = std::make_shared<FunctionalContinuation<void*>>(
    cont ? cont->get_context() : nullptr,
    [self, downstream = this->downstream_, value, cont](Result<void*> op_res) {
        if (!op_res.is_success()) {
            if (cont) cont->resume_with(op_res);
            return;
        }
        void* raw = op_res.get_or_throw();
        bool condition = raw && *static_cast<bool*>(raw);
        delete static_cast<bool*>(raw);
        
        if (condition) {
            void* emit_res = downstream->emit(std::move(value), cont);
            if (!intrinsics::is_coroutine_suspended(emit_res)) {
                if (cont) cont->resume_with(Result<void*>::success(nullptr));
            }
        } else {
            if (cont) cont->resume_with(Result<void*>::success(nullptr));
        }
    }
);

void* res = predicate(value, bridge_cont.get());
if (intrinsics::is_coroutine_suspended(res)) {
    return intrinsics::get_COROUTINE_SUSPENDED();
}
// Synchronous fast-path handling...
```

### Pattern C: Standalone Suspend Function (`Suspend.hpp` Macro Pattern)

Use this pattern for multi-step suspend algorithms:

```cpp
class MyCoroutine : public ContinuationImpl {
public:
    void* _label = nullptr;
    int spilled_local = 0;  // Spilled variable across suspension

    explicit MyCoroutine(std::shared_ptr<Continuation<void*>> parent)
        : ContinuationImpl(std::move(parent)) {}

    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)

        spilled_local = 42;
        // Current frame (this) must be passed to suspending operation:
        coroutine_yield(this, delay(100, this));

        // State is preserved across resumption:
        use_value(spilled_local);

        coroutine_end(this)
    }
};
```

---

## 6. Porting Checklist & Verification

When transliterating or auditing any Kotlin coroutine source file:

- [ ] **No stack-allocated collectors**: Verify that all `FlowCollector` instances are allocated on the heap or managed as part of an enclosing frame.
- [ ] **No parent continuation passing**: Confirm that intermediate suspension calls pass `this` / `shared_from_this()`, not `parent_completion`.
- [ ] **No post-suspend straight-line logic**: Ensure that code following a suspend call is enclosed in a state machine resume block or a `FunctionalContinuation` callback.
- [ ] **Suspend return type**: Suspend functions return `void*`, never `bool`, `int`, or `void`.
- [ ] **No TODO / FIXME in source**: If a semantic gap exists, file an issue or document it in the audit index; do not leave TODO comments in source.
- [ ] **CMake Integration**: Add `kxs_enable_suspend(<target>)` in `CMakeLists.txt` for any target containing `Suspend.hpp` state machines.
- [ ] **CTest Suite**: Add regression coverage to `src/tests/` verifying both synchronous execution and true asynchronous resumption under AddressSanitizer.
