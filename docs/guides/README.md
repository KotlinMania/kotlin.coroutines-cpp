# Guides Documentation Index

This directory contains upstream Kotlin coroutines conceptual guides, hands-on tutorials, API specifications, and debugging walkthroughs.

---

## 🧭 Core Guide & Tutorials

### [coroutines-guide.md](coroutines-guide.md)
**Purpose:** Upstream Kotlin Coroutines Guide table of contents and introduction.  
**Contents:** High-level overview of coroutines concepts, suspending functions, and guide roadmap.

### [coroutines-basics.md](coroutines-basics.md)
**Purpose:** Foundations of coroutine creation, scope, and structured concurrency.  
**Contents:**
- First coroutine with `launch` and `delay`
- Bridging blocking and non-blocking worlds with `runBlocking`
- Structured concurrency, scope builders (`coroutineScope`), and child jobs
- Explicit job control and waiting for completion
- Comparison between lightweight coroutines and OS/JVM threads

### [coroutines-and-channels.md](coroutines-and-channels.md)
**Purpose:** Comprehensive hands-on tutorial: introduction to coroutines and channels.  
**Contents:**
- Blocking requests vs non-blocking UI
- Asynchronous callbacks vs suspending functions
- Concurrent decomposition with `async` and `await`
- Intermediate progress reporting with channels (rendezvous, buffered, conflated, unlimited)
- Testing with virtual time and `runTest`

---

## ⚡ Concurrency & Asynchrony Primitives

### [cancellation-and-timeouts.md](cancellation-and-timeouts.md)
**Purpose:** Cancellation mechanics, cooperative checks, and timeout handling.  
**Contents:**
- Cancelling coroutine execution and cancellation propagation
- Cooperative cancellation: `yield`, `isActive`, and handling `CancellationException`
- Cleanup in `finally` blocks and running non-cancellable blocks (`NonCancellable`)
- Timeouts with `withTimeout` and `withTimeoutOrNull`, and resource management

### [composing-suspending-functions.md](composing-suspending-functions.md)
**Purpose:** Sequential vs concurrent composition of suspending functions.  
**Contents:**
- Sequential by default execution
- Concurrent decomposition using `async`
- Lazily started `async` (`CoroutineStart.LAZY`)
- Structured concurrency with async and failure handling in coroutine hierarchies

### [coroutine-context-and-dispatchers.md](coroutine-context-and-dispatchers.md)
**Purpose:** Execution threading, dispatchers, and context composition.  
**Contents:**
- Dispatchers: `Dispatchers.Default`, `Dispatchers.IO`, `Dispatchers.Unconfined`, and new thread pools
- Jumping between threads and debugging coroutines with thread logging
- `Job` in the context, children of a coroutine, and parental responsibilities
- Combining context elements with the `+` operator and `CoroutineName`

### [flow.md](flow.md)
**Purpose:** Asynchronous cold stream processing with Flow.  
**Contents:**
- Representing multiple values: Sequences vs Suspending functions vs Flow
- Flows are cold, cancellation basics, and flow builders (`flow`, `flowOf`, `asFlow`)
- Intermediate flow operators (`map`, `filter`, `transform`, `take`)
- Terminal operators (`toList`, `first`, `reduce`, `fold`)
- Flow context, `flowOn`, buffering, conflation, and flattening operators (`flatMapConcat`, `flatMapMerge`, `flatMapLatest`)
- Exception transparency, `catch` operator, and declarative completion with `onCompletion`

### [channels.md](channels.md)
**Purpose:** Inter-coroutine communication and stream pipelines.  
**Contents:**
- Channel basics: `send` and `receive`
- Closing and iteration over channels
- Building channel pipelines and producer-consumer patterns
- Prime numbers pipeline example
- Fan-out, fan-in, buffered channels, and ticker channels

---

## 🛡️ Error Handling, State & Coordination

### [exception-handling.md](exception-handling.md)
**Purpose:** Exception propagation, supervisors, and failure handling.  
**Contents:**
- Exception propagation: `launch` vs `async`
- `CoroutineExceptionHandler` for uncaught exceptions
- Cancellation and exceptions: bidirectional vs supervisor cancellation
- `SupervisorJob` and `supervisorScope` semantics

### [shared-mutable-state-and-concurrency.md](shared-mutable-state-and-concurrency.md)
**Purpose:** Synchronization patterns for concurrent coroutines.  
**Contents:**
- The problem of shared mutable state and thread concurrency
- Volatiles, thread-safe data structures, and atomic variables
- Thread confinement (fine-grained vs coarse-grained)
- Mutual exclusion with `Mutex`
- Actor-based concurrency

### [select-expression.md](select-expression.md)
**Purpose:** Selecting between multiple suspending clauses simultaneously.  
**Contents:**
- Selecting from channels (`onReceive`, `onReceiveCatching`)
- Selecting on channel close and selecting to send (`onSend`)
- Selecting deferred values (`onAwait`)
- Switching over a channel of deferred values

---

## 🔍 Tooling, Debugging & Compatibility

### [debug-coroutines-with-idea.md](debug-coroutines-with-idea.md)
**Purpose:** Walkthrough for debugging coroutines using IntelliJ IDEA.  
**Contents:** Breakpoints in suspending code, Coroutines tab inspection, and handling optimized-out variables.

### [debug-flow-with-idea.md](debug-flow-with-idea.md)
**Purpose:** Walkthrough for debugging Kotlin Flow using IntelliJ IDEA.  
**Contents:** Setting breakpoints in emitters and collectors, concurrent flow debugging with `buffer()`.

### [debugging.md](debugging.md)
**Purpose:** Debugging facilities, debug agents, and stacktrace recovery.  
**Contents:** JVM debug mode (`-Dkotlinx.coroutines.debug`), stacktrace recovery, and debug agent integration.

### [compatibility.md](compatibility.md)
**Purpose:** API stability guarantees, experimental flags, and deprecation policies.  
**Contents:** Experimental and internal API markings, binary and source compatibility guarantees.

### [knit.properties](knit.properties)
**Purpose:** Knit tool configuration file defining package and directory mappings for guide code snippet extraction.
