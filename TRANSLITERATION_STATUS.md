# Kotlin to C++ Transliteration Status

## Batch 5: Flow Operators/Terminal

### Completed Files (9/16)

1. **Context.cpp** ✓
   - Flow context operators
   - Functions: buffer(), conflate(), flowOn(), cancellable()
   - Classes: CancellableFlowImpl

2. **Delay.cpp** ✓
   - Delay and timing operators
   - Functions: debounce(), sample(), timeout()
   - Requires: select, Duration, TimeoutCancellationException

3. **Distinct.cpp** ✓
   - Distinct value filtering
   - Functions: distinctUntilChanged(), distinctUntilChangedBy()
   - Class: DistinctFlowImpl

4. **Limit.cpp** ✓
   - Flow limiting operators
   - Functions: drop(), dropWhile(), take(), takeWhile(), transformWhile()
   - Requires: AbortFlowException, collectWhile()

5. **terminal/Collect.cpp** ✓
   - Terminal collection operators
   - Functions: collect(), launchIn(), collectIndexed(), collectLatest(), emitAll()

6. **terminal/Collection.cpp** ✓
   - Collection conversion operators
   - Functions: toList(), toSet(), toCollection()

7. **terminal/Count.cpp** ✓
   - Counting operators
   - Functions: count()

8. **terminal/Logic.cpp** ✓
   - Logic operators
   - Functions: any(), all(), none()

9. **terminal/Reduce.cpp** ✓
   - Reduction operators
   - Functions: reduce(), fold(), single(), first(), last() and variants

### Remaining Files (7/16)

10. **Emitters.cpp** - transform(), onStart(), onCompletion(), onEmpty()
11. **Errors.cpp** - catch(), retry(), retryWhen()
12. **Lint.cpp** - Deprecation warnings and lint suppressions
13. **Merge.cpp** - flatMapConcat(), flatMapMerge(), merge(), flattenMerge()
14. **Share.cpp** - shareIn(), stateIn(), onSubscription() [LARGEST FILE]
15. **Transform.cpp** - filter(), map(), scan(), withIndex(), onEach(), chunked()
16. **Zip.cpp** - combine(), zip() operators

### Translation Notes

All completed files include:
- Header comment with original Kotlin path
- TODO list for untranslated constructs
- Preserved documentation comments
- Namespace mapping: `kotlinx::coroutines::flow`
- Template-based generic functions
- C++ naming conventions (snake_case for functions)
- Placeholder implementations noting coroutine semantics

### Key TODOs for Implementation

- Suspend functions and coroutine context
- Channel types and buffer strategies
- Flow interfaces (Flow, FlowCollector, StateFlow, SharedFlow)
- Exception types (AbortFlowException, TimeoutCancellationException)
- Utility functions (collectWhile, scopedFlow, etc.)
- Job and CoroutineScope types
- Duration type and time utilities
