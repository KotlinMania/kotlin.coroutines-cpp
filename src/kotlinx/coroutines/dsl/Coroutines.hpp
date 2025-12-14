#pragma once
/**
 * @file Coroutines.hpp
 * @brief Master DSL header for kotlinx.coroutines suspend functions.
 *
 * Include this single header to get all DSL wrappers for use with
 * coroutine_begin/coroutine_yield/coroutine_end macros.
 *
 * Usage:
 * ```cpp
 * #include <kotlinx/coroutines/dsl/Coroutines.hpp>
 * using namespace kotlinx::coroutines::dsl;
 *
 * class MyCoroutine : public ContinuationImpl {
 *     void* _label = nullptr;
 *
 *     void* invoke_suspend(Result<void*> result) override {
 *         coroutine_begin(this)
 *
 *         coroutine_yield(this, delay(100, completion_));
 *         coroutine_yield(this, yield(completion_));
 *         coroutine_yield(this, send(channel, value, completion_));
 *         coroutine_yield(this, receive(channel, completion_));
 *
 *         coroutine_end(this)
 *     }
 * };
 * ```
 */

// Core state machine macros
#include "kotlinx/coroutines/dsl/Suspend.hpp"

// Suspend function wrappers
#include "kotlinx/coroutines/dsl/Await.hpp"
#include "kotlinx/coroutines/dsl/Cancellable.hpp"

// Core types
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <memory>
#include <chrono>

// Forward declarations for types not yet fully ported
namespace kotlinx {
namespace coroutines {

class Job;
template <typename T> class Deferred;

namespace channels {
template <typename E> class SendChannel;
template <typename E> class ReceiveChannel;
template <typename E> class Channel;
} // namespace channels

namespace sync {
class Mutex;
class Semaphore;
} // namespace sync

} // namespace coroutines
} // namespace kotlinx

namespace kotlinx {
namespace coroutines {
namespace dsl {

// =============================================================================
// Delay functions (IMPLEMENTED)
// =============================================================================

// TODO(port): These delegate to kotlinx::coroutines::delay()
void* delay(long long time_millis, std::shared_ptr<Continuation<void*>> cont);
void* delay(std::chrono::milliseconds duration, std::shared_ptr<Continuation<void*>> cont);
void* delay(std::chrono::nanoseconds duration, std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Yield (IMPLEMENTED)
// =============================================================================

void* yield(std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Job operations (PARTIAL - join needs suspend implementation)
// =============================================================================

// TODO(port): Job::join() needs proper suspend_cancellable_coroutine implementation
void* join(Job& job, std::shared_ptr<Continuation<void*>> cont);
void* join(std::shared_ptr<Job> job, std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Deferred operations (IMPLEMENTED via Await.hpp)
// =============================================================================

// await() is already in dsl/Await.hpp

// =============================================================================
// Channel operations (TODO - needs suspend implementation)
// =============================================================================

// TODO(port): Channel send/receive need suspend_cancellable_coroutine
template <typename E>
void* send(channels::SendChannel<E>& channel, E element, std::shared_ptr<Continuation<void*>> cont);

template <typename E>
void* send(channels::Channel<E>& channel, E element, std::shared_ptr<Continuation<void*>> cont);

template <typename E>
void* receive(channels::ReceiveChannel<E>& channel, std::shared_ptr<Continuation<void*>> cont);

template <typename E>
void* receive(channels::Channel<E>& channel, std::shared_ptr<Continuation<void*>> cont);

template <typename E>
void* receive_catching(channels::ReceiveChannel<E>& channel, std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Mutex operations (TODO - needs suspend implementation)
// =============================================================================

// TODO(port): Mutex::lock() needs suspend_cancellable_coroutine
void* lock(sync::Mutex& mutex, std::shared_ptr<Continuation<void*>> cont);

// TODO(port): with_lock needs state machine: lock -> block -> unlock
template <typename Block>
void* with_lock(sync::Mutex& mutex, Block&& block, std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Semaphore operations (TODO - needs suspend implementation)
// =============================================================================

// TODO(port): Semaphore::acquire() needs suspend_cancellable_coroutine
void* acquire(sync::Semaphore& semaphore, std::shared_ptr<Continuation<void*>> cont);

// TODO(port): with_permit needs state machine: acquire -> block -> release
template <typename Block>
void* with_permit(sync::Semaphore& semaphore, Block&& block, std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Timeout operations (TODO - needs implementation)
// =============================================================================

// TODO(port): with_timeout needs Job cancellation + race
template <typename Block>
void* with_timeout(std::chrono::milliseconds timeout, Block&& block, std::shared_ptr<Continuation<void*>> cont);

template <typename Block>
void* with_timeout_or_null(std::chrono::milliseconds timeout, Block&& block, std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Select expression (TODO - complex, needs full port)
// =============================================================================

// TODO(port): select() needs full SelectBuilder implementation
// template <typename R>
// void* select(SelectBuilder<R>& builder, std::shared_ptr<Continuation<void*>> cont);

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx
