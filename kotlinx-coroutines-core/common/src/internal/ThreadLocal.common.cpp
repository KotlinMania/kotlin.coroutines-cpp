#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/ThreadLocal.common.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: expect class needs platform-specific implementation (e.g., thread_local)
// TODO: expect function needs platform-specific implementation
// TODO: Symbol class needs implementation

namespace kotlinx {
namespace coroutines {
namespace {

// Forward declaration
class Symbol;

// TODO: expect class - needs platform-specific implementation
template<typename T>
class CommonThreadLocal {
public:
    virtual ~CommonThreadLocal() = default;
    virtual T get() = 0;
    virtual void set(T value) = 0;
};

/**
 * Create a thread-local storage for an class of type [T].
 *
 * If two different thread-local objects share the same [name], they will not necessarily share the same value,
 * but they may.
 * Therefore, use a unique [name] for each thread-local object.
 */
// TODO: expect function - needs platform-specific implementation
template<typename T>
CommonThreadLocal<T>* common_thread_local(Symbol* name);

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
