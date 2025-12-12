# kotlinx.coroutines-cpp Documentation Restoration Project

## Project Overview
This is a C++ implementation of Kotlin coroutines, translating the original Kotlin/Native coroutine library to idiomatic C++. The project aims to provide production-ready coroutine support with proper LLVM integration and platform-specific optimizations.

## Current Status
- **Core Infrastructure**: ✅ Complete (66% overall completion)
- **Suspension System**: ✅ Recently fixed - eliminated exception-based hacks
- **Platform Dispatchers**: ❌ Missing (Darwin/nativeOther at 0% completion)
- **Flow/Select**: ⚠️ Partially broken (suspend semantics being restored)
- **Documentation**: ❌ Inconsistent - needs restoration and standardization

## Code Conventions & Standards

### Naming Conventions
- **Classes**: `PascalCase` (e.g., `CancellableContinuationImpl`, `CoroutineDispatcher`)
- **Methods/Functions**: `snake_case` (e.g., `try_suspend()`, `is_coroutine_suspended()`)
- **Variables**: `snake_case` (e.g., `decision_`, `state_`)
- **Constants**: `SCREAMING_SNAKE_CASE` (e.g., `COROUTINE_SUSPENDED`)
- **Namespaces**: `lowercase` with dots (e.g., `kotlinx::coroutines`)

### Documentation Standards
- **Format**: Google-style C++ docstrings
- **Placement**: Before function/class declarations in headers
- **Content**: Brief description, parameter details, return values, notes
- **References**: Update any Kotlin-style references to C++ equivalents

### Key Architecture Patterns
- **LLVM Integration**: Clang plugin generates state machines that optimize to `indirectbr`
- **Memory Management**: `std::shared_ptr` for coroutine lifecycle
- **Thread Safety**: Atomic operations for decision/state management
- **Platform Abstraction**: Dispatcher pattern for different platforms

## Critical Files Recently Modified
- `include/kotlinx/coroutines/CancellableContinuationImpl.hpp` - Fixed suspension logic
- `include/kotlinx/coroutines/Select.hpp` - Updated suspend semantics
- `include/kotlinx/coroutines/intrinsics/Intrinsics.hpp` - Core suspension markers

## Documentation Tasks
1. **Restore Missing Docstrings** - Find original Kotlin docs and translate
2. **Update Code References** - Change Kotlin naming to C++ conventions
3. **Standardize Format** - Ensure Google-style C++ docstrings
4. **Add Architecture Notes** - Document C++-specific implementation details
5. **Cross-Reference** - Link related components and patterns

## Quality Goals
- All public APIs have complete, accurate docstrings
- Implementation details explained where complex
- Performance implications documented
- Thread safety guarantees clearly stated
- Platform-specific behavior noted

## Testing Integration
- Documentation examples should be compilable
- Performance claims should be testable
- Thread safety notes should match actual behavior