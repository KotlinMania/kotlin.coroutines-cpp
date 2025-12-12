#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/StackTraceRecovery.common.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: expect functions need platform-specific implementations
// TODO: Continuation, Throwable, StackTraceElement need C++ equivalents
// TODO: CoroutineStackFrame struct needs implementation
// TODO: @PublishedApi annotation - JVM-specific
// TODO: suspend inline functions not directly translatable

#include <exception>

namespace kotlinx {
    namespace coroutines {
        namespace {
            // Forward declarations
            template<typename T>
            class Continuation;
            class StackTraceElement;
            class CoroutineStackFrame;

            /**
 * Tries to recover stacktrace for given [exception] and [continuation].
 * Stacktrace recovery tries to restore [continuation] stack frames using its debug metadata with [CoroutineStackFrame] API
 * and then reflectively instantiate exception of given type with original exception as a cause and
 * sets new stacktrace for wrapping exception.
 * Some frames may be missing due to tail-call elimination.
 *
 * Works only on JVM with enabled debug-mode.
 */
            // TODO: expect function - needs platform-specific implementation
            template<typename E>
            E *recover_stack_trace(E *exception, Continuation<void> *continuation) {
                return exception;
            }

            /**
 * initCause on JVM, nop on other platforms
 */
            // TODO: expect function - needs platform-specific implementation
            // TODO: @Suppress("EXTENSION_SHADOWED_BY_MEMBER")
            inline void init_cause(std::exception *self, std::exception *cause) {
                // No-op in common implementation
            }

            /**
 * Tries to recover stacktrace for given [exception]. Used in non-suspendable points of awaiting.
 * Stacktrace recovery tries to instantiate exception of given type with original exception as a cause.
 * Wrapping exception will have proper stacktrace as it's instantiated in the right context.
 *
 * Works only on JVM with enabled debug-mode.
 */
            // TODO: expect function - needs platform-specific implementation
            template<typename E>
            E *recover_stack_trace(E *exception) {
                return exception;
            }

            // Name conflict with recoverStackTrace
            // TODO: expect suspend inline function - not directly translatable
            // [[noreturn]] inline void recover_and_throw(std::exception* exception) {
            //     throw *exception;
            // }

            /**
 * The opposite of [recoverStackTrace].
 * It is guaranteed that `unwrap(recoverStackTrace(e)) === e`
 */
            // TODO: @PublishedApi annotation
            // TODO: expect function - needs platform-specific implementation
            template<typename E>
            E *unwrap(E *exception) {
                return exception;
            }

            // TODO: expect class - needs platform-specific implementation
            class StackTraceElement {
                // Platform-specific implementation
            };

            // TODO: expect struct - needs platform-specific implementation
            class CoroutineStackFrame {
            public:
                virtual ~CoroutineStackFrame() = default;

                virtual CoroutineStackFrame *get_caller_frame() = 0;

                virtual StackTraceElement *get_stack_trace_element() = 0;
            };
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx