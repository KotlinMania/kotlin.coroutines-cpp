#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt
//
// TODO: actual keyword - platform-specific implementation
// TODO: CancellationException inheritance

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace {

// TODO: Remove imports, fully qualify or add includes:
// import kotlinx.coroutines.*
// import kotlinx.coroutines.flow.*

// TODO: actual class
class AbortFlowException : CancellationException {
public:
    void* owner;

    AbortFlowException(void* owner)
        : CancellationException("Flow was aborted, no more elements needed")
        , owner(owner)
    {
    }
};

// TODO: actual class
class ChildCancelledException : CancellationException {
public:
    ChildCancelledException()
        : CancellationException("Child of the scoped flow was cancelled")
    {
    }
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
