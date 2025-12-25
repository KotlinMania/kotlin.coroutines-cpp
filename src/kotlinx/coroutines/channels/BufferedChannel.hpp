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
#include "kotlinx/coroutines/Waiter.hpp"
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
#include <array>

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
// Type checking for cell contents
// ============================================================================
// Cells can contain:
// - Symbol pointers (BUFFERED, IN_BUFFER, CHANNEL_CLOSED, DONE_RCV, etc.)
// - Waiter* (CancellableContinuation, SelectInstance, etc.)
// - WaiterEB* (wrapper for waiters during expandBuffer)
//
// To distinguish WaiterEB from Waiter, WaiterEB stores a magic marker.
// This avoids the need for RTTI on void* casts.

// Magic marker stored in WaiterEB to identify it
constexpr uintptr_t WAITER_EB_MAGIC = 0xEB'EB'EB'EB'EB'EB'EB'EBULL;

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
    // Magic marker to distinguish WaiterEB from Waiter in type-erased storage
    const uintptr_t magic = WAITER_EB_MAGIC;
    // @JvmField val waiter: Waiter
    Waiter* waiter;

    explicit WaiterEB(Waiter* w) : waiter(w) {}

    std::string to_string() const {
        std::ostringstream oss;
        oss << "WaiterEB(" << static_cast<void*>(waiter) << ")";
        return oss.str();
    }
};

// Check if state is not a known symbol (i.e., might be a waiter)
inline bool is_waiter_type(void* state) {
    // If state is nullptr or matches a known symbol, it's not a waiter
    if (state == nullptr) return false;
    if (state == &DONE_RCV()) return false;
    if (state == &BUFFERED()) return false;
    if (state == &IN_BUFFER()) return false;
    if (state == &CHANNEL_CLOSED()) return false;
    if (state == &INTERRUPTED_RCV()) return false;
    if (state == &INTERRUPTED_SEND()) return false;
    if (state == &RESUMING_BY_EB()) return false;
    if (state == &RESUMING_BY_RCV()) return false;
    if (state == &FAILED()) return false;
    if (state == &SUSPEND()) return false;
    if (state == &SUSPEND_NO_WAITER()) return false;
    // Otherwise it's a waiter type (either Waiter* or WaiterEB*)
    return true;
}

// Check if a pointer (known to be a waiter type) is WaiterEB
inline bool is_waiter_eb(void* state) {
    if (!is_waiter_type(state)) return false;
    // Check magic marker at the start of the object
    auto* candidate = static_cast<WaiterEB*>(state);
    return candidate->magic == WAITER_EB_MAGIC;
}

// Check if a pointer is a plain Waiter (not WaiterEB)
inline bool is_waiter(void* state) {
    return is_waiter_type(state) && !is_waiter_eb(state);
}

// Extract the Waiter* from state (handles both Waiter* and WaiterEB*)
inline Waiter* get_waiter(void* state) {
    if (is_waiter_eb(state)) {
        return static_cast<WaiterEB*>(state)->waiter;
    }
    return static_cast<Waiter*>(state);
}

// ============================================================================
// Lines 3024-3031: ReceiveCatching wrapper class
// ============================================================================

/**
 * To distinguish suspended receive() and receiveCatching() operations,
 * the latter uses this wrapper for its continuation.
 */
template <typename E>
class ReceiveCatching : public Waiter {
public:
    // @JvmField val cont: CancellableContinuationImpl<ChannelResult<E>>
    CancellableContinuationImpl<ChannelResult<E>>* cont;

    explicit ReceiveCatching(CancellableContinuationImpl<ChannelResult<E>>* c) : cont(c) {}

    // Waiter interface delegation
    void invoke_on_cancellation(internal::SegmentBase* segment, int index) override {
        cont->invoke_on_cancellation(segment, index);
    }
};

// ============================================================================
// Lines 234-236: SendBroadcast wrapper class
// ============================================================================

/**
 * Used by BroadcastChannel to wrap send continuations.
 * Implements Waiter by delegating to the underlying CancellableContinuationImpl.
 */
class SendBroadcast : public Waiter {
public:
    // @JvmField val cont: CancellableContinuation<Boolean>
    CancellableContinuationImpl<bool>* cont;

    explicit SendBroadcast(CancellableContinuationImpl<bool>* c) : cont(c) {}

    // Waiter interface delegation
    void invoke_on_cancellation(internal::SegmentBase* segment, int index) override {
        cont->invoke_on_cancellation(segment, index);
    }
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

    // C++ lifetime management: holds shared_ptr to waiters stored in state slots.
    // In Kotlin, GC keeps waiters alive. In C++, we need explicit ownership.
    // The raw void* in data_[] is used for CAS operations; this array keeps the object alive.
    std::array<std::shared_ptr<Waiter>, SEGMENT_SIZE> waiter_refs_;

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

    // ##################################
    // # C++ Waiter Lifetime Management #
    // ##################################

    // Store a shared_ptr to keep the waiter alive while its raw pointer is in the state slot.
    // Call this when storing a waiter in the segment state.
    void set_waiter_ref(int index, std::shared_ptr<Waiter> waiter) {
        waiter_refs_[index] = std::move(waiter);
    }

    // Clear the waiter ref when the waiter is no longer needed (resumed, cancelled, etc.)
    void clear_waiter_ref(int index) {
        waiter_refs_[index].reset();
    }

    // Get the waiter ref (for cases where we need to pass it on)
    std::shared_ptr<Waiter> get_waiter_ref(int index) const {
        return waiter_refs_[index];
    }

    // ########################
    // # Cancellation Support #
    // ########################

