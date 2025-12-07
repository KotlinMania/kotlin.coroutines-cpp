#pragma once
#include "Channel.hpp"
#include <atomic>
#include <memory>
#include <vector>
#include <deque>
#include <condition_variable>
#include <mutex>
#include <cstdint>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace channels {

// Forward declarations
template <typename E> class BufferedChannel;

/**
 * Number of cells in each segment.
 * This mirrors the Kotlin implementation's segment size.
 */
constexpr int SEGMENT_SIZE = 32;

/**
 * Special buffer end values for different channel types.
 * These constants match the Kotlin implementation.
 */
constexpr long long BUFFER_END_RENDEZVOUS = -1L;
constexpr long long BUFFER_END_UNLIMITED = -2L;

/**
 * Cell states for the lock-free channel implementation.
 * Each cell in a segment can be in one of these states.
 */
enum class CellState : std::uintptr_t {
    EMPTY = 0,
    BUFFERED = 1,
    WAITER_SENDER = 2,
    WAITER_RECEIVER = 3,
    CHANNEL_CLOSED = 4,
    INTERRUPTED_SEND = 5,
    INTERRUPTED_RECEIVE = 6,
    IN_BUFFER = 7,
    NULL_SEGMENT = 8
};

/**
 * Base class for waiters (suspended coroutines) in the channel.
 * This represents a continuation that is waiting in a channel cell.
 */
class Waiter {
public:
    virtual ~Waiter() = default;
    virtual void resume() = 0;
    virtual void resume_with_exception(std::exception_ptr exception) = 0;
    virtual void on_cancellation() = 0;
};

/**
 * A segment in the channel's lock-free linked list structure.
 * Each segment contains a fixed number of cells that can store
 * either buffered elements or waiting coroutines.
 */
template <typename E>
class ChannelSegment {
public:
    const long long id;
    std::atomic<ChannelSegment<E>*> prev;
    std::atomic<ChannelSegment<E>*> next;
    BufferedChannel<E>* channel;
    
    // Two registers per slot: state + element/waiter
    std::atomic<std::uintptr_t> states[SEGMENT_SIZE];
    std::atomic<void*> data[SEGMENT_SIZE];
    
    ChannelSegment(long long id, ChannelSegment<E>* prev, BufferedChannel<E>* channel);
    virtual ~ChannelSegment() = default;
    
    /**
     * Gets the state of the cell at the specified index.
     * Uses acquire memory ordering for proper synchronization.
     */
    CellState get_state(int index) const {
        return static_cast<CellState>(states[index].load(std::memory_order_acquire));
    }
    
    /**
     * Attempts to update the state of the cell at the specified index.
     * Uses compare-and-swap with acquire-release semantics.
     */
    bool cas_state(int index, CellState expected, CellState desired) {
        std::uintptr_t expected_val = static_cast<std::uintptr_t>(expected);
        std::uintptr_t desired_val = static_cast<std::uintptr_t>(desired);
        return states[index].compare_exchange_strong(
            expected_val,
            desired_val,
            std::memory_order_acq_rel,
            std::memory_order_acquire
        );
    }
    
    /**
     * Gets the data (element or waiter) stored in the cell.
     */
    void* get_data(int index) const {
        return data[index].load(std::memory_order_acquire);
    }
    
    /**
     * Sets the data in the cell with release semantics.
     */
    void set_data(int index, void* value) {
        data[index].store(value, std::memory_order_release);
    }
    
    /**
     * Attempts to update both state and data atomically.
     * This is a critical operation for maintaining consistency.
     */
    bool cas_state_and_data(int index, CellState expected_state, void* expected_data,
                           CellState desired_state, void* desired_data);
    
    /**
     * Cleans up the previous segment to prevent memory leaks.
     */
    void clean_prev();
    
    /**
     * Called when a slot in this segment is cleaned up.
     */
    virtual void on_slot_cleaned() {}
    
    /**
     * Checks if this segment has been removed from the linked list.
     */
    bool is_removed() const {
        return get_state(SEGMENT_SIZE - 1) == CellState::NULL_SEGMENT;
    }
    
    /**
     * Marks this segment as removed.
     */
    void mark_removed() {
        states[SEGMENT_SIZE - 1].store(static_cast<std::uintptr_t>(CellState::NULL_SEGMENT), 
                                      std::memory_order_release);
    }
    
    /**
     * Closes this segment for further modifications.
     */
    ChannelSegment<E>* close();
};

