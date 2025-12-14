#pragma once
// @file:Suppress("PrivatePropertyName")
//
// Kotlin source: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt
// Lines: 1-3116
//
// Kotlin imports:
// - kotlinx.atomicfu.*
// - kotlinx.coroutines.*
// - kotlinx.coroutines.channels.ChannelResult.Companion.{closed, failure, success}
// - kotlinx.coroutines.internal.*
// - kotlinx.coroutines.selects.*
// - kotlinx.coroutines.selects.TrySelectDetailedResult.*
// - kotlin.coroutines.*
// - kotlin.js.*
// - kotlin.jvm.*
// - kotlin.math.*
// - kotlin.reflect.*

#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/ConcurrentLinkedList.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include <atomic>
#include <memory>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cassert>
#include <cstdint>

namespace kotlinx {
namespace coroutines {
namespace channels {

// Forward declarations
template <typename E> class BufferedChannel;
template <typename E> class ChannelSegment;

// ============================================================================
// Lines 2962-2973: Buffer end constants
// ============================================================================

// If the channel is rendezvous or unlimited, the `bufferEnd` counter
// should be initialized with the corresponding value below and never change.
// In this case, the `expandBuffer(..)` operation does nothing.
constexpr int64_t BUFFER_END_RENDEZVOUS = 0L; // no buffer
constexpr int64_t BUFFER_END_UNLIMITED = INT64_MAX; // infinite buffer

inline int64_t initial_buffer_end(int capacity) {
    // Line 2969-2973
    if (capacity == CHANNEL_RENDEZVOUS) return BUFFER_END_RENDEZVOUS;
    if (capacity == CHANNEL_UNLIMITED) return BUFFER_END_UNLIMITED;
    return static_cast<int64_t>(capacity);
}

// ============================================================================
// Lines 2934-2945: Segment size and wait iterations
// ============================================================================

// Line 2937-2938: Number of cells in each segment.
// @JvmField
// internal val SEGMENT_SIZE = systemProp("kotlinx.coroutines.bufferedChannel.segmentSize", 32)
constexpr int SEGMENT_SIZE = 32;

// Line 2940-2945: Number of iterations to wait in waitExpandBufferCompletion
// until the numbers of started and completed expandBuffer calls coincide.
constexpr int EXPAND_BUFFER_COMPLETION_WAIT_ITERATIONS = 10000;

// ============================================================================
// Lines 2975-3009: Cell state symbols
// ============================================================================

// Line 2980-2982: The cell stores a buffered element.
// @JvmField internal val BUFFERED = Symbol("BUFFERED")
inline internal::Symbol& BUFFERED() {
    static internal::Symbol instance("BUFFERED");
    return instance;
}

// Line 2983-2985: Concurrent expandBuffer() can inform the upcoming sender
// that it should buffer the element.
// private val IN_BUFFER = Symbol("SHOULD_BUFFER")
inline internal::Symbol& IN_BUFFER() {
    static internal::Symbol instance("SHOULD_BUFFER");
    return instance;
}

// Line 2986-2991: Indicates that a receiver (RCV suffix) is resuming the suspended sender
// private val RESUMING_BY_RCV = Symbol("S_RESUMING_BY_RCV")
inline internal::Symbol& RESUMING_BY_RCV() {
    static internal::Symbol instance("S_RESUMING_BY_RCV");
    return instance;
}

// Line 2992-2995: Indicates that expandBuffer() is resuming the suspended sender
// private val RESUMING_BY_EB = Symbol("RESUMING_BY_EB")
inline internal::Symbol& RESUMING_BY_EB() {
    static internal::Symbol instance("RESUMING_BY_EB");
    return instance;
}

// Line 2996-3000: When a receiver comes to the cell already covered by a sender
// but the cell is still in EMPTY or IN_BUFFER state, it breaks the cell.
// private val POISONED = Symbol("POISONED")
inline internal::Symbol& POISONED() {
    static internal::Symbol instance("POISONED");
    return instance;
}

// Line 3001-3003: When the element is successfully transferred to a receiver.
// private val DONE_RCV = Symbol("DONE_RCV")
inline internal::Symbol& DONE_RCV() {
    static internal::Symbol instance("DONE_RCV");
    return instance;
}

// Line 3004-3005: Cancelled sender.
// private val INTERRUPTED_SEND = Symbol("INTERRUPTED_SEND")
inline internal::Symbol& INTERRUPTED_SEND() {
    static internal::Symbol instance("INTERRUPTED_SEND");
    return instance;
}

// Line 3006-3007: Cancelled receiver.
// private val INTERRUPTED_RCV = Symbol("INTERRUPTED_RCV")
inline internal::Symbol& INTERRUPTED_RCV() {
    static internal::Symbol instance("INTERRUPTED_RCV");
    return instance;
}

// Line 3008-3009: Indicates that the channel is closed.
// internal val CHANNEL_CLOSED = Symbol("CHANNEL_CLOSED")
inline internal::Symbol& CHANNEL_CLOSED() {
    static internal::Symbol instance("CHANNEL_CLOSED");
    return instance;
}

// ============================================================================
// Lines 3039-3041: Internal results for updateCellReceive
// ============================================================================

// private val SUSPEND = Symbol("SUSPEND")
inline internal::Symbol& SUSPEND() {
    static internal::Symbol instance("SUSPEND");
    return instance;
}

// private val SUSPEND_NO_WAITER = Symbol("SUSPEND_NO_WAITER")
inline internal::Symbol& SUSPEND_NO_WAITER() {
    static internal::Symbol instance("SUSPEND_NO_WAITER");
    return instance;
}

// private val FAILED = Symbol("FAILED")
inline internal::Symbol& FAILED() {
    static internal::Symbol instance("FAILED");
    return instance;
}

// ============================================================================
// Lines 3043-3051: Internal results for updateCellSend
// ============================================================================

// private const val RESULT_RENDEZVOUS = 0
constexpr int RESULT_RENDEZVOUS = 0;
// private const val RESULT_BUFFERED = 1
constexpr int RESULT_BUFFERED = 1;
// private const val RESULT_SUSPEND = 2
constexpr int RESULT_SUSPEND = 2;
// private const val RESULT_SUSPEND_NO_WAITER = 3
constexpr int RESULT_SUSPEND_NO_WAITER = 3;
// private const val RESULT_CLOSED = 4
constexpr int RESULT_CLOSED = 4;
// private const val RESULT_FAILED = 5
constexpr int RESULT_FAILED = 5;

// ============================================================================
// Lines 3053-3057: Special value for BufferedChannelIterator.receiveResult
// ============================================================================

// private val NO_RECEIVE_RESULT = Symbol("NO_RECEIVE_RESULT")
inline internal::Symbol& NO_RECEIVE_RESULT() {
    static internal::Symbol instance("NO_RECEIVE_RESULT");
    return instance;
}

// ============================================================================
// Lines 3059-3065: Close handler synchronization markers
// ============================================================================

// private val CLOSE_HANDLER_CLOSED = Symbol("CLOSE_HANDLER_CLOSED")
inline internal::Symbol& CLOSE_HANDLER_CLOSED() {
    static internal::Symbol instance("CLOSE_HANDLER_CLOSED");
    return instance;
}

// private val CLOSE_HANDLER_INVOKED = Symbol("CLOSE_HANDLER_INVOKED")
inline internal::Symbol& CLOSE_HANDLER_INVOKED() {
    static internal::Symbol instance("CLOSE_HANDLER_INVOKED");
    return instance;
}

// ============================================================================
// Lines 3067-3072: Absence of closing cause marker
// ============================================================================

// private val NO_CLOSE_CAUSE = Symbol("NO_CLOSE_CAUSE")
inline internal::Symbol& NO_CLOSE_CAUSE() {
    static internal::Symbol instance("NO_CLOSE_CAUSE");
    return instance;
}

// ============================================================================
// Lines 3074-3089: Channel close statuses
// ============================================================================

/*
  The channel close statuses. The transition scheme is the following:
    +--------+   +----------------------+   +-----------+
    | ACTIVE |-->| CANCELLATION_STARTED |-->| CANCELLED |
    +--------+   +----------------------+   +-----------+
        |                                         ^
        |             +--------+                  |
        +------------>| CLOSED |------------------+
                      +--------+
  We need `CANCELLATION_STARTED` to synchronize concurrent closing and cancellation.
*/
constexpr int CLOSE_STATUS_ACTIVE = 0;
constexpr int CLOSE_STATUS_CANCELLATION_STARTED = 1;
constexpr int CLOSE_STATUS_CLOSED = 2;
constexpr int CLOSE_STATUS_CANCELLED = 3;

// ============================================================================
// Lines 3091-3102: Senders counter and close status bit manipulation
// ============================================================================

/*
  The `senders` counter and the channel close status
  are stored in a single 64-bit register to save the space
  and reduce the number of reads in sending operations.
*/
constexpr int SENDERS_CLOSE_STATUS_SHIFT = 60;
constexpr int64_t SENDERS_COUNTER_MASK = (1LL << SENDERS_CLOSE_STATUS_SHIFT) - 1;

inline int64_t senders_counter(int64_t value) {
    return value & SENDERS_COUNTER_MASK;
}

inline int senders_close_status(int64_t value) {
    return static_cast<int>(value >> SENDERS_CLOSE_STATUS_SHIFT);
}

inline int64_t construct_senders_and_close_status(int64_t counter, int close_status) {
    return (static_cast<int64_t>(close_status) << SENDERS_CLOSE_STATUS_SHIFT) + counter;
}

// ============================================================================
// Lines 3104-3115: Expand buffer counter and pause flag bit manipulation
// ============================================================================

/*
  The `completedExpandBuffersAndPauseFlag` 64-bit counter contains
  the number of completed `expandBuffer()` attempts along with a special
  flag that pauses progress to avoid starvation in `waitExpandBufferCompletion(..)`.
*/
constexpr int64_t EB_COMPLETED_PAUSE_EXPAND_BUFFERS_BIT = 1LL << 62;
constexpr int64_t EB_COMPLETED_COUNTER_MASK = EB_COMPLETED_PAUSE_EXPAND_BUFFERS_BIT - 1;

inline int64_t eb_completed_counter(int64_t value) {
    return value & EB_COMPLETED_COUNTER_MASK;
}

inline bool eb_pause_expand_buffers(int64_t value) {
    return (value & EB_COMPLETED_PAUSE_EXPAND_BUFFERS_BIT) != 0;
}

inline int64_t construct_eb_completed_and_pause_flag(int64_t counter, bool pause_eb) {
    return (pause_eb ? EB_COMPLETED_PAUSE_EXPAND_BUFFERS_BIT : 0) + counter;
}

// ============================================================================
// Lines 3018-3020: WaiterEB wrapper class
// ============================================================================

/**
 * When the cell is already covered by both sender and receiver,
 * the expandBuffer() procedure cannot distinguish which kind of operation
 * is stored in the cell. Thus, it wraps the waiter with this descriptor.
 */
class WaiterEB {
public:
    // @JvmField val waiter: Waiter
    void* waiter;