    // Line 2849-2911: override fun onCancellation(index: Int, cause: Throwable?, context: CoroutineContext)
    void on_cancellation(int index, std::exception_ptr cause,
                         std::shared_ptr<CoroutineContext> context) override {
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
                    // C++ lifetime: release the waiter ref since we're done with it
                    clear_waiter_ref(index);
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
// Line 2946-2960: tryResume0 extension function
// ============================================================================

// private fun <T> CancellableContinuation<T>.tryResume0(
//     value: T,
//     onCancellation: ((cause: Throwable, value: T, context: CoroutineContext) -> Unit)? = null
// ): Boolean
//
// Note: Uses CancellableContinuationImpl because the 3-argument try_resume is only there.
template <typename T>
bool try_resume_0(
    CancellableContinuationImpl<T>* cont,
    T value,
    std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)> on_cancellation = nullptr
) {
    // Line 2955: tryResume(value, null, onCancellation).let { token ->
    void* token = cont->try_resume(value, nullptr, on_cancellation);
    // Line 2956-2959: if (token != null) { completeResume(token); true } else false
    if (token != nullptr) {
        cont->complete_resume(token);
        return true;
    }
    return false;
}

// Overload for ChannelResult<T> which needs special handling
template <typename T>
bool try_resume_0(
    CancellableContinuationImpl<ChannelResult<T>>* cont,
    ChannelResult<T> value,
    std::function<void(std::exception_ptr, ChannelResult<T>, std::shared_ptr<CoroutineContext>)> on_cancellation = nullptr
) {
    void* token = cont->try_resume(value, nullptr, on_cancellation);
    if (token != nullptr) {
        cont->complete_resume(token);
        return true;
    }
    return false;
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
        // Lines 241-348: sendImpl inline function
        // Line 270: var segment = sendSegment.value
        ChannelSegment<E>* segment = send_segment_.load(std::memory_order_acquire);

        while (true) {
            // Line 274: val sendersAndCloseStatusCur = sendersAndCloseStatus.getAndIncrement()
            int64_t senders_and_close_status_cur = senders_and_close_status_.fetch_add(1, std::memory_order_acq_rel);
            // Line 275: val s = sendersAndCloseStatusCur.sendersCounter
            int64_t s = channels::senders_counter(senders_and_close_status_cur);
            // Line 277: val closed = sendersAndCloseStatusCur.isClosedForSend0
            bool closed = is_closed_for_send_internal(senders_and_close_status_cur);
            // Line 279-280: val id = s / SEGMENT_SIZE; val i = (s % SEGMENT_SIZE).toInt()
            int64_t id = s / SEGMENT_SIZE;
            int i = static_cast<int>(s % SEGMENT_SIZE);

            // Line 283-297: if (segment.id != id) { segment = findSegmentSend(...) }
            if (segment->id != id) {
                segment = find_segment_send(id, segment);
                if (segment == nullptr) {
                    // Line 292-296: if (closed) return onClosed() else continue
                    if (closed) {
                        // Line 123: onClosed = { onClosedSend(element) }
                        return on_closed_send(element, std::shared_ptr<Continuation<void*>>(
                            completion, [](Continuation<void*>*){}));
                    } else {
                        continue;
                    }
                }
            }

            // Line 301: when (updateCellSend(segment, i, element, s, waiter, closed))
            // Line 114: waiter = null (no waiter yet for send())
            int result = update_cell_send(segment, i, element, s, nullptr, closed);

            switch (result) {
                case RESULT_RENDEZVOUS:
                    // Line 302-308: segment.cleanPrev(); return onRendezvousOrBuffered()
                    segment->clean_prev();
                    // Line 117: onRendezvousOrBuffered = {} (just return Unit)
                    return nullptr;

                case RESULT_BUFFERED:
                    // Line 310-312: return onRendezvousOrBuffered()
                    return nullptr;

                case RESULT_SUSPEND:
                    // Line 314-324: This shouldn't happen with waiter=null
                    // but if closed, we installed INTERRUPTED_SEND
                    if (closed) {
                        segment->on_slot_cleaned();
                        // Line 123: onClosed = { onClosedSend(element) }
                        return on_closed_send(element, std::shared_ptr<Continuation<void*>>(
                            completion, [](Continuation<void*>*){}));
                    }
                    // Line 119: onSuspend = { _, _ -> assert { false } }
                    assert(false && "RESULT_SUSPEND with null waiter");
                    return nullptr;

                case RESULT_CLOSED:
                    // Line 326-332: if (s < receiversCounter) segment.cleanPrev(); return onClosed()
                    if (s < receivers_counter()) segment->clean_prev();
                    return on_closed_send(element, std::shared_ptr<Continuation<void*>>(
                        completion, [](Continuation<void*>*){}));

                case RESULT_FAILED:
                    // Line 334-339: segment.cleanPrev(); continue
                    segment->clean_prev();
                    continue;

                case RESULT_SUSPEND_NO_WAITER:
                    // Line 341-344: return onNoWaiterSuspend(segment, i, element, s)
                    // Line 127: onNoWaiterSuspend = { segm, i, elem, s -> sendOnNoWaiterSuspend(...) }
                    return send_on_no_waiter_suspend(segment, i, element, s, completion);

                default:
                    assert(false && "unexpected result from update_cell_send");
                    return nullptr;
            }
        }
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
        // Line 219-221: check(onUndeliveredElement == null)
        if (on_undelivered_element_) {
            throw std::logic_error("the `onUndeliveredElement` feature is unsupported for `sendBroadcast(e)`");
        }

        // Line 222-228: sendImpl with SendBroadcast waiter
        // Create a CancellableContinuationImpl for the result
        auto cont = std::make_shared<CancellableContinuationImpl<bool>>(
            std::dynamic_pointer_cast<Continuation<bool>>(
                std::shared_ptr<Continuation<void*>>(continuation, [](Continuation<void*>*){})
            ),
            MODE_CANCELLABLE
        );

        auto* waiter = new SendBroadcast(cont.get());

        // Get segment and counter
        int64_t senders_and_close_status_cur = senders_and_close_status_.fetch_add(1, std::memory_order_acq_rel);
        int64_t s = channels::senders_counter(senders_and_close_status_cur);
        bool closed = is_closed_for_send_internal(senders_and_close_status_cur);

        int64_t id = s / SEGMENT_SIZE;
        int i = static_cast<int>(s % SEGMENT_SIZE);

        ChannelSegment<E>* segment = send_segment_.load(std::memory_order_acquire);
        if (segment->id != id) {
            segment = find_segment_send(id, segment);
            if (segment == nullptr) {
                if (closed) {
                    // Line 227: onClosed = { cont.resume(false) }
                    delete waiter;
                    cont->resume_with(Result<bool>::success(false));
                    return COROUTINE_SUSPENDED;
                }
            }
        }

        int result = update_cell_send(segment, i, element, s, waiter, closed);
        switch (result) {
            case RESULT_RENDEZVOUS:
            case RESULT_BUFFERED:
                // Line 225: onRendezvousOrBuffered = { cont.resume(true) }
                delete waiter;
                cont->resume_with(Result<bool>::success(true));
                return COROUTINE_SUSPENDED;
            case RESULT_SUSPEND:
                // Line 226: onSuspend = { _, _ -> }
                if (!closed) {
                    prepare_sender_for_suspension(waiter, segment, i);
                }
                return COROUTINE_SUSPENDED;
            case RESULT_CLOSED:
                // Line 227: onClosed = { cont.resume(false) }
                delete waiter;
                cont->resume_with(Result<bool>::success(false));
                return COROUTINE_SUSPENDED;
            default:
                // RESULT_FAILED - retry
                delete waiter;
                return send_broadcast(element, continuation);
        }
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
                    auto* wrapper = new WaiterEB(static_cast<Waiter*>(state));
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
    // `open` in Kotlin means virtual in C++
    virtual bool cancel_impl(std::exception_ptr cause) {
        if (!cause) {
            cause = std::make_exception_ptr(std::runtime_error("Channel was cancelled"));
        }
        return close_or_cancel_impl(cause, true);
    }

    // Line 1816-1835: protected open fun closeOrCancelImpl(...)
    // `open` in Kotlin means virtual in C++
    virtual bool close_or_cancel_impl(std::exception_ptr cause, bool cancel) {
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

    // -------------------------------------------------------------------------
    // Lines 352-371: protected fun trySendDropOldest(element: E): ChannelResult<Unit>
    // Note: temporarily in BufferedChannel due to KT-65554
    // -------------------------------------------------------------------------
    ChannelResult<void> try_send_drop_oldest(E element) {
        // Line 353-370: sendImpl with BUFFERED as waiter
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
                        // Line 370: onClosed = { return closed(sendException) }
                        return ChannelResult<void>::closed(send_exception());
                    }
                    continue;
                }
            }

            // Line 358: waiter = BUFFERED
            int result = update_cell_send(segment, i, element, s,
                static_cast<void*>(&BUFFERED()), closed);