/**
 * The buffered channel implementation, which also serves as a rendezvous channel when capacity is zero.
 * 
 * This implementation follows the "Fast and Scalable Channels in Kotlin Coroutines" algorithm
 * (https://arxiv.org/abs/2211.04986) by Nikita Koval, Roman Elizarov, and Dan Alistarh.
 * 
 * The high-level structure is based on a conceptually infinite array for storing elements 
 * and waiting requests, with separate counters for send and receive operations that have 
 * ever been performed. An additional counter indicates the end of the logical buffer.
 * 
 * The key insight is that both send() and receive() start by incrementing their counters,
 * which assigns them a unique cell to process. For rendezvous channels (capacity = 0),
 * the operation either suspends (storing its continuation in the cell) or makes a 
 * rendezvous with the opposite request. Each cell can be processed by exactly one 
 * send() and one receive(). For buffered channels, send() operations can add elements 
 * without suspension if the logical buffer contains the cell.
 */
template <typename E>
class BufferedChannel : public Channel<E> {
public:
    /**
     * Creates a new BufferedChannel with the specified capacity.
     * @param capacity Channel capacity; use Channel::RENDEZVOUS for rendezvous channel
     *                 and Channel::UNLIMITED for unlimited capacity.
     * @param onUndeliveredElement Optional handler for elements that cannot be delivered
     */
    BufferedChannel(int capacity, std::function<void(const E&, std::exception_ptr)> onUndeliveredElement = nullptr);
    
    virtual ~BufferedChannel();
    
    // SendChannel interface implementation
    bool is_closed_for_send() const override;
    void send(E element) override;
    ChannelResult<void> try_send(E element) override;
    bool close(std::exception_ptr cause = nullptr) override;
    void invoke_on_close(std::function<void(std::exception_ptr)> handler) override;
    
    // ReceiveChannel interface implementation  
    bool is_closed_for_receive() const override;
    bool is_empty() const override;
    E receive() override;
    ChannelResult<E> receive_catching() override;
    ChannelResult<E> try_receive() override;
    std::shared_ptr<ChannelIterator<E>> iterator() override;
    void cancel(std::exception_ptr cause = nullptr) override;

private:
    // Core atomic counters - these are the heart of the lock-free algorithm
    std::atomic<long long> sendersAndCloseStatus;  // Combined counter + close flag
    std::atomic<long long> receivers;              // Receiver counter
    std::atomic<long long> bufferEnd;              // Buffer end counter
    std::atomic<long long> completedExpandBuffersAndPauseFlag;  // Expansion synchronization
    
    // Segment references for the lock-free linked list
    std::atomic<ChannelSegment<E>*> sendSegment;
    std::atomic<ChannelSegment<E>*> receiveSegment;
    std::atomic<ChannelSegment<E>*> bufferEndSegment;
    
    // Channel configuration
    int capacity_;
    std::function<void(const E&, std::exception_ptr)> onUndeliveredElement_;
    
    // Close state management
    std::atomic<bool> closed_;
    std::exception_ptr closeCause_;  // Not atomic, protected by mutex
    std::vector<std::function<void(std::exception_ptr)>> closeHandlers_;
    mutable std::mutex closeMutex_;
    
    // Helper methods for counter manipulation
    static long long initialBufferEnd(int capacity);
    static long long extractSendersCounter(long long sendersAndCloseStatus);
    static bool extractClosedStatus(long long sendersAndCloseStatus);
    static long long packSendersAndCloseStatus(long long senders, bool closed);
    
    // Core send/receive algorithms (these will be implemented in the .cpp file)
    void sendImpl(const E& element);
    void sendImplSuspend(ChannelSegment<E>* segment, int index, const E& element, long long s, void* waiter);
    E receiveImpl();
    E receiveImplSuspend(ChannelSegment<E>* segment, int index, long long r);
    
    // Segment management
    ChannelSegment<E>* findSegmentSend(long long id, ChannelSegment<E>* startFrom);
    ChannelSegment<E>* findSegmentReceive(long long id, ChannelSegment<E>* startFrom);
    ChannelSegment<E>* createSegment(long long id, ChannelSegment<E>* prev);
    
    // Cell state management
    void updateCellSend(ChannelSegment<E>* segment, int index, const E& element, long long s, void* waiter, bool closed);
    void updateCellReceive(ChannelSegment<E>* segment, int index, long long r, void* waiter);
    
    // Buffer management
    bool shouldSendSuspend(long long sendersAndCloseStatus) const;
    bool bufferOrRendezvousSend(long long senders) const;
    bool isRendezvousOrUnlimited() const;
    
    // Close and cleanup operations
    void completeClose(long long sendersCur);
    ChannelSegment<E>* closeLinkedList();
    void removeUnprocessedElements(ChannelSegment<E>* lastSegment, long long sendersCounter);
    void cancelSuspendedReceiveRequests(ChannelSegment<E>* lastSegment, long long sendersCounter);
    
    // Iterator implementation
    class BufferedChannelIterator;
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx