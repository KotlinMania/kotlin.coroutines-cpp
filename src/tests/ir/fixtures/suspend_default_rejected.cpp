// NOTE(port): Match Kotlin's rejection of suspension in a default parameter.
#include <functional>
[[suspend]] void* source();
[[suspend]] void* invalid(void* value = source()) { return value; }
struct InvalidConstructor {
    explicit InvalidConstructor(void* value = source()) {}
};
void invalid_capture(std::function<void*()> callable = [value = source()] { return value; });
// NOTE(port): The compiler's explicit inline IR flag permits the scope walk
// to reach the outer suspend function's default parameter.
[[suspend]] void* invalid_inline(void* value = [] [[clang::annotate("kotlin.ir.Inline")]] {
    return source();
}()) { return value; }