            switch (result) {
                case RESULT_RENDEZVOUS:
                    segment->clean_prev();
                    // Line 361: onRendezvousOrBuffered = { return success(Unit) }
                    return ChannelResult<void>::success();
                case RESULT_BUFFERED:
                    // Line 361: onRendezvousOrBuffered = { return success(Unit) }
                    return ChannelResult<void>::success();
                case RESULT_SUSPEND:
                    // Line 365-367: onSuspend - buffer overflowed, drop oldest
                    drop_first_element_until_the_specified_cell_is_in_the_buffer(
                        segment->id * SEGMENT_SIZE + i);
                    // Line 367: return success(Unit)
                    return ChannelResult<void>::success();
                case RESULT_CLOSED:
                    if (s < receivers_counter()) segment->clean_prev();
                    // Line 370: onClosed = { return closed(sendException) }
                    return ChannelResult<void>::closed(send_exception());
                case RESULT_FAILED:
                    segment->clean_prev();
                    continue;
                default:
                    continue;
            }
        }
    }

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

    // -------------------------------------------------------------------------
    // Lines 131-139: private suspend fun onClosedSend(element: E): Unit
    // NB: return type could've been Nothing, but it breaks TCO
    // -------------------------------------------------------------------------
    void* on_closed_send(E element, std::shared_ptr<Continuation<void*>> completion) {
        // Line 131: suspendCancellableCoroutine { continuation ->
        // Line 132: onUndeliveredElement?.callUndeliveredElementCatchingException(element)?.let {
        std::exception_ptr ex = call_undelivered_element_catching_exception(element);
        if (ex) {
            // Line 133-135: If it crashes, add send exception as suppressed for better diagnostics
            // TODO(port): C++ doesn't have addSuppressed - just use the exception as-is
            // Line 135: continuation.resumeWithStackTrace(it)
            completion->resume_with(Result<void*>::failure(ex));
            return COROUTINE_SUSPENDED;
        }
        // Line 138: continuation.resumeWithStackTrace(sendException)
        completion->resume_with(Result<void*>::failure(send_exception()));
        return COROUTINE_SUSPENDED;
    }

    // -------------------------------------------------------------------------
    // Lines 141-164: private suspend fun sendOnNoWaiterSuspend(...)
    // -------------------------------------------------------------------------
    void* send_on_no_waiter_suspend(
        ChannelSegment<E>* segment,
        int index,
        E element,
        int64_t s,
        Continuation<void*>* completion
    ) {
        // Line 150: suspendCancellableCoroutineReusable sc@{ cont ->
        // Create a CancellableContinuationImpl like suspendCancellableCoroutineReusable does
        // Note: completion is Continuation<void*>*, wrap in shared_ptr with no-op deleter
        auto completion_wrapper = std::shared_ptr<Continuation<void>>(
            reinterpret_cast<Continuation<void>*>(completion),
            [](Continuation<void>*){} // no-op deleter, we don't own completion
        );
        auto cont = std::make_shared<CancellableContinuationImpl<void>>(
            completion_wrapper, MODE_CANCELLABLE_REUSABLE
        );

        // Line 151-163: sendImplOnNoWaiter(...)
        send_impl_on_no_waiter(
            segment, index, element, s,
            // Line 154: waiter = cont
            cont.get(),
            // Line 159: onRendezvousOrBuffered = { cont.resume(Unit) }
            [cont]() { cont->resume({}); },
            // Line 162: onClosed = { onClosedSendOnNoWaiterSuspend(element, cont) }
            [this, element, cont]() { on_closed_send_on_no_waiter_suspend(element, cont.get()); }
        );
        return COROUTINE_SUSPENDED;
    }

    // -------------------------------------------------------------------------
    // Lines 166-176: private fun Waiter.prepareSenderForSuspension(...)
    // -------------------------------------------------------------------------
    void prepare_sender_for_suspension(Waiter* waiter, ChannelSegment<E>* segment, int index) {
        // C++ lifetime management: store the shared_ptr to keep the waiter alive
        // while it's stored in the segment. The waiter provides its own shared_ptr
        // via shared_from_this_waiter() if it supports shared ownership.
        if (auto sp = waiter->shared_from_this_waiter()) {
            segment->set_waiter_ref(index, sp);
        }
        // Line 174-175: To distinguish cancelled senders and receivers,
        // senders equip the index value with an additional marker,
        // adding SEGMENT_SIZE to the value.
        waiter->invoke_on_cancellation(segment, index + SEGMENT_SIZE);
    }

    // -------------------------------------------------------------------------
    // Lines 178-181: private fun onClosedSendOnNoWaiterSuspend(...)
    // -------------------------------------------------------------------------
    void on_closed_send_on_no_waiter_suspend(E element, CancellableContinuation<void>* cont) {
        // Line 179: onUndeliveredElement?.callUndeliveredElement(element, cont.context)
        // Note: C++ OnUndeliveredElement takes only the element, context is implicit
        if (on_undelivered_element_) {
            on_undelivered_element_(element);
        }
        // Line 180: cont.resumeWithException(recoverStackTrace(sendException, cont))
        cont->resume_with(Result<void>::failure(send_exception()));
    }

    // -------------------------------------------------------------------------
    // Lines 735-738: private fun Waiter.prepareReceiverForSuspension(...)
    // -------------------------------------------------------------------------
    void prepare_receiver_for_suspension(Waiter* waiter, ChannelSegment<E>* segment, int index) {
        // C++ lifetime management: store the shared_ptr to keep the waiter alive
        if (auto sp = waiter->shared_from_this_waiter()) {
            segment->set_waiter_ref(index, sp);
        }
        // Line 736: onReceiveEnqueued()
        on_receive_enqueued();
        // Line 737: invokeOnCancellation(segment, index)
        waiter->invoke_on_cancellation(segment, index);
    }

    // -------------------------------------------------------------------------
    // Lines 740-742: private fun onClosedReceiveOnNoWaiterSuspend(...)
    // -------------------------------------------------------------------------
    template<typename T>
    void on_closed_receive_on_no_waiter_suspend(CancellableContinuation<T>* cont) {
        // Line 741: cont.resumeWithException(receiveException)
        cont->resume_with(Result<T>::failure(receive_exception()));
    }

    // -------------------------------------------------------------------------
    // Lines 778-780: private fun onClosedReceiveCatchingOnNoWaiterSuspend(...)
    // -------------------------------------------------------------------------
    void on_closed_receive_catching_on_no_waiter_suspend(CancellableContinuation<ChannelResult<E>>* cont) {
        // Line 779: cont.resume(closed(closeCause))
        cont->resume_with(Result<ChannelResult<E>>::success(ChannelResult<E>::closed(close_cause())));
    }

    // -------------------------------------------------------------------------
    // Lines 963-1004: private inline fun receiveImplOnNoWaiter(...)
    // -------------------------------------------------------------------------
    void receive_impl_on_no_waiter(
        ChannelSegment<E>* segment,
        int index,
        int64_t r,
        Waiter* waiter,
        std::function<void(E)> on_element_retrieved,
        std::function<void()> on_closed
    ) {
        // Line 984: val updCellResult = updateCellReceive(segment, index, r, waiter)
        void* upd_cell_result = update_cell_receive(segment, index, r, waiter);

        // Line 985-1003: when { ... }
        if (upd_cell_result == static_cast<void*>(&SUSPEND())) {
            // Line 987: waiter.prepareReceiverForSuspension(segment, index)
            prepare_receiver_for_suspension(waiter, segment, index);
        } else if (upd_cell_result == static_cast<void*>(&FAILED())) {
            // Line 990: if (r < sendersCounter) segment.cleanPrev()
            if (r < senders_counter()) segment->clean_prev();
            // Line 991-996: receiveImpl(waiter, onElementRetrieved, ..., onClosed)
            receive_impl_with_waiter(waiter, on_element_retrieved, on_closed);
        } else {
            // Line 999: segment.cleanPrev()
            segment->clean_prev();
            // Line 1001: onElementRetrieved(updCellResult as E)
            on_element_retrieved(*static_cast<E*>(upd_cell_result));
        }
    }

    // -------------------------------------------------------------------------
    // Helper for receive_impl_on_no_waiter RESULT_FAILED case
    // -------------------------------------------------------------------------
    void receive_impl_with_waiter(
        Waiter* waiter,
        std::function<void(E)> on_element_retrieved,
        std::function<void()> on_closed
    ) {
        // Increment receivers counter and get segment/index
        int64_t r = receivers_.fetch_add(1, std::memory_order_acq_rel);
        int64_t id = r / SEGMENT_SIZE;
        int index = static_cast<int>(r % SEGMENT_SIZE);

        ChannelSegment<E>* segment = find_segment_receive(id, receive_segment_.load(std::memory_order_acquire));
        if (segment == nullptr) {
            // Channel closed
            on_closed();
            return;
        }

        // Try again with new cell
        receive_impl_on_no_waiter(segment, index, r, waiter, on_element_retrieved, on_closed);
    }

    // =========================================================================
    // Lines 1470-1567: Select Expression Support
    // =========================================================================

    // -------------------------------------------------------------------------
    // Lines 1493-1496: private fun onClosedSelectOnSend(element: E, select: SelectInstance<*>)
    // -------------------------------------------------------------------------
    void on_closed_select_on_send(E element, selects::SelectInstance<void*>* select) {
        // Line 1494: onUndeliveredElement?.callUndeliveredElement(element, select.context)
        if (on_undelivered_element_) {
            on_undelivered_element_(element);
        }
        // Line 1495: select.selectInRegistrationPhase(CHANNEL_CLOSED)
        select->select_in_registration_phase(static_cast<void*>(&CHANNEL_CLOSED()));
    }

    // -------------------------------------------------------------------------
    // Lines 1499-1501: private fun processResultSelectSend(ignoredParam: Any?, selectResult: Any?): Any?
    // -------------------------------------------------------------------------
    void* process_result_select_send(void* /*ignored_param*/, void* select_result) {
        // Line 1500: if (selectResult === CHANNEL_CLOSED) throw sendException
        if (select_result == static_cast<void*>(&CHANNEL_CLOSED())) {
            std::rethrow_exception(send_exception());
        }
        // Line 1501: else this
        return static_cast<void*>(this);
    }

    // -------------------------------------------------------------------------
    // Lines 1483-1490: protected open fun registerSelectForSend(select: SelectInstance<*>, element: Any?)
    // -------------------------------------------------------------------------
