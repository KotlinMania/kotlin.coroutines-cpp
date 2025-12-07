#include "kotlinx/coroutines/core_fwd.hpp"
// Original Kotlin package: kotlinx.coroutines.debug.junit5
// Line-by-line C++ transliteration from Kotlin
//
// TODO: JUnit5 annotations: @ExtendWith, @Inherited, @MustBeDocumented, @ResourceLock, @Retention, @Target
// TODO: AnnotationRetention, AnnotationTarget - Kotlin annotation meta-annotations
// TODO: ResourceAccessMode - JUnit5 resource locking
// TODO: Timeout - JUnit5 timeout annotation
// TODO: annotation class - Kotlin annotation definition, use C++ attributes or macros

// Forward declarations
namespace kotlinx {
namespace coroutines {
namespace debug {
namespace junit5 {

class CoroutinesTimeoutExtension;

} // namespace junit5
} // namespace debug
} // namespace coroutines
} // namespace kotlinx

namespace kotlinx {
namespace coroutines {
namespace debug {
namespace junit5 {

// Coroutines timeout annotation that is similar to JUnit5's Timeout annotation. It allows running test methods in a
// separate thread, failing them after the provided time limit and interrupting the thread.
//
// Additionally, it installs DebugProbes and dumps all coroutines at the moment of the timeout. It also cancels
// coroutines on timeout if cancelOnTimeout set to `true`. The dump contains the coroutine creation stack traces.
//
// This annotation has an effect on test, test factory, test template, and lifecycle methods and test classes that are
// annotated with it.
//
// Annotating a class is the same as annotating every test, test factory, and test template method (but not lifecycle
// methods) of that class and its inner test classes, unless any of them is annotated with CoroutinesTimeout, in which
// case their annotation overrides the one on the containing class.
//
// Declaring CoroutinesTimeout on a test factory checks that it finishes in the specified time, but does not check
// whether the methods that it produces obey the timeout as well.
//
// Example usage:
// ```
// @CoroutinesTimeout(100)
// class CoroutinesTimeoutSimpleTest {
//     // does not time out, as the annotation on the method overrides the class-level one
//     @CoroutinesTimeout(1000)
//     @Test
//     auto class_timeout_is_overridden() {
//         runBlocking {
//             delay(150)
//         }
//     }
//
//     // times out in 100 ms, timeout value is taken from the class-level annotation
//     @Test
//     auto class_timeout_is_used() {
//         runBlocking {
//             delay(150)
//         }
//     }
// }
// ```
//
// @see Timeout

// TODO: @ExtendWith(CoroutinesTimeoutExtension::class)
// TODO: @Inherited
// TODO: @MustBeDocumented
// TODO: @ResourceLock("coroutines timeout", mode = ResourceAccessMode.READ)
// TODO: @Retention(value = AnnotationRetention.RUNTIME)
// TODO: @Target(AnnotationTarget.CLASS, AnnotationTarget.FUNCTION)
// TODO: annotation class - in C++ this would be attributes or macros, not directly translatable
struct CoroutinesTimeout {
    long test_timeout_ms;
    bool cancel_on_timeout = false;
};

// Note: In C++, annotations are not a language feature. This would typically be implemented
// using macros, attributes, or a testing framework's built-in mechanisms.
// The actual annotation processing logic is in CoroutinesTimeoutExtension.

} // namespace junit5
} // namespace debug
} // namespace coroutines
} // namespace kotlinx
