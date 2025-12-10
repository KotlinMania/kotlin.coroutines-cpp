#pragma once
#include <functional>
#include <exception>

namespace kotlinx {
namespace coroutines {

/**
 * Handler for [Job.invokeOnCompletion] and [CancellableContinuation.invokeOnCancellation].
 */
using CompletionHandler = std::function<void(std::exception_ptr)>;

} // namespace coroutines
} // namespace kotlinx
