#pragma once
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <exception>

namespace kotlinx {
namespace coroutines {

/**
 * An optional element in the coroutine context to handle uncaught exceptions.
 * Only one handler can be present in the context.
 */
class CoroutineExceptionHandler : public virtual CoroutineContext::Element {
public:
    // Key for CoroutineExceptionHandler in CoroutineContext
    // Key for CoroutineExceptionHandler in CoroutineContext
    struct KeyType : CoroutineContext::Key {};
    static inline KeyType key_instance;
    static constexpr CoroutineContext::Key* typeKey = &key_instance;

    CoroutineContext::Key* key() const override { return typeKey; }

    virtual ~CoroutineExceptionHandler() = default;

    /**
     * Handles uncaught [exception] in the given [context].
     * It is invoked if coroutine does not handle exception itself.
     */
    virtual void handle_exception(CoroutineContext& context, std::exception_ptr exception) = 0;
};

// Global helper for handling uncaught exceptions in coroutines
void handle_coroutine_exception(CoroutineContext& context, std::exception_ptr exception);

} // namespace coroutines
} // namespace kotlinx