protected:
    virtual void register_select_for_send(selects::SelectInstance<void*>* select, void* element_any) {
        E element = *static_cast<E*>(element_any);
        // Line 1484-1490: sendImpl(...)
        // TODO(port): This calls into sendImpl which is an inline function in Kotlin.
        // For now, provide a simplified implementation that just calls selectInRegistrationPhase.
        // Full implementation requires integrating with the sendImpl state machine.
        send_impl_with_select(
            element,
            select,
            [select]() { select->select_in_registration_phase(nullptr); }, // onRendezvousOrBuffered: Unit
            [](ChannelSegment<E>*, int) {}, // onSuspend
            [this, element, select]() { on_closed_select_on_send(element, select); } // onClosed
        );
    }

private:
    // -------------------------------------------------------------------------
    // Lines 1539-1541: private fun onClosedSelectOnReceive(select: SelectInstance<*>)
    // -------------------------------------------------------------------------
    void on_closed_select_on_receive(selects::SelectInstance<void*>* select) {
        // Line 1540: select.selectInRegistrationPhase(CHANNEL_CLOSED)
        select->select_in_registration_phase(static_cast<void*>(&CHANNEL_CLOSED()));
    }

    // -------------------------------------------------------------------------
    // Lines 1544-1546: private fun processResultSelectReceive(ignoredParam: Any?, selectResult: Any?): Any?
    // -------------------------------------------------------------------------
    void* process_result_select_receive(void* /*ignored_param*/, void* select_result) {
        // Line 1545: if (selectResult === CHANNEL_CLOSED) throw receiveException
        if (select_result == static_cast<void*>(&CHANNEL_CLOSED())) {
            std::rethrow_exception(receive_exception());
        }
        // Line 1546: else selectResult
        return select_result;
    }

    // -------------------------------------------------------------------------
    // Lines 1549-1553: private fun processResultSelectReceiveOrNull(ignoredParam: Any?, selectResult: Any?): Any?
    // -------------------------------------------------------------------------
    void* process_result_select_receive_or_null(void* /*ignored_param*/, void* select_result) {
        // Line 1550-1552: if (selectResult === CHANNEL_CLOSED) { if (closeCause == null) null else throw receiveException }
        if (select_result == static_cast<void*>(&CHANNEL_CLOSED())) {
            if (!close_cause()) {
                return nullptr;
            } else {
                std::rethrow_exception(receive_exception());
            }
        }
        // Line 1553: else selectResult
        return select_result;
    }

    // -------------------------------------------------------------------------
    // Lines 1556-1558: private fun processResultSelectReceiveCatching(ignoredParam: Any?, selectResult: Any?): Any?
    // -------------------------------------------------------------------------
    void* process_result_select_receive_catching(void* /*ignored_param*/, void* select_result) {
        // Line 1557: if (selectResult === CHANNEL_CLOSED) closed(closeCause)
        if (select_result == static_cast<void*>(&CHANNEL_CLOSED())) {
            // Return a boxed ChannelResult::closed
            auto* result = new ChannelResult<E>(ChannelResult<E>::closed(close_cause()));
            return static_cast<void*>(result);
        }
        // Line 1558: else success(selectResult as E)
        E element = *static_cast<E*>(select_result);
        auto* result = new ChannelResult<E>(ChannelResult<E>::success(element));
        return static_cast<void*>(result);
    }

    // -------------------------------------------------------------------------
    // Lines 1531-1537: private fun registerSelectForReceive(select: SelectInstance<*>, ignoredParam: Any?)
    // -------------------------------------------------------------------------
    void register_select_for_receive(selects::SelectInstance<void*>* select, void* /*ignored_param*/) {
        // Line 1532-1537: receiveImpl(...)
        // TODO(port): This calls into receiveImpl which is an inline function in Kotlin.
        // For now, provide a simplified implementation.
        receive_impl_with_select(
            select,
            [select](E elem) { select->select_in_registration_phase(new E(elem)); }, // onElementRetrieved
            [](ChannelSegment<E>*, int, void*) {}, // onSuspend
            [this, select]() { on_closed_select_on_receive(select); } // onClosed
        );
    }

    // -------------------------------------------------------------------------
    // Lines 1561-1567: private val onUndeliveredElementReceiveCancellationConstructor
    // -------------------------------------------------------------------------
    selects::OnCancellationConstructor get_on_undelivered_element_receive_cancellation_constructor() {
        if (!on_undelivered_element_) {
            return nullptr;
        }
        // Line 1562-1566: Returns a lambda that checks if element !== CHANNEL_CLOSED
        return [this](void* /*select*/, void* /*param*/, void* element) -> selects::OnCancellationAction {
            return [this, element](std::exception_ptr, void*, std::shared_ptr<CoroutineContext>) {
                if (element != static_cast<void*>(&CHANNEL_CLOSED())) {
                    on_undelivered_element_(*static_cast<E*>(element));
                }
            };
        };
    }

    // -------------------------------------------------------------------------
    // Lines 373-421: private inline fun sendImplOnNoWaiter(...)
    // -------------------------------------------------------------------------
    void send_impl_on_no_waiter(
        ChannelSegment<E>* segment,
        int index,
        E element,
        int64_t s,
        Waiter* waiter,
        std::function<void()> on_rendezvous_or_buffered,
        std::function<void()> on_closed
    ) {
        // Line 394: when (updateCellSend(segment, index, element, s, waiter, false))
        int result = update_cell_send(segment, index, element, s, waiter, false);
        switch (result) {
            case RESULT_RENDEZVOUS:
                // Line 396: segment.cleanPrev()
                segment->clean_prev();
                // Line 397: onRendezvousOrBuffered()
                on_rendezvous_or_buffered();
                break;
            case RESULT_BUFFERED:
                // Line 400: onRendezvousOrBuffered()
                on_rendezvous_or_buffered();
                break;
            case RESULT_SUSPEND:
                // Line 403: waiter.prepareSenderForSuspension(segment, index)
                prepare_sender_for_suspension(waiter, segment, index);
                break;
            case RESULT_CLOSED:
                // Line 406: if (s < receiversCounter) segment.cleanPrev()
                if (s < receivers_counter()) segment->clean_prev();
                // Line 407: onClosed()
                on_closed();
                break;
            case RESULT_FAILED:
                // Line 410: segment.cleanPrev()
                segment->clean_prev();
                // Line 411-417: sendImpl(element, waiter, onRendezvousOrBuffered, ..., onClosed)
                // TODO(port): Full sendImpl needs inline expansion
                // For now, recursively retry via simplified path
                send_impl_with_waiter(element, waiter, on_rendezvous_or_buffered, on_closed);
                break;
            default:
                // Line 419: error("unexpected")
                assert(false && "unexpected result from update_cell_send");
        }
    }

    // -------------------------------------------------------------------------
    // Lines 244-369: Helper for send_impl_on_no_waiter RESULT_FAILED case
    // Simplified version that attempts send with an existing waiter
    // -------------------------------------------------------------------------
    void send_impl_with_waiter(
        E element,
        Waiter* waiter,
        std::function<void()> on_rendezvous_or_buffered,
        std::function<void()> on_closed
    ) {
        // Increment senders counter and get a segment/index
        int64_t s = senders_and_close_status_.fetch_add(1, std::memory_order_acq_rel) & SENDERS_COUNTER_MASK;
        int64_t id = s / SEGMENT_SIZE;
        int index = static_cast<int>(s % SEGMENT_SIZE);

        ChannelSegment<E>* segment = find_segment_send(id, send_segment_.load(std::memory_order_acquire));
        if (segment == nullptr) {
            // Channel closed
            on_closed();
            return;
        }

        // Try again with new cell
        send_impl_on_no_waiter(segment, index, element, s, waiter, on_rendezvous_or_buffered, on_closed);
    }

    // -------------------------------------------------------------------------
    // Stub for sendImpl with select (Kotlin inline function)
    // TODO(port): Integrate with full sendImpl state machine
    // -------------------------------------------------------------------------
    void send_impl_with_select(
        E element,
        selects::SelectInstance<void*>* waiter,
        std::function<void()> on_rendezvous_or_buffered,
        std::function<void(ChannelSegment<E>*, int)> on_suspend,
        std::function<void()> on_closed
    ) {
        // Simplified: attempt trySend first
        auto result = send_impl_try_send(element);
        if (result.is_success()) {
            on_rendezvous_or_buffered();
        } else if (result.is_closed()) {
            on_closed();
        } else {
            // Would need to suspend - for now just call on_suspend with nulls
            // TODO(port): Full suspension logic for select
            on_suspend(nullptr, 0);
        }
    }

    // -------------------------------------------------------------------------
    // Stub for receiveImpl with select (Kotlin inline function)
    // TODO(port): Integrate with full receiveImpl state machine
    // -------------------------------------------------------------------------
    void receive_impl_with_select(
        selects::SelectInstance<void*>* waiter,
        std::function<void(E)> on_element_retrieved,
        std::function<void(ChannelSegment<E>*, int, void*)> on_suspend,
        std::function<void()> on_closed
    ) {
        // Simplified: attempt tryReceive first
        auto result = receive_impl_try_receive();
        if (result.is_success()) {
            on_element_retrieved(result.get_or_throw());
        } else if (result.is_closed()) {
            on_closed();
        } else {
            // Would need to suspend - for now just call on_suspend with nulls
            // TODO(port): Full suspension logic for select
            on_suspend(nullptr, 0, nullptr);
        }
    }

