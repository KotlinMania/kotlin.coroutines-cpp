---
applyTo: "kotlinx-coroutines-core/**/src/**/*.cpp"
---

# Custom Instructions for Implementation Files

When implementing state machines and logic in `.cpp` files:

1. **Find the Kotlin source** - Every `.cpp` has a corresponding `.kt` in `tmp/kotlinx.coroutines/`. Use it as the algorithmic reference.

2. **Use concrete types** - Avoid templates in `.cpp` unless required by the header ABI. Prefer explicit specializations or type erasure via `void*`.

3. **Suspend mechanics (current direction)**:
   - Implement suspend points using the Continuation ABI and helpers.
   - Call `suspend_cancellable_coroutine<T>(...)` where applicable and check `is_coroutine_suspended(result)`.
   - Return `intrinsics::COROUTINE_SUSPENDED` to signal suspension from suspend entry points.
   - The legacy `SUSPEND_*` macro DSL is still present in some files for backward compatibility; avoid introducing new usages and add a deprecation note if you must touch them.
   - We are migrating to a Clang plugin that rewrites a minimal suspend DSL into state machines; track migration notes in audit docs / PR text (not source comments). See `docs/cpp_port/docking_ring.md`.

   Example pattern:
   ```cpp
   void* foo(Args..., Continuation<void*>* cont) {
       void* r = suspend_cancellable_coroutine<int>([](auto& c){ /* ... */ }, cont);
       if (is_coroutine_suspended(r)) return intrinsics::get_COROUTINE_SUSPENDED();
       return r; // boxed result pointer or nullptr for Unit
   }
   ```

4. **Continuation ownership**:
   - Store continuation as `std::shared_ptr<Continuation<void*>>`
   - Use `dynamic_cast` or `static_cast` as needed (track ownership gaps in audit docs / PR text)
   - Release intercepted continuations via `release_intercepted()`

5. **Exception handling**:
   - Use `std::exception_ptr` for error boxing
   - Map Kotlin `Throwable` to C++ exceptions
   - Propagate via `Result<void*>::failure(...)`

6. **No TODOs in source**:
   - Do not add `TODO` / `FIXME` / `XXX` / `HACK` comments.
   - If there is a gap, either port the missing piece or document the blocker in audit docs / PR text.

7. **Testing**:
   - After implementing, ensure `./test_suspend` passes
   - Compile without real errors: `g++ -std=c++17 -Wall -Wextra -I include your_file.cpp`
