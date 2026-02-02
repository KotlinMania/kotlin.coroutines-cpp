// port-lint: source internal/Scopes.kt
/**
 * @file Scopes.cpp
 * @brief Implementation of ScopeCoroutine and ContextScope.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/internal/ScopeCoroutine.hpp`.
 */

#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include "kotlinx/coroutines/Timeout.hpp"

namespace kotlinx {
namespace coroutines {

/**
 * Checks if the exception is a TimeoutCancellationException from the given coroutine.
 *
 * Kotlin equivalent (inverse logic):
 *   private fun ScopeCoroutine<*>.notOwnTimeout(cause: Throwable): Boolean =
 *       cause !is TimeoutCancellationException || cause.coroutine !== this
 *
 * @param ex the exception to check
 * @param coroutine_ptr pointer to the coroutine to compare against
 * @return true if ex IS a TimeoutCancellationException from coroutine_ptr, false otherwise
 */
bool is_own_timeout_exception(std::exception_ptr ex, const void* coroutine_ptr) {
    if (!ex) return false;

    try {
        std::rethrow_exception(ex);
    } catch (const TimeoutCancellationException& e) {
        // It's a TimeoutCancellationException - check if it's from this coroutine
        return e.coroutine.get() == coroutine_ptr;
    } catch (...) {
        // Not a TimeoutCancellationException
        return false;
    }
}

namespace internal {

// Template implementations in header

} // namespace internal
} // namespace coroutines
} // namespace kotlinx