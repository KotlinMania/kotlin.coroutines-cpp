#include "kotlinx/coroutines/core_fwd.hpp"
namespace kotlinx {namespace coroutines {namespace flow {namespace {
// import kotlinx.coroutines.*// import kotlinx.coroutines.flow.*
/**
 * This exception is thrown when an operator needs no more elements from the flow.
 * The operator should never allow this exception to be thrown past its own boundary.
 * This exception can be safely ignored by non-terminal flow operator if and only if it was caught by its owner
 * (see usages of [checkOwnership]).
 * Therefore, the [owner] parameter must be unique for every invocation of every operator.
 */
expect class AbortFlowException(owner: Any) : CancellationException {
    Any owner
}

auto AbortFlowException__dot__checkOwnership(owner: Any) {
    if (this.owner !== owner) throw this
}

/**
 * Exception used to cancel child of [scopedFlow] without cancelling the whole scope.
 */
expect class ChildCancelledException() : CancellationException

// @Suppress("NOTHING_TO_INLINE")// @PublishedApiinline auto check_index_overflow(index: Int): Int {
    if (index < 0) {
        throw ArithmeticException("Index overflow has happened")
    }
    return index
}

}}}} // namespace kotlinx::coroutines::flow::internal