    explicit WaiterEB(void* w) : waiter(w) {}

    std::string to_string() const {
        std::ostringstream oss;
        oss << "WaiterEB(" << waiter << ")";
        return oss.str();
    }
};

// ============================================================================
// Lines 3024-3031: ReceiveCatching wrapper class
// ============================================================================

/**
 * To distinguish suspended receive() and receiveCatching() operations,
 * the latter uses this wrapper for its continuation.
 */
template <typename E>
class ReceiveCatching {
public:
    // @JvmField val cont: CancellableContinuationImpl<ChannelResult<E>>
    CancellableContinuationImpl<ChannelResult<E>>* cont;

    explicit ReceiveCatching(CancellableContinuationImpl<ChannelResult<E>>* c) : cont(c) {}
};

// ============================================================================
// Lines 2796-2921: ChannelSegment class
// ============================================================================

/**
 * Line 2796-2802:
 * The channel is represented as a list of segments, which simulates an infinite array.
 * Each segment has its own id, which increases from the beginning. These ids help
 * to update sendSegment, receiveSegment, and bufferEndSegment correctly.
 *
 * internal class ChannelSegment<E>(id: Long, prev: ChannelSegment<E>?, channel: BufferedChannel<E>?, pointers: Int)
 *     : Segment<ChannelSegment<E>>(id, prev, pointers)
 */
template <typename E>
class ChannelSegment : public internal::Segment<ChannelSegment<E>> {
private:
    // Line 2803: private val _channel: BufferedChannel<E>? = channel
    BufferedChannel<E>* channel_;

    // Line 2806: private val data = atomicArrayOfNulls<Any?>(SEGMENT_SIZE * 2)
    // 2 registers per slot: state + element
    std::atomic<void*> data_[SEGMENT_SIZE * 2];

public:
    // Constructor
    ChannelSegment(int64_t id, ChannelSegment<E>* prev, BufferedChannel<E>* channel, int pointers)
        : internal::Segment<ChannelSegment<E>>(id, prev, pointers)
        , channel_(channel) {
        for (int i = 0; i < SEGMENT_SIZE * 2; ++i) {
            data_[i].store(nullptr, std::memory_order_relaxed);
        }
    }

    // Line 2804: val channel get() = _channel!!
    BufferedChannel<E>* channel() const {
        assert(channel_ != nullptr);
        return channel_;
    }

    // Line 2807: override val numberOfSlots: Int get() = SEGMENT_SIZE
    int number_of_slots() const override { return SEGMENT_SIZE; }

    // ########################################
    // # Manipulation with the Element Fields #
    // ########################################

    // Line 2813-2815: internal fun storeElement(index: Int, element: E)
    void store_element(int index, E element) {
        set_element_lazy(index, reinterpret_cast<void*>(new E(std::move(element))));
    }

    // Line 2817-2818: internal fun getElement(index: Int) = data[index * 2].value as E
    E get_element(int index) const {
        void* ptr = data_[index * 2].load(std::memory_order_acquire);
        if (ptr == nullptr) return E{};
        return *reinterpret_cast<E*>(ptr);
    }

    // Line 2820: internal fun retrieveElement(index: Int): E = getElement(index).also { cleanElement(index) }
    E retrieve_element(int index) {
        E elem = get_element(index);
        clean_element(index);
        return elem;
    }

    // Line 2822-2824: internal fun cleanElement(index: Int)
    void clean_element(int index) {
        void* ptr = data_[index * 2].exchange(nullptr, std::memory_order_acq_rel);
        if (ptr != nullptr) {
            delete reinterpret_cast<E*>(ptr);
        }
    }

    // Line 2826-2828: private fun setElementLazy(index: Int, value: Any?)
    void set_element_lazy(int index, void* value) {
        data_[index * 2].store(value, std::memory_order_release);
    }

    // ######################################
    // # Manipulation with the State Fields #
    // ######################################

    // Line 2834: internal fun getState(index: Int): Any? = data[index * 2 + 1].value
    void* get_state(int index) const {
        return data_[index * 2 + 1].load(std::memory_order_acquire);
    }

    // Line 2836-2838: internal fun setState(index: Int, value: Any?)
    void set_state(int index, void* value) {
        data_[index * 2 + 1].store(value, std::memory_order_release);
    }

    // Line 2840: internal fun casState(index: Int, from: Any?, to: Any?)
    bool cas_state(int index, void* from, void* to) {
        return data_[index * 2 + 1].compare_exchange_strong(from, to,
            std::memory_order_acq_rel, std::memory_order_acquire);
    }

    // Line 2842: internal fun getAndSetState(index: Int, update: Any?)
    void* get_and_set_state(int index, void* update) {
        return data_[index * 2 + 1].exchange(update, std::memory_order_acq_rel);
    }

    // ########################
    // # Cancellation Support #
    // ########################

    // Line 2849-2911: override fun onCancellation(index: Int, cause: Throwable?, context: CoroutineContext)
    void on_cancellation(int index, std::exception_ptr cause,
                         std::shared_ptr<CoroutineContext> /*context*/) override {
        // Line 2852-2853: To distinguish cancelled senders and receivers,
        // senders equip the index value with an additional marker, adding SEGMENT_SIZE.
        bool is_sender = index >= SEGMENT_SIZE;
        // Unwrap the index.
        if (is_sender) index = index - SEGMENT_SIZE;

        // Line 2856: Read the element, which may be needed to call onUndeliveredElement.
        E element = get_element(index);

        // Line 2858-2910: Update the cell state (CAS-loop)
        while (true) {
            void* cur = get_state(index);

            // Line 2864-2882: The cell stores a waiter.
            // cur is Waiter || cur is WaiterEB
            if (cur != nullptr &&
                cur != static_cast<void*>(&INTERRUPTED_SEND()) &&
                cur != static_cast<void*>(&INTERRUPTED_RCV()) &&
                cur != static_cast<void*>(&RESUMING_BY_EB()) &&
                cur != static_cast<void*>(&RESUMING_BY_RCV()) &&
                cur != static_cast<void*>(&DONE_RCV()) &&
                cur != static_cast<void*>(&BUFFERED()) &&
                cur != static_cast<void*>(&CHANNEL_CLOSED())) {
                // Update state to INTERRUPTED_SEND or INTERRUPTED_RCV
                void* update = is_sender ?
                    static_cast<void*>(&INTERRUPTED_SEND()) :
                    static_cast<void*>(&INTERRUPTED_RCV());
                if (cas_state(index, cur, update)) {
                    // The waiter has been successfully cancelled.
                    clean_element(index);
                    on_cancelled_request(index, !is_sender);
                    // Call onUndeliveredElement if needed (sender case).
                    if (is_sender && channel_->on_undelivered_element()) {
                        channel_->on_undelivered_element()(element);
                    }
                    return;
                }
                continue;
            }

            // Line 2885-2893: The cell already indicates that the operation is cancelled.
            if (cur == static_cast<void*>(&INTERRUPTED_SEND()) ||
                cur == static_cast<void*>(&INTERRUPTED_RCV())) {
                clean_element(index);
                if (is_sender && channel_->on_undelivered_element()) {
                    channel_->on_undelivered_element()(element);
                }
                return;
            }

            // Line 2897-2901: An opposite operation is resuming this request; wait.
            if (cur == static_cast<void*>(&RESUMING_BY_EB()) ||
                cur == static_cast<void*>(&RESUMING_BY_RCV())) {
                continue;
            }

            // Line 2903-2904: This request was successfully resumed (prompt cancellation).
            if (cur == static_cast<void*>(&DONE_RCV()) ||
                cur == static_cast<void*>(&BUFFERED())) {
                return;
            }

            // Line 2906-2907: The cell state indicates that the channel is closed.
            if (cur == static_cast<void*>(&CHANNEL_CLOSED())) {
                return;
            }

            // Line 2908: else -> error("unexpected state: $cur")
            assert(false && "unexpected state in onCancellation");
            return;
        }
    }

