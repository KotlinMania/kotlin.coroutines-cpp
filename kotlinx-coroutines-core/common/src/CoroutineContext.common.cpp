// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/CoroutineContext.common.kt
//
// TODO: expect functions - platform-specific implementations needed
// TODO: @InternalCoroutinesApi, @PublishedApi - no C++ equivalents

#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include <string>
#include <functional>
#include "kotlinx/coroutines/context_impl.hpp"

namespace kotlinx {
namespace coroutines {

std::shared_ptr<CoroutineContext> CoroutineContext::operator+(std::shared_ptr<CoroutineContext> other) const {
    if (!other) return std::const_pointer_cast<CoroutineContext>(shared_from_this());
    
    return other->fold<std::shared_ptr<CoroutineContext>>(
        std::const_pointer_cast<CoroutineContext>(shared_from_this()), 
        [](std::shared_ptr<CoroutineContext> acc, std::shared_ptr<Element> element) {
            auto removed = acc->minus_key(element->key());
            if (!removed) {
                return std::static_pointer_cast<CoroutineContext>(element); 
            }
            return std::static_pointer_cast<CoroutineContext>(std::make_shared<CombinedContext>(removed, element));
        }
    );
}

// TODO: import kotlin.coroutines.* - use custom coroutine types

/**
 * Creates a context for a new coroutine. It installs [Dispatchers.Default] when no other dispatcher or
 * [ContinuationInterceptor] is specified and adds optional support for debugging facilities (when turned on)
 * and copyable-thread-local facilities on JVM.
 */
// TODO: expect fun - platform-specific implementation needed
// TODO: Extension on CoroutineScope
CoroutineContext new_coroutine_context(CoroutineScope* scope, CoroutineContext context); // Declaration only

/**
 * Creates a context for coroutine builder functions that do not launch a new coroutine, e.g. [withContext].
 * @suppress
 */
// TODO: @InternalCoroutinesApi - no C++ equivalent
// TODO: expect fun - platform-specific implementation needed
// TODO: Extension on CoroutineContext
CoroutineContext new_coroutine_context(CoroutineContext base_context, CoroutineContext added_context); // Declaration only

// TODO: @PublishedApi - no C++ equivalent (for inline functions)
// TODO: @Suppress("PropertyName") - no C++ equivalent
// TODO: expect auto - platform-specific, visibility
// TODO: DefaultDelay needs platform-specific implementation
// Delay* DefaultDelay; // Declaration only

// countOrElement -- pre-cached value for ThreadContext.kt
// TODO: expect inline fun - platform-specific inline function
// TODO: Template parameter T
template<typename T>
T with_coroutine_context(CoroutineContext context, void* count_or_element, std::function<T()> block); // Declaration only

// TODO: expect inline fun - platform-specific inline function
template<typename T>
T with_continuation_context(Continuation<void>* continuation, void* count_or_element, std::function<T()> block); // Declaration only

// TODO: expect fun - platform-specific
// TODO: Extension on Continuation<*>
std::string to_debug_string(Continuation<void>* continuation); // Declaration only

// TODO: expect auto - platform-specific extension property
// TODO: Extension on CoroutineContext
// std::string* coroutineName(CoroutineContext context); // Returns nullable string pointer

} // namespace coroutines
} // namespace kotlinx
