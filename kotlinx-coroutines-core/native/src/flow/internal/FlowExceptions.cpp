// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/flow/internal/FlowExceptions.kt
//
// TODO: actual keyword - platform-specific implementation
// TODO: CancellationException inheritance

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

// TODO: Remove imports, fully qualify or add includes:
// import kotlinx.coroutines.*
// import kotlinx.coroutines.flow.*

// TODO: internal actual class
class AbortFlowException : public CancellationException {
public:
    void* owner;

    AbortFlowException(void* owner)
        : CancellationException("Flow was aborted, no more elements needed")
        , owner(owner)
    {
    }
};

// TODO: internal actual class
class ChildCancelledException : public CancellationException {
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
