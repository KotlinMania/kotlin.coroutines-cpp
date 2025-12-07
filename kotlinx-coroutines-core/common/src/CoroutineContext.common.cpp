// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/CoroutineContext.common.kt
//
// TODO: expect functions - platform-specific implementations needed
// TODO: @InternalCoroutinesApi, @PublishedApi - no C++ equivalents

namespace kotlinx {
namespace coroutines {

// TODO: import kotlin.coroutines.* - use custom coroutine types

/**
 * Creates a context for a new coroutine. It installs [Dispatchers.Default] when no other dispatcher or
 * [ContinuationInterceptor] is specified and adds optional support for debugging facilities (when turned on)
 * and copyable-thread-local facilities on JVM.
 */
// TODO: expect fun - platform-specific implementation needed
// TODO: Extension on CoroutineScope
CoroutineContext newCoroutineContext(CoroutineScope* scope, CoroutineContext context); // Declaration only

/**
 * Creates a context for coroutine builder functions that do not launch a new coroutine, e.g. [withContext].
 * @suppress
 */
// TODO: @InternalCoroutinesApi - no C++ equivalent
// TODO: expect fun - platform-specific implementation needed
// TODO: Extension on CoroutineContext
CoroutineContext newCoroutineContext(CoroutineContext base_context, CoroutineContext added_context); // Declaration only

// TODO: @PublishedApi - no C++ equivalent (for inline functions)
// TODO: @Suppress("PropertyName") - no C++ equivalent
// TODO: internal expect val - platform-specific, internal visibility
// TODO: DefaultDelay needs platform-specific implementation
// Delay* DefaultDelay; // Declaration only

// countOrElement -- pre-cached value for ThreadContext.kt
// TODO: internal expect inline fun - platform-specific inline function
// TODO: Template parameter T
template<typename T>
T withCoroutineContext(CoroutineContext context, void* count_or_element, std::function<T()> block); // Declaration only

// TODO: internal expect inline fun - platform-specific inline function
template<typename T>
T withContinuationContext(Continuation<void>* continuation, void* count_or_element, std::function<T()> block); // Declaration only

// TODO: internal expect fun - platform-specific
// TODO: Extension on Continuation<*>
std::string toDebugString(Continuation<void>* continuation); // Declaration only

// TODO: internal expect val - platform-specific extension property
// TODO: Extension on CoroutineContext
// std::string* coroutineName(CoroutineContext context); // Returns nullable string pointer

} // namespace coroutines
} // namespace kotlinx