    // Line 2913-2920: fun onCancelledRequest(index: Int, receiver: Boolean)
    void on_cancelled_request(int index, bool receiver) {
        if (receiver) {
            channel_->wait_expand_buffer_completion(this->id * SEGMENT_SIZE + index);
        }
        this->on_slot_cleaned();
    }
};

// ============================================================================
// Line 2932: NULL_SEGMENT singleton
// ============================================================================

// private val NULL_SEGMENT = ChannelSegment<Any?>(id = -1, prev = null, channel = null, pointers = 0)
template <typename E>
inline ChannelSegment<E>* null_segment() {
    static ChannelSegment<E> instance(-1, nullptr, nullptr, 0);
    return &instance;
}

// ============================================================================
// Lines 2924-2931: createSegment function
// ============================================================================

// Line 2926-2931: private fun <E> createSegment(id: Long, prev: ChannelSegment<E>)
template <typename E>
ChannelSegment<E>* create_segment(int64_t id, ChannelSegment<E>* prev) {
    return new ChannelSegment<E>(
        id,
        prev,
        prev->channel(),
        0  // pointers
    );
}

// ============================================================================
// Lines 19-2794: BufferedChannel class
// ============================================================================

/**
 * Lines 19-32:
 * The buffered channel implementation, which also serves as a rendezvous channel when the capacity is zero.
 * The high-level structure bases on a conceptually infinite array for storing elements and waiting requests,
 * separate counters of send and receive invocations that were ever performed, and an additional counter
 * that indicates the end of the logical buffer by counting the number of array cells it ever contained.
 *
 * The key idea is that both send and receive start by incrementing their counters, assigning the array cell
 * referenced by the counter. In case of rendezvous channels, the operation either suspends and stores its continuation
 * in the cell or makes a rendezvous with the opposite request. Each cell can be processed by exactly one send and
 * one receive. As for buffered channels, sends can also add elements without suspension if the logical buffer
 * contains the cell, while the receive operation updates the end of the buffer when its synchronization finishes.
 *
 * Please see the "Fast and Scalable Channels in Kotlin Coroutines" paper
 * (https://arxiv.org/abs/2211.04986) by Nikita Koval, Roman Elizarov, and Dan Alistarh
 * for the detailed algorithm description.
 *
 * internal open class BufferedChannel<E>(
 *     private val capacity: Int,
 *     @JvmField internal val onUndeliveredElement: OnUndeliveredElement<E>? = null
 * ) : Channel<E>
 */
template <typename E>
class BufferedChannel : public Channel<E> {
public:
    // Line 33-45: Constructor and init block
    BufferedChannel(int capacity, OnUndeliveredElement<E> on_undelivered_element = nullptr)
        : capacity_(capacity)
        , on_undelivered_element_(on_undelivered_element)
        , senders_and_close_status_(0L)
        , receivers_(0L)
        , buffer_end_(initial_buffer_end(capacity))
        , completed_expand_buffers_and_pause_flag_(initial_buffer_end(capacity))
        , close_cause_(static_cast<void*>(&NO_CLOSE_CAUSE()))
        , close_handler_(nullptr) {
        // Line 43: require(capacity >= 0) { "Invalid channel capacity: $capacity, should be >=0" }
        if (capacity < 0 && capacity != CHANNEL_UNLIMITED) {
            throw std::invalid_argument("Invalid channel capacity: " + std::to_string(capacity) + ", should be >=0");
        }

        // Line 93-103: Second init block - create first segment
        // @Suppress("LeakingThis")
        // val firstSegment = ChannelSegment(id = 0, prev = null, channel = this, pointers = 3)
        auto* first_segment = new ChannelSegment<E>(0, nullptr, this, 3);
        send_segment_.store(first_segment, std::memory_order_release);
        receive_segment_.store(first_segment, std::memory_order_release);

        // Line 98-102: If this channel is rendezvous or has unlimited capacity, the algorithm never
        // invokes the buffer expansion procedure, and the corresponding segment reference
        // points to a special NULL_SEGMENT.
        if (is_rendezvous_or_unlimited()) {
            buffer_end_segment_.store(null_segment<E>(), std::memory_order_release);
        } else {
            buffer_end_segment_.store(first_segment, std::memory_order_release);
        }
    }

    // Destructor
    ~BufferedChannel() override {
        close(std::make_exception_ptr(std::runtime_error("Channel destroyed")));
    }

    // Line 40: @JvmField internal val onUndeliveredElement
    OnUndeliveredElement<E> on_undelivered_element() const { return on_undelivered_element_; }

    // =========================================================================
    // Lines 63-103: Counters and segment references
    // =========================================================================

    // Line 67: internal val sendersCounter: Long get() = sendersAndCloseStatus.value.sendersCounter
    int64_t senders_counter() const {
        return channels::senders_counter(senders_and_close_status_.load(std::memory_order_acquire));
    }

    // Line 68: internal val receiversCounter: Long get() = receivers.value
    int64_t receivers_counter() const {
        return receivers_.load(std::memory_order_acquire);
    }

    // Line 69: private val bufferEndCounter: Long get() = bufferEnd.value
    int64_t buffer_end_counter() const {
        return buffer_end_.load(std::memory_order_acquire);
    }

    // Line 86-87: private val isRendezvousOrUnlimited get() = ...
    bool is_rendezvous_or_unlimited() const {
        int64_t bec = buffer_end_counter();
        return bec == BUFFER_END_RENDEZVOUS || bec == BUFFER_END_UNLIMITED;
    }

    // =========================================================================
    // Lines 105-608: The send operations
    // =========================================================================

    // Line 109-128: override suspend fun send(element: E): Unit
    void* send(E element, Continuation<void*>* completion) override {
        // Fast path - try to send immediately
        auto result = try_send(std::move(element));
        if (result.is_success()) {
            return nullptr; // Completed immediately
        }

        if (result.is_closed()) {
            auto cause = result.exception_or_null();
            if (cause) std::rethrow_exception(cause);
            throw ClosedSendChannelException("Channel was closed");
        }

        // Need to suspend - full implementation requires sendImpl with continuation
        // This is a simplified version; full suspend path needs state machine
        (void)completion;
        throw std::logic_error("BufferedChannel::send suspend path requires full state machine implementation");
        return nullptr;
    }

    // Line 183-208: override fun trySend(element: E): ChannelResult<Unit>
    ChannelResult<void> try_send(E element) override {
        // Line 185: Do not try to send the element if the plain send(e) operation would suspend.
        if (should_send_suspend(senders_and_close_status_.load(std::memory_order_acquire))) {
            return ChannelResult<void>::failure();
        }

        // This channel either has waiting receivers or is closed.
        // Try to send the element using sendImpl logic.
        return send_impl_try_send(std::move(element));
    }

    // Line 621-627: private fun shouldSendSuspend(curSendersAndCloseStatus: Long): Boolean
    bool should_send_suspend(int64_t cur_senders_and_close_status) const {
        // Line 624: Does not suspend if the channel is already closed.
        if (is_closed_for_send_internal(cur_senders_and_close_status)) return false;
        // Line 626: Does not suspend if a rendezvous may happen or the buffer is not full.
        return !buffer_or_rendezvous_send(channels::senders_counter(cur_senders_and_close_status));
    }

    // Line 633-634: private fun bufferOrRendezvousSend(curSenders: Long): Boolean
    bool buffer_or_rendezvous_send(int64_t cur_senders) const {
        return cur_senders < buffer_end_counter() ||
               cur_senders < receivers_counter() + capacity_;
    }

    // Line 645: internal open fun shouldSendSuspend(): Boolean
    virtual bool should_send_suspend() const {
        return should_send_suspend(senders_and_close_status_.load(std::memory_order_acquire));
    }

    // Line 218-231: internal open suspend fun sendBroadcast(element: E): Boolean
    virtual void* send_broadcast(E element, Continuation<void*>* continuation) {
        // Default implementation suspends like send()
        // ConflatedBufferedChannel overrides to never suspend
        (void)continuation;
        throw std::logic_error("sendBroadcast suspend path not implemented");
    }

    // =========================================================================
    // Lines 671-1004: The receive operations
    // =========================================================================

