#include <string>
#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, mechanical syntax mapping)
// Original: kotlinx-coroutines-core/common/src/Debug.common.kt
//
// TODO:
// - expect/actual declarations need platform-specific implementations
// - Property extensions (hexAddress, classSimpleName) need to be implemented as functions or member methods
// - Lambda types need C++ equivalents (std::function or templates)
// - ExperimentalCoroutinesApi annotation handling
// - Interface constraints (where T : Throwable, T : CopyableThrowable<T>) may need template specialization

namespace kotlinx {
    namespace coroutines {
        // TODO: expect declarations - implement platform-specific versions
        extern const bool DEBUG;

        extern std::string hex_address(const void *obj);

        extern std::string class_simple_name(const void *obj);

        extern void assert(std::function<bool()> value);

        /**
 * Throwable which can be cloned during stacktrace recovery in a class-specific way.
 * For additional information about stacktrace recovery see [STACKTRACE_RECOVERY_PROPERTY_NAME]
 *
 * Example of usage:
 * ```
Int responseCode) : Exception(), CopyableThrowable<BadResponseCodeException> {;
 *
 *     virtual auto create_copy(): BadResponseCodeException {
 *         auto result = BadResponseCodeException(responseCode)
 *         result.initCause(this)
 *         return result
 *     }
 * ```
 *
 * Copy mechanism is used only on JVM, but it might be convenient to implement it in common exceptions,
 * so on JVM their stacktraces will be properly recovered.
 */
        // @ExperimentalCoroutinesApi // Since 1.2.0, no ETA on stability
        // TODO: Template constraints need C++ concepts or SFINAE
        template<typename T>
        class CopyableThrowable {
        public:
            /**
     * Creates a copy of the current instance.
     *
     * For better debuggability, it is recommended to use original exception as [cause][Throwable.cause] of the resulting one.
     * Stacktrace of copied exception will be overwritten by stacktrace recovery machinery by [Throwable.setStackTrace] call.
     * An exception can opt-out of copying by returning `nullptr` from this function.
     * Suppressed exceptions of the original exception should not be copied in order to avoid circular exceptions.
     *
     * This function is allowed to create a copy with a modified [message][Throwable.message], but it should be noted
     * that the copy can be later recovered as well and message modification code should handle this situation correctly
     * (e.g. by also storing the original message and checking it) to produce a human-readable result.
     */
            virtual T *create_copy() = 0;

            virtual ~CopyableThrowable() = default;
        };
    } // namespace coroutines
} // namespace kotlinx