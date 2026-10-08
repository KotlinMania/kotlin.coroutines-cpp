#pragma once
// port-lint: source channels/BufferedChannel.kt
/**
 * @file BufferedChannel.hpp
 * @brief The buffered channel implementation
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt
 *
 * The buffered channel implementation, which also serves as a rendezvous channel when the capacity is zero.
 * The high-level structure bases on a conceptually infinite array for storing elements and waiting requests,
 * separate counters of [send] and [receive] invocations that were ever performed, and an additional counter
 * that indicates the end of the logical buffer by counting the number of array cells it ever contained.
 *
 * The key idea is that both [send] and [receive] start by incrementing their counters, assigning the array cell
 * referenced by the counter. In case of rendezvous channels, the operation either suspends and stores its continuation
 * in the cell or makes a rendezvous with the opposite request. Each cell can be processed by exactly one [send] and
 * one [receive]. As for buffered channels, [send]-s can also add elements without suspension if the logical buffer
 * contains the cell, while the [receive] operation updates the end of the buffer when its synchronization finishes.
 *
 * Please see the ["Fast and Scalable Channels in Kotlin Coroutines"](https://arxiv.org/abs/2211.04986)
 * paper by Nikita Koval, Roman Elizarov, and Dan Alistarh for the detailed algorithm description.
 */

#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/CancellableReusable.hpp"
#include "kotlinx/coroutines/Waiter.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/ConcurrentLinkedList.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/EventLoop.hpp"
#include <atomic>
#include <memory>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cassert>
#include <cstdint>
#include <cstring>
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
    if (capacity == CHANNEL_RENDEZVOUS) return BUFFER_END_RENDEZVOUS;
    if (capacity == CHANNEL_UNLIMITED) return BUFFER_END_UNLIMITED;
    return static_cast<int64_t>(capacity);
}

// ============================================================================
// Lines 2934-2945: Segment size and wait iterations
// ============================================================================

// @JvmField
// internal val SEGMENT_SIZE = systemProp("kotlinx.coroutines.bufferedChannel.segmentSize", 32)
constexpr int SEGMENT_SIZE = 32;

// until the numbers of started and completed expandBuffer calls coincide.
constexpr int EXPAND_BUFFER_COMPLETION_WAIT_ITERATIONS = 10000;

// ============================================================================
// Lines 2975-3009: Cell state symbols
// ============================================================================
//
// Each cell in the channel segment can be in various states during its lifecycle.
// These states are represented by special Symbol objects.
//
// The cell state machine for a sender:
//   null/IN_BUFFER -> [element stored] -> BUFFERED (if buffered)
//                  -> [receiver arrives] -> DONE_RCV (rendezvous)
//                  -> [cancelled] -> INTERRUPTED_SEND
//                  -> [closed] -> CHANNEL_CLOSED
//
// The cell state machine for a receiver:
//   null -> [no element] -> [waiter stored] -> [element arrives] -> DONE_RCV
//       -> [cancelled] -> INTERRUPTED_RCV
//       -> [closed] -> CHANNEL_CLOSED
//
// POISONED is used when a cell is broken due to concurrent access issues.

/**
 * Indicates that the element has been buffered in the cell.
 * This state is set when a sender stores an element that doesn't require
 * a rendezvous (the buffer has capacity).
 *
 * Transliterated from: @JvmField internal val BUFFERED = Symbol("BUFFERED")
 */
inline internal::Symbol& BUFFERED() {
    static internal::Symbol instance("BUFFERED");
    return instance;
}

/**
 * Indicates that a sender should buffer the element in this cell.
 * This is a transitional state during the expandBuffer procedure,
 * signaling to the sender that it should store the element.
 *
 * Transliterated from: private val IN_BUFFER = Symbol("SHOULD_BUFFER")
 */
inline internal::Symbol& IN_BUFFER() {
    static internal::Symbol instance("SHOULD_BUFFER");
    return instance;
}

/**
 * Indicates that a receiver is in the process of resuming the sender
 * stored in this cell. This transitional state prevents races between
 * concurrent receivers.
 *
 * Transliterated from: private val RESUMING_BY_RCV = Symbol("S_RESUMING_BY_RCV")
 */
inline internal::Symbol& RESUMING_BY_RCV() {
    static internal::Symbol instance("S_RESUMING_BY_RCV");
    return instance;
}

/**
 * Indicates that the expandBuffer procedure is in the process of resuming
 * the sender stored in this cell. Similar to RESUMING_BY_RCV but for the
 * buffer expansion path.
 *
 * Transliterated from: private val RESUMING_BY_EB = Symbol("RESUMING_BY_EB")
 */
inline internal::Symbol& RESUMING_BY_EB() {
    static internal::Symbol instance("RESUMING_BY_EB");
    return instance;
}

/**
 * Indicates that this cell is broken (poisoned) due to concurrent access.
 * When a receiver observes that the senders counter is greater than the
 * current index, but the cell is still in EMPTY or IN_BUFFER state, it
 * breaks (poisons) the cell to signal that something went wrong.
 *
 * Transliterated from: private val POISONED = Symbol("POISONED")
 */
inline internal::Symbol& POISONED() {
    static internal::Symbol instance("POISONED");
    return instance;
}

/**
 * Indicates that a receiver has completed processing this cell.
 * This terminal state is set after a receiver either retrieved an element
 * or made a rendezvous with a sender.
 *
 * Transliterated from: private val DONE_RCV = Symbol("DONE_RCV")
 */
inline internal::Symbol& DONE_RCV() {
    static internal::Symbol instance("DONE_RCV");
    return instance;
}

/**
 * Indicates that the sender waiting in this cell was interrupted (cancelled).
 * This allows the cell to be reused and the segment to be garbage collected.
 *
 * Transliterated from: private val INTERRUPTED_SEND = Symbol("INTERRUPTED_SEND")
 */
inline internal::Symbol& INTERRUPTED_SEND() {
    static internal::Symbol instance("INTERRUPTED_SEND");
    return instance;
}

/**
 * Indicates that the receiver waiting in this cell was interrupted (cancelled).
 * This allows the cell to be reused and the segment to be garbage collected.
 *
 * Transliterated from: private val INTERRUPTED_RCV = Symbol("INTERRUPTED_RCV")
 */
inline internal::Symbol& INTERRUPTED_RCV() {
    static internal::Symbol instance("INTERRUPTED_RCV");
    return instance;
}

/**
 * Indicates that the channel has been closed. This state is set in cells
 * when operations detect that the channel is closed, preventing further
 * operations on those cells.
 *
 * Transliterated from: internal val CHANNEL_CLOSED = Symbol("CHANNEL_CLOSED")
 */
inline internal::Symbol& CHANNEL_CLOSED() {
    static internal::Symbol instance("CHANNEL_CLOSED");
    return instance;
}

// ============================================================================
// Lines 3039-3041: Internal results for updateCellReceive
// ============================================================================
//
// These symbols are returned by updateCellReceive to indicate the outcome
// of attempting to process a cell for receiving.

/** Indicates the receive operation should suspend. */
inline internal::Symbol& SUSPEND() {
    static internal::Symbol instance("SUSPEND");
    return instance;
}

/** Indicates suspension is needed but no waiter was provided. */
inline internal::Symbol& SUSPEND_NO_WAITER() {
    static internal::Symbol instance("SUSPEND_NO_WAITER");
    return instance;
}

/** Indicates the cell processing failed and should be retried with next cell. */
inline internal::Symbol& FAILED() {
    static internal::Symbol instance("FAILED");
    return instance;
}

// ============================================================================
// Lines 3043-3051: Internal results for updateCellSend
// ============================================================================
//
// These constants are returned by updateCellSend to indicate the outcome
// of attempting to process a cell for sending.

/** A rendezvous with a receiver occurred - element was transferred directly. */
constexpr int RESULT_RENDEZVOUS = 0;

/** The element was successfully buffered in the cell. */
constexpr int RESULT_BUFFERED = 1;

/** The operation suspended and stored a waiter in the cell. */
constexpr int RESULT_SUSPEND = 2;

/** The operation needs to suspend but no waiter was provided. */
constexpr int RESULT_SUSPEND_NO_WAITER = 3;

/** The channel is closed. */
constexpr int RESULT_CLOSED = 4;

/** The cell processing failed (e.g., interrupted receiver) - retry with next cell. */
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
    // NOTE(port): Read the existing erased-storage tag through object bytes, not an unrelated type.
    uintptr_t marker;
    std::memcpy(&marker, state, sizeof(marker));
    return marker == WAITER_EB_MAGIC;
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
class ReceiveCatching : public Waiter, public std::enable_shared_from_this<ReceiveCatching<E>> {
public:
    std::shared_ptr<CancellableContinuationImpl<ChannelResult<E>>> cont_sp;
    CancellableContinuationImpl<ChannelResult<E>>* cont;

    explicit ReceiveCatching(std::shared_ptr<CancellableContinuationImpl<ChannelResult<E>>> c)
        : cont_sp(c), cont(c.get()) {}

    explicit ReceiveCatching(CancellableContinuationImpl<ChannelResult<E>>* c)
        : cont_sp(nullptr), cont(c) {}

    void invoke_on_cancellation(internal::SegmentBase* segment, int index) override {
        if (cont) {
            cont->invoke_on_cancellation(segment, index);
        }
    }

    std::shared_ptr<Waiter> shared_from_this_waiter() override {
        return this->shared_from_this();
    }
};

// ============================================================================
// Lines 234-236: SendBroadcast wrapper class
// ============================================================================

/**
 * Used by BroadcastChannel to wrap send continuations.
 * Implements Waiter by delegating to the underlying CancellableContinuationImpl.
 */
class SendBroadcast : public Waiter, public std::enable_shared_from_this<SendBroadcast> {
public:
    std::shared_ptr<CancellableContinuationImpl<bool>> cont;

    explicit SendBroadcast(std::shared_ptr<CancellableContinuationImpl<bool>> continuation)
        : cont(std::move(continuation)) {}

    void invoke_on_cancellation(internal::SegmentBase* segment, int index) override {
        cont->invoke_on_cancellation(segment, index);
    }

