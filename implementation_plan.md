# Implementation Plan: Artisan Refactoring of Kotlin Coroutines C++

## Objective
Systematically eliminate technical debt by replacing stubbed or empty C++ files with robust, faithful implementations derived from the original Kotlin source code. This task emphasizes "artisan quality" — leveraging modern C++ features, ensuring full semantic equivalence, and avoiding shortcuts.

## Work Phases

### Phase 1: Core Flow Infrastructure
**Goal**: Build the foundational pipes for data flow.


### Phase 2: Complex Flow Operators
**Goal**: Enable rich composition of flows.


### Phase 3: Runtime & Dispatching
**Goal**: Ensure tasks are executed reliably and concurrently.


### Phase 4: Test Framework
**Goal**: Verify the system behavior.
- [ ] **TestBuilders** (`test/TestBuilders.hpp`)
    - Implement `runTest`.
    - Validate against `TestDispatcher`.

## Methodology
1.  **Read Native Intent**: Check Kotlin source history for exact behavior.
2.  **Transliterate Faithfully**: Map constructs one-to-one where possible, adapting for C++ memory references.
3.  **Strict Typing**: Use proper Const correctness and smart pointers.
4.  **No Stubs**: If a dependency is missing, implement the dependency first (recursively), or stub strictly as a "blocking TODO" that halts compilation if used, rather than a silent failure.
