/**
 * @file Timeout.hpp
 * @brief Timeout functions: with_timeout and with_timeout_or_null
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Timeout.kt
 *
 * Provides timeout-based execution control for coroutines. The key functions are:
 *
 * - with_timeout: Runs a block with a timeout, throws TimeoutCancellationException if exceeded
 * - with_timeout_or_null: Runs a block with a timeout, returns nullptr if exceeded
 *
 * **The timeout event is asynchronous with respect to the code running in the block** and may happen
 * at any time, even right before the return from inside the timeout block. Keep this in mind if you
 * open or acquire some resource inside the block that needs closing or release outside the block.
 *
 * See the Asynchronous timeout and resources section of the coroutines guide for details:
 * https://kotlinlang.org/docs/reference/coroutines/cancellation-and-timeouts.html#asynchronous-timeout-and-resources
 *
 * Implementation note: how the time is tracked exactly is an implementation detail of the context's
 * CoroutineDispatcher.
 */
#ifndef KOTLINX_COROUTINES_TIMEOUT_HPP
#define KOTLINX_COROUTINES_TIMEOUT_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"

namespace kotlinx {
namespace coroutines {

    // Forward declaration of Delay is sufficient as we use pointers/interface
    class Delay;

    /**
     * This exception is thrown by with_timeout to indicate timeout.
     *
     * Transliterated from:
     * public class TimeoutCancellationException internal constructor(message: String, coroutine: Job?)
     *     : CancellationException(message), CopyableThrowable<TimeoutCancellationException>
     */
    class TimeoutCancellationException : public CancellationException {
    public:
        // Internal constructor logic matching Kotlin
        TimeoutCancellationException(const std::string& message, std::shared_ptr<Job> coroutine);
        
        // Constructor for stack trace recovery
        explicit TimeoutCancellationException(const std::string& message);

        std::exception_ptr create_copy() const;

        std::shared_ptr<Job> coroutine;
    };

    // Helper factory function
    TimeoutCancellationException make_timeout_cancellation_exception(
        long time,
        Delay* delay,
        std::shared_ptr<Job> coroutine
    );

    namespace detail {
        
        template <typename U, typename T>
        class TimeoutCoroutine : public internal::ScopeCoroutine<T>, public Runnable {
        public:
            long time;

            TimeoutCoroutine(long time, std::shared_ptr<Continuation<T>> uCont)
                : internal::ScopeCoroutine<T>(uCont->get_context(), uCont), time(time) {}

            void run() override {
                // context.delay logic
                Delay* delay = nullptr;
                if (auto ctx = this->get_context()) {
                     // We can't easily access get_delay helper here without moving it up or making it friendly
                     // For now, we rely on cached delay or look it up again
                     auto element = ctx->get(ContinuationInterceptor::type_key);
                     if (element) delay = dynamic_cast<Delay*>(element.get());
                }
                if (!delay) delay = &get_default_delay();

                auto exception = make_timeout_cancellation_exception(time, delay, std::dynamic_pointer_cast<Job>(this->JobSupport::shared_from_this()));
                this->cancel(std::make_exception_ptr(exception));
            }

            std::string name_string() const override {
                 return internal::ScopeCoroutine<T>::name_string() + "(timeMillis=" + std::to_string(time) + ")";
            }
        };

        // Helper to get Delay from context
        inline Delay* get_delay(const CoroutineContext& context) {
            auto element = context.get(ContinuationInterceptor::type_key);
            if (element) {
                 if (auto* delay = dynamic_cast<Delay*>(element.get())) {
                     return delay;
                 }
            }
            return &get_default_delay();
        }

        template <typename U, typename T>
        void* setup_timeout(std::shared_ptr<TimeoutCoroutine<U, T>> coroutine, std::function<T(CoroutineScope&)> block) {
            auto cont = coroutine->u_cont;
            auto context = cont->get_context();
            
            Delay* delay = get_delay(*context);
            
            // coroutine.disposeOnCompletion(context.delay.invokeOnTimeout(coroutine.time, coroutine, coroutine.context))
            // Cast coroutine (shared_ptr<TimeoutCoroutine>) to shared_ptr<Runnable>
            auto runnable = std::static_pointer_cast<Runnable>(coroutine);
            auto handle = delay->invoke_on_timeout(coroutine->time, runnable, *coroutine->get_context());
            coroutine->dispose_on_completion(handle);

            // restart the block using a new coroutine with a new job,
            // however, start it undispatched, because we already are in the proper context
            return coroutine->start_undispatched_or_return_ignore_timeout(coroutine, block);
        }

    } // namespace detail

