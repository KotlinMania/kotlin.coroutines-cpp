// Original Kotlin package: kotlinx.coroutines.debug
// Line-by-line C++ transliteration from Kotlin
//
// TODO: @file:Suppress - Kotlin compiler directives
// TODO: @JvmName - Kotlin JVM annotation for specifying JVM method names
// TODO: Continuation<*> - Kotlin continuation type with star projection
// TODO: internal visibility - translate to anonymous namespace or comment
// TODO: Unit return type - translate to void

// Forward declarations
namespace kotlin {
namespace coroutines {
template<typename T>
class Continuation;
}
}

namespace kotlinx {
namespace coroutines {
namespace debug {

// Empty class used to replace installed agent in the end of debug session

// TODO: @JvmName("probeCoroutineResumed")
// TODO: internal -> anonymous namespace or comment
inline void probe_coroutine_resumed_no_op(kotlin::coroutines::Continuation<void>* frame) {
    // Unit in Kotlin -> void in C++, do nothing
}

// TODO: @JvmName("probeCoroutineSuspended")
inline void probe_coroutine_suspended_no_op(kotlin::coroutines::Continuation<void>* frame) {
    // Unit in Kotlin -> void in C++, do nothing
}

// TODO: @JvmName("probeCoroutineCreated")
template<typename T>
inline kotlin::coroutines::Continuation<T>* probe_coroutine_created_no_op(
    kotlin::coroutines::Continuation<T>* completion
) {
    return completion;
}

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