    // Line 686-706: override suspend fun receive(): E
    void* receive(Continuation<void*>* continuation) override {
        auto result = try_receive();
        if (result.is_success()) {
            // Return boxed element
            return new E(result.get_or_throw());
        }
        if (result.is_closed()) {
            if (result.exception_or_null()) {
                std::rethrow_exception(result.exception_or_null());
            }
            throw ClosedReceiveChannelException("Channel was closed");
        }

        // Need to suspend - full state machine implementation required
        (void)continuation;
        throw std::logic_error("BufferedChannel::receive suspend path requires full state machine implementation");
        return nullptr;
    }

    // Line 751-760: override suspend fun receiveCatching(): ChannelResult<E>
    void* receive_catching(Continuation<void*>* continuation) override {
        auto result = try_receive();
        if (result.is_success() || result.is_closed()) {
            // Return boxed ChannelResult
            return new ChannelResult<E>(std::move(result));
        }

        // Need to suspend - full state machine implementation required
        (void)continuation;
        throw std::logic_error("BufferedChannel::receive_catching suspend path requires full state machine implementation");
        return nullptr;
    }

    // Line 782-818: override fun tryReceive(): ChannelResult<E>
    ChannelResult<E> try_receive() override {
        // Line 783-784: Read the receivers counter first.
        int64_t r = receivers_.load(std::memory_order_acquire);
        int64_t senders_and_close_status_cur = senders_and_close_status_.load(std::memory_order_acquire);

        // Line 787-789: Is this channel closed for receive?
        if (is_closed_for_receive_internal(senders_and_close_status_cur)) {
            return ChannelResult<E>::closed(close_cause());
        }

        // Line 791-792: Do not try to receive an element if the plain receive() would suspend.
        int64_t s = channels::senders_counter(senders_and_close_status_cur);
        if (r >= s) return ChannelResult<E>::failure();

        // Try to retrieve an element using receiveImpl logic
        return receive_impl_try_receive();
    }

    // =========================================================================
    // Lines 1186-1467: The expandBuffer() procedure
    // =========================================================================

    // Line 1190-1252: private fun expandBuffer()
    void expand_buffer() {
        // Line 1193: Do not need to take any action if this channel is rendezvous or unlimited.
        if (is_rendezvous_or_unlimited()) return;

        // Line 1196: Read the current segment of the expandBuffer() procedure.
        ChannelSegment<E>* segment = buffer_end_segment_.load(std::memory_order_acquire);

        // Line 1198: Try to expand the buffer until succeed.
        while (true) {
            // Line 1201: Increment the logical end of the buffer.
            int64_t b = buffer_end_.fetch_add(1, std::memory_order_acq_rel);
            int64_t id = b / SEGMENT_SIZE;

            // Line 1210-1217: After that, read the current senders counter.
            int64_t s = senders_counter();
            if (s <= b) {
                // Should bufferEndSegment be moved forward?
                if (segment->id < id && segment->next() != nullptr) {
                    move_segment_buffer_end_to_specified_or_last(id, segment);
                }
                inc_completed_expand_buffer_attempts();
                return;
            }

            // Line 1221-1229: Find the required segment if needed
            if (segment->id != id) {
                segment = find_segment_buffer_end(id, segment, b);
                if (segment == nullptr) continue;
            }

            // Line 1233-1250: Try to add the cell to the logical buffer
            int i = static_cast<int>(b % SEGMENT_SIZE);
            if (update_cell_expand_buffer(segment, i, b)) {
                inc_completed_expand_buffer_attempts();
                return;
            } else {
                inc_completed_expand_buffer_attempts();
                continue;
            }
        }
    }

    // Line 1254-1296: private fun updateCellExpandBuffer(...)
    bool update_cell_expand_buffer(ChannelSegment<E>* segment, int index, int64_t b) {
        // Fast-path
        void* state = segment->get_state(index);

        // Check if state is a waiter (not a known symbol)
        if (state != nullptr &&
            state != static_cast<void*>(&BUFFERED()) &&
            state != static_cast<void*>(&IN_BUFFER()) &&
            state != static_cast<void*>(&INTERRUPTED_SEND()) &&
            state != static_cast<void*>(&POISONED()) &&
            state != static_cast<void*>(&DONE_RCV()) &&
            state != static_cast<void*>(&INTERRUPTED_RCV()) &&
            state != static_cast<void*>(&CHANNEL_CLOSED()) &&
            state != static_cast<void*>(&RESUMING_BY_RCV()) &&
            state != static_cast<void*>(&RESUMING_BY_EB())) {
            // Usually, a sender is stored in the cell.
            if (b >= receivers_counter()) {
                if (segment->cas_state(index, state, static_cast<void*>(&RESUMING_BY_EB()))) {
                    // Try to resume the sender
                    bool resumed = try_resume_sender(state, segment, index);
                    if (resumed) {
                        segment->set_state(index, static_cast<void*>(&BUFFERED()));
                        return true;
                    } else {
                        segment->set_state(index, static_cast<void*>(&INTERRUPTED_SEND()));
                        segment->on_cancelled_request(index, false);
                        return false;
                    }
                }
            }
        }

        return update_cell_expand_buffer_slow(segment, index, b);
    }

    // Line 1298-1379: private fun updateCellExpandBufferSlow(...)
    bool update_cell_expand_buffer_slow(ChannelSegment<E>* segment, int index, int64_t b) {
        while (true) {
            void* state = segment->get_state(index);

            // Check for waiter
            bool is_waiter = (state != nullptr &&
                state != static_cast<void*>(&BUFFERED()) &&
                state != static_cast<void*>(&IN_BUFFER()) &&
                state != static_cast<void*>(&INTERRUPTED_SEND()) &&
                state != static_cast<void*>(&POISONED()) &&
                state != static_cast<void*>(&DONE_RCV()) &&
                state != static_cast<void*>(&INTERRUPTED_RCV()) &&
                state != static_cast<void*>(&CHANNEL_CLOSED()) &&
                state != static_cast<void*>(&RESUMING_BY_RCV()) &&
                state != static_cast<void*>(&RESUMING_BY_EB()));

            if (is_waiter) {
                if (b < receivers_counter()) {
                    // Cannot distinguish sender from receiver, wrap in WaiterEB
                    auto* wrapper = new WaiterEB(state);
                    if (segment->cas_state(index, state, wrapper)) {
                        return true;
                    }
                    delete wrapper;
                } else {
                    // The cell stores a suspended sender
                    if (segment->cas_state(index, state, static_cast<void*>(&RESUMING_BY_EB()))) {
                        bool resumed = try_resume_sender(state, segment, index);
                        if (resumed) {
                            segment->set_state(index, static_cast<void*>(&BUFFERED()));
                            return true;
                        } else {
                            segment->set_state(index, static_cast<void*>(&INTERRUPTED_SEND()));
                            segment->on_cancelled_request(index, false);
                            return false;
                        }
                    }
                }
            } else if (state == static_cast<void*>(&INTERRUPTED_SEND())) {
                return false;
            } else if (state == nullptr) {
                // The cell is empty
                if (segment->cas_state(index, nullptr, static_cast<void*>(&IN_BUFFER()))) {
                    return true;
                }
            } else if (state == static_cast<void*>(&BUFFERED())) {
                return true;
            } else if (state == static_cast<void*>(&POISONED()) ||
                       state == static_cast<void*>(&DONE_RCV()) ||
                       state == static_cast<void*>(&INTERRUPTED_RCV())) {
                return true;
            } else if (state == static_cast<void*>(&CHANNEL_CLOSED())) {
                return true;
            } else if (state == static_cast<void*>(&RESUMING_BY_RCV())) {
                // Spin wait
                continue;
            }
        }
    }

    // Line 1388-1399: private fun incCompletedExpandBufferAttempts(nAttempts: Long = 1)
    void inc_completed_expand_buffer_attempts(int64_t n_attempts = 1) {
        int64_t result = completed_expand_buffers_and_pause_flag_.fetch_add(n_attempts, std::memory_order_acq_rel) + n_attempts;
        if (eb_pause_expand_buffers(result)) {
            while (eb_pause_expand_buffers(completed_expand_buffers_and_pause_flag_.load(std::memory_order_acquire))) {
                // spin
            }
        }
    }

