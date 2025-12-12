#include "kotlinx/coroutines/internal/ThreadContext.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace internal {
            Symbol NO_THREAD_ELEMENTS("NO_THREAD_ELEMENTS");

            // TODO: full implementation of thread context switching.
            // Currently stubs that do nothing, as ThreadContextElement logic needs full porting.

            void *update_thread_context(const CoroutineContext &context, void *count_or_element) {
                if (count_or_element == &NO_THREAD_ELEMENTS) return &NO_THREAD_ELEMENTS;
                // Real implementation requires iterating context elements and calling updateThreadContext on them.
                return &NO_THREAD_ELEMENTS;
            }

            void restore_thread_context(const CoroutineContext &context, void *old_state) {
                if (old_state == &NO_THREAD_ELEMENTS) return;
                // Real implementation requires restoring.
            }

            void *thread_context_elements(const CoroutineContext &context) {
                // Should fold context to count ThreadContextElements
                return &NO_THREAD_ELEMENTS;
            }
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx