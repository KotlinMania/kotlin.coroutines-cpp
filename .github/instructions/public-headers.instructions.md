---
applyTo: "include/kotlinx/coroutines/**/*.hpp"
---

# Custom Instructions for Public Headers

When modifying or creating public API headers in `include/kotlinx/coroutines/**/*.hpp`:

## What Belongs in .hpp (Public Headers)

**Only include**:
- Abstract base classes and pure virtual interfaces
- Public class/struct declarations (types visible to users)
- Public method declarations (signatures only)
- Public constants and enums
- Forward declarations for public types
- Public inline helper functions (only if critical for ABI or zero-cost abstractions)
- Template definitions for public APIs (minimal; prefer concrete specializations in .cpp)
- Documentation and doxygen comments

**Do NOT include**:
- Implementations of non-inline methods (move to .cpp)
- Private helper classes
- Private member functions or variables
- Large algorithmic code
- Dependency includes (except those needed for public API)

## What Belongs in .cpp (Implementations)

**Move to corresponding .cpp file**:
- All non-inline method implementations
- Private helper classes and functions
- Static helper functions
- Concrete template specializations
- Heavy includes (keep .hpp clean)
- Complex algorithmic code
- Implementation-specific exceptions and errors

## Rules for Public APIs

1. **Match Kotlin API exactly** - Find the corresponding `.kt` file in `tmp/kotlinx.coroutines/` and mirror its public interface.

2. **Method naming** - Convert Kotlin `camelCase` to C++ `snake_case`:
   - `isActive()` → `is_active()`
   - `resumeWith(result)` → `resume_with(result)`
   - `getContext()` → `get_context()`

3. **Suspend function signatures (Continuation ABI)**:
   - Must return `void*` (type-erased result pointer or `COROUTINE_SUSPENDED` sentinel)
   - Must accept `Continuation<void*>* cont` as the last parameter
   - Declare signatures in .hpp; implement details in .cpp using the Continuation ABI helpers
   - Migration: annotate implementations that will move to the Clang suspend plugin with `// TODO(suspend-plugin): migrate` (see `docs/cpp_port/docking_ring.md`)
   - Example in .hpp: `virtual void* await(Continuation<void*>* cont) = 0;`

4. **Keep headers slim**:
   - Headers should be readable in under 1 minute
   - Complex logic goes to .cpp
   - Users should see only the contract, not the mechanism

5. **Documentation**:
   - Convert Kotlin KDoc to C++ doxygen comments
   - Include reference to original Kotlin source file path at top
   - Mark unresolved semantic gaps with tagged TODOs
   - Do NOT document private/implementation details in .hpp

6. **Update audit** - After changes, update `docs/API_AUDIT.md` and `docs/audits/*.md` with file:line references.

## Example Structure

**Good .hpp**:
```cpp
// Original: tmp/kotlinx.coroutines/Job.kt

namespace kotlinx::coroutines {

class Job {
public:
    virtual ~Job() = default;
    
    virtual bool is_active() const = 0;
    virtual void* join(Continuation<void*>* cont) = 0;
    virtual bool cancel() = 0;
    
    // Suspend function - see Job.cpp for implementation
    virtual void* await(Continuation<void*>* cont) = 0;
};

} // namespace
```

**Implementation in .cpp**:
```cpp
// Use Continuation ABI helpers. Legacy SUSPEND_* macros are deprecated.
void* JobImpl::await(Continuation<void*>* cont) {
    void* r = suspend_cancellable_coroutine<void>([](auto& c){ /* schedule resume */ }, cont);
    if (is_coroutine_suspended(r)) return intrinsics::get_COROUTINE_SUSPENDED();
    return r; // nullptr for Unit
}
```

## Headers to Review as Examples

- `include/kotlinx/coroutines/Continuation.hpp` - Clean interface
- `include/kotlinx/coroutines/Job.hpp` - Shows suspend function patterns
- `include/kotlinx/coroutines/CoroutineDispatcher.hpp` - Scheduler interface