    // Line 1410-1467: internal fun waitExpandBufferCompletion(globalIndex: Long)
    void wait_expand_buffer_completion(int64_t global_index) {
        // Line 1413: Do nothing if this channel is rendezvous or unlimited.
        if (is_rendezvous_or_unlimited()) return;

        // Line 1417: Wait until the number of started buffer expansion calls >= cell index.
        while (buffer_end_counter() <= global_index) {
            // spin
        }

        // Line 1422: Wait in a fixed-size spin-loop
        for (int i = 0; i < EXPAND_BUFFER_COMPLETION_WAIT_ITERATIONS; ++i) {
            int64_t b = buffer_end_counter();
            int64_t eb_completed = eb_completed_counter(
                completed_expand_buffers_and_pause_flag_.load(std::memory_order_acquire));
            if (b == eb_completed && b == buffer_end_counter()) return;
        }

        // To avoid starvation, pause further expandBuffer() calls.
        int64_t cur = completed_expand_buffers_and_pause_flag_.load(std::memory_order_acquire);
        while (true) {
            int64_t updated = construct_eb_completed_and_pause_flag(eb_completed_counter(cur), true);
            if (completed_expand_buffers_and_pause_flag_.compare_exchange_weak(cur, updated,
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                break;
            }
        }

        // Now wait in an infinite spin-loop until the counters coincide.
        while (true) {
            int64_t b = buffer_end_counter();
            int64_t eb_completed_and_bit = completed_expand_buffers_and_pause_flag_.load(std::memory_order_acquire);
            int64_t eb_completed = eb_completed_counter(eb_completed_and_bit);
            bool pause_expand_buffers = eb_pause_expand_buffers(eb_completed_and_bit);

            if (b == eb_completed && b == buffer_end_counter()) {
                // Unset the flag
                cur = completed_expand_buffers_and_pause_flag_.load(std::memory_order_acquire);
                while (true) {
                    int64_t updated = construct_eb_completed_and_pause_flag(eb_completed_counter(cur), false);
                    if (completed_expand_buffers_and_pause_flag_.compare_exchange_weak(cur, updated,
                            std::memory_order_acq_rel, std::memory_order_acquire)) {
                        break;
                    }
                }
                return;
            }

            if (!pause_expand_buffers) {
                completed_expand_buffers_and_pause_flag_.compare_exchange_weak(eb_completed_and_bit,
                    construct_eb_completed_and_pause_flag(eb_completed, true),
                    std::memory_order_acq_rel, std::memory_order_acquire);
            }
        }
    }

    // =========================================================================
    // Lines 1569-1744: Iterator Support
    // =========================================================================

    // Line 1573: override fun iterator(): ChannelIterator<E>
    std::unique_ptr<ChannelIterator<E>> iterator() override {
        return std::make_unique<BufferedChannelIterator>(this);
    }

    // =========================================================================
    // Lines 1746-2211: Closing and Cancellation
    // =========================================================================

    // Line 1756: protected val closeCause get() = _closeCause.value as Throwable?
    std::exception_ptr close_cause() const {
        void* cause = close_cause_.load(std::memory_order_acquire);
        if (cause == static_cast<void*>(&NO_CLOSE_CAUSE()) || cause == nullptr) {
            return nullptr;
        }
        return *reinterpret_cast<std::exception_ptr*>(cause);
    }

    // Line 1759: protected val sendException get() = closeCause ?: ClosedSendChannelException(...)
    std::exception_ptr send_exception() const {
        auto cause = close_cause();
        if (cause) return cause;
        return std::make_exception_ptr(ClosedSendChannelException("Channel was closed"));
    }

    // Line 1762: private val receiveException get() = closeCause ?: ClosedReceiveChannelException(...)
    std::exception_ptr receive_exception() const {
        auto cause = close_cause();
        if (cause) return cause;
        return std::make_exception_ptr(ClosedReceiveChannelException("Channel was closed"));
    }

    // Line 1786-1787: override fun close(cause: Throwable?): Boolean
    bool close(std::exception_ptr cause = nullptr) override {
        return close_or_cancel_impl(cause, false);
    }

    // Line 1790: final override fun cancel(cause: Throwable?): Boolean
    bool cancel_with_cause(std::exception_ptr cause) {
        return cancel_impl(cause);
    }

    // Line 1793: final override fun cancel()
    void cancel(std::exception_ptr cause = nullptr) override {
        cancel_impl(cause);
    }

    // Line 1797-1798: internal open fun cancelImpl(cause: Throwable?): Boolean
    bool cancel_impl(std::exception_ptr cause) {
        if (!cause) {
            cause = std::make_exception_ptr(std::runtime_error("Channel was cancelled"));
        }
        return close_or_cancel_impl(cause, true);
    }

    // Line 1816-1835: protected open fun closeOrCancelImpl(...)
    bool close_or_cancel_impl(std::exception_ptr cause, bool cancel) {
        // Line 1821: If this is a cancel(..) invocation, set the cancellation started bit.
        if (cancel) mark_cancellation_started();

        // Line 1824: Try to install the specified cause.
        void* no_cause = static_cast<void*>(&NO_CLOSE_CAUSE());
        void* cause_ptr = cause ? new std::exception_ptr(cause) : nullptr;
        bool closed_by_this_operation = close_cause_.compare_exchange_strong(
            no_cause, cause_ptr, std::memory_order_acq_rel, std::memory_order_acquire);

        if (!closed_by_this_operation && cause_ptr) {
            delete reinterpret_cast<std::exception_ptr*>(cause_ptr);
        }

        // Line 1826: Mark this channel as closed or cancelled.
        if (cancel) {
            mark_cancelled();
        } else {
            mark_closed();
        }

        // Line 1828: Complete the closing or cancellation procedure.
        complete_close_or_cancel();

        // Line 1831-1834: Invoke handlers if this operation installed the cause.
        on_closed_idempotent();
        if (closed_by_this_operation) {
            invoke_close_handler_internal();
        }

        return closed_by_this_operation;
    }

    // Line 1784: protected open fun onClosedIdempotent() {}
    virtual void on_closed_idempotent() {}

    // Line 1859-1885: override fun invokeOnClose(handler: (cause: Throwable?) -> Unit)
    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override {
        void* null_handler = nullptr;
        auto* handler_ptr = new std::function<void(std::exception_ptr)>(handler);

        if (close_handler_.compare_exchange_strong(null_handler, handler_ptr,
                std::memory_order_acq_rel, std::memory_order_acquire)) {
            return;
        }

        delete handler_ptr;

        // Either another handler is already set, or this channel is closed.
        while (true) {
            void* cur = close_handler_.load(std::memory_order_acquire);
            if (cur == static_cast<void*>(&CLOSE_HANDLER_CLOSED())) {
                if (close_handler_.compare_exchange_strong(cur,
                        static_cast<void*>(&CLOSE_HANDLER_INVOKED()),
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    handler(close_cause());
                    return;
                }
            } else if (cur == static_cast<void*>(&CLOSE_HANDLER_INVOKED())) {
                throw std::logic_error("Another handler was already registered and successfully invoked");
            } else {
                throw std::logic_error("Another handler is already registered");
            }
        }
    }

    // Line 1896-1905: private fun markClosed()
    void mark_closed() {
        while (true) {
            int64_t cur = senders_and_close_status_.load(std::memory_order_acquire);
            int status = senders_close_status(cur);
            if (status == CLOSE_STATUS_ACTIVE) {
                int64_t update = construct_senders_and_close_status(channels::senders_counter(cur), CLOSE_STATUS_CLOSED);
                if (senders_and_close_status_.compare_exchange_weak(cur, update,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    return;
                }
            } else if (status == CLOSE_STATUS_CANCELLATION_STARTED) {
                int64_t update = construct_senders_and_close_status(channels::senders_counter(cur), CLOSE_STATUS_CANCELLED);
                if (senders_and_close_status_.compare_exchange_weak(cur, update,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    return;
                }
            } else {
                return; // already closed or cancelled
            }
        }
    }

    // Line 1913-1916: private fun markCancelled()
    void mark_cancelled() {
        while (true) {
            int64_t cur = senders_and_close_status_.load(std::memory_order_acquire);
            int64_t update = construct_senders_and_close_status(channels::senders_counter(cur), CLOSE_STATUS_CANCELLED);
            if (senders_and_close_status_.compare_exchange_weak(cur, update,
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                return;
            }
        }
    }

    // Line 1924-1929: private fun markCancellationStarted()
    void mark_cancellation_started() {
        while (true) {
            int64_t cur = senders_and_close_status_.load(std::memory_order_acquire);
            int status = senders_close_status(cur);
            if (status == CLOSE_STATUS_ACTIVE) {
                int64_t update = construct_senders_and_close_status(channels::senders_counter(cur), CLOSE_STATUS_CANCELLATION_STARTED);
                if (senders_and_close_status_.compare_exchange_weak(cur, update,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    return;
                }
            } else {
                return; // already closed or cancelled
            }
        }
    }

    // Line 1934-1936: private fun completeCloseOrCancel()
    void complete_close_or_cancel() {
        // must finish the started close/cancel if one is detected.
        (void)is_closed_for_send();
    }

    // Line 2213-2214: override val isClosedForSend: Boolean
    bool is_closed_for_send() const override {
        return is_closed_for_send_internal(senders_and_close_status_.load(std::memory_order_acquire));
    }

    // Line 2216-2217: private val Long.isClosedForSend0
    bool is_closed_for_send_internal(int64_t senders_and_close_status_cur) const {
        return is_closed(senders_and_close_status_cur, false);
    }

    // Line 2220-2221: override val isClosedForReceive: Boolean
    bool is_closed_for_receive() const override {
        return is_closed_for_receive_internal(senders_and_close_status_.load(std::memory_order_acquire));
    }

    // Line 2223-2224: private val Long.isClosedForReceive0
    bool is_closed_for_receive_internal(int64_t senders_and_close_status_cur) const {
        return is_closed(senders_and_close_status_cur, true);
    }

    // Line 2226-2256: private fun isClosed(...)
    bool is_closed(int64_t senders_and_close_status_cur, bool is_closed_for_receive) const {
        int status = senders_close_status(senders_and_close_status_cur);
        switch (status) {
            case CLOSE_STATUS_ACTIVE:
            case CLOSE_STATUS_CANCELLATION_STARTED:
                return false;
            case CLOSE_STATUS_CLOSED:
                const_cast<BufferedChannel*>(this)->complete_close(channels::senders_counter(senders_and_close_status_cur));
                if (is_closed_for_receive) {
                    return !has_elements();
                }
                return true;
            case CLOSE_STATUS_CANCELLED:
                const_cast<BufferedChannel*>(this)->complete_cancel(channels::senders_counter(senders_and_close_status_cur));
                return true;
            default:
                assert(false && "unexpected close status");
                return false;
        }
    }

    // Line 2259-2268: override val isEmpty: Boolean
    bool is_empty() const override {
        if (is_closed_for_receive()) return false;
        if (has_elements()) return false;
        return !is_closed_for_receive();
    }

    // Line 2279-2308: internal fun hasElements(): Boolean
    bool has_elements() const {
        while (true) {
            ChannelSegment<E>* segment = receive_segment_.load(std::memory_order_acquire);
            int64_t r = receivers_counter();
            int64_t s = senders_counter();
            if (s <= r) return false;

            int64_t id = r / SEGMENT_SIZE;
            if (segment->id != id) {
                segment = const_cast<BufferedChannel*>(this)->find_segment_receive(id, segment);
                if (segment == nullptr) {
                    if (receive_segment_.load(std::memory_order_acquire)->id < id) {
                        return false;
                    }
                    continue;
                }
            }
            segment->clean_prev();

            int i = static_cast<int>(r % SEGMENT_SIZE);
            if (is_cell_non_empty(segment, i, r)) return true;

            // Update receivers counter
            int64_t expected = r;
            receivers_.compare_exchange_weak(expected, r + 1,
                std::memory_order_acq_rel, std::memory_order_acquire);
        }
    }

    // Line 2319-2373: private fun isCellNonEmpty(...)
    bool is_cell_non_empty(ChannelSegment<E>* segment, int index, int64_t global_index) const {
        while (true) {
            void* state = segment->get_state(index);

            if (state == nullptr || state == static_cast<void*>(&IN_BUFFER())) {
                if (segment->cas_state(index, state, static_cast<void*>(&POISONED()))) {
                    const_cast<BufferedChannel*>(this)->expand_buffer();
                    return false;
                }
            } else if (state == static_cast<void*>(&BUFFERED())) {
                return true;
            } else if (state == static_cast<void*>(&INTERRUPTED_SEND())) {
                return false;
            } else if (state == static_cast<void*>(&CHANNEL_CLOSED())) {
                return false;
            } else if (state == static_cast<void*>(&DONE_RCV())) {
                return false;
            } else if (state == static_cast<void*>(&POISONED())) {
                return false;
            } else if (state == static_cast<void*>(&RESUMING_BY_EB())) {
                return true;
            } else if (state == static_cast<void*>(&RESUMING_BY_RCV())) {
                return false;
            } else {
                // The cell stores a suspended request
                return global_index == receivers_counter();
            }
        }
    }

    // =========================================================================
    // Lines 1469-1567: Select Expression
    // =========================================================================

    // Line 1475-1480: override val onSend
    selects::SelectClause2<E, SendChannel<E>*>& on_send() override {
        // Select support not yet implemented
        throw std::logic_error("BufferedChannel::on_send select clause not yet implemented");
    }

    // Line 1504-1510: override val onReceive
    selects::SelectClause1<E>& on_receive() override {
        // Select support not yet implemented
        throw std::logic_error("BufferedChannel::on_receive select clause not yet implemented");
    }

    // Line 1512-1518: override val onReceiveCatching
    selects::SelectClause1<ChannelResult<E>>& on_receive_catching() override {
        // Select support not yet implemented
        throw std::logic_error("BufferedChannel::on_receive_catching select clause not yet implemented");
    }

    // =========================================================================
    // Lines 2591-2697: Debug Functions
    // =========================================================================

    // Line 2596-2654: override fun toString(): String
    std::string to_string() const {
        std::ostringstream sb;

        int status = senders_close_status(senders_and_close_status_.load(std::memory_order_acquire));
        if (status == CLOSE_STATUS_CLOSED) sb << "closed,";
        else if (status == CLOSE_STATUS_CANCELLED) sb << "cancelled,";

        sb << "capacity=" << capacity_ << ",";
        sb << "data=[";
        // Simplified - full implementation would traverse segments
        sb << "]";

        return sb.str();
    }

protected:
    // Line 38-39: Channel capacity and onUndeliveredElement
    int capacity_;
    OnUndeliveredElement<E> on_undelivered_element_;

    // Line 1938: protected open val isConflatedDropOldest get() = false
    virtual bool is_conflated_drop_oldest() const { return false; }

private:
    // =========================================================================
    // Lines 63-91: Counters and state
    // =========================================================================

    // Line 63: private val sendersAndCloseStatus = atomic(0L)
    std::atomic<int64_t> senders_and_close_status_;

    // Line 64: private val receivers = atomic(0L)
    mutable std::atomic<int64_t> receivers_;

    // Line 65: private val bufferEnd = atomic(initialBufferEnd(capacity))
    std::atomic<int64_t> buffer_end_;

    // Line 84: private val completedExpandBuffersAndPauseFlag = atomic(bufferEndCounter)
    std::atomic<int64_t> completed_expand_buffers_and_pause_flag_;

    // Line 89-91: Segment references
    std::atomic<ChannelSegment<E>*> send_segment_;
    mutable std::atomic<ChannelSegment<E>*> receive_segment_;
    std::atomic<ChannelSegment<E>*> buffer_end_segment_;

    // Line 1754: private val _closeCause = atomic<Any?>(NO_CLOSE_CAUSE)
    mutable std::atomic<void*> close_cause_;

    // Line 1778: private val closeHandler = atomic<Any?>(null)
    std::atomic<void*> close_handler_;

    // =========================================================================
    // Private helper methods
    // =========================================================================

    // Simplified sendImpl for trySend
    ChannelResult<void> send_impl_try_send(E element) {
        ChannelSegment<E>* segment = send_segment_.load(std::memory_order_acquire);

        while (true) {
            int64_t senders_and_close_status_cur = senders_and_close_status_.fetch_add(1, std::memory_order_acq_rel);
            int64_t s = channels::senders_counter(senders_and_close_status_cur);
            bool closed = is_closed_for_send_internal(senders_and_close_status_cur);

            int64_t id = s / SEGMENT_SIZE;
            int i = static_cast<int>(s % SEGMENT_SIZE);

            if (segment->id != id) {
                segment = find_segment_send(id, segment);
                if (segment == nullptr) {
                    if (closed) {
                        return ChannelResult<void>::closed(send_exception());
                    }
                    continue;
                }
            }

            int result = update_cell_send(segment, i, element, s,
                static_cast<void*>(&INTERRUPTED_SEND()), closed);

            switch (result) {
                case RESULT_RENDEZVOUS:
                    segment->clean_prev();
                    return ChannelResult<void>::success();
                case RESULT_BUFFERED:
                    return ChannelResult<void>::success();
                case RESULT_SUSPEND:
                    if (closed) {
                        segment->on_slot_cleaned();
                        return ChannelResult<void>::closed(send_exception());
                    }
                    segment->on_slot_cleaned();
                    return ChannelResult<void>::failure();
                case RESULT_CLOSED:
                    if (s < receivers_counter()) segment->clean_prev();
                    return ChannelResult<void>::closed(send_exception());
                case RESULT_FAILED:
                    segment->clean_prev();
                    continue;
                default:
                    continue;
            }
        }
    }

    // Simplified receiveImpl for tryReceive
    ChannelResult<E> receive_impl_try_receive() {
        ChannelSegment<E>* segment = receive_segment_.load(std::memory_order_acquire);

        while (true) {
            if (is_closed_for_receive()) {
                return ChannelResult<E>::closed(close_cause());
            }

            int64_t r = receivers_.fetch_add(1, std::memory_order_acq_rel);
            int64_t id = r / SEGMENT_SIZE;
            int i = static_cast<int>(r % SEGMENT_SIZE);

            if (segment->id != id) {
                segment = find_segment_receive(id, segment);
                if (segment == nullptr) continue;
            }

            void* result = update_cell_receive(segment, i, r, static_cast<void*>(&INTERRUPTED_RCV()));

            if (result == static_cast<void*>(&SUSPEND())) {
                // Emulate cancelled receive
                wait_expand_buffer_completion(r);
                segment->on_slot_cleaned();
                return ChannelResult<E>::failure();
            } else if (result == static_cast<void*>(&FAILED())) {
                if (r < senders_counter()) segment->clean_prev();
                continue;
            } else {
                segment->clean_prev();
                return ChannelResult<E>::success(*reinterpret_cast<E*>(result));
            }
        }
    }

    // Line 423-496: private fun updateCellSend(...)
    int update_cell_send(ChannelSegment<E>* segment, int index, E element,
                         int64_t s, void* waiter, bool closed) {
        // Fast path
        segment->store_element(index, std::move(element));
        if (closed) return update_cell_send_slow(segment, index, element, s, waiter, closed);

        void* state = segment->get_state(index);

        if (state == nullptr) {
            if (buffer_or_rendezvous_send(s)) {
                if (segment->cas_state(index, nullptr, static_cast<void*>(&BUFFERED()))) {
                    return RESULT_BUFFERED;
                }
            } else {
                if (waiter == nullptr) {
                    return RESULT_SUSPEND_NO_WAITER;
                }
                if (segment->cas_state(index, nullptr, waiter)) {
                    return RESULT_SUSPEND;
                }
            }
        }

        // Check if state is a waiter
        if (state != nullptr &&
            state != static_cast<void*>(&BUFFERED()) &&
            state != static_cast<void*>(&IN_BUFFER()) &&
            state != static_cast<void*>(&INTERRUPTED_RCV()) &&
            state != static_cast<void*>(&POISONED()) &&
            state != static_cast<void*>(&CHANNEL_CLOSED())) {
            segment->clean_element(index);
            if (try_resume_receiver(state, element)) {
                segment->set_state(index, static_cast<void*>(&DONE_RCV()));
                on_receive_dequeued();
                return RESULT_RENDEZVOUS;
            } else {
                if (segment->get_and_set_state(index, static_cast<void*>(&INTERRUPTED_RCV()))
                    != static_cast<void*>(&INTERRUPTED_RCV())) {
                    segment->on_cancelled_request(index, true);
                }
                return RESULT_FAILED;
            }
        }

        return update_cell_send_slow(segment, index, element, s, waiter, closed);
    }

    // Line 501-608: private fun updateCellSendSlow(...)
    int update_cell_send_slow(ChannelSegment<E>* segment, int index, E element,
                              int64_t s, void* waiter, bool closed) {
        while (true) {
            void* state = segment->get_state(index);

            if (state == nullptr) {
                if (buffer_or_rendezvous_send(s) && !closed) {
                    if (segment->cas_state(index, nullptr, static_cast<void*>(&BUFFERED()))) {
                        return RESULT_BUFFERED;
                    }
                } else {
                    if (closed) {
                        if (segment->cas_state(index, nullptr, static_cast<void*>(&INTERRUPTED_SEND()))) {
                            segment->on_cancelled_request(index, false);
                            return RESULT_CLOSED;
                        }
                    } else if (waiter == nullptr) {
                        return RESULT_SUSPEND_NO_WAITER;
                    } else {
                        if (segment->cas_state(index, nullptr, waiter)) {
                            return RESULT_SUSPEND;
                        }
                    }
                }
            } else if (state == static_cast<void*>(&IN_BUFFER())) {
                if (segment->cas_state(index, state, static_cast<void*>(&BUFFERED()))) {
                    return RESULT_BUFFERED;
                }
            } else if (state == static_cast<void*>(&INTERRUPTED_RCV())) {
                segment->clean_element(index);
                return RESULT_FAILED;
            } else if (state == static_cast<void*>(&POISONED())) {
                segment->clean_element(index);
                return RESULT_FAILED;
            } else if (state == static_cast<void*>(&CHANNEL_CLOSED())) {
                segment->clean_element(index);
                complete_close_or_cancel();
                return RESULT_CLOSED;
            } else {
                // state is Waiter or WaiterEB
                segment->clean_element(index);
                void* receiver = state;
                WaiterEB* waiter_eb = dynamic_cast<WaiterEB*>(reinterpret_cast<WaiterEB*>(state));
                if (waiter_eb) receiver = waiter_eb->waiter;

                if (try_resume_receiver(receiver, element)) {
                    segment->set_state(index, static_cast<void*>(&DONE_RCV()));
                    on_receive_dequeued();
                    return RESULT_RENDEZVOUS;
                } else {
                    if (segment->get_and_set_state(index, static_cast<void*>(&INTERRUPTED_RCV()))
                        != static_cast<void*>(&INTERRUPTED_RCV())) {
                        segment->on_cancelled_request(index, true);
                    }
                    return RESULT_FAILED;
                }
            }
        }
    }

    // Line 1006-1052: private fun updateCellReceive(...)
    void* update_cell_receive(ChannelSegment<E>* segment, int index, int64_t r, void* waiter) {
        // Fast path
        void* state = segment->get_state(index);

        if (state == nullptr) {
            int64_t senders = senders_counter();
            if (r >= senders) {
                if (waiter == nullptr) {
                    return static_cast<void*>(&SUSPEND_NO_WAITER());
                }
                if (segment->cas_state(index, nullptr, waiter)) {
                    expand_buffer();
                    return static_cast<void*>(&SUSPEND());
                }
            }
        } else if (state == static_cast<void*>(&BUFFERED())) {
            if (segment->cas_state(index, state, static_cast<void*>(&DONE_RCV()))) {
                expand_buffer();
                E elem = segment->retrieve_element(index);
                return new E(std::move(elem));
            }
        }

        return update_cell_receive_slow(segment, index, r, waiter);
    }

    // Line 1054-1165: private fun updateCellReceiveSlow(...)
    void* update_cell_receive_slow(ChannelSegment<E>* segment, int index, int64_t r, void* waiter) {
        while (true) {
            void* state = segment->get_state(index);

            if (state == nullptr || state == static_cast<void*>(&IN_BUFFER())) {
                int64_t senders = senders_counter();
                if (r < senders) {
                    if (segment->cas_state(index, state, static_cast<void*>(&POISONED()))) {
                        expand_buffer();
                        return static_cast<void*>(&FAILED());
                    }
                } else {
                    if (waiter == nullptr) {
                        return static_cast<void*>(&SUSPEND_NO_WAITER());
                    }
                    if (segment->cas_state(index, state, waiter)) {
                        expand_buffer();
                        return static_cast<void*>(&SUSPEND());
                    }
                }
            } else if (state == static_cast<void*>(&BUFFERED())) {
                if (segment->cas_state(index, state, static_cast<void*>(&DONE_RCV()))) {
                    expand_buffer();
                    E elem = segment->retrieve_element(index);
                    return new E(std::move(elem));
                }
            } else if (state == static_cast<void*>(&INTERRUPTED_SEND())) {
                return static_cast<void*>(&FAILED());
            } else if (state == static_cast<void*>(&POISONED())) {
                return static_cast<void*>(&FAILED());
            } else if (state == static_cast<void*>(&CHANNEL_CLOSED())) {
                expand_buffer();
                return static_cast<void*>(&FAILED());
            } else if (state == static_cast<void*>(&RESUMING_BY_EB())) {
                continue; // spin wait
            } else {
                // state is a sender
                if (segment->cas_state(index, state, static_cast<void*>(&RESUMING_BY_RCV()))) {
                    bool help_expand_buffer = (dynamic_cast<WaiterEB*>(reinterpret_cast<WaiterEB*>(state)) != nullptr);
                    void* sender = state;
                    WaiterEB* waiter_eb = dynamic_cast<WaiterEB*>(reinterpret_cast<WaiterEB*>(state));
                    if (waiter_eb) sender = waiter_eb->waiter;

                    if (try_resume_sender(sender, segment, index)) {
                        segment->set_state(index, static_cast<void*>(&DONE_RCV()));
                        expand_buffer();
                        E elem = segment->retrieve_element(index);
                        return new E(std::move(elem));
                    } else {
                        segment->set_state(index, static_cast<void*>(&INTERRUPTED_SEND()));
                        segment->on_cancelled_request(index, false);
                        if (help_expand_buffer) expand_buffer();
                        return static_cast<void*>(&FAILED());
                    }
                }
            }
        }
    }

    // Line 652-669: private fun Any.tryResumeReceiver(element: E): Boolean
    bool try_resume_receiver(void* receiver, E element) {
        // Simplified - full implementation would handle different receiver types
        (void)receiver;
        (void)element;
        return true;
    }

    // Line 1167-1184: private fun Any.tryResumeSender(...)
    bool try_resume_sender(void* sender, ChannelSegment<E>* segment, int index) {
        // Simplified - full implementation would handle different sender types
        (void)sender;
        (void)segment;
        (void)index;
        return true;
    }

    // Line 678: protected open fun onReceiveEnqueued() {}
    virtual void on_receive_enqueued() {}

    // Line 684: protected open fun onReceiveDequeued() {}
    virtual void on_receive_dequeued() {}

    // Line 1841-1857: private fun invokeCloseHandler()
    void invoke_close_handler_internal() {
        void* handler = close_handler_.exchange(static_cast<void*>(&CLOSE_HANDLER_INVOKED()),
            std::memory_order_acq_rel);

        if (handler == nullptr) {
            close_handler_.store(static_cast<void*>(&CLOSE_HANDLER_CLOSED()), std::memory_order_release);
            return;
        }

        if (handler == static_cast<void*>(&CLOSE_HANDLER_CLOSED()) ||
            handler == static_cast<void*>(&CLOSE_HANDLER_INVOKED())) {
            return;
        }

        auto* fn = reinterpret_cast<std::function<void(std::exception_ptr)>*>(handler);
        (*fn)(close_cause());
        delete fn;
    }

    // Line 1943-1967: private fun completeClose(sendersCur: Long): ChannelSegment<E>
    ChannelSegment<E>* complete_close(int64_t senders_cur) {
        ChannelSegment<E>* last_segment = close_linked_list();

        if (is_conflated_drop_oldest()) {
            int64_t last_buffered_cell = mark_all_empty_cells_as_closed(last_segment);
            if (last_buffered_cell != -1) {
                drop_first_element_until_the_specified_cell_is_in_the_buffer(last_buffered_cell);
            }
        }

        cancel_suspended_receive_requests(last_segment, senders_cur);
        return last_segment;
    }

    // Line 1972-1979: private fun completeCancel(sendersCur: Long)
    void complete_cancel(int64_t senders_cur) {
        ChannelSegment<E>* last_segment = complete_close(senders_cur);
        remove_unprocessed_elements(last_segment);
    }

    // Line 1984-1992: private fun closeLinkedList(): ChannelSegment<E>
    ChannelSegment<E>* close_linked_list() {
        ChannelSegment<E>* last_segment = buffer_end_segment_.load(std::memory_order_acquire);
        ChannelSegment<E>* send_seg = send_segment_.load(std::memory_order_acquire);
        ChannelSegment<E>* recv_seg = receive_segment_.load(std::memory_order_acquire);

        if (send_seg->id > last_segment->id) last_segment = send_seg;
        if (recv_seg->id > last_segment->id) last_segment = recv_seg;

        return last_segment->close();
    }

    // Line 2003-2033: private fun markAllEmptyCellsAsClosed(...)
    int64_t mark_all_empty_cells_as_closed(ChannelSegment<E>* last_segment) {
        ChannelSegment<E>* segment = last_segment;
        while (true) {
            for (int index = SEGMENT_SIZE - 1; index >= 0; --index) {
                int64_t global_index = segment->id * SEGMENT_SIZE + index;
                if (global_index < receivers_counter()) return -1;

                while (true) {
                    void* state = segment->get_state(index);
                    if (state == nullptr || state == static_cast<void*>(&IN_BUFFER())) {
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            segment->on_slot_cleaned();
                            break;
                        }
                    } else if (state == static_cast<void*>(&BUFFERED())) {
                        return global_index;
                    } else {
                        break;
                    }
                }
            }
            segment = segment->prev();
            if (segment == nullptr) return -1;
        }
    }

    // Line 826-873: protected fun dropFirstElementUntilTheSpecifiedCellIsInTheBuffer(...)
    void drop_first_element_until_the_specified_cell_is_in_the_buffer(int64_t global_cell_index) {
        assert(is_conflated_drop_oldest());
        // Implementation would mirror Kotlin logic
        (void)global_cell_index;
    }

    // Line 2041-2134: private fun removeUnprocessedElements(...)
    void remove_unprocessed_elements(ChannelSegment<E>* last_segment) {
        // Simplified - full implementation would remove elements and cancel senders
        (void)last_segment;
    }

    // Line 2140-2187: private fun cancelSuspendedReceiveRequests(...)
    void cancel_suspended_receive_requests(ChannelSegment<E>* last_segment, int64_t senders_counter) {
        // Simplified - full implementation would cancel all suspended receivers
        (void)last_segment;
        (void)senders_counter;
    }

    // Line 2393-2432: private fun findSegmentSend(...)
    ChannelSegment<E>* find_segment_send(int64_t id, ChannelSegment<E>* start_from) {
        // Simplified segment finding
        ChannelSegment<E>* segment = start_from;
        while (segment != nullptr && segment->id < id) {
            ChannelSegment<E>* next = segment->next();
            if (next == nullptr) {
                next = create_segment<E>(segment->id + 1, segment);
                if (!segment->try_set_next(next)) {
                    delete next;
                    next = segment->next();
                }
            }
            segment = next;
        }

        if (segment != nullptr) {
            ChannelSegment<E>* expected = send_segment_.load(std::memory_order_acquire);
            while (expected->id < segment->id) {
                if (send_segment_.compare_exchange_weak(expected, segment,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    break;
                }
            }
        }

        return segment;
    }

    // Line 2448-2491: private fun findSegmentReceive(...)
    ChannelSegment<E>* find_segment_receive(int64_t id, ChannelSegment<E>* start_from) {
        ChannelSegment<E>* segment = start_from;
        while (segment != nullptr && segment->id < id) {
            ChannelSegment<E>* next = segment->next();
            if (next == nullptr) {
                next = create_segment<E>(segment->id + 1, segment);
                if (!segment->try_set_next(next)) {
                    delete next;
                    next = segment->next();
                }
            }
            segment = next;
        }

        if (segment != nullptr) {
            ChannelSegment<E>* expected = receive_segment_.load(std::memory_order_acquire);
            while (expected->id < segment->id) {
                if (receive_segment_.compare_exchange_weak(expected, segment,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    break;
                }
            }
        }

        return segment;
    }

    // Line 2497-2535: private fun findSegmentBufferEnd(...)
    ChannelSegment<E>* find_segment_buffer_end(int64_t id, ChannelSegment<E>* start_from, int64_t current_buffer_end_counter) {
        (void)current_buffer_end_counter;

        ChannelSegment<E>* segment = start_from;
        while (segment != nullptr && segment->id < id) {
            ChannelSegment<E>* next = segment->next();
            if (next == nullptr) {
                next = create_segment<E>(segment->id + 1, segment);
                if (!segment->try_set_next(next)) {
                    delete next;
                    next = segment->next();
                }
            }
            segment = next;
        }

        if (segment != nullptr) {
            ChannelSegment<E>* expected = buffer_end_segment_.load(std::memory_order_acquire);
            while (expected->id < segment->id) {
                if (buffer_end_segment_.compare_exchange_weak(expected, segment,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    break;
                }
            }
        }

        return segment;
    }

    // Line 2543-2561: private fun moveSegmentBufferEndToSpecifiedOrLast(...)
    void move_segment_buffer_end_to_specified_or_last(int64_t id, ChannelSegment<E>* start_from) {
        ChannelSegment<E>* segment = start_from;
        while (segment->id < id) {
            ChannelSegment<E>* next = segment->next();
            if (next == nullptr) break;
            segment = next;
        }

        while (true) {
            while (segment->is_removed()) {
                ChannelSegment<E>* next = segment->next();
                if (next == nullptr) break;
                segment = next;
            }

            ChannelSegment<E>* expected = buffer_end_segment_.load(std::memory_order_acquire);
            if (expected->id >= segment->id) return;
            if (buffer_end_segment_.compare_exchange_weak(expected, segment,
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                return;
            }
        }
    }

    // =========================================================================
    // Line 1595-1744: BufferedChannelIterator inner class
    // =========================================================================

    class BufferedChannelIterator : public ChannelIterator<E> {
    public:
        BufferedChannelIterator(BufferedChannel<E>* channel)
            : channel_(channel)
            , receive_result_(static_cast<void*>(&NO_RECEIVE_RESULT()))
            , continuation_(nullptr) {}

        // Line 1616-1640: override suspend fun hasNext(): Boolean
        void* has_next(Continuation<void*>* continuation) override {
            if (receive_result_ != static_cast<void*>(&NO_RECEIVE_RESULT()) &&
                receive_result_ != static_cast<void*>(&CHANNEL_CLOSED())) {
                return new bool(true);  // Boxing result
            }

            auto result = channel_->try_receive();
            if (result.is_success()) {
                receive_result_ = new E(result.get_or_throw());
                return new bool(true);
            }
            if (result.is_closed()) {
                receive_result_ = static_cast<void*>(&CHANNEL_CLOSED());
                auto cause = channel_->close_cause();
                if (cause) std::rethrow_exception(cause);
                return new bool(false);
            }

            // Would need to suspend - for now return false
            (void)continuation;
            return new bool(false);
        }

        // Line 1698-1707: override fun next(): E
        E next() override {
            if (receive_result_ == static_cast<void*>(&NO_RECEIVE_RESULT())) {
                throw std::logic_error("`hasNext()` has not been invoked");
            }

            void* result = receive_result_;
            receive_result_ = static_cast<void*>(&NO_RECEIVE_RESULT());

            if (result == static_cast<void*>(&CHANNEL_CLOSED())) {
                auto cause = channel_->close_cause();
                if (cause) std::rethrow_exception(cause);
                throw ClosedReceiveChannelException("Channel was closed");
            }

            E* elem_ptr = reinterpret_cast<E*>(result);
            E elem = std::move(*elem_ptr);
            delete elem_ptr;
            return elem;
        }

    private:
        BufferedChannel<E>* channel_;
        void* receive_result_;
        CancellableContinuationImpl<bool>* continuation_;
    };
};

// ============================================================================
// Line 2946-2960: tryResume0 extension function
// ============================================================================

// private fun <T> CancellableContinuation<T>.tryResume0(...): Boolean
template <typename T>
bool try_resume_0(CancellableContinuation<T>* cont, T value) {
    // Simplified implementation
    (void)cont;
    (void)value;
    return true;
}

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
