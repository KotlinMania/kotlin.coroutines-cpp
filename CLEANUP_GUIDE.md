# Kotlin to C++ Syntax Cleanup Guide

This directory contains tools and documentation for cleaning up the Kotlin-to-C++ transliteration.

## Current Status

See [SYNTAX_CLEANUP_STATUS.md](SYNTAX_CLEANUP_STATUS.md) for detailed progress tracking.

- **Total files**: 630+ .cpp files across the project
- **Core files**: 111 .cpp files in kotlinx-coroutines-core/common/src
- **Fixed so far**: ~30 files cleaned

## Common Fixes Needed

### 1. Missing `public` Keyword in Inheritance

**Problem**: C++ defaults to private inheritance for classes
```cpp
class Foo : Bar { ... }  // WRONG - private inheritance
```

**Fix**:
```cpp
class Foo : public Bar { ... }  // CORRECT - public inheritance
```

**Tool**: Use `fix_inheritance.sh` script
```bash
./fix_inheritance.sh path/to/file.cpp
# Or batch process:
find kotlinx-coroutines-core/common/src -name "*.cpp" -exec ./fix_inheritance.sh {} \;
```

### 2. Kotlin Function Syntax

**Problem**: `fun` keyword not valid in C++
```kotlin
fun myFunction(): Int { return 42; }
override fun invoke(cause: Throwable?) { ... }
suspend fun collect(collector: FlowCollector<T>) { ... }
```

**Fix**:
```cpp
int myFunction() { return 42; }
void invoke(Throwable* cause) override { ... }
void collect(FlowCollector<T>* collector) { /* TODO: suspend */ }
```

### 3. Kotlin Parameter Syntax

**Problem**: Kotlin uses `name: Type`, C++ uses `Type name`
```kotlin
fun foo(param: Int, other: String): Boolean
```

**Fix**:
```cpp
bool foo(int param, std::string other)
```

### 4. For Loop Syntax

**Problem**: Kotlin for loops are different
```kotlin
for (collection value) { ... }
for (range value) { ... }
```

**Fix**:
```cpp
for (auto value : collection) { ... }
for (int value = range.start; value <= range.end; value += range.step) { ... }
```

### 5. Elvis Operator `?:`

**Problem**: Not valid C++ syntax
```kotlin
val x = y ?: defaultValue
```

**Fix**:
```cpp
auto x = y ? y : defaultValue;
// Or with nullability:
auto x = (y != nullptr) ? y : defaultValue;
```

### 6. Safe Call Operator `?.`

**Problem**: Not valid C++ syntax
```kotlin
obj?.method()
```

**Fix**:
```cpp
if (obj != nullptr) {
    obj->method();
}
// Or:
if (obj) obj->method();
```

### 7. When Expressions

**Problem**: Kotlin `when` is not C++ syntax
```kotlin
when (x) {
    1 -> doSomething()
    2 -> doOtherThing()
    else -> doDefault()
}
```

**Fix**:
```cpp
switch (x) {
    case 1:
        doSomething();
        break;
    case 2:
        doOtherThing();
        break;
    default:
        doDefault();
        break;
}
// Or for complex conditions, use if-else
```

## Batch Processing Tools

### Find Files Needing Fixes

```bash
# Find files with missing public inheritance
grep -r "class [A-Za-z_]* : [A-Z]" --include="*.cpp" kotlinx-coroutines-core/common/src | grep -v "public"

# Find files with Kotlin fun syntax
grep -r "^\s*fun " --include="*.cpp" kotlinx-coroutines-core/

# Find files with when expressions
grep -r "when (" --include="*.cpp" kotlinx-coroutines-core/

# Find files with Elvis operator (excluding comments)
grep -r "[^:]?:[^:]" --include="*.cpp" kotlinx-coroutines-core/ | grep -v "//"

# Find files with safe call operator
grep -r "\?\." --include="*.cpp" kotlinx-coroutines-core/ | grep -v "//"
```

### Scripts Available

- `fix_inheritance.sh` - Automatically fixes missing `public` keywords
- More scripts to be added as patterns are identified

## Manual Fix Workflow

1. Open file in IDE
2. Check for compilation errors
3. Apply fixes based on the patterns above
4. Update SYNTAX_CLEANUP_STATUS.md
5. Commit changes

## Priority Order

1. **Core coroutine files** (Job, Continuation, Dispatcher)
   - These are the foundation and used everywhere
   
2. **Flow implementation files**
   - Second most important, heavily used
   
3. **Channel implementation files**
   - Core communication primitive
   
4. **Test files**
   - Lower priority, but important for validation
   
5. **Build configuration files**
   - Need conversion from Gradle to CMake

## Reference

- Original Kotlin source: commit `dc708a1d1696ec1b3389063f4925e9061dfc0932`
- Documentation: kotlinx.coroutines official docs

## Notes

- Many template parameter errors shown by IDE are false positives
- Focus on actual syntax errors that would prevent compilation
- Comment blocks with code examples may show errors but are harmless
- Some Kotlin syntax in comments is intentional (documentation)