    std::shared_ptr<Waiter> shared_from_this_waiter() override {
        return shared_from_this();
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
    BufferedChannel<E>* channel_;

    // Two logical registers per slot: an owning element reference and a CAS state.
    std::array<std::shared_ptr<E>, SEGMENT_SIZE> elements_;
    std::atomic<void*> states_[SEGMENT_SIZE];

    // Owns waiters while their raw pointers are stored in state slots.
    // Raw state pointers are used for CAS; these references retain owned waiter objects.
    std::array<std::shared_ptr<Waiter>, SEGMENT_SIZE> waiter_refs_;

public:
    // Constructor
    ChannelSegment(int64_t id, ChannelSegment<E>* prev, BufferedChannel<E>* channel, int pointers)
        : internal::Segment<ChannelSegment<E>>(id, prev, pointers)
        , channel_(channel) {
        for (int i = 0; i < SEGMENT_SIZE; ++i) {
            states_[i].store(nullptr, std::memory_order_relaxed);
        }
    }

    BufferedChannel<E>* channel() const {
        assert(channel_ != nullptr);
        return channel_;
    }

    int number_of_slots() const override { return SEGMENT_SIZE; }

    // ########################################
    // # Manipulation with the Element Fields #
    // ########################################
    //
    // Lines 2817-2851: Each slot in the segment stores two values:
    // - An owning element reference in elements_[index]
    // - A CAS state in states_[index]
    //
    // The element field stores the value being sent through the channel.
    // Following the safe publication pattern, the element is stored BEFORE
    // updating the state, ensuring receivers always see a valid element.

    /** Stores an owning element reference before publishing the cell state. */
    void store_element(int index, E element) {
        set_element_lazy(index, std::make_shared<E>(std::move(element)));
    }

    /** Copies the element while retaining its loaded reference. */
    E get_element(int index) const {
        auto element = std::atomic_load_explicit(&elements_[index], std::memory_order_acquire);
        return element ? *element : E{};
    }

    /** Copies the element, then clears the stored reference. */
    E retrieve_element(int index) {
        E element = get_element(index);
        clean_element(index);
        return element;
    }

    /** Releases the stored element reference. */
    void clean_element(int index) {
        set_element_lazy(index, nullptr);
    }

    /** Publishes an element reference with release semantics. */
    void set_element_lazy(int index, std::shared_ptr<E> value) {
        std::atomic_store_explicit(&elements_[index], std::move(value), std::memory_order_release);
    }

    // ######################################
    // # Manipulation with the State Fields #
    // ######################################
    //
    // Lines 2853-2885: The state field tracks the cell's lifecycle:
    // - nullptr: empty cell, not yet processed
    // - Waiter*: waiting sender or receiver continuation
    // - WaiterEB*: waiter wrapped during expandBuffer procedure
    // - Symbol pointers: terminal states (BUFFERED, DONE_RCV, etc.)
    //
    // State transitions follow the cell state machine documented above.

    /**
     * Reads the current state of the specified slot.
     */
    void* get_state(int index) const {
        return states_[index].load(std::memory_order_acquire);
    }

    /**
     * Sets the state of the specified slot unconditionally.
     * Used when the caller has already established exclusive access.
     */
    void set_state(int index, void* value) {
        states_[index].store(value, std::memory_order_release);
    }

    /**
     * Atomically compares and sets the state of the specified slot.
     * Returns true if the CAS succeeded (state was 'from' and is now 'to').
     */
    bool cas_state(int index, void* from, void* to) {
        return states_[index].compare_exchange_strong(from, to,
            std::memory_order_acq_rel, std::memory_order_acquire);
    }

    /**
     * Atomically exchanges the state and returns the previous value.
     * Used for unconditional state updates that need the old value.
     */
    void* get_and_set_state(int index, void* update) {
        return states_[index].exchange(update, std::memory_order_acq_rel);
    }

    // ##################################
    // # C++ Waiter Lifetime Management #
    // ##################################

    // Store a shared_ptr to keep the waiter alive while its raw pointer is in the state slot.
    // Call this when storing a waiter in the segment state.
    void set_waiter_ref(int index, std::shared_ptr<Waiter> waiter) {
        std::atomic_store(&waiter_refs_[index], std::move(waiter));
    }

    // Clear the waiter ref when the waiter is no longer needed (resumed, cancelled, etc.)
    void clear_waiter_ref(int index) {
        std::atomic_store(&waiter_refs_[index], std::shared_ptr<Waiter>{});
    }

    // Get the waiter ref (for cases where we need to pass it on)
    std::shared_ptr<Waiter> get_waiter_ref(int index) const {
        return std::atomic_load(&waiter_refs_[index]);
    }

    // ########################
    // # Cancellation Support #
    // ########################
    //
    // Lines 2887-2921: Handles cancellation of waiters stored in this segment.
    // When a coroutine is cancelled while waiting in a cell, this method
    // updates the cell state to INTERRUPTED_SEND or INTERRUPTED_RCV
    // and invokes the onUndeliveredElement handler if applicable.
    //
    // The index encodes whether this is a sender or receiver:
    // - Senders add SEGMENT_SIZE to the index as a marker
    // - Receivers use the raw index

    /**
     * Called when a waiter stored in this segment is cancelled.
     * Updates the cell state machine and cleans up resources.
     *
     * Transliterated from: override fun onCancellation(index: Int, cause: Throwable?, context: CoroutineContext)
     */
    void on_cancellation(int index, std::exception_ptr cause,
                         std::shared_ptr<CoroutineContext> context) override {
        // senders equip the index value with an additional marker, adding SEGMENT_SIZE.
        bool is_sender = index >= SEGMENT_SIZE;
        // Unwrap the index.
        if (is_sender) index = index - SEGMENT_SIZE;

        E element = get_element(index);

        while (true) {
            void* cur = get_state(index);

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

            if (cur == static_cast<void*>(&INTERRUPTED_SEND()) ||
                cur == static_cast<void*>(&INTERRUPTED_RCV())) {
                clean_element(index);
                if (is_sender && channel_->on_undelivered_element()) {
                    channel_->on_undelivered_element()(element);
                }
                return;
            }

            if (cur == static_cast<void*>(&RESUMING_BY_EB()) ||
                cur == static_cast<void*>(&RESUMING_BY_RCV())) {
                continue;
            }

            if (cur == static_cast<void*>(&DONE_RCV()) ||
                cur == static_cast<void*>(&BUFFERED())) {
                return;
            }

            if (cur == static_cast<void*>(&CHANNEL_CLOSED())) {
                return;
            }

            assert(false && "unexpected state in onCancellation");
            return;
        }
    }

    /**
     * Called after a request (sender or receiver) in this cell is cancelled.
     * For receivers, waits for any in-progress expandBuffer to complete.
     * Then marks the slot as cleaned for segment garbage collection.
     *
     * Transliterated from: fun onCancelledRequest(index: Int, receiver: Boolean)
     */
    void on_cancelled_request(int index, bool receiver) {
        if (receiver) {
            channel_->wait_expand_buffer_completion(this->id * SEGMENT_SIZE + index);
        }
        this->on_slot_cleaned();
    }
};

// ============================================================================
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
    void* token = cont->try_resume(value, nullptr, on_cancellation);
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
    BufferedChannel(int capacity, OnUndeliveredElement<E> on_undelivered_element = nullptr)
        : capacity_(capacity)
        , on_undelivered_element_(on_undelivered_element)
        , senders_and_close_status_(0L)
        , receivers_(0L)
        , buffer_end_(initial_buffer_end(capacity))
        , completed_expand_buffers_and_pause_flag_(initial_buffer_end(capacity))
        , close_cause_(static_cast<void*>(&NO_CLOSE_CAUSE()))
        , close_handler_(nullptr) {
        if (capacity < 0 && capacity != CHANNEL_UNLIMITED) {
            throw std::invalid_argument("Invalid channel capacity: " + std::to_string(capacity) + ", should be >=0");
        }

        // @Suppress("LeakingThis")
        // val firstSegment = ChannelSegment(id = 0, prev = null, channel = this, pointers = 3)
        auto* first_segment = new ChannelSegment<E>(0, nullptr, this, 3);
        send_segment_.store(first_segment, std::memory_order_release);
        receive_segment_.store(first_segment, std::memory_order_release);

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

    OnUndeliveredElement<E> on_undelivered_element() const { return on_undelivered_element_; }

    // =========================================================================
    // Lines 63-103: Counters and segment references
    // =========================================================================

    int64_t senders_counter() const {
        return channels::senders_counter(senders_and_close_status_.load(std::memory_order_acquire));
    }

    int64_t receivers_counter() const {
        return receivers_.load(std::memory_order_acquire);
    }

    int64_t buffer_end_counter() const {
        return buffer_end_.load(std::memory_order_acquire);
    }

    bool is_rendezvous_or_unlimited() const {
        int64_t bec = buffer_end_counter();
        return bec == BUFFER_END_RENDEZVOUS || bec == BUFFER_END_UNLIMITED;
    }

    // =========================================================================
    // Lines 105-608: The send operations
    // =========================================================================
    //
    // The send algorithm works as follows:
    // 1. Increment the senders counter atomically to claim a cell
    // 2. Find or create the segment containing that cell
    // 3. Try to either:
    //    - Buffer the element (if capacity available)
    //    - Rendezvous with a waiting receiver
    //    - Suspend until a receiver arrives
    // 4. Handle closure/cancellation appropriately
    //
    // The sendImpl inline function (lines 241-348) is the core implementation
    // that all send variants (send, trySend, sendBroadcast) delegate to.

    /**
     * Sends the specified element to this channel, suspending if the buffer
     * is full or there are no waiting receivers (for rendezvous channels).
     *
     * This function can be cancelled while waiting for buffer space or a receiver.
     * If cancelled, throws CancellationException and the element is not delivered.
     * If the channel is closed, throws ClosedSendChannelException.
     *
     * Transliterated from: override suspend fun send(element: E): Unit
     */
    void* send(E element, Continuation<void*>* completion) override {
        // Lines 241-348: sendImpl inline function
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
                        return on_closed_send(element, std::shared_ptr<Continuation<void*>>(
                            completion, [](Continuation<void*>*){}));
                    } else {
                        continue;
                    }
                }
            }

            int result = update_cell_send(segment, i, element, s, nullptr, closed);

            switch (result) {
                case RESULT_RENDEZVOUS:
                    segment->clean_prev();
                    return nullptr;

                case RESULT_BUFFERED:
                    return nullptr;

                case RESULT_SUSPEND:
                    // but if closed, we installed INTERRUPTED_SEND
                    if (closed) {
                        segment->on_slot_cleaned();
                        return on_closed_send(element, std::shared_ptr<Continuation<void*>>(
                            completion, [](Continuation<void*>*){}));
                    }
                    assert(false && "RESULT_SUSPEND with null waiter");
                    return nullptr;

                case RESULT_CLOSED:
                    if (s < receivers_counter()) segment->clean_prev();
                    return on_closed_send(element, std::shared_ptr<Continuation<void*>>(
                        completion, [](Continuation<void*>*){}));

                case RESULT_FAILED:
                    segment->clean_prev();
                    continue;

                case RESULT_SUSPEND_NO_WAITER:
                    return send_on_no_waiter_suspend(segment, i, element, s, completion);

                default:
                    assert(false && "unexpected result from update_cell_send");
                    return nullptr;
            }
        }
    }

    /**
     * Attempts to send the specified element to this channel without suspending.
     *
     * Returns ChannelResult.success() if the element was sent (buffered or rendezvous).
     * Returns ChannelResult.failure() if the channel buffer is full and no receivers waiting.
     * Returns ChannelResult.closed() if the channel is closed.
     *
     * Unlike send(), this method never suspends and returns immediately.
     *
     * Transliterated from: override fun trySend(element: E): ChannelResult<Unit>
     */
    ChannelResult<void> try_send(E element) override {
        if (should_send_suspend(senders_and_close_status_.load(std::memory_order_acquire))) {
            return ChannelResult<void>::failure();
        }

        // This channel either has waiting receivers or is closed.
        // Try to send the element using sendImpl logic.
        return send_impl_try_send(std::move(element));
    }

    /**
     * Checks whether a send invocation would suspend given the current counters.
     * Returns false if the channel is closed (send would fail, not suspend).
     *
     * Transliterated from: private fun shouldSendSuspend(curSendersAndCloseStatus: Long): Boolean
     */
    bool should_send_suspend(int64_t cur_senders_and_close_status) const {
        if (is_closed_for_send_internal(cur_senders_and_close_status)) return false;
        return !buffer_or_rendezvous_send(channels::senders_counter(cur_senders_and_close_status));
    }

    /**
     * Returns true when the specified send should place its element to the
     * working cell without suspension. This happens when:
     * - The buffer has space (curSenders < bufferEnd)
     * - A rendezvous may happen (curSenders < receivers + capacity)
     *
     * Transliterated from: private fun bufferOrRendezvousSend(curSenders: Long): Boolean
     */
    bool buffer_or_rendezvous_send(int64_t cur_senders) const {
        return cur_senders < buffer_end_counter() ||
               cur_senders < receivers_counter() + capacity_;
    }

    /**
     * Checks whether a send invocation is bound to suspend with current values.
     * This is a convenience wrapper around should_send_suspend().
     *
     * Transliterated from: internal open fun shouldSendSuspend(): Boolean
     */
    virtual bool should_send_suspend() const {
        return should_send_suspend(senders_and_close_status_.load(std::memory_order_acquire));
    }

    /**
     * Sends an element for a broadcast subscription, returning an owned bool result.
     * Returns true when sent and false when the channel is closed.
     * An undelivered-element handler is unsupported.
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:218-236,244-349
    virtual void* send_broadcast(E element, Continuation<void*>* continuation) {
        return suspend_cancellable_coroutine<bool>([&](CancellableContinuation<bool>& continuation) {
            if (on_undelivered_element_) {
                throw std::logic_error("the `onUndeliveredElement` feature is unsupported for `sendBroadcast(e)`");
            }
            auto cont = dynamic_cast<CancellableContinuationImpl<bool>&>(continuation).shared_from_this();
            auto waiter = std::make_shared<SendBroadcast>(cont);
            ChannelSegment<E>* segment = send_segment_.load(std::memory_order_acquire);
            while (true) {
                int64_t current = senders_and_close_status_.fetch_add(1, std::memory_order_acq_rel);
                int64_t s = channels::senders_counter(current);
                bool closed = is_closed_for_send_internal(current);
                int64_t id = s / SEGMENT_SIZE;
                int i = static_cast<int>(s % SEGMENT_SIZE);
                if (segment->id != id) {
                    segment = find_segment_send(id, segment);
                    if (segment == nullptr) {
                        if (closed) {
                            cont->resume(false, nullptr);
                            return;
                        }
                        continue;
                    }
                }
                switch (update_cell_send(segment, i, element, s, waiter.get(), closed)) {
                    case RESULT_RENDEZVOUS:
                        segment->clean_prev();
                        cont->resume(true, nullptr);
                        return;
                    case RESULT_BUFFERED:
                        cont->resume(true, nullptr);
                        return;
                    case RESULT_SUSPEND:
                        if (closed) {
                            segment->on_slot_cleaned();
                            cont->resume(false, nullptr);
                        } else {
                            prepare_sender_for_suspension(waiter.get(), segment, i);
                        }
                        return;
                    case RESULT_CLOSED:
                        if (s < receivers_counter()) segment->clean_prev();
                        cont->resume(false, nullptr);
                        return;
                    case RESULT_FAILED:
                        segment->clean_prev();
                        continue;
                    default:
                        throw std::logic_error("unexpected broadcast send cell result");
                }
            }
        }, continuation);
    }

    // =========================================================================
    // Lines 671-1004: The receive operations
    // =========================================================================
    //
    // The receive algorithm is symmetric to send:
    // 1. Increment the receivers counter atomically to claim a cell
    // 2. Find the segment containing that cell
    // 3. Try to either:
    //    - Retrieve a buffered element
    //    - Rendezvous with a waiting sender
    //    - Suspend until a sender arrives
    // 4. After successful retrieval, call expandBuffer() to maintain buffer invariant
    //
    // Key difference from send: receive calls expandBuffer() after completion
    // to move the logical buffer end forward.

    /**
     * Receives an element from this channel, suspending if the channel is empty.
     *
     * Throws ClosedReceiveChannelException if the channel is closed (and empty).
     * Throws the close cause exception if the channel was cancelled.
     *
     * Transliterated from: override suspend fun receive(): E
     */
    void* receive(Continuation<void*>* continuation) override {
        ChannelSegment<E>* segment = receive_segment_.load(std::memory_order_acquire);
        while (true) {
            if (is_closed_for_receive()) {
                auto cause = receive_exception();
                if (cause) {
                    std::rethrow_exception(cause);
                }
                throw ClosedReceiveChannelException("Channel was closed");
            }
            int64_t r = receivers_.fetch_add(1, std::memory_order_acq_rel);
            int64_t id = r / SEGMENT_SIZE;
            int i = static_cast<int>(r % SEGMENT_SIZE);
            if (segment->id != id) {
                segment = find_segment_receive(id, segment);
                if (segment == nullptr) {
                    continue;
                }
            }
            void* upd_cell_result = update_cell_receive(segment, i, r, nullptr);
            if (upd_cell_result == static_cast<void*>(&SUSPEND())) {
                assert(false && "unexpected SUSPEND with null waiter");
            } else if (upd_cell_result == static_cast<void*>(&FAILED())) {
                if (r < senders_counter()) segment->clean_prev();
                continue;
            } else if (upd_cell_result == static_cast<void*>(&SUSPEND_NO_WAITER())) {
                return receive_on_no_waiter_suspend(segment, i, r, continuation);
            } else {
                segment->clean_prev();
                return upd_cell_result;
            }
        }
    }

    /**
     * Receives an element from this channel, wrapping the result in ChannelResult.
     *
     * Unlike receive(), this method does not throw on channel closure.
     * Instead, it returns ChannelResult.closed(cause) which can be inspected.
     *
     * Transliterated from: override suspend fun receiveCatching(): ChannelResult<E>
     */
    void* receive_catching(Continuation<void*>* continuation) override {
        ChannelSegment<E>* segment = receive_segment_.load(std::memory_order_acquire);
        while (true) {
            if (is_closed_for_receive()) {
                return new ChannelResult<E>(ChannelResult<E>::closed(close_cause()));
            }
            int64_t r = receivers_.fetch_add(1, std::memory_order_acq_rel);
            int64_t id = r / SEGMENT_SIZE;
            int i = static_cast<int>(r % SEGMENT_SIZE);
            if (segment->id != id) {
                segment = find_segment_receive(id, segment);
                if (segment == nullptr) {
                    continue;
                }
            }
            void* upd_cell_result = update_cell_receive(segment, i, r, nullptr);
            if (upd_cell_result == static_cast<void*>(&SUSPEND())) {
                assert(false && "unexpected SUSPEND with null waiter");
            } else if (upd_cell_result == static_cast<void*>(&FAILED())) {
                if (r < senders_counter()) segment->clean_prev();
                continue;
            } else if (upd_cell_result == static_cast<void*>(&SUSPEND_NO_WAITER())) {
                return receive_catching_on_no_waiter_suspend(segment, i, r, continuation);
            } else {
                segment->clean_prev();
                E* elem_ptr = static_cast<E*>(upd_cell_result);
                E elem = std::move(*elem_ptr);
                delete elem_ptr;
                return new ChannelResult<E>(ChannelResult<E>::success(std::move(elem)));
            }
        }
    }

    /**
     * Attempts to receive an element from this channel without suspending.
     *
     * Returns ChannelResult.success(element) if an element was available.
     * Returns ChannelResult.failure() if the channel is empty.
     * Returns ChannelResult.closed(cause) if the channel is closed.
     *
     * Unlike receive(), this method never suspends and returns immediately.
     *
     * Transliterated from: override fun tryReceive(): ChannelResult<E>
     */
    ChannelResult<E> try_receive() override {
        int64_t r = receivers_.load(std::memory_order_acquire);
        int64_t senders_and_close_status_cur = senders_and_close_status_.load(std::memory_order_acquire);

        if (is_closed_for_receive_internal(senders_and_close_status_cur)) {
            return ChannelResult<E>::closed(close_cause());
        }

        int64_t s = channels::senders_counter(senders_and_close_status_cur);
        if (r >= s) return ChannelResult<E>::failure();

        // Try to retrieve an element using receiveImpl logic
        return receive_impl_try_receive();
    }

    // =========================================================================
    // Lines 1186-1467: The expandBuffer() procedure
    // =========================================================================
    //
    // The expandBuffer() procedure is critical for buffered channels. After a
    // receiver retrieves an element, it calls expandBuffer() to move the
    // logical buffer end forward, allowing another sender to buffer an element.
    //
    // Algorithm overview:
    // 1. Increment bufferEnd counter to claim the next cell for buffering
    // 2. If no sender has reached this cell yet (senders <= bufferEnd),
    //    just update the segment reference and finish
    // 3. Otherwise, find the cell and update its state:
    //    - If a sender waiter is found, try to resume it (mark as BUFFERED)
    //    - If receiver might be in cell, wrap waiter in WaiterEB and delegate
    //    - Handle various terminal states (INTERRUPTED, POISONED, etc.)
    //
    // The procedure uses RESUMING_BY_EB as an intermediate state to synchronize
    // with concurrent receive operations trying to resume the same sender.

    /**
     * Expands the logical buffer by one cell.
     * Called after each successful receive to maintain the buffer size invariant.
     *
     * For rendezvous (capacity=0) or unlimited channels, this is a no-op
     * since those channels don't have a logical buffer to expand.
     *
     * Transliterated from: private fun expandBuffer()
     */
    void expand_buffer() {
        if (is_rendezvous_or_unlimited()) return;

        ChannelSegment<E>* segment = buffer_end_segment_.load(std::memory_order_acquire);

        while (true) {
            int64_t b = buffer_end_.fetch_add(1, std::memory_order_acq_rel);
            int64_t id = b / SEGMENT_SIZE;

            int64_t s = senders_counter();
            if (s <= b) {
                // Should bufferEndSegment be moved forward?
                if (segment->id < id && segment->next() != nullptr) {
                    move_segment_buffer_end_to_specified_or_last(id, segment);
                }
                inc_completed_expand_buffer_attempts();
                return;
            }

            if (segment->id != id) {
                segment = find_segment_buffer_end(id, segment, b);
                if (segment == nullptr) continue;
            }

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

    /**
     * Updates the working cell of the expandBuffer() procedure.
     * This is the fast-path that handles the common case of a sender waiter.
     *
     * Returns true if the cell was successfully added to the buffer,
     * false if the operation should skip this cell and try the next.
     *
     * Transliterated from: private fun updateCellExpandBuffer(segment, index, b): Boolean
     */
    bool update_cell_expand_buffer(ChannelSegment<E>* segment, int index, int64_t b) {
        // Fast-path: check if we can directly resume a sender
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
                    segment->clear_waiter_ref(index);
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

    /**
     * Slow-path for updateCellExpandBuffer when the fast-path CAS fails.
     * Handles all cell states according to the state machine.
     *
     * State transitions:
     * - Waiter (sender): CAS to RESUMING_BY_EB, try resume, set BUFFERED or INTERRUPTED_SEND
     * - Waiter (unknown): Wrap in WaiterEB to delegate to pairwise operation
     * - nullptr: CAS to IN_BUFFER
     * - BUFFERED/DONE_RCV/POISONED/etc.: Cell already processed, return true
     * - RESUMING_BY_RCV: Spin wait for concurrent operation
     *
     * Transliterated from: private fun updateCellExpandBufferSlow(segment, index, b): Boolean
     */
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
                        segment->clear_waiter_ref(index);
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

    /**
     * Increments the counter of completed expandBuffer() attempts.
     * Pauses if the PAUSE_EXPAND_BUFFERS flag is set to avoid starvation.
     *
     * Transliterated from: private fun incCompletedExpandBufferAttempts(nAttempts: Long = 1)
     */
    void inc_completed_expand_buffer_attempts(int64_t n_attempts = 1) {
        int64_t result = completed_expand_buffers_and_pause_flag_.fetch_add(n_attempts, std::memory_order_acq_rel) + n_attempts;
        if (eb_pause_expand_buffers(result)) {
            while (eb_pause_expand_buffers(completed_expand_buffers_and_pause_flag_.load(std::memory_order_acquire))) {
                // spin
            }
        }
    }

    /**
     * Waits for any in-progress expandBuffer() calls that may affect the cell
     * at the specified global index. This is called when a receiver is cancelled
     * to ensure the expandBuffer procedure won't touch the cancelled cell.
     *
     * The algorithm:
     * 1. Wait until bufferEnd > globalIndex (cell is covered by buffer)
     * 2. Spin briefly waiting for started == completed expandBuffer calls
     * 3. If spin fails, set PAUSE flag to prevent new expandBuffer calls
     * 4. Wait until counters match, then unset flag
     *
     * Transliterated from: private fun waitExpandBufferCompletion(globalIndex: Long)
     */
    void wait_expand_buffer_completion(int64_t global_index) {
        if (is_rendezvous_or_unlimited()) return;

        while (buffer_end_counter() <= global_index) {
            // spin
        }

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

    std::unique_ptr<ChannelIterator<E>> iterator() override {
        return std::make_unique<IteratorHandle>(std::make_shared<BufferedChannelIterator>(this));
    }

    // =========================================================================
    // Lines 1746-2211: Closing and Cancellation
    // =========================================================================
    //
    // Channel close status transitions:
    //   ACTIVE → CLOSED        (via close())
    //   ACTIVE → CANCELLATION_STARTED → CANCELLED (via cancel())
    //   CLOSED → CANCELLED     (via cancel() after close())
    //
    // Closing a channel:
    // - Prevents new send() operations
    // - Allows pending receives to complete
    // - Does not remove buffered elements
    //
    // Cancelling a channel:
    // - Prevents new send() and receive() operations
    // - Removes all buffered elements (calling onUndeliveredElement)
    // - Resumes all waiting senders/receivers with CancellationException

    /**
     * Returns the close cause if this channel is closed/cancelled with a cause,
     * or nullptr if not closed or closed without a cause.
     */
    std::exception_ptr close_cause() const {
        void* cause = close_cause_.load(std::memory_order_acquire);
        if (cause == static_cast<void*>(&NO_CLOSE_CAUSE()) || cause == nullptr) {
            return nullptr;
        }
        return *reinterpret_cast<std::exception_ptr*>(cause);
    }

    std::exception_ptr send_exception() const {
        auto cause = close_cause();
        if (cause) return cause;
        return std::make_exception_ptr(ClosedSendChannelException("Channel was closed"));
    }

    std::exception_ptr receive_exception() const {
        auto cause = close_cause();
        if (cause) return cause;
        return std::make_exception_ptr(ClosedReceiveChannelException("Channel was closed"));
    }

    /**
     * Closes this channel for sending. All subsequent send() calls will throw
     * ClosedSendChannelException. Buffered elements remain available for receive.
     *
     * Returns true if this invocation closed the channel, false if already closed.
     *
     * Transliterated from: override fun close(cause: Throwable?): Boolean
     */
    bool close(std::exception_ptr cause = nullptr) override {
        return close_or_cancel_impl(cause, false);
    }

    /**
     * Cancels this channel with the specified cause.
     * Returns true if this invocation cancelled the channel.
     */
    bool cancel_with_cause(std::exception_ptr cause) {
        return cancel_impl(cause);
    }

    /**
     * Cancels this channel. All buffered elements are discarded (calling
     * onUndeliveredElement if set), and all waiting senders/receivers are
     * resumed with CancellationException.
     *
     * Transliterated from: override fun cancel(cause: CancellationException?)
     */
    void cancel(std::exception_ptr cause = nullptr) override {
        cancel_impl(cause);
    }

    /**
     * Internal cancel implementation. Creates a default cancellation exception
     * if cause is null, then delegates to close_or_cancel_impl.
     *
     * Transliterated from: protected open fun cancelImpl(cause: Throwable?): Boolean
     */
    virtual bool cancel_impl(std::exception_ptr cause) {
        if (!cause) {
            cause = std::make_exception_ptr(std::runtime_error("Channel was cancelled"));
        }
        return close_or_cancel_impl(cause, true);
    }

    // `open` in Kotlin means virtual in C++
    virtual bool close_or_cancel_impl(std::exception_ptr cause, bool cancel) {
        if (cancel) mark_cancellation_started();

        void* no_cause = static_cast<void*>(&NO_CLOSE_CAUSE());
        void* cause_ptr = cause ? new std::exception_ptr(cause) : nullptr;
        bool closed_by_this_operation = close_cause_.compare_exchange_strong(
            no_cause, cause_ptr, std::memory_order_acq_rel, std::memory_order_acquire);

        if (!closed_by_this_operation && cause_ptr) {
            delete reinterpret_cast<std::exception_ptr*>(cause_ptr);
        }

        if (cancel) {
            mark_cancelled();
        } else {
            mark_closed();
        }

        complete_close_or_cancel();

        on_closed_idempotent();
        if (closed_by_this_operation) {
            invoke_close_handler_internal();
        }

        return closed_by_this_operation;
    }

    virtual void on_closed_idempotent() {}

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

    void complete_close_or_cancel() {
        // must finish the started close/cancel if one is detected.
        (void)is_closed_for_send();
    }

    bool is_closed_for_send() const override {
        return is_closed_for_send_internal(senders_and_close_status_.load(std::memory_order_acquire));
    }

    bool is_closed_for_send_internal(int64_t senders_and_close_status_cur) const {
        return is_closed(senders_and_close_status_cur, false);
    }

    bool is_closed_for_receive() const override {
        return is_closed_for_receive_internal(senders_and_close_status_.load(std::memory_order_acquire));
    }

    bool is_closed_for_receive_internal(int64_t senders_and_close_status_cur) const {
        return is_closed(senders_and_close_status_cur, true);
    }

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

    bool is_empty() const override {
        if (is_closed_for_receive()) return false;
        if (has_elements()) return false;
        return !is_closed_for_receive();
    }

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

    /**
     * Upstream:
     *   override val onSend: SelectClause2<E, BufferedChannel<E>>
     *       get() = SelectClause2Impl(
     *           clauseObject = this,
     *           regFunc = BufferedChannel<*>::registerSelectForSend as RegistrationFunction,
     *           processResFunc = BufferedChannel<*>::processResultSelectSend as ProcessResultFunction)
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1475-1480
    selects::SelectClause2<E, SendChannel<E>*>& on_send() override {
        if (!on_send_clause_) {
            on_send_clause_ = std::make_unique<selects::SelectClause2Impl<E, SendChannel<E>*>>(
                /*clauseObject=*/this,
                /*regFunc=*/[this](void* /*clause*/, void* select, void* element) {
                    this->register_select_for_send(
                        static_cast<selects::SelectInstance<void*>*>(select),
                        static_cast<E*>(element));
                },
                /*processResFunc=*/[this](void* /*clause*/, void* ignored, void* result) {
                    auto* channel = static_cast<BufferedChannel<E>*>(
                        this->process_result_select_send(ignored, result));
                    // NOTE(port): Project the actual receiver to its virtual interface before erasure.
                    return static_cast<void*>(static_cast<SendChannel<E>*>(channel));
                });
        }
        return *on_send_clause_;
    }

    /**
     * Upstream:
     *   override val onReceive: SelectClause1<E>
     *       get() = SelectClause1Impl(
     *           clauseObject = this,
     *           regFunc = BufferedChannel<*>::registerSelectForReceive as RegistrationFunction,
     *           processResFunc = BufferedChannel<*>::processResultSelectReceive as ProcessResultFunction,
     *           onCancellationConstructor = onUndeliveredElementReceiveCancellationConstructor)
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1504-1510
    selects::SelectClause1<E>& on_receive() override {
        if (!on_receive_clause_) {
            on_receive_clause_ = std::make_unique<selects::SelectClause1Impl<E>>(
                /*clauseObject=*/this,
                /*regFunc=*/[this](void* /*clause*/, void* select, void* ignored) {
                    this->register_select_for_receive(
                        static_cast<selects::SelectInstance<void*>*>(select), ignored);
                },
                /*processResFunc=*/[this](void* /*clause*/, void* ignored, void* result) {
                    auto* box = static_cast<E*>(this->process_result_select_receive(ignored, result));
                    if constexpr (std::is_pointer_v<E>) {
                        // NOTE(port): Unbox the pointer value without adopting its borrowed pointee.
                        std::unique_ptr<E> owner(box);
                        return static_cast<void*>(*owner);
                    } else {
                        return static_cast<void*>(box);
                    }
                },
                select_receive_cancellation_constructor());
        }
        return *on_receive_clause_;
    }

    /**
     * Upstream:
     *   override val onReceiveCatching: SelectClause1<ChannelResult<E>>
     *       get() = SelectClause1Impl(
     *           clauseObject = this, regFunc = registerSelectForReceive,
     *           processResFunc = processResultSelectReceiveCatching,
     *           onCancellationConstructor = onUndeliveredElementReceiveCancellationConstructor)
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1513-1519
    selects::SelectClause1<ChannelResult<E>>& on_receive_catching() override {
        if (!on_receive_catching_clause_) {
            on_receive_catching_clause_ =
                std::make_unique<selects::SelectClause1Impl<ChannelResult<E>>>(
                    /*clauseObject=*/this,
                    /*regFunc=*/[this](void* /*clause*/, void* select, void* ignored) {
                        this->register_select_for_receive(
                            static_cast<selects::SelectInstance<void*>*>(select), ignored);
                    },
                    /*processResFunc=*/[this](void* /*clause*/, void* ignored, void* result) {
                        return this->process_result_select_receive_catching(ignored, result);
                    },
                    select_receive_cancellation_constructor());
        }
        return *on_receive_catching_clause_;
    }

    // =========================================================================
    // Lines 2591-2697: Debug Functions
    // =========================================================================

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
    int capacity_;
    OnUndeliveredElement<E> on_undelivered_element_;

    virtual bool is_conflated_drop_oldest() const { return false; }

    // -------------------------------------------------------------------------
    // Lines 352-371: protected fun trySendDropOldest(element: E): ChannelResult<Unit>
    // Note: temporarily in BufferedChannel due to KT-65554
    // -------------------------------------------------------------------------
    ChannelResult<void> try_send_drop_oldest(E element) {
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
                static_cast<void*>(&BUFFERED()), closed);

            switch (result) {
                case RESULT_RENDEZVOUS:
                    segment->clean_prev();
                    return ChannelResult<void>::success();
                case RESULT_BUFFERED:
                    return ChannelResult<void>::success();
                case RESULT_SUSPEND:
                    drop_first_element_until_the_specified_cell_is_in_the_buffer(
                        segment->id * SEGMENT_SIZE + i);
                    return ChannelResult<void>::success();
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

private:
    // Lazy clause caches for the on_send / on_receive / on_receive_catching overrides.
    // Upstream's `val onSend: SelectClause2 get() = SelectClause2Impl(...)` allocates a fresh
    // clause object on every access; we cache one per channel since the clause's identity is
    // always `this`.
    std::unique_ptr<selects::SelectClause2Impl<E, SendChannel<E>*>> on_send_clause_;
    std::unique_ptr<selects::SelectClause1Impl<E>> on_receive_clause_;
    std::unique_ptr<selects::SelectClause1Impl<ChannelResult<E>>> on_receive_catching_clause_;

    // =========================================================================
    // Lines 63-91: Counters and state
    // =========================================================================

    std::atomic<int64_t> senders_and_close_status_;

    mutable std::atomic<int64_t> receivers_;

    std::atomic<int64_t> buffer_end_;

    std::atomic<int64_t> completed_expand_buffers_and_pause_flag_;

    std::atomic<ChannelSegment<E>*> send_segment_;
    mutable std::atomic<ChannelSegment<E>*> receive_segment_;
    std::atomic<ChannelSegment<E>*> buffer_end_segment_;

    mutable std::atomic<void*> close_cause_;

    std::atomic<void*> close_handler_;

    // =========================================================================
    // Private helper methods
    // =========================================================================

    // -------------------------------------------------------------------------
    // Lines 131-139: private suspend fun onClosedSend(element: E): Unit
    // NB: return type could've been Nothing, but it breaks TCO
    // -------------------------------------------------------------------------
    void* on_closed_send(E element, std::shared_ptr<Continuation<void*>> completion) {
        std::exception_ptr ex = call_undelivered_element_catching_exception(element);
        if (ex) {
            // Upstream: cause?.addSuppressed(this)
            // C++ has no Throwable.addSuppressed analogue, so the original exception is
            // rethrown as-is — the C++ caller still observes a single exception value.
            completion->resume_with(Result<void*>::failure(ex));
            return COROUTINE_SUSPENDED;
        }
        completion->resume_with(Result<void*>::failure(send_exception()));
        return COROUTINE_SUSPENDED;
    }

    // -------------------------------------------------------------------------
    // Lines 141-164: private suspend fun sendOnNoWaiterSuspend(...)
    // -------------------------------------------------------------------------
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:141-164
    void* send_on_no_waiter_suspend(
        ChannelSegment<E>* segment,
        int index,
        E element,
        int64_t s,
        Continuation<void*>* completion
    ) {
        return dsl::suspend_cancellable_coroutine_reusable_void(completion, [&](auto* cont) {
            send_impl_on_no_waiter(
                segment, index, element, s,
                cont,
                [cont]() { cont->resume(nullptr); },
                [this, element, cont]() { on_closed_send_on_no_waiter_suspend(element, cont); }
            );
        });
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
        // senders equip the index value with an additional marker,
        // adding SEGMENT_SIZE to the value.
        waiter->invoke_on_cancellation(segment, index + SEGMENT_SIZE);
    }

    // -------------------------------------------------------------------------
    // Lines 178-181: private fun onClosedSendOnNoWaiterSuspend(...)
    // -------------------------------------------------------------------------
    void on_closed_send_on_no_waiter_suspend(E element, CancellableContinuation<void>* cont) {
        // Note: C++ OnUndeliveredElement takes only the element, context is implicit
        if (on_undelivered_element_) {
            on_undelivered_element_(element);
        }
        cont->resume_with(Result<void>::failure(send_exception()));
    }

    // -------------------------------------------------------------------------
    // Lines 708-733: private suspend fun receiveOnNoWaiterSuspend(...)
    // -------------------------------------------------------------------------
    void* receive_on_no_waiter_suspend(
        ChannelSegment<E>* segment,
        int index,
        int64_t r,
        Continuation<void*>* completion
    ) {
        return dsl::suspend_cancellable_coroutine_reusable<E>(completion, [&](auto* cont) {
            receive_impl_on_no_waiter(
                segment, index, r,
                cont,
                [this, cont](E element) {
                    std::function<void(std::exception_ptr)> on_cancellation = nullptr;
                    if (on_undelivered_element_) {
                        on_cancellation = [this, element](std::exception_ptr) {
                            on_undelivered_element_(element);
                        };
                    }
                    cont->resume(std::move(element), on_cancellation);
                },
                [this, cont]() { on_closed_receive_on_no_waiter_suspend(cont); }
            );
        });
    }

    // -------------------------------------------------------------------------
    // Lines 762-776: private suspend fun receiveCatchingOnNoWaiterSuspend(...)
    // -------------------------------------------------------------------------
    void* receive_catching_on_no_waiter_suspend(
        ChannelSegment<E>* segment,
        int index,
        int64_t r,
        Continuation<void*>* completion
    ) {
        return dsl::suspend_cancellable_coroutine_reusable<ChannelResult<E>>(completion, [&](auto* cont) {
            auto waiter = std::make_shared<ReceiveCatching<E>>(cont->shared_from_this());
            receive_impl_on_no_waiter(
                segment, index, r,
                waiter.get(),
                [this, cont](E element) {
                    std::function<void(std::exception_ptr)> on_cancellation = nullptr;
                    if (on_undelivered_element_) {
                        on_cancellation = [this, element](std::exception_ptr) {
                            on_undelivered_element_(element);
                        };
                    }
                    cont->resume(ChannelResult<E>::success(std::move(element)), on_cancellation);
                },
                [this, cont]() { on_closed_receive_catching_on_no_waiter_suspend(cont); }
            );
        });
    }

    // -------------------------------------------------------------------------
    // Lines 735-738: private fun Waiter.prepareReceiverForSuspension(...)
    // -------------------------------------------------------------------------
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:735-738
    void prepare_receiver_for_suspension(Waiter* waiter, ChannelSegment<E>* segment, int index) {
        // C++ lifetime management: store the shared_ptr to keep the waiter alive
        if (auto sp = waiter->shared_from_this_waiter()) {
            segment->set_waiter_ref(index, sp);
        }
        on_receive_enqueued();
        waiter->invoke_on_cancellation(segment, index);
    }

    // -------------------------------------------------------------------------
    // Lines 740-742: private fun onClosedReceiveOnNoWaiterSuspend(...)
    // -------------------------------------------------------------------------
    template<typename T>
    void on_closed_receive_on_no_waiter_suspend(CancellableContinuation<T>* cont) {
        cont->resume_with(Result<T>::failure(receive_exception()));
    }

    // -------------------------------------------------------------------------
    // Lines 778-780: private fun onClosedReceiveCatchingOnNoWaiterSuspend(...)
    // -------------------------------------------------------------------------
    void on_closed_receive_catching_on_no_waiter_suspend(CancellableContinuation<ChannelResult<E>>* cont) {
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
        void* upd_cell_result = update_cell_receive(segment, index, r, waiter);

        if (upd_cell_result == static_cast<void*>(&SUSPEND())) {
            prepare_receiver_for_suspension(waiter, segment, index);
        } else if (upd_cell_result == static_cast<void*>(&FAILED())) {
            if (r < senders_counter()) segment->clean_prev();
            receive_impl_with_waiter(waiter, on_element_retrieved, on_closed);
        } else {
            segment->clean_prev();
            auto* elem_ptr = static_cast<E*>(upd_cell_result);
            E elem = std::move(*elem_ptr);
            delete elem_ptr;
            on_element_retrieved(std::move(elem));
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
        if (on_undelivered_element_) {
            on_undelivered_element_(element);
        }
        select->select_in_registration_phase(static_cast<void*>(&CHANNEL_CLOSED()));
    }

    // -------------------------------------------------------------------------
    // Lines 1499-1501: private fun processResultSelectSend(ignoredParam: Any?, selectResult: Any?): Any?
    // -------------------------------------------------------------------------
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1499-1501
    void* process_result_select_send(void* /*ignored_param*/, void* select_result) {
        if (select_result == static_cast<void*>(&CHANNEL_CLOSED())) {
            std::rethrow_exception(send_exception());
        }
        return static_cast<void*>(this);
    }

    // -------------------------------------------------------------------------
    // Lines 1483-1490: protected open fun registerSelectForSend(select: SelectInstance<*>, element: Any?)
    // -------------------------------------------------------------------------
protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1483-1490
    virtual void register_select_for_send(selects::SelectInstance<void*>* select, void* element_any) {
        E element = *static_cast<E*>(element_any);
        // Upstream calls into the inline `sendImpl(...)` machinery. In the C++ port the
        // inline call site is `send_impl`, which routes through the same four
        // onRendezvousOrBuffered / onSuspend / onClosed / onNoWaiterSuspend continuations
        // the Kotlin source uses, applied to a select-aware waiter.
        send_impl(
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
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1539-1541
    void on_closed_select_on_receive(selects::SelectInstance<void*>* select) {
        select->select_in_registration_phase(static_cast<void*>(&CHANNEL_CLOSED()));
    }

    // -------------------------------------------------------------------------
    // Lines 1544-1546: private fun processResultSelectReceive(ignoredParam: Any?, selectResult: Any?): Any?
    // -------------------------------------------------------------------------
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1544-1546
    void* process_result_select_receive(void* /*ignored_param*/, void* select_result) {
        if (select_result == static_cast<void*>(&CHANNEL_CLOSED())) {
            std::rethrow_exception(receive_exception());
        }
        return select_result;
    }

    // -------------------------------------------------------------------------
    // Lines 1549-1553: private fun processResultSelectReceiveOrNull(ignoredParam: Any?, selectResult: Any?): Any?
    // -------------------------------------------------------------------------
    void* process_result_select_receive_or_null(void* /*ignored_param*/, void* select_result) {
        if (select_result == static_cast<void*>(&CHANNEL_CLOSED())) {
            if (!close_cause()) {
                return nullptr;
            } else {
                std::rethrow_exception(receive_exception());
            }
        }
        return select_result;
    }

    // -------------------------------------------------------------------------
    // Lines 1556-1558: private fun processResultSelectReceiveCatching(ignoredParam: Any?, selectResult: Any?): Any?
    // -------------------------------------------------------------------------
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1556-1558
    void* process_result_select_receive_catching(void* /*ignored_param*/, void* select_result) {
        if (select_result == static_cast<void*>(&CHANNEL_CLOSED())) {
            // Return a boxed ChannelResult::closed
            auto* result = new ChannelResult<E>(ChannelResult<E>::closed(close_cause()));
            return static_cast<void*>(result);
        }
        // NOTE(port): The result adapter owns and releases the erased element box.
        std::unique_ptr<E> element(static_cast<E*>(select_result));
        auto* result = new ChannelResult<E>(ChannelResult<E>::success(std::move(*element)));
        return static_cast<void*>(result);
    }

    // -------------------------------------------------------------------------
    // Lines 1531-1537: private fun registerSelectForReceive(select: SelectInstance<*>, ignoredParam: Any?)
    // -------------------------------------------------------------------------
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1531-1537
    void register_select_for_receive(selects::SelectInstance<void*>* select, void* /*ignored_param*/) {
        receive_impl(
            select,
            [select](E elem) { select->select_in_registration_phase(new E(std::move(elem))); },
            [](ChannelSegment<E>*, int, int64_t) {},
            [this, select]() { on_closed_select_on_receive(select); }
        );
    }

    // -------------------------------------------------------------------------
    // Lines 1561-1567: private val onUndeliveredElementReceiveCancellationConstructor
    // -------------------------------------------------------------------------
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1561-1567
    selects::OnCancellationConstructor get_on_undelivered_element_receive_cancellation_constructor() {
        if (!on_undelivered_element_) {
            return nullptr;
        }
        return [this](void* /*select*/, void* /*param*/, void* element) -> selects::OnCancellationAction {
            return [this, element](std::exception_ptr, void*, std::shared_ptr<CoroutineContext>) {
                if (element != static_cast<void*>(&CHANNEL_CLOSED())) {
                    on_undelivered_element_(*static_cast<E*>(element));
                }
            };
        };
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1561-1567
    selects::OnCancellationConstructor select_receive_cancellation_constructor() {
        // NOTE(port): A cancelled receiving adapter must also release its owning ABI box,
        // including when Kotlin has no user-supplied undelivered-element callback.
        auto constructor = get_on_undelivered_element_receive_cancellation_constructor();
        return [constructor](void* select, void* param, void* element) -> selects::OnCancellationAction {
            auto action = constructor ? constructor(select, param, element) : nullptr;
            return [element, action](std::exception_ptr cause, void* value, std::shared_ptr<CoroutineContext> context) {
                std::unique_ptr<E> owner(element == &CHANNEL_CLOSED() ? nullptr : static_cast<E*>(element));
                if (action) action(cause, value, std::move(context));
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
        int result = update_cell_send(segment, index, element, s, waiter, false);
        switch (result) {
            case RESULT_RENDEZVOUS:
                segment->clean_prev();
                on_rendezvous_or_buffered();
                break;
            case RESULT_BUFFERED:
                on_rendezvous_or_buffered();
                break;
            case RESULT_SUSPEND:
                prepare_sender_for_suspension(waiter, segment, index);
                break;
            case RESULT_CLOSED:
                if (s < receivers_counter()) segment->clean_prev();
                on_closed();
                break;
            case RESULT_FAILED:
                segment->clean_prev();
                // Upstream's inline `sendImpl` recursively retries the cell acquisition
                // via the same `send_impl_on_no_waiter` path. The C++ port factors out
                // the retry into `send_impl_with_waiter`, which re-runs the segment
                // acquisition + cell update with the existing waiter handle.
                send_impl_with_waiter(element, waiter, on_rendezvous_or_buffered, on_closed);
                break;
            default:
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

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:241-349,1483-1490
    void send_impl(
        E element,
        selects::SelectInstance<void*>* select,
        std::function<void()> on_rendezvous_or_buffered,
        std::function<void(ChannelSegment<E>*, int)> on_suspend,
        std::function<void()> on_closed
    ) {
        // NOTE(port): Kotlin's checked Waiter cast adjusts the multiple-inheritance subobject.
        auto* waiter = dynamic_cast<Waiter*>(select);
        if (!waiter) throw std::bad_cast();
        ChannelSegment<E>* segment = send_segment_.load(std::memory_order_acquire);
        while (true) {
            int64_t current = senders_and_close_status_.fetch_add(1, std::memory_order_acq_rel);
            int64_t s = channels::senders_counter(current);
            bool closed = is_closed_for_send_internal(current);
            int64_t id = s / SEGMENT_SIZE;
            int i = static_cast<int>(s % SEGMENT_SIZE);
            if (segment->id != id) {
                auto* found = find_segment_send(id, segment);
                if (!found) {
                    if (closed) { on_closed(); return; }
                    continue;
                }
                segment = found;
            }
            switch (update_cell_send(segment, i, element, s, waiter, closed)) {
                case RESULT_RENDEZVOUS:
                    segment->clean_prev();
                    on_rendezvous_or_buffered();
                    return;
                case RESULT_BUFFERED:
                    on_rendezvous_or_buffered();
                    return;
                case RESULT_SUSPEND:
                    if (closed) {
                        segment->on_slot_cleaned();
                        on_closed();
                        return;
                    }
                    prepare_sender_for_suspension(waiter, segment, i);
                    on_suspend(segment, i);
                    return;
                case RESULT_CLOSED:
                    if (s < receivers_counter()) segment->clean_prev();
                    on_closed();
                    return;
                case RESULT_FAILED:
                    segment->clean_prev();
                    continue;
                case RESULT_SUSPEND_NO_WAITER:
                    throw std::logic_error("unexpected");
            }
        }
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:875-961,1531-1537
    void receive_impl(
        selects::SelectInstance<void*>* select,
        std::function<void(E)> on_element_retrieved,
        std::function<void(ChannelSegment<E>*, int, int64_t)> on_suspend,
        std::function<void()> on_closed
    ) {
        // NOTE(port): Preserve the actual Waiter subobject in erased cell storage.
        auto* waiter = dynamic_cast<Waiter*>(select);
        if (!waiter) throw std::bad_cast();
        ChannelSegment<E>* segment = receive_segment_.load(std::memory_order_acquire);
        while (true) {
            if (is_closed_for_receive()) { on_closed(); return; }
            int64_t r = receivers_.fetch_add(1, std::memory_order_acq_rel);
            int64_t id = r / SEGMENT_SIZE;
            int i = static_cast<int>(r % SEGMENT_SIZE);
            if (segment->id != id) {
                auto* found = find_segment_receive(id, segment);
                if (!found) continue;
                segment = found;
            }
            void* result = update_cell_receive(segment, i, r, waiter);
            if (result == &SUSPEND()) {
                prepare_receiver_for_suspension(waiter, segment, i);
                on_suspend(segment, i, r);
                return;
            }
            if (result == &FAILED()) {
                if (r < senders_counter()) segment->clean_prev();
                continue;
            }
            if (result == &SUSPEND_NO_WAITER()) throw std::logic_error("unexpected");
            segment->clean_prev();
            // NOTE(port): Transfer the owning ABI result into the source E callback.
            std::unique_ptr<E> element(static_cast<E*>(result));
            on_element_retrieved(std::move(*element));
            return;
        }
    }

public:
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
    // -------------------------------------------------------------------------
    void resume_receiver_on_closed_channel(Waiter* waiter) {
        resume_waiter_on_closed_channel(waiter, /* receiver = */ true);
    }

    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    void resume_sender_on_cancelled_channel(Waiter* waiter) {
        resume_waiter_on_closed_channel(waiter, /* receiver = */ false);
    }

    // -------------------------------------------------------------------------
    // Lines 2201-2209: private fun Waiter.resumeWaiterOnClosedChannel(receiver: Boolean)
    // -------------------------------------------------------------------------
    void resume_waiter_on_closed_channel(Waiter* waiter, bool receiver) {
        // Check if it's a SendBroadcast
        if (auto* sb = dynamic_cast<SendBroadcast*>(waiter)) {
            sb->cont->resume_with(Result<bool>::success(false));
            return;
        }
        if (receiver) {
            if (auto* cont = dynamic_cast<CancellableContinuationImpl<E>*>(waiter)) {
                cont->resume_with(Result<E>::failure(receive_exception()));
                return;
            }
        } else {
            if (auto* cont = dynamic_cast<CancellableContinuationImpl<void>*>(waiter)) {
                cont->resume_with(Result<void>::failure(send_exception()));
                return;
            }
        }
        // Check if it's a ReceiveCatching
        if (auto* rc = dynamic_cast<ReceiveCatching<E>*>(waiter)) {
            rc->cont->resume_with(Result<ChannelResult<E>>::success(ChannelResult<E>::closed(close_cause())));
            return;
        }
        // Check if it's a BufferedChannelIterator
        if (auto* iter = dynamic_cast<BufferedChannelIterator*>(waiter)) {
            iter->try_resume_has_next_on_closed_channel();
            return;
        }
        // Check if it's a SelectInstance
        if (auto* sel = dynamic_cast<selects::SelectInstance<void*>*>(waiter)) {
            sel->try_select(static_cast<void*>(this), static_cast<void*>(&CHANNEL_CLOSED()));
            return;
        }
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
                std::unique_ptr<E> element(static_cast<E*>(result));
                return ChannelResult<E>::success(std::move(*element));
            }
        }
    }

    int update_cell_send(ChannelSegment<E>* segment, int index, E element,
                         int64_t s, void* waiter, bool closed) {
        // Fast path
        segment->store_element(index, element);
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

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:501-605
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
                void* receiver = get_waiter(state);

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

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1054-1164
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
                    bool help_expand_buffer = is_waiter_eb(state);
                    void* sender = get_waiter(state);

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

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:652-671
    bool try_resume_receiver(void* receiver, E element) {
        if (auto* select = dynamic_cast<selects::SelectInstance<void*>*>(static_cast<Waiter*>(receiver))) {
            // NOTE(port): A rejected select rendezvous retains no result box.
            auto result = std::make_unique<E>(std::move(element));
            if (!select->try_select(static_cast<void*>(this), result.get())) return false;
            result.release();
            return true;
        }
        if (auto* rc = dynamic_cast<ReceiveCatching<E>*>(static_cast<Waiter*>(receiver))) {
            auto on_cancellation = bind_cancellation_fun_result();
            return try_resume_0(rc->cont, ChannelResult<E>::success(element), on_cancellation);
        }
        if (auto* iter = dynamic_cast<BufferedChannelIterator*>(static_cast<Waiter*>(receiver))) {
            return iter->try_resume_has_next(element);
        }
        if (auto* cont = dynamic_cast<CancellableContinuationImpl<E>*>(static_cast<Waiter*>(receiver))) {
            auto on_cancellation = bind_cancellation_fun();
            return try_resume_0(cont, element, on_cancellation);
        }
        throw std::runtime_error("Unexpected receiver type in tryResumeReceiver");
    }

    bool try_resume_sender(void* sender, ChannelSegment<E>* segment, int index) {
        if (auto* cont = dynamic_cast<CancellableContinuationImpl<void>*>(static_cast<Waiter*>(sender))) {
            // For void/Unit, tryResume0 just needs to be called
            void* token = cont->try_resume(nullptr);
            if (token != nullptr) {
                cont->complete_resume(token);
                return true;
            }
            return false;
        }
        if (auto* select = dynamic_cast<selects::SelectImplementation<void*>*>(static_cast<Waiter*>(sender))) {
            auto try_select_result = select->try_select_detailed(static_cast<void*>(this), nullptr);
            if (try_select_result == selects::TrySelectDetailedResult::REREGISTER) {
                segment->clean_element(index);
            }
            return try_select_result == selects::TrySelectDetailedResult::SUCCESSFUL;
        }
        if (auto* sb = dynamic_cast<SendBroadcast*>(static_cast<Waiter*>(sender))) {
            void* token = sb->cont->try_resume(true, nullptr);
            if (token != nullptr) {
                sb->cont->complete_resume(token);
                return true;
            }
            return false;
        }
        throw std::runtime_error("Unexpected sender type in tryResumeSender");
    }

    virtual void on_receive_enqueued() {}

    virtual void on_receive_dequeued() {}

    // -------------------------------------------------------------------------
    // From OnUndeliveredElement.kt Lines 8-24:
    // internal fun <E> OnUndeliveredElement<E>.callUndeliveredElementCatchingException(...)
    // -------------------------------------------------------------------------
    std::exception_ptr call_undelivered_element_catching_exception(
        E element,
        std::exception_ptr undelivered_element_exception = nullptr
    ) {
        try {
            if (on_undelivered_element_) {
                on_undelivered_element_(element);
            }
        } catch (...) {
            std::exception_ptr ex = std::current_exception();
            if (undelivered_element_exception) {
                // Upstream: cause?.addSuppressed(suppressed) — C++ has no addSuppressed
                // analogue; the original exception is returned unchanged.
                return undelivered_element_exception;
            } else {
                return ex;
            }
        }
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
        return [this](std::exception_ptr, ChannelResult<E> result, std::shared_ptr<CoroutineContext>) {
            if (result.is_success()) {
                on_undelivered_element_(result.get_or_throw());
            }
        };
    }

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

    void complete_cancel(int64_t senders_cur) {
        ChannelSegment<E>* last_segment = complete_close(senders_cur);
        remove_unprocessed_elements(last_segment);
    }

    ChannelSegment<E>* close_linked_list() {
        ChannelSegment<E>* last_segment = buffer_end_segment_.load(std::memory_order_acquire);
        ChannelSegment<E>* send_seg = send_segment_.load(std::memory_order_acquire);
        ChannelSegment<E>* recv_seg = receive_segment_.load(std::memory_order_acquire);

        if (send_seg->id > last_segment->id) last_segment = send_seg;
        if (recv_seg->id > last_segment->id) last_segment = recv_seg;

        return last_segment->close();
    }

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

    void drop_first_element_until_the_specified_cell_is_in_the_buffer(int64_t global_cell_index) {
        assert(is_conflated_drop_oldest());
        ChannelSegment<E>* segment = receive_segment_.load(std::memory_order_acquire);
        while (true) {
            int64_t r = receivers_.load(std::memory_order_acquire);
            if (global_cell_index < std::max(r + capacity_, buffer_end_counter())) {
                return;
            }
            if (!receivers_.compare_exchange_weak(r, r + 1,
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                continue;
            }
            int64_t id = r / SEGMENT_SIZE;
            int i = static_cast<int>(r % SEGMENT_SIZE);
            if (segment->id != id) {
                ChannelSegment<E>* found = find_segment_receive(id, segment);
                if (found == nullptr) {
                    continue;
                }
                segment = found;
            }
            void* upd_cell_result = update_cell_receive(segment, i, r, nullptr);
            if (upd_cell_result == &FAILED()) {
                // To avoid memory leaks, we also need to reset the `prev` pointer.
                if (r < senders_counter()) {
                    segment->clean_prev();
                }
            } else {
                // Clean the reference to the previous segment.
                segment->clean_prev();
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

    void remove_unprocessed_elements(ChannelSegment<E>* last_segment) {
        auto on_undelivered_element = on_undelivered_element_;
        std::exception_ptr undelivered_element_exception = nullptr;
        std::vector<Waiter*> suspended_senders;
        std::vector<std::shared_ptr<Waiter>> sender_owners;
        ChannelSegment<E>* segment = last_segment;
        bool process_segments_done = false;
        while (!process_segments_done) {
            for (int index = SEGMENT_SIZE - 1; index >= 0; --index) {
                int64_t global_index = segment->id * SEGMENT_SIZE + index;
                bool update_cell_done = false;
                while (!update_cell_done) {
                    void* state = segment->get_state(index);
                    if (state == &DONE_RCV()) {
                        process_segments_done = true;
                        update_cell_done = true;
                        break;
                    } else if (state == &BUFFERED()) {
                        if (global_index < receivers_counter()) {
                            process_segments_done = true;
                            update_cell_done = true;
                            break;
                        }
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            if (on_undelivered_element) {
                                E element = segment->get_element(index);
                                std::exception_ptr ex = call_undelivered_element_catching_exception(element, undelivered_element_exception);
                                if (ex && !undelivered_element_exception) {
                                    undelivered_element_exception = ex;
                                }
                            }
                            segment->clean_element(index);
                            segment->on_slot_cleaned();
                            update_cell_done = true;
                        }
                    } else if (state == &IN_BUFFER() || state == nullptr) {
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            segment->on_slot_cleaned();
                            update_cell_done = true;
                        }
                    } else if (is_waiter(state) || is_waiter_eb(state)) {
                        if (global_index < receivers_counter()) {
                            process_segments_done = true;
                            update_cell_done = true;
                            break;
                        }
                        Waiter* sender = is_waiter_eb(state) ?
                            static_cast<WaiterEB*>(state)->waiter :
                            static_cast<Waiter*>(state);
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            sender_owners.push_back(segment->get_waiter_ref(index));
                            segment->clear_waiter_ref(index);
                            if (on_undelivered_element) {
                                E element = segment->get_element(index);
                                std::exception_ptr ex = call_undelivered_element_catching_exception(element, undelivered_element_exception);
                                if (ex && !undelivered_element_exception) {
                                    undelivered_element_exception = ex;
                                }
                            }
                            suspended_senders.push_back(sender);
                            segment->clean_element(index);
                            segment->on_slot_cleaned();
                            update_cell_done = true;
                        }
                    } else if (state == &RESUMING_BY_EB() || state == &RESUMING_BY_RCV()) {
                        // As the cell is covered by a receiver, finish immediately.
                        process_segments_done = true;
                        update_cell_done = true;
                        break;
                    } else {
                        update_cell_done = true;
                    }
                }
                if (process_segments_done) break;
            }
            ChannelSegment<E>* prev = segment->prev();
            if (prev == nullptr) break;
            segment = prev;
        }
        for (auto it = suspended_senders.rbegin(); it != suspended_senders.rend(); ++it) {
            resume_sender_on_cancelled_channel(*it);
        }
        if (undelivered_element_exception) {
            std::rethrow_exception(undelivered_element_exception);
        }
    }

    void cancel_suspended_receive_requests(ChannelSegment<E>* last_segment, int64_t senders_counter_val) {
        std::vector<Waiter*> suspended_receivers;
        std::vector<std::shared_ptr<Waiter>> receiver_owners;
        ChannelSegment<E>* segment = last_segment;
        bool process_segments_done = false;
        while (segment != nullptr && !process_segments_done) {
            for (int index = SEGMENT_SIZE - 1; index >= 0; --index) {
                if (segment->id * SEGMENT_SIZE + index < senders_counter_val) {
                    process_segments_done = true;
                    break;
                }
                bool cell_update_done = false;
                while (!cell_update_done) {
                    void* state = segment->get_state(index);
                    if (state == nullptr || state == &IN_BUFFER()) {
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            segment->on_slot_cleaned();
                            cell_update_done = true;
                        }
                    } else if (is_waiter_eb(state)) {
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            suspended_receivers.push_back(static_cast<WaiterEB*>(state)->waiter);
                            receiver_owners.push_back(segment->get_waiter_ref(index));
                            segment->clear_waiter_ref(index);
                            segment->on_cancelled_request(index, true);
                            cell_update_done = true;
                        }
                    } else if (is_waiter(state)) {
                        if (segment->cas_state(index, state, static_cast<void*>(&CHANNEL_CLOSED()))) {
                            suspended_receivers.push_back(static_cast<Waiter*>(state));
                            receiver_owners.push_back(segment->get_waiter_ref(index));
                            segment->clear_waiter_ref(index);
                            segment->on_cancelled_request(index, true);
                            cell_update_done = true;
                        }
                    } else {
                        cell_update_done = true;
                    }
                }
            }
            segment = segment->prev();
        }
        for (auto it = suspended_receivers.rbegin(); it != suspended_receivers.rend(); ++it) {
            resume_receiver_on_closed_channel(*it);
        }
    }

    /**
     * Updates the `senders` counter if its value is lower that the specified one.
     * Senders use this function to efficiently skip a sequence of cancelled receivers.
     *
     * Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:2570-2576
     */
    void update_senders_counter_if_lower(int64_t value) {
        int64_t cur = senders_and_close_status_.load(std::memory_order_acquire);
        while (true) {
            int64_t cur_counter = channels::senders_counter(cur);
            if (cur_counter >= value) return;
            int64_t update = construct_senders_and_close_status(value, channels::senders_close_status(cur));
            if (senders_and_close_status_.compare_exchange_weak(cur, update,
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                return;
            }
        }
    }

    /**
     * Updates the `receivers` counter if its value is lower that the specified one.
     * Receivers use this function to efficiently skip a sequence of cancelled senders.
     *
     * Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:2585-2589
     */
    void update_receivers_counter_if_lower(int64_t value) {
        int64_t cur = receivers_.load(std::memory_order_acquire);
        while (true) {
            if (cur >= value) return;
            if (receivers_.compare_exchange_weak(cur, value,
                    std::memory_order_acq_rel, std::memory_order_acquire)) {
                return;
            }
        }
    }

    /**
     * Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:2390-2432
     */
    ChannelSegment<E>* find_segment_send(int64_t id, ChannelSegment<E>* start_from) {
        auto res = internal::find_segment_and_move_forward(
            send_segment_,
            id,
            start_from,
            [](int64_t seg_id, ChannelSegment<E>* prev) {
                return create_segment<E>(seg_id, prev);
            }
        );
        if (res.is_closed()) {
            complete_close_or_cancel();
            if (start_from->id * SEGMENT_SIZE < receivers_counter()) {
                start_from->clean_prev();
            }
            return nullptr;
        } else {
            auto* segment = res.segment();
            if (segment->id > id) {
                update_senders_counter_if_lower(segment->id * SEGMENT_SIZE);
                if (segment->id * SEGMENT_SIZE < receivers_counter()) {
                    segment->clean_prev();
                }
                return nullptr;
            } else {
                assert(segment->id == id);
                return segment;
            }
        }
    }

    /**
     * Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:2448-2491
     */
    ChannelSegment<E>* find_segment_receive(int64_t id, ChannelSegment<E>* start_from) {
        auto res = internal::find_segment_and_move_forward(
            receive_segment_,
            id,
            start_from,
            [](int64_t seg_id, ChannelSegment<E>* prev) {
                return create_segment<E>(seg_id, prev);
            }
        );
        if (res.is_closed()) {
            complete_close_or_cancel();
            if (start_from->id * SEGMENT_SIZE < senders_counter()) {
                start_from->clean_prev();
            }
            return nullptr;
        } else {
            auto* segment = res.segment();
            if (!is_rendezvous_or_unlimited() && id <= buffer_end_counter() / SEGMENT_SIZE) {
                internal::move_forward(buffer_end_segment_, segment);
            }
            if (segment->id > id) {
                update_receivers_counter_if_lower(segment->id * SEGMENT_SIZE);
                if (segment->id * SEGMENT_SIZE < senders_counter()) {
                    segment->clean_prev();
                }
                return nullptr;
            } else {
                assert(segment->id == id);
                return segment;
            }
        }
    }

    /**
     * Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:2497-2535
     */
    ChannelSegment<E>* find_segment_buffer_end(int64_t id, ChannelSegment<E>* start_from, int64_t current_buffer_end_counter) {
        auto res = internal::find_segment_and_move_forward(
            buffer_end_segment_,
            id,
            start_from,
            [](int64_t seg_id, ChannelSegment<E>* prev) {
                return create_segment<E>(seg_id, prev);
            }
        );
        if (res.is_closed()) {
            complete_close_or_cancel();
            move_segment_buffer_end_to_specified_or_last(id, start_from);
            inc_completed_expand_buffer_attempts();
            return nullptr;
        } else {
            auto* segment = res.segment();
            if (segment->id > id) {
                int64_t expected = current_buffer_end_counter + 1;
                if (buffer_end_.compare_exchange_weak(expected, segment->id * SEGMENT_SIZE,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    inc_completed_expand_buffer_attempts(segment->id * SEGMENT_SIZE - current_buffer_end_counter);
                } else {
                    inc_completed_expand_buffer_attempts();
                }
                return nullptr;
            } else {
                assert(segment->id == id);
                return segment;
            }
        }
    }

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
    // Lines 1612-1744: Iterator Support
    // =========================================================================

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:1612-1744
    class BufferedChannelIterator : public ChannelIterator<E>, public Waiter,
                                    public std::enable_shared_from_this<BufferedChannelIterator> {
    public:
        BufferedChannelIterator(BufferedChannel<E>* channel)
            : channel_(channel)
            , receive_result_(static_cast<void*>(&NO_RECEIVE_RESULT()))
            , continuation_sp_(nullptr) {}

        ~BufferedChannelIterator() override {
            if (receive_result_ != static_cast<void*>(&NO_RECEIVE_RESULT()) &&
                receive_result_ != static_cast<void*>(&CHANNEL_CLOSED())) {
                delete static_cast<E*>(receive_result_);
            }
        }

        std::shared_ptr<Waiter> shared_from_this_waiter() override {
            return this->shared_from_this();
        }

        void* has_next(Continuation<void*>* continuation) override {
            if (receive_result_ != static_cast<void*>(&NO_RECEIVE_RESULT()) &&
                receive_result_ != static_cast<void*>(&CHANNEL_CLOSED())) {
                return new bool(true);
            }

            ChannelSegment<E>* segment = channel_->receive_segment_.load(std::memory_order_acquire);
            while (true) {
                if (channel_->is_closed_for_receive()) {
                    bool has_more = on_closed_has_next();
                    return new bool(has_more);
                }
                int64_t r = channel_->receivers_.fetch_add(1, std::memory_order_acq_rel);
                int64_t id = r / SEGMENT_SIZE;
                int i = static_cast<int>(r % SEGMENT_SIZE);
                if (segment->id != id) {
                    segment = channel_->find_segment_receive(id, segment);
                    if (segment == nullptr) {
                        continue;
                    }
                }
                void* upd_cell_result = channel_->update_cell_receive(segment, i, r, nullptr);
                if (upd_cell_result == static_cast<void*>(&SUSPEND())) {
                    assert(false && "unexpected SUSPEND with null waiter");
                } else if (upd_cell_result == static_cast<void*>(&FAILED())) {
                    if (r < channel_->senders_counter()) segment->clean_prev();
                    continue;
                } else if (upd_cell_result == static_cast<void*>(&SUSPEND_NO_WAITER())) {
                    return has_next_on_no_waiter_suspend(segment, i, r, continuation);
                } else {
                    segment->clean_prev();
                    receive_result_ = upd_cell_result;
                    return new bool(true);
                }
            }
        }

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

        bool try_resume_has_next(E element) {
            auto cont = std::atomic_load_explicit(&continuation_sp_, std::memory_order_acquire);
            std::atomic_store_explicit(&continuation_sp_, std::shared_ptr<CancellableContinuationImpl<bool>>{},
                                       std::memory_order_release);
            if (!cont) return false;
            receive_result_ = new E(std::move(element));
            std::function<void(std::exception_ptr, bool, std::shared_ptr<CoroutineContext>)> on_cancellation = nullptr;
            if (channel_->on_undelivered_element()) {
                auto elem_copy = *static_cast<E*>(receive_result_);
                auto channel = channel_;
                on_cancellation = [channel, elem_copy](std::exception_ptr, bool, std::shared_ptr<CoroutineContext>) {
                    channel->on_undelivered_element()(elem_copy);
                };
            }
            void* token = cont->try_resume(true, nullptr, on_cancellation);
            if (token != nullptr) {
                cont->complete_resume(token);
                return true;
            }
            return false;
        }

        void try_resume_has_next_on_closed_channel() {
            auto cont = std::atomic_load_explicit(&continuation_sp_, std::memory_order_acquire);
            std::atomic_store_explicit(&continuation_sp_, std::shared_ptr<CancellableContinuationImpl<bool>>{},
                                       std::memory_order_release);
            receive_result_ = static_cast<void*>(&CHANNEL_CLOSED());
            if (!cont) return;
            auto cause = channel_->close_cause();
            if (!cause) {
                cont->resume_with(Result<bool>::success(false));
            } else {
                cont->resume_with(Result<bool>::failure(cause));
            }
        }

        void invoke_on_cancellation(internal::SegmentBase* segment, int index) override {
            auto cont = std::atomic_load_explicit(&continuation_sp_, std::memory_order_acquire);
            if (cont) {
                cont->invoke_on_cancellation(segment, index);
            }
        }

    private:
        bool on_closed_has_next() {
            receive_result_ = static_cast<void*>(&CHANNEL_CLOSED());
            auto cause = channel_->close_cause();
            if (cause) {
                std::rethrow_exception(cause);
            }
            return false;
        }

        void* has_next_on_no_waiter_suspend(
            ChannelSegment<E>* segment,
            int index,
            int64_t r,
            Continuation<void*>* continuation
        ) {
            return dsl::suspend_cancellable_coroutine_reusable<bool>(continuation, [&](auto* cont) {
                std::atomic_store_explicit(&continuation_sp_, cont->shared_from_this(), std::memory_order_release);
                channel_->receive_impl_on_no_waiter(
                    segment, index, r,
                    this,
                    [this, cont](E element) {
                        receive_result_ = new E(std::move(element));
                        std::atomic_store_explicit(&continuation_sp_, std::shared_ptr<CancellableContinuationImpl<bool>>{},
                                                   std::memory_order_release);
                        std::function<void(std::exception_ptr)> on_cancellation = nullptr;
                        if (channel_->on_undelivered_element()) {
                            auto elem_copy = *static_cast<E*>(receive_result_);
                            auto channel = channel_;
                            on_cancellation = [channel, elem_copy](std::exception_ptr) {
                                channel->on_undelivered_element()(elem_copy);
                            };
                        }
                        cont->resume(true, on_cancellation);
                    },
                    [this]() { on_closed_has_next_no_waiter_suspend(); }
                );
            });
        }

        void on_closed_has_next_no_waiter_suspend() {
            auto cont = std::atomic_load_explicit(&continuation_sp_, std::memory_order_acquire);
            std::atomic_store_explicit(&continuation_sp_, std::shared_ptr<CancellableContinuationImpl<bool>>{},
                                       std::memory_order_release);
            this->receive_result_ = static_cast<void*>(&CHANNEL_CLOSED());
            if (!cont) return;
            auto cause = channel_->close_cause();
            if (!cause) {
                cont->resume(false, nullptr);
            } else {
                cont->resume_with_exception(cause);
            }
        }

        BufferedChannel<E>* channel_;
        void* receive_result_;
        // Atomic reference accesses retain a loaded continuation across concurrent field clearing.
        std::shared_ptr<CancellableContinuationImpl<bool>> continuation_sp_;
    };

private:
    /** Unique public handle owning the shared iterator used as a channel-cell waiter. */
    class IteratorHandle final : public ChannelIterator<E> {
        std::shared_ptr<BufferedChannelIterator> iterator_;
    public:
        explicit IteratorHandle(std::shared_ptr<BufferedChannelIterator> iterator)
            : iterator_(std::move(iterator)) {}

        void* has_next(Continuation<void*>* continuation) override {
            return iterator_->has_next(continuation);
        }

        E next() override { return iterator_->next(); }
    };

};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
