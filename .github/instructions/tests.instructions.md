---
applyTo: "test_*.cpp,**/test/**/*.cpp,test_*.hpp,**/test/**/*.hpp"
---

# Custom Instructions for Test Files

When working with test files:

1. **Public vs Private Split** - Follow the same rules as production code:
   - **Headers (.hpp)**: Public test interfaces, test base classes, helper templates
   - **Source (.cpp)**: Test implementations, private test fixtures, internal test utilities
   - **Example**: Test helper classes that other tests use → .hpp; test implementations → .cpp

2. **Suspend mechanics (current direction)**
   - Prefer exercising the Continuation ABI and helpers over the legacy macro DSL.
   - Use `suspend_cancellable_coroutine<T>(...)` and check `intrinsics::COROUTINE_SUSPENDED` with `is_coroutine_suspended(result)`.
   - Legacy `SUSPEND_*` macros remain in some tests for backward compatibility; avoid introducing new usages and add a deprecation note if you must.
   - See `docs/cpp_port/docking_ring.md` for the Clang suspend plugin plan that will back tests going forward.

   Example pattern (non-void suspend):
   ```cpp
   void* r = suspend_cancellable_coroutine<int>([](auto& cont){
       // Either resume now or later
       // cont.resume(42);
   }, continuation);
   if (is_coroutine_suspended(r)) {
       // Will resume later
   }
   ```

3. **All tests must compile and run**:
   ```bash
   g++ -std=c++17 -I include test_*.cpp -o test
   ./test  # Must show "All tests passed"
   ```

4. **IDE false positives are expected**:
   - Suspend constructs (legacy macros or upcoming plugin annotations) can confuse IntelliJ's parser
   - See `docs/cpp_port/docking_ring.md` notes on IDE support
   - Add `// NOLINT` comments to suppress spurious errors
   - Trust the compiler, not the IDE

5. **Test structure**:
   - Each test function validates one feature (suspend, chaining, etc.)
   - Include both synchronous and asynchronous paths
   - Verify cancellation behavior where applicable

6. **Never break existing tests** - Before submitting changes:
   ```bash
   ./test_suspend && echo "✅ All tests pass"
   ```

