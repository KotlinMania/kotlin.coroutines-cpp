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

#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/sync/Mutex.hpp"
#include "kotlinx/coroutines/sync/Semaphore.hpp"

namespace kotlinx {
namespace coroutines {
namespace dsl {

// =============================================================================
// Delay functions
// =============================================================================

void* delay(long long time_millis, std::shared_ptr<Continuation<void*>> cont);
void* delay(std::chrono::milliseconds duration, std::shared_ptr<Continuation<void*>> cont);
void* delay(std::chrono::nanoseconds duration, std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Yield
// =============================================================================

void* yield(std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Job operations
// =============================================================================

void* join(Job& job, std::shared_ptr<Continuation<void*>> cont);
void* join(std::shared_ptr<Job> job, std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Deferred operations (via Await.hpp)
// =============================================================================

// await() is in dsl/Await.hpp

// =============================================================================
// Channel operations (declarations only - implementation in Channel.hpp)
// =============================================================================

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
// Mutex operations
// =============================================================================

void* lock(sync::Mutex& mutex, std::shared_ptr<Continuation<void*>> cont);

template <typename Block>
void* with_lock(sync::Mutex& mutex, Block&& block, std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Semaphore operations
// =============================================================================

void* acquire(sync::Semaphore& semaphore, std::shared_ptr<Continuation<void*>> cont);

template <typename Block>
void* with_permit(sync::Semaphore& semaphore, Block&& block, std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Timeout operations (declarations only)
// =============================================================================

template <typename Block>
void* with_timeout(std::chrono::milliseconds timeout, Block&& block, std::shared_ptr<Continuation<void*>> cont);

template <typename Block>
void* with_timeout_or_null(std::chrono::milliseconds timeout, Block&& block, std::shared_ptr<Continuation<void*>> cont);

// =============================================================================
// Select expression (declarations only)
// =============================================================================

// template <typename R>
// void* select(SelectBuilder<R>& builder, std::shared_ptr<Continuation<void*>> cont);

} // namespace dsl
} // namespace coroutines
} // namespace kotlinx