    /**
     * Runs a given suspending block of code inside a coroutine with the specified timeout and throws
     * a TimeoutCancellationException if the timeout was exceeded.
     * If the given time_millis is non-positive, TimeoutCancellationException is thrown immediately.
     *
     * The code that is executing inside the block is cancelled on timeout and the active or next invocation of
     * the cancellable suspending function inside the block throws a TimeoutCancellationException.
     *
     * The sibling function that does not throw an exception on timeout is with_timeout_or_null.
     * Note that the timeout action can be specified for a select invocation with SelectBuilder::on_timeout clause.
     *
     * **The timeout event is asynchronous with respect to the code running in the block** and may happen at any time,
     * even right before the return from inside the timeout block. Keep this in mind if you open or acquire some
     * resource inside the block that needs closing or release outside the block.
     *
     * @param time_millis timeout time in milliseconds.
     * @param block the coroutine code to execute with timeout.
     * @param completion the continuation for resume after block completes or times out.
     * @return the result of the block or COROUTINE_SUSPENDED.
     * @throws TimeoutCancellationException if the timeout was exceeded.
     *
     * Transliterated from:
     * public suspend fun <T> withTimeout(timeMillis: Long, block: suspend CoroutineScope.() -> T): T
     */
    template <typename T>
    void* with_timeout(long time_millis, std::function<T(CoroutineScope&)> block, std::shared_ptr<Continuation<void*>> completion) {
        if (time_millis <= 0) {
            throw TimeoutCancellationException("Timed out immediately");
        }

        // Cast completion to Continuation<T>.
        // In strict ABI, completion is Continuation<void*>. We assume T is layout-compatible or erased.
        auto uCont = std::reinterpret_pointer_cast<Continuation<T>>(completion);

        auto coroutine = std::make_shared<detail::TimeoutCoroutine<T, T>>(time_millis, uCont);
        return detail::setup_timeout(coroutine, block);
    }

    template <typename T>
    void* with_timeout(std::chrono::nanoseconds timeout, std::function<T(CoroutineScope&)> block, std::shared_ptr<Continuation<void*>> completion) {
         return with_timeout(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(), block, completion);
    }
    
    /**
     * Runs a given suspending block of code inside a coroutine with a specified timeout and returns
     * nullptr if this timeout was exceeded.
     * If the given time_millis is non-positive, nullptr is returned immediately.
     *
     * The code that is executing inside the block is cancelled on timeout and the active or next invocation of
     * cancellable suspending function inside the block throws a TimeoutCancellationException.
     *
     * The sibling function that throws an exception on timeout is with_timeout.
     * Note that the timeout action can be specified for a select invocation with SelectBuilder::on_timeout clause.
     *
     * **The timeout event is asynchronous with respect to the code running in the block** and may happen at any time,
     * even right before the return from inside the timeout block. Keep this in mind if you open or acquire some
     * resource inside the block that needs closing or release outside the block.
     *
     * @param time_millis timeout time in milliseconds.
     * @param block the coroutine code to execute with timeout.
     * @param completion the continuation for resume after block completes or times out.
     * @return the result of the block, nullptr if timeout occurred, or COROUTINE_SUSPENDED.
     *
     * Transliterated from:
     * public suspend fun <T> withTimeoutOrNull(timeMillis: Long, block: suspend CoroutineScope.() -> T): T?
     */
    template <typename T>
    void* with_timeout_or_null(long time_millis, std::function<T(CoroutineScope&)> block, std::shared_ptr<Continuation<void*>> completion) {
        if (time_millis <= 0) return nullptr;

        auto uCont = std::reinterpret_pointer_cast<Continuation<T*>>(completion);
        std::shared_ptr<detail::TimeoutCoroutine<T*, T*>> coroutine = nullptr;
        
        try {
            auto timeoutCoroutine = std::make_shared<detail::TimeoutCoroutine<T*, T*>>(time_millis, uCont);
            coroutine = timeoutCoroutine;
            return detail::setup_timeout(timeoutCoroutine, block);
        } catch (const TimeoutCancellationException& e) {
            if (e.coroutine == coroutine) {
                return nullptr;
            }
            throw;
        }
    }

    template <typename T>
    void* with_timeout_or_null(std::chrono::nanoseconds timeout, std::function<T(CoroutineScope&)> block, std::shared_ptr<Continuation<void*>> completion) {
        return with_timeout_or_null(std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(), block, completion);
    }

} // namespace coroutines
} // namespace kotlinx

#endif // KOTLINX_COROUTINES_TIMEOUT_HPP
