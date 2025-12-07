#include <string>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, mechanical syntax mapping)
// Original: kotlinx-coroutines-core/common/src/Timeout.cpp
//
// TODO:
// - Contract annotations need C++ contracts or documentation
// - Suspend functions and coroutine infrastructure
// - Duration type (kotlin.time.Duration → std::chrono)
// - suspendCoroutineUninterceptedOrReturn
// - TimeoutCoroutine implementation
// - @Transient annotation (JVM serialization-specific)

#include <chrono>
#include <exception>
#include <functional>

namespace kotlinx {
namespace coroutines {

class CoroutineScope;
class CancellationException;
class Job;

/**
 * Runs a given suspending [block] of code inside a coroutine with a specified [timeout][timeMillis] and throws
 * a [TimeoutCancellationException] if the timeout was exceeded.
 * If the given [timeMillis] is non-positive, [TimeoutCancellationException] is thrown immediately.
 *
 * The code that is executing inside the [block] is cancelled on timeout and the active or next invocation of
 * the cancellable suspending function inside the block throws a [TimeoutCancellationException].
 *
 * The sibling function that does not throw an exception on timeout is [withTimeoutOrNull].
 * Note that the timeout action can be specified for a [select] invocation with [onTimeout][SelectBuilder.onTimeout] clause.
 *
 * **The timeout event is asynchronous with respect to the code running in the block** and may happen at any time,
 * even right before the return from inside the timeout [block]. Keep this in mind if you open or acquire some
 * resource inside the [block] that needs closing or release outside the block.
 * See the
 * [Asynchronous timeout and resources](https://kotlinlang.org/docs/reference/coroutines/cancellation-and-timeouts.html#asynchronous-timeout-and-resources)
 * section of the coroutines guide for details.
 *
 * > Implementation note: how the time is tracked exactly is an implementation detail of the context's [CoroutineDispatcher].
 *
 * @param timeMillis timeout time in milliseconds.
 */
// TODO: suspend function - coroutine semantics not implemented
template<typename T>
T with_timeout(long time_millis, std::function<T(CoroutineScope&)> block);

/**
 * Runs a given suspending [block] of code inside a coroutine with the specified [timeout] and throws
 * a [TimeoutCancellationException] if the timeout was exceeded.
 * If the given [timeout] is non-positive, [TimeoutCancellationException] is thrown immediately.
 *
 * The code that is executing inside the [block] is cancelled on timeout and the active or next invocation of
 * the cancellable suspending function inside the block throws a [TimeoutCancellationException].
 *
 * The sibling function that does not throw an exception on timeout is [withTimeoutOrNull].
 * Note that the timeout action can be specified for a [select] invocation with [onTimeout][SelectBuilder.onTimeout] clause.
 *
 * **The timeout event is asynchronous with respect to the code running in the block** and may happen at any time,
 * even right before the return from inside the timeout [block]. Keep this in mind if you open or acquire some
 * resource inside the [block] that needs closing or release outside the block.
 * See the
 * [Asynchronous timeout and resources](https://kotlinlang.org/docs/reference/coroutines/cancellation-and-timeouts.html#asynchronous-timeout-and-resources)
 * section of the coroutines guide for details.
 *
 * > Implementation note: how the time is tracked exactly is an implementation detail of the context's [CoroutineDispatcher].
 */
// TODO: suspend function - coroutine semantics not implemented
template<typename T>
T with_timeout(std::chrono::nanoseconds timeout, std::function<T(CoroutineScope&)> block);

/**
 * Runs a given suspending block of code inside a coroutine with a specified [timeout][timeMillis] and returns
 * `nullptr` if this timeout was exceeded.
 * If the given [timeMillis] is non-positive, `nullptr` is returned immediately.
 *
 * The code that is executing inside the [block] is cancelled on timeout and the active or next invocation of
 * cancellable suspending function inside the block throws a [TimeoutCancellationException].
 *
 * The sibling function that throws an exception on timeout is [withTimeout].
 * Note that the timeout action can be specified for a [select] invocation with [onTimeout][SelectBuilder.onTimeout] clause.
 *
 * **The timeout event is asynchronous with respect to the code running in the block** and may happen at any time,
 * even right before the return from inside the timeout [block]. Keep this in mind if you open or acquire some
 * resource inside the [block] that needs closing or release outside the block.
 * See the
 * [Asynchronous timeout and resources](https://kotlinlang.org/docs/reference/coroutines/cancellation-and-timeouts.html#asynchronous-timeout-and-resources)
 * section of the coroutines guide for details.
 *
 * > Implementation note: how the time is tracked exactly is an implementation detail of the context's [CoroutineDispatcher].
 *
 * @param timeMillis timeout time in milliseconds.
 */
// TODO: suspend function - coroutine semantics not implemented
template<typename T>
T* with_timeout_or_null(long time_millis, std::function<T(CoroutineScope&)> block);

/**
 * Runs a given suspending block of code inside a coroutine with the specified [timeout] and returns
 * `nullptr` if this timeout was exceeded.
 * If the given [timeout] is non-positive, `nullptr` is returned immediately.
 *
 * The code that is executing inside the [block] is cancelled on timeout and the active or next invocation of
 * cancellable suspending function inside the block throws a [TimeoutCancellationException].
 *
 * The sibling function that throws an exception on timeout is [withTimeout].
 * Note that the timeout action can be specified for a [select] invocation with [onTimeout][SelectBuilder.onTimeout] clause.
 *
 * **The timeout event is asynchronous with respect to the code running in the block** and may happen at any time,
 * even right before the return from inside the timeout [block]. Keep this in mind if you open or acquire some
 * resource inside the [block] that needs closing or release outside the block.
 * See the
 * [Asynchronous timeout and resources](https://kotlinlang.org/docs/reference/coroutines/cancellation-and-timeouts.html#asynchronous-timeout-and-resources)
 * section of the coroutines guide for details.
 *
 * > Implementation note: how the time is tracked exactly is an implementation detail of the context's [CoroutineDispatcher].
 */
// TODO: suspend function - coroutine semantics not implemented
template<typename T>
T* with_timeout_or_null(std::chrono::nanoseconds timeout, std::function<T(CoroutineScope&)> block);

/**
 * This exception is thrown by [withTimeout] to indicate timeout.
 */
class TimeoutCancellationException : CancellationException {
private:
    // @JvmField @Transient
    Job* coroutine_;

public:
    TimeoutCancellationException(const std::string& message, Job* coroutine)
        : CancellationException(message), coroutine_(coroutine) {}

    /**
     * Creates a timeout exception with the given message.
     * This constructor is needed for exception stack-traces recovery.
     */
    explicit TimeoutCancellationException(const std::string& message)
        : TimeoutCancellationException(message, nullptr) {}

    // TODO: CopyableThrowable interface
    TimeoutCancellationException* create_copy() const;

    Job* get_coroutine() const { return coroutine_; }
};

// TODO: Internal factory function for TimeoutCancellationException
// TimeoutCancellationException* make_timeout_cancellation_exception(long time, Delay* delay, Job* coroutine);

} // namespace coroutines
} // namespace kotlinx
