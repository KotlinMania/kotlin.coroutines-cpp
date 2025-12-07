#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/ThreadContext.common.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: expect function needs platform-specific implementation
// TODO: CoroutineContext needs C++ equivalent

namespace kotlinx {
namespace coroutines {
namespace {

// Forward declaration
class CoroutineContext;

// TODO: expect function - needs platform-specific implementation
void* thread_context_elements(CoroutineContext* context);

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