public:
    // -------------------------------------------------------------------------
    // Lines 1475-1480: override val onSend: SelectClause2<E, BufferedChannel<E>>
    // -------------------------------------------------------------------------
    selects::SelectClause2Impl<E, BufferedChannel<E>> get_on_send() {
        return selects::SelectClause2Impl<E, BufferedChannel<E>>(
            static_cast<void*>(this),
            // regFunc
            [this](void* /*clause_object*/, void* select, void* param) {
                register_select_for_send(
                    static_cast<selects::SelectInstance<void*>*>(select),
                    param
                );
            },
            // processResFunc
            [this](void* /*clause_object*/, void* param, void* clause_result) {
                return process_result_select_send(param, clause_result);
            }
        );
    }

    // -------------------------------------------------------------------------
    // Lines 1504-1510: override val onReceive: SelectClause1<E>
    // -------------------------------------------------------------------------
    selects::SelectClause1Impl<E> get_on_receive() {
        return selects::SelectClause1Impl<E>(
            static_cast<void*>(this),
            // regFunc
            [this](void* /*clause_object*/, void* select, void* param) {
                register_select_for_receive(
                    static_cast<selects::SelectInstance<void*>*>(select),
                    param
                );
            },
            // processResFunc
            [this](void* /*clause_object*/, void* param, void* clause_result) {
                return process_result_select_receive(param, clause_result);
            },
            // onCancellationConstructor
            get_on_undelivered_element_receive_cancellation_constructor()
        );
    }

    // -------------------------------------------------------------------------
    // Lines 1513-1519: override val onReceiveCatching: SelectClause1<ChannelResult<E>>
    // -------------------------------------------------------------------------
    selects::SelectClause1Impl<ChannelResult<E>> get_on_receive_catching() {
        return selects::SelectClause1Impl<ChannelResult<E>>(
            static_cast<void*>(this),
            // regFunc
            [this](void* /*clause_object*/, void* select, void* param) {
                register_select_for_receive(
                    static_cast<selects::SelectInstance<void*>*>(select),
                    param
                );
            },
            // processResFunc
            [this](void* /*clause_object*/, void* param, void* clause_result) {
                return process_result_select_receive_catching(param, clause_result);
            },
            // onCancellationConstructor
            get_on_undelivered_element_receive_cancellation_constructor()
        );
    }

    // -------------------------------------------------------------------------
    // Lines 1522-1528: override val onReceiveOrNull: SelectClause1<E?>
    // -------------------------------------------------------------------------
    selects::SelectClause1Impl<E*> get_on_receive_or_null() {
        return selects::SelectClause1Impl<E*>(
            static_cast<void*>(this),
            // regFunc
            [this](void* /*clause_object*/, void* select, void* param) {
                register_select_for_receive(
                    static_cast<selects::SelectInstance<void*>*>(select),
                    param
                );
            },
            // processResFunc
            [this](void* /*clause_object*/, void* param, void* clause_result) {
                return process_result_select_receive_or_null(param, clause_result);
            },
            // onCancellationConstructor
            get_on_undelivered_element_receive_cancellation_constructor()
        );
    }

    // =========================================================================
    // Lines 2193-2209: Resume waiters on closed/cancelled channel
    // =========================================================================

    // -------------------------------------------------------------------------
    // Line 2193: private fun Waiter.resumeReceiverOnClosedChannel()
    // -------------------------------------------------------------------------
    void resume_receiver_on_closed_channel(Waiter* waiter) {
        resume_waiter_on_closed_channel(waiter, /* receiver = */ true);
    }

    // -------------------------------------------------------------------------
    // Line 2199: private fun Waiter.resumeSenderOnCancelledChannel()
    // -------------------------------------------------------------------------
    void resume_sender_on_cancelled_channel(Waiter* waiter) {
        resume_waiter_on_closed_channel(waiter, /* receiver = */ false);
    }

    // -------------------------------------------------------------------------
    // Lines 2201-2209: private fun Waiter.resumeWaiterOnClosedChannel(receiver: Boolean)
    // -------------------------------------------------------------------------
    void resume_waiter_on_closed_channel(Waiter* waiter, bool receiver) {
        // Line 2202-2208: when (this) { ... }
        // Check if it's a SendBroadcast
        if (auto* sb = dynamic_cast<SendBroadcast*>(waiter)) {
            // Line 2203: is SendBroadcast -> cont.resume(false)
            sb->cont->resume_with(Result<bool>::success(false));
            return;
        }
        // Check if it's a CancellableContinuation (via CancellableContinuationImpl)
        if (auto* cc = dynamic_cast<CancellableContinuationImpl<void*>*>(waiter)) {
            // Line 2204: is CancellableContinuation<*> -> resumeWithException(...)
            auto exc = receiver ? receive_exception() : send_exception();
            cc->resume_with(Result<void*>::failure(exc));
            return;
        }
        // Check if it's a ReceiveCatching
        if (auto* rc = dynamic_cast<ReceiveCatching<E>*>(waiter)) {
            // Line 2205: is ReceiveCatching<*> -> cont.resume(closed(closeCause))
            rc->cont->resume_with(Result<ChannelResult<E>>::success(ChannelResult<E>::closed(close_cause())));
            return;
        }
        // Check if it's a BufferedChannelIterator
        if (auto* iter = dynamic_cast<BufferedChannelIterator*>(waiter)) {
            // Line 2206: is BufferedChannel<*>.BufferedChannelIterator -> tryResumeHasNextOnClosedChannel()
            iter->try_resume_has_next_on_closed_channel();
            return;
        }
        // Check if it's a SelectInstance
        if (auto* sel = dynamic_cast<selects::SelectInstance<void*>*>(waiter)) {
            // Line 2207: is SelectInstance<*> -> trySelect(this@BufferedChannel, CHANNEL_CLOSED)
            sel->try_select(static_cast<void*>(this), static_cast<void*>(&CHANNEL_CLOSED()));
            return;
        }
        // Line 2208: else -> error("Unexpected waiter: $this")
        throw std::runtime_error("Unexpected waiter type in resumeWaiterOnClosedChannel");
    }

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
                // C++ lifetime: release the waiter ref since we're done with it
                segment->clear_waiter_ref(index);
                segment->set_state(index, static_cast<void*>(&DONE_RCV()));
                on_receive_dequeued();
                return RESULT_RENDEZVOUS;
            } else {
                // C++ lifetime: release the waiter ref since we're done with it
                segment->clear_waiter_ref(index);
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
                    // C++ lifetime: release the waiter ref since we're done with it
                    segment->clear_waiter_ref(index);
                    segment->set_state(index, static_cast<void*>(&DONE_RCV()));
                    on_receive_dequeued();
                    return RESULT_RENDEZVOUS;
                } else {
                    // C++ lifetime: release the waiter ref since we're done with it
                    segment->clear_waiter_ref(index);
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
                        // C++ lifetime: release the waiter ref since we're done with it
                        segment->clear_waiter_ref(index);
                        segment->set_state(index, static_cast<void*>(&DONE_RCV()));
                        expand_buffer();
                        E elem = segment->retrieve_element(index);
                        return new E(std::move(elem));
                    } else {
                        // C++ lifetime: release the waiter ref since we're done with it
                        segment->clear_waiter_ref(index);
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
        // Line 653: is SelectInstance<*> -> trySelect(this@BufferedChannel, element)
        if (auto* select = dynamic_cast<selects::SelectInstance<void*>*>(static_cast<Waiter*>(receiver))) {
            return select->try_select(static_cast<void*>(this), new E(element));
        }
        // Line 656-658: is ReceiveCatching<*> -> cont.tryResume0(success(element), onUndeliveredElement?.bindCancellationFunResult())
        if (auto* rc = dynamic_cast<ReceiveCatching<E>*>(static_cast<Waiter*>(receiver))) {
            auto on_cancellation = bind_cancellation_fun_result();
            return try_resume_0(rc->cont, ChannelResult<E>::success(element), on_cancellation);
        }
        // Line 660-662: is BufferedChannelIterator -> tryResumeHasNext(element)
        if (auto* iter = dynamic_cast<BufferedChannelIterator*>(static_cast<Waiter*>(receiver))) {
            return iter->try_resume_has_next(element);
        }
        // Line 664-666: is CancellableContinuation<*> -> tryResume0(element, onUndeliveredElement?.bindCancellationFun())
        if (auto* cont = dynamic_cast<CancellableContinuationImpl<E>*>(static_cast<Waiter*>(receiver))) {
            auto on_cancellation = bind_cancellation_fun();
            return try_resume_0(cont, element, on_cancellation);
        }
        // Line 668: else -> error("Unexpected receiver type: $this")
        throw std::runtime_error("Unexpected receiver type in tryResumeReceiver");
    }

    // Line 1167-1184: private fun Any.tryResumeSender(segment: ChannelSegment<E>, index: Int): Boolean
    bool try_resume_sender(void* sender, ChannelSegment<E>* segment, int index) {
        // Line 1168-1172: is CancellableContinuation<*> -> tryResume0(Unit)
        if (auto* cont = dynamic_cast<CancellableContinuationImpl<void>*>(static_cast<Waiter*>(sender))) {
            // For void/Unit, tryResume0 just needs to be called
            void* token = cont->try_resume(nullptr);
            if (token != nullptr) {
                cont->complete_resume(token);
                return true;
            }
            return false;
        }
        // Line 1173-1181: is SelectInstance<*> -> ...
        if (auto* select = dynamic_cast<selects::SelectImplementation<void*>*>(static_cast<Waiter*>(sender))) {
            // Line 1175: val trySelectResult = trySelectDetailed(clauseObject = this@BufferedChannel, result = Unit)
            auto try_select_result = select->try_select_detailed(static_cast<void*>(this), nullptr);
            // Line 1177-1178: if (trySelectResult === REREGISTER) segment.cleanElement(index)
            if (try_select_result == selects::TrySelectDetailedResult::REREGISTER) {
                segment->clean_element(index);
            }
            // Line 1180: trySelectResult === SUCCESSFUL
            return try_select_result == selects::TrySelectDetailedResult::SUCCESSFUL;
        }
        // Line 1182: is SendBroadcast -> cont.tryResume0(true)
        if (auto* sb = dynamic_cast<SendBroadcast*>(static_cast<Waiter*>(sender))) {
            void* token = sb->cont->try_resume(true, nullptr);
            if (token != nullptr) {
                sb->cont->complete_resume(token);
                return true;
            }
            return false;
        }
        // Line 1183: else -> error("Unexpected waiter: $this")
        throw std::runtime_error("Unexpected sender type in tryResumeSender");
    }

    // Line 678: protected open fun onReceiveEnqueued() {}
    virtual void on_receive_enqueued() {}

    // Line 684: protected open fun onReceiveDequeued() {}
    virtual void on_receive_dequeued() {}

    // -------------------------------------------------------------------------
    // From OnUndeliveredElement.kt Lines 8-24:
    // internal fun <E> OnUndeliveredElement<E>.callUndeliveredElementCatchingException(...)
    // -------------------------------------------------------------------------
    std::exception_ptr call_undelivered_element_catching_exception(
        E element,
        std::exception_ptr undelivered_element_exception = nullptr
    ) {
        // Line 12-21: try { invoke(element) } catch (ex: Throwable) { ... }
        try {
            if (on_undelivered_element_) {
                on_undelivered_element_(element);
            }
        } catch (...) {
            std::exception_ptr ex = std::current_exception();
            // Line 17-20: if existing exception and cause differs, add suppressed; else create new
            if (undelivered_element_exception) {
                // TODO(port): C++ doesn't have addSuppressed - just return the original
                return undelivered_element_exception;
            } else {
                // Line 20: return UndeliveredElementException("...", ex)
                return ex;
            }
        }
        // Line 23: return undeliveredElementException
        return undelivered_element_exception;
    }

    // -------------------------------------------------------------------------
    // Lines 2780-2782: private fun OnUndeliveredElement<E>.bindCancellationFun(element: E)
    // Returns a cancellation handler that invokes onUndeliveredElement with the captured element.
    // -------------------------------------------------------------------------
    std::function<void(std::exception_ptr, E, std::shared_ptr<CoroutineContext>)>
    bind_cancellation_fun(E element) {
        if (!on_undelivered_element_) {
            return nullptr;
        }
        // Line 2781-2782: { _: Throwable, _, context: CoroutineContext -> callUndeliveredElement(element, context) }
        // Note: C++ OnUndeliveredElement only takes element (context implicit)
        return [this, element](std::exception_ptr, E, std::shared_ptr<CoroutineContext>) {
            on_undelivered_element_(element);
        };
    }

    // -------------------------------------------------------------------------
    // Lines 2784-2793: private fun OnUndeliveredElement<E>.bindCancellationFun()
    // Returns a cancellation handler that receives the element as parameter.
    // -------------------------------------------------------------------------
    std::function<void(std::exception_ptr, E, std::shared_ptr<CoroutineContext>)>
    bind_cancellation_fun() {
        if (!on_undelivered_element_) {
            return nullptr;
        }
        // Line 2791-2792: fun onCancellationImplDoNotCall(cause: Throwable, element: E, context: CoroutineContext)
        return [this](std::exception_ptr, E element, std::shared_ptr<CoroutineContext>) {
            on_undelivered_element_(element);
        };
    }

    // -------------------------------------------------------------------------
    // Lines 2767-2778: private fun OnUndeliveredElement<E>.bindCancellationFunResult()
    // For ChannelResult<E> - extracts element and invokes onUndeliveredElement.
    // -------------------------------------------------------------------------
    std::function<void(std::exception_ptr, ChannelResult<E>, std::shared_ptr<CoroutineContext>)>
    bind_cancellation_fun_result() {
        if (!on_undelivered_element_) {
            return nullptr;
        }
        // Line 2774-2777: onUndeliveredElement!!.callUndeliveredElement(element.getOrNull()!!, context)
        return [this](std::exception_ptr, ChannelResult<E> result, std::shared_ptr<CoroutineContext>) {
            if (result.is_success()) {
                on_undelivered_element_(result.get_or_throw());
            }
        };
    }

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
        // Line 827: assert { isConflatedDropOldest }
        assert(is_conflated_drop_oldest());
        // Line 830: var segment = receiveSegment.value
        ChannelSegment<E>* segment = receive_segment_.load(std::memory_order_acquire);
        // Line 831: while (true)
        while (true) {
            // Line 834: val r = this.receivers.value
            int64_t r = receivers_.load(std::memory_order_acquire);
            // Line 835: if (globalCellIndex < max(r + capacity, bufferEndCounter)) return
            if (global_cell_index < std::max(r + capacity_, buffer_end_counter())) {
                return;
            }
            // Line 838: if (!this.receivers.compareAndSet(r, r + 1)) continue
            if (!receivers_.compare_exchange_weak(r, r + 1,
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                continue;
            }
            // Line 840: val id = r / SEGMENT_SIZE
            int64_t id = r / SEGMENT_SIZE;
            // Line 841: val i = (r % SEGMENT_SIZE).toInt()
            int i = static_cast<int>(r % SEGMENT_SIZE);
            // Line 844: if (segment.id != id)
            if (segment->id != id) {
                // Line 846: segment = findSegmentReceive(id, segment) ?:
                ChannelSegment<E>* found = find_segment_receive(id, segment);
                if (found == nullptr) {
                    // Line 853: continue
                    continue;
                }
                segment = found;
            }
            // Line 856: val updCellResult = updateCellReceive(segment, i, r, null)
            void* upd_cell_result = update_cell_receive(segment, i, r, nullptr);
            // Line 857-871: when { ... }
            if (upd_cell_result == &FAILED()) {
                // Line 858-862: The cell is poisoned; restart from the beginning.
                // To avoid memory leaks, we also need to reset the `prev` pointer.
                if (r < senders_counter()) {
                    segment->clean_prev();
                }
            } else {
                // Line 864-869: A buffered element was retrieved from the cell.
                // Clean the reference to the previous segment.
                segment->clean_prev();
                // Line 869: onUndeliveredElement?.callUndeliveredElementCatchingException(updCellResult as E)?.let { throw it }
                if (on_undelivered_element_) {
                    E element = *static_cast<E*>(upd_cell_result);
                    std::exception_ptr ex = call_undelivered_element_catching_exception(element);
                    if (ex) {
                        std::rethrow_exception(ex);
                    }
                }
            }
        }
    }

    // Line 2041-2134: private fun removeUnprocessedElements(...)
    void remove_unprocessed_elements(ChannelSegment<E>* last_segment) {
        // Line 2046: val onUndeliveredElement = onUndeliveredElement
        auto on_undelivered_element = on_undelivered_element_;
        // Line 2047: var undeliveredElementException: UndeliveredElementException? = null
        std::exception_ptr undelivered_element_exception = nullptr;
        // Line 2054: var suspendedSenders = InlineList<Waiter>()
        std::vector<Waiter*> suspended_senders;
        // Line 2055: var segment = lastSegment
        ChannelSegment<E>* segment = last_segment;
        // Line 2056: process_segments@ while (true)
        bool process_segments_done = false;
        while (!process_segments_done) {
            // Line 2057: for (index in SEGMENT_SIZE - 1 downTo 0)
            for (int index = SEGMENT_SIZE - 1; index >= 0; --index) {
                // Line 2059: val globalIndex = segment.id * SEGMENT_SIZE + index
                int64_t global_index = segment->id * SEGMENT_SIZE + index;
                // Line 2061: update_cell@ while (true)
                bool update_cell_done = false;
                while (!update_cell_done) {
                    // Line 2063: val state = segment.getState(index)
                    void* state = segment->get_state(index);
                    // Line 2064-2124: when { ... }
                    if (state == &DONE_RCV()) {
                        // Line 2066: break@process_segments
                        process_segments_done = true;
                        update_cell_done = true;
                        break;
                    } else if (state == &BUFFERED()) {
                        // Line 2068-2083: The cell stores a buffered element
                        // Line 2070: if (globalIndex < receiversCounter) break@process_segments
                        if (global_index < receivers_counter()) {
                            process_segments_done = true;
                            update_cell_done = true;
                            break;
                        }
                        // Line 2072: if (segment.casState(index, state, CHANNEL_CLOSED))
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            // Line 2074-2077: If onUndeliveredElement lambda is non-null, call it
                            if (on_undelivered_element) {
                                E element = segment->get_element(index);
                                std::exception_ptr ex = call_undelivered_element_catching_exception(element, undelivered_element_exception);
                                if (ex && !undelivered_element_exception) {
                                    undelivered_element_exception = ex;
                                }
                            }
                            // Line 2080-2081: Clean the element field and inform the segment
                            segment->clean_element(index);
                            segment->on_slot_cleaned();
                            update_cell_done = true;
                        }
                    } else if (state == &IN_BUFFER() || state == nullptr) {
                        // Line 2086-2092: The cell is empty
                        // Line 2088: if (segment.casState(index, state, CHANNEL_CLOSED))
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            // Line 2090: Inform the segment that the slot is cleaned
                            segment->on_slot_cleaned();
                            update_cell_done = true;
                        }
                    } else if (is_waiter(state) || is_waiter_eb(state)) {
                        // Line 2095-2115: The cell stores a suspended waiter
                        // Line 2097: if (globalIndex < receiversCounter) break@process_segments
                        if (global_index < receivers_counter()) {
                            process_segments_done = true;
                            update_cell_done = true;
                            break;
                        }
                        // Line 2099-2100: Obtain the sender
                        Waiter* sender = is_waiter_eb(state) ?
                            static_cast<WaiterEB*>(state)->waiter :
                            static_cast<Waiter*>(state);
                        // Line 2102: if (segment.casState(index, state, CHANNEL_CLOSED))
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            // Line 2104-2107: If onUndeliveredElement lambda is non-null, call it
                            if (on_undelivered_element) {
                                E element = segment->get_element(index);
                                std::exception_ptr ex = call_undelivered_element_catching_exception(element, undelivered_element_exception);
                                if (ex && !undelivered_element_exception) {
                                    undelivered_element_exception = ex;
                                }
                            }
                            // Line 2109: Save the sender for further cancellation
                            suspended_senders.push_back(sender);
                            // Line 2112-2113: Clean the element field and inform the segment
                            segment->clean_element(index);
                            segment->on_slot_cleaned();
                            update_cell_done = true;
                        }
                    } else if (state == &RESUMING_BY_EB() || state == &RESUMING_BY_RCV()) {
                        // Line 2119: A concurrent receiver is resuming a suspended sender.
                        // As the cell is covered by a receiver, finish immediately.
                        process_segments_done = true;
                        update_cell_done = true;
                        break;
                    } else {
                        // Line 2123: else -> break@update_cell
                        update_cell_done = true;
                    }
                }
                if (process_segments_done) break;
            }
            // Line 2128: segment = segment.prev ?: break
            ChannelSegment<E>* prev = segment->prev();
            if (prev == nullptr) break;
            segment = prev;
        }
        // Line 2131: suspendedSenders.forEachReversed { it.resumeSenderOnCancelledChannel() }
        for (auto it = suspended_senders.rbegin(); it != suspended_senders.rend(); ++it) {
            resume_sender_on_cancelled_channel(*it);
        }
        // Line 2133: undeliveredElementException?.let { throw it }
        if (undelivered_element_exception) {
            std::rethrow_exception(undelivered_element_exception);
        }
    }

    // Line 2140-2187: private fun cancelSuspendedReceiveRequests(...)
    void cancel_suspended_receive_requests(ChannelSegment<E>* last_segment, int64_t senders_counter_val) {
        // Line 2148: var suspendedReceivers = InlineList<Waiter>()
        std::vector<Waiter*> suspended_receivers;
        // Line 2149: var segment: ChannelSegment<E>? = lastSegment
        ChannelSegment<E>* segment = last_segment;
        // Line 2150: process_segments@ while (segment != null)
        bool process_segments_done = false;
        while (segment != nullptr && !process_segments_done) {
            // Line 2151: for (index in SEGMENT_SIZE - 1 downTo 0)
            for (int index = SEGMENT_SIZE - 1; index >= 0; --index) {
                // Line 2153: if (segment.id * SEGMENT_SIZE + index < sendersCounter) break@process_segments
                if (segment->id * SEGMENT_SIZE + index < senders_counter_val) {
                    process_segments_done = true;
                    break;
                }
                // Line 2155: cell_update@ while (true)
                bool cell_update_done = false;
                while (!cell_update_done) {
                    // Line 2156: val state = segment.getState(index)
                    void* state = segment->get_state(index);
                    // Line 2157-2179: when { ... }
                    if (state == nullptr || state == &IN_BUFFER()) {
                        // Line 2158-2162: state === null || state === IN_BUFFER
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            segment->on_slot_cleaned();
                            cell_update_done = true;
                        }
                    } else if (is_waiter_eb(state)) {
                        // Line 2164-2169: state is WaiterEB
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            // Line 2166: suspendedReceivers += state.waiter
                            suspended_receivers.push_back(static_cast<WaiterEB*>(state)->waiter);
                            // Line 2167: segment.onCancelledRequest(index = index, receiver = true)
                            segment->on_cancelled_request(index, true);
                            cell_update_done = true;
                        }
                    } else if (is_waiter(state)) {
                        // Line 2171-2176: state is Waiter
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            // Line 2173: suspendedReceivers += state
                            suspended_receivers.push_back(static_cast<Waiter*>(state));
                            // Line 2174: segment.onCancelledRequest(index = index, receiver = true)
                            segment->on_cancelled_request(index, true);
                            cell_update_done = true;
                        }
                    } else {
                        // Line 2178: else -> break@cell_update
                        cell_update_done = true;
                    }
                }
            }
            // Line 2183: segment = segment.prev
            segment = segment->prev();
        }
        // Line 2186: suspendedReceivers.forEachReversed { it.resumeReceiverOnClosedChannel() }
        for (auto it = suspended_receivers.rbegin(); it != suspended_receivers.rend(); ++it) {
            resume_receiver_on_closed_channel(*it);
        }
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

    // Line 1595-1744: BufferedChannelIterator inner class
    // Implements both ChannelIterator and Waiter interfaces
    class BufferedChannelIterator : public ChannelIterator<E>, public Waiter {
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

        // -------------------------------------------------------------------------
        // Lines 1709-1720: fun tryResumeHasNext(element: E): Boolean
        // -------------------------------------------------------------------------
        bool try_resume_has_next(E element) {
            // Line 1712: val cont = this.continuation!!
            CancellableContinuationImpl<bool>* cont = continuation_;
            assert(cont != nullptr);
            // Line 1713: this.continuation = null
            continuation_ = nullptr;
            // Line 1715: this.receiveResult = element
            receive_result_ = new E(element);
            // Line 1719: return cont.tryResume0(true, onUndeliveredElement?.bindCancellationFun(element))
            // Note: Kotlin uses (Throwable, Any?, CoroutineContext) signature and ignores the value.
            // In C++, we capture the element and ignore the bool value passed in.
            std::function<void(std::exception_ptr, bool, std::shared_ptr<CoroutineContext>)> on_cancellation = nullptr;
            if (channel_->on_undelivered_element()) {
                auto elem_copy = element;
                auto channel = channel_;
                on_cancellation = [channel, elem_copy](std::exception_ptr, bool, std::shared_ptr<CoroutineContext>) {
                    channel->on_undelivered_element()(elem_copy);
                };
            }
            // Inline try_resume_0 logic since it's defined after the class
            void* token = cont->try_resume(true, nullptr, on_cancellation);
            if (token != nullptr) {
                cont->complete_resume(token);
                return true;
            }
            return false;
        }

        // -------------------------------------------------------------------------
        // Lines 1722-1742: fun tryResumeHasNextOnClosedChannel()
        // -------------------------------------------------------------------------
        void try_resume_has_next_on_closed_channel() {
            // Line 1728: val cont = this.continuation!!
            CancellableContinuationImpl<bool>* cont = continuation_;
            assert(cont != nullptr);
            // Line 1729: this.continuation = null
            continuation_ = nullptr;
            // Line 1733: this.receiveResult = CHANNEL_CLOSED
            receive_result_ = static_cast<void*>(&CHANNEL_CLOSED());
            // Line 1737-1741: Resume with result based on closeCause
            auto cause = channel_->close_cause();
            if (!cause) {
                // Line 1739: cont.resume(false)
                cont->resume_with(Result<bool>::success(false));
            } else {
                // Line 1741: cont.resumeWithException(recoverStackTrace(cause, cont))
                cont->resume_with(Result<bool>::failure(cause));
            }
        }

        // Waiter interface implementation
        // Line 1711-1716: invokeOnCancellation delegating to continuation
        void invoke_on_cancellation(internal::SegmentBase* segment, int index) override {
            if (continuation_) {
                continuation_->invoke_on_cancellation(segment, index);
            }
        }

    private:
        BufferedChannel<E>* channel_;
        void* receive_result_;
        CancellableContinuationImpl<bool>* continuation_;
    };
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
