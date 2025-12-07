/**
 * @file BufferedChannel.cpp
 * @brief Implementation of BufferedChannel.
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

#include "BufferedChannel.hpp"
#include <stdexcept>
#include <cassert>

namespace kotlinx {
namespace coroutines {
namespace channels {

// Static helper methods for counter manipulation

template <typename E>
long long BufferedChannel<E>::initialBufferEnd(int capacity) {
    if (capacity == Channel<E>::RENDEZVOUS) {
        return BUFFER_END_RENDEZVOUS;
    } else if (capacity == Channel<E>::UNLIMITED) {
        return BUFFER_END_UNLIMITED;
    }
    return static_cast<long long>(capacity);
}

template <typename E>
long long BufferedChannel<E>::extractSendersCounter(long long sendersAndCloseStatus) {
    // In the original implementation, the close status is stored in the highest bit
    // and the senders counter is in the lower bits
    return sendersAndCloseStatus & 0x7FFFFFFFFFFFFFFFLL; // Mask out the close bit
}

template <typename E>
bool BufferedChannel<E>::extractClosedStatus(long long sendersAndCloseStatus) {
    // Close status is stored in the highest bit
    return (sendersAndCloseStatus & 0x8000000000000000LL) != 0;
}

template <typename E>
long long BufferedChannel<E>::packSendersAndCloseStatus(long long senders, bool closed) {
    // Pack senders counter and close status into a single atomic value
    return senders | (closed ? 0x8000000000000000LL : 0);
}

// ChannelSegment implementation

template <typename E>
ChannelSegment<E>::ChannelSegment(long long id, ChannelSegment<E>* prev, BufferedChannel<E>* channel)
    : id(id), prev(prev), next(nullptr), channel(channel) {
    // Initialize all cells to EMPTY state
    for (int i = 0; i < SEGMENT_SIZE; ++i) {
        states[i].store(static_cast<std::uintptr_t>(CellState::EMPTY), std::memory_order_relaxed);
        data[i].store(nullptr, std::memory_order_relaxed);
    }
}

template <typename E>
bool ChannelSegment<E>::cas_state_and_data(int index, CellState expected_state, void* expected_data,
                                          CellState desired_state, void* desired_data) {
    // This is a critical operation that must update both state and data consistently
    // In a real implementation, this might need double-wide CAS or other techniques
    // For now, we'll use a simple approach with state first, then data
    
    if (!cas_state(index, expected_state, desired_state)) {
        return false;
    }
    
    // If state update succeeded, update data
    set_data(index, desired_data);
    return true;
}

template <typename E>
void ChannelSegment<E>::clean_prev() {
    auto prev_segment = prev.load(std::memory_order_acquire);
    if (prev_segment) {
        delete prev_segment;
        prev.store(nullptr, std::memory_order_release);
    }
}

template <typename E>
ChannelSegment<E>* ChannelSegment<E>::close() {
    // Mark this segment as closed and return it
    // This prevents further segments from being added after this one
    mark_removed();
    return this;
}

// BufferedChannel implementation

template <typename E>
BufferedChannel<E>::BufferedChannel(int capacity, std::function<void(const E&, std::exception_ptr)> onUndeliveredElement)
    : capacity_(capacity), onUndeliveredElement_(onUndeliveredElement) {
    
    // Validate capacity
    if (capacity < 0 && capacity != Channel<E>::RENDEZVOUS && 
        capacity != Channel<E>::UNLIMITED && capacity != Channel<E>::CONFLATED) {
        throw std::invalid_argument("Invalid channel capacity: " + std::to_string(capacity));
    }
    
    // Initialize atomic counters
    long long initialBufferEndValue = initialBufferEnd(capacity);
    sendersAndCloseStatus.store(0, std::memory_order_relaxed);
    receivers.store(0, std::memory_order_relaxed);
    bufferEnd.store(initialBufferEndValue, std::memory_order_relaxed);
    completedExpandBuffersAndPauseFlag.store(initialBufferEndValue, std::memory_order_relaxed);
    
    // Initialize close state
    closed_.store(false, std::memory_order_relaxed);
    closeCause_ = nullptr;
    
    // Create the first segment
    auto firstSegment = new ChannelSegment<E>(0, nullptr, this);
    sendSegment.store(firstSegment, std::memory_order_relaxed);
    receiveSegment.store(firstSegment, std::memory_order_relaxed);
    
    // For rendezvous or unlimited channels, bufferEndSegment points to a special NULL_SEGMENT
    if (isRendezvousOrUnlimited()) {
        bufferEndSegment.store(nullptr, std::memory_order_relaxed); // NULL_SEGMENT equivalent
    } else {
        bufferEndSegment.store(firstSegment, std::memory_order_relaxed);
    }
}

template <typename E>
BufferedChannel<E>::~BufferedChannel() {
    // Clean up all segments in the linked list
    auto segment = sendSegment.load(std::memory_order_acquire);
    while (segment) {
        auto next = segment->next.load(std::memory_order_acquire);
        delete segment;
        segment = next;
    }
}

template <typename E>
bool BufferedChannel<E>::is_closed_for_send() const {
    return extractClosedStatus(sendersAndCloseStatus.load(std::memory_order_acquire));
}

template <typename E>
bool BufferedChannel<E>::is_closed_for_receive() const {
    return closed_.load(std::memory_order_acquire) && is_empty();
}

template <typename E>
bool BufferedChannel<E>::is_empty() const {
    auto currentReceivers = receivers.load(std::memory_order_acquire);
    auto currentSenders = extractSendersCounter(sendersAndCloseStatus.load(std::memory_order_acquire));
    return currentReceivers >= currentSenders;
}

template <typename E>
bool BufferedChannel<E>::isRendezvousOrUnlimited() const {
    auto bufferEndCounter = bufferEnd.load(std::memory_order_acquire);
    return bufferEndCounter == BUFFER_END_RENDEZVOUS || bufferEndCounter == BUFFER_END_UNLIMITED;
}

template <typename E>
bool BufferedChannel<E>::shouldSendSuspend(long long sendersAndCloseStatus) const {
    if (extractClosedStatus(sendersAndCloseStatus)) {
        return false; // Will throw exception instead
    }
    
    auto senders = extractSendersCounter(sendersAndCloseStatus);
    auto currentReceivers = receivers.load(std::memory_order_acquire);
    auto currentBufferEnd = bufferEnd.load(std::memory_order_acquire);
    
    // For rendezvous channels, suspend if no waiting receivers
    if (currentBufferEnd == BUFFER_END_RENDEZVOUS) {
        return senders >= currentReceivers;
    }
    
    // For unlimited channels, never suspend
    if (currentBufferEnd == BUFFER_END_UNLIMITED) {
        return false;
    }
    
    // For buffered channels, suspend if buffer is full
    return (senders - currentReceivers) >= currentBufferEnd;
}

template <typename E>
bool BufferedChannel<E>::bufferOrRendezvousSend(long long senders) const {
    auto currentReceivers = receivers.load(std::memory_order_acquire);
    auto currentBufferEnd = bufferEnd.load(std::memory_order_acquire);
    
    // Check if this send can be buffered (for buffered channels)
    if (currentBufferEnd > 0 && currentBufferEnd != BUFFER_END_UNLIMITED) {
        return senders < currentBufferEnd;
    }
    
    // For rendezvous channels, check if there's a waiting receiver
    return senders < currentReceivers;
}

template <typename E>
void BufferedChannel<E>::send(E element) {
    // Check if channel is closed
    if (is_closed_for_send()) {
        std::lock_guard<std::mutex> lock(closeMutex_);
        if (closeCause_) {
            std::rethrow_exception(closeCause_);
        }
        throw std::runtime_error("Channel is closed for send");
    }
    
    // Check if send would suspend - if so, we need to handle suspension
    auto currentSendersAndCloseStatus = sendersAndCloseStatus.load(std::memory_order_acquire);
    if (shouldSendSuspend(currentSendersAndCloseStatus)) {
        // In a real implementation, this would suspend the coroutine
        // For now, we'll block or throw
        throw std::runtime_error("Send would suspend - not implemented in this stub");
    }
    
    // Fast path - try to send without suspension
    sendImpl(element);
}

template <typename E>
void BufferedChannel<E>::sendImpl(const E& element) {
    while (true) {
        // Read the segment reference before incrementing the counter
        auto segment = sendSegment.load(std::memory_order_acquire);
        
        // Atomically increment the senders counter
        auto currentSendersAndCloseStatus = sendersAndCloseStatus.load(std::memory_order_acquire);
        auto newSendersAndCloseStatus = packSendersAndCloseStatus(
            extractSendersCounter(currentSendersAndCloseStatus) + 1,
            extractClosedStatus(currentSendersAndCloseStatus)
        );
        
        if (sendersAndCloseStatus.compare_exchange_strong(
            currentSendersAndCloseStatus, 
            newSendersAndCloseStatus,
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
            
            // Successfully incremented counter - now work with the assigned cell
            auto senders = extractSendersCounter(newSendersAndCloseStatus);
            auto segmentId = senders / SEGMENT_SIZE;
            auto cellIndex = static_cast<int>(senders % SEGMENT_SIZE);
            
            // Find or create the target segment
            auto targetSegment = findSegmentSend(segmentId, segment);
            if (!targetSegment) {
                // Segment not found (channel closed or cancelled)
                if (extractClosedStatus(sendersAndCloseStatus.load(std::memory_order_acquire))) {
                    std::lock_guard<std::mutex> lock(closeMutex_);
                    if (closeCause_) {
                        std::rethrow_exception(closeCause_);
                    }
                    throw std::runtime_error("Channel is closed for send");
                }
                continue; // Retry
            }
            
            // Update the cell with the element
            updateCellSend(targetSegment, cellIndex, element, senders, nullptr, false);
            return;
        }
        // CAS failed - retry the whole operation
    }
}

template <typename E>
ChannelResult<void> BufferedChannel<E>::try_send(E element) {
    if (is_closed_for_send()) {
        std::lock_guard<std::mutex> lock(closeMutex_);
        return ChannelResult<void>::closed_result(closeCause_);
    }
    
    auto currentSendersAndCloseStatus = sendersAndCloseStatus.load(std::memory_order_acquire);
    if (shouldSendSuspend(currentSendersAndCloseStatus)) {
        return ChannelResult<void>::failure();
    }
    
    try {
        sendImpl(element);
        return ChannelResult<void>::success(nullptr);
    } catch (...) {
        return ChannelResult<void>::failure();
    }
}

template <typename E>
E BufferedChannel<E>::receive() {
    // Check if channel is closed for receive
    if (is_closed_for_receive()) {
        std::lock_guard<std::mutex> lock(closeMutex_);
        if (closeCause_) {
            std::rethrow_exception(closeCause_);
        }
        throw std::runtime_error("Channel is closed for receive");
    }
    
    // Fast path - try to receive without suspension
    return receiveImpl();
}

template <typename E>
E BufferedChannel<E>::receiveImpl() {
    while (true) {
        // Read the segment reference before incrementing the counter
        auto segment = receiveSegment.load(std::memory_order_acquire);
        
        // Atomically increment the receivers counter
        auto currentReceivers = receivers.load(std::memory_order_acquire);
        auto newReceivers = currentReceivers + 1;
        
        if (receivers.compare_exchange_strong(
            currentReceivers, 
            newReceivers,
            std::memory_order_acq_rel,
            std::memory_order_acquire)) {
            
            // Successfully incremented counter - now work with the assigned cell
            auto segmentId = newReceivers / SEGMENT_SIZE;
            auto cellIndex = static_cast<int>(newReceivers % SEGMENT_SIZE);
            
            // Find or create the target segment
            auto targetSegment = findSegmentReceive(segmentId, segment);
            if (!targetSegment) {
                // Segment not found (channel closed or cancelled)
                if (closed_.load(std::memory_order_acquire)) {
                    std::lock_guard<std::mutex> lock(closeMutex_);
                    if (closeCause_) {
                        std::rethrow_exception(closeCause_);
                    }
                    throw std::runtime_error("Channel is closed for receive");
                }
                continue; // Retry
            }
            
            // Update the cell to retrieve the element
            updateCellReceive(targetSegment, cellIndex, newReceivers, nullptr);
            return E{}; // Return default element for now
        }
        // CAS failed - retry the whole operation
    }
}

template <typename E>
ChannelResult<E> BufferedChannel<E>::receive_catching() {
    try {
        E element = receiveImpl();
        return ChannelResult<E>::success(new E(element));
    } catch (...) {
        std::lock_guard<std::mutex> lock(closeMutex_);
        return ChannelResult<E>::closed_result(closeCause_);
    }
}

template <typename E>
ChannelResult<E> BufferedChannel<E>::try_receive() {
    if (is_empty()) {
        if (closed_.load(std::memory_order_acquire)) {
            std::lock_guard<std::mutex> lock(closeMutex_);
            return ChannelResult<E>::closed_result(closeCause_);
        }
        return ChannelResult<E>::failure();
    }
    
    try {
        E element = receiveImpl();
        return ChannelResult<E>::success(new E(element));
    } catch (...) {
        return ChannelResult<E>::failure();
    }
}

template <typename E>
bool BufferedChannel<E>::close(std::exception_ptr cause) {
    // Try to close the channel atomically
    auto currentSendersAndCloseStatus = sendersAndCloseStatus.load(std::memory_order_acquire);
    if (extractClosedStatus(currentSendersAndCloseStatus)) {
        return false; // Already closed
    }
    
    auto newSendersAndCloseStatus = packSendersAndCloseStatus(
        extractSendersCounter(currentSendersAndCloseStatus),
        true // Mark as closed
    );
    
    if (sendersAndCloseStatus.compare_exchange_strong(
        currentSendersAndCloseStatus,
        newSendersAndCloseStatus,
        std::memory_order_acq_rel,
        std::memory_order_acquire)) {
        
        // Successfully marked as closed
        {
            std::lock_guard<std::mutex> lock(closeMutex_);
            closed_.store(true, std::memory_order_release);
            closeCause_ = cause;
        }
        
        // Complete the close procedure
        completeClose(extractSendersCounter(newSendersAndCloseStatus));
        
        // Invoke close handlers
        {
            std::lock_guard<std::mutex> lock(closeMutex_);
            for (auto& handler : closeHandlers_) {
                try {
                    handler(cause);
                } catch (...) {
                    // Ignore handler exceptions
                }
            }
            closeHandlers_.clear();
        }
        
        return true;
    }
    
    return false; // CAS failed - already closed
}

template <typename E>
void BufferedChannel<E>::invoke_on_close(std::function<void(std::exception_ptr)> handler) {
    if (closed_.load(std::memory_order_acquire)) {
        // Already closed - invoke immediately
        std::lock_guard<std::mutex> lock(closeMutex_);
        try {
            handler(closeCause_);
        } catch (...) {
            // Ignore handler exceptions
        }
    } else {
        std::lock_guard<std::mutex> lock(closeMutex_);
        closeHandlers_.push_back(handler);
    }
}

template <typename E>
std::shared_ptr<ChannelIterator<E>> BufferedChannel<E>::iterator() {
    // TODO: Implement BufferedChannelIterator
    throw std::runtime_error("Iterator not implemented yet");
}

template <typename E>
void BufferedChannel<E>::cancel(std::exception_ptr cause) {
    close(cause);
}

template <typename E>
void BufferedChannel<E>::updateCellSend(ChannelSegment<E>* segment, int index, const E& element, long long s, void* waiter, bool closed) {
    // This is a simplified version of the cell update logic
    // The full implementation would handle all the state machine transitions
    
    while (true) {
        auto currentState = segment->get_state(index);
        
        if (currentState == CellState::EMPTY) {
            // Try to put the element in the empty cell
            if (segment->cas_state(index, CellState::EMPTY, CellState::BUFFERED)) {
                segment->set_data(index, new E(element));
                return;
            }
        } else if (currentState == CellState::WAITER_RECEIVER) {
            // Rendezvous - try to complete with waiting receiver
            auto receiverWaiter = static_cast<Waiter*>(segment->get_data(index));
            if (segment->cas_state(index, CellState::WAITER_RECEIVER, CellState::EMPTY)) {
                // Complete the rendezvous
                if (receiverWaiter) {
                    receiverWaiter->resume();
                }
                return;
            }
        } else if (currentState == CellState::CHANNEL_CLOSED) {
            // Channel is closed
            if (onUndeliveredElement_) {
                std::lock_guard<std::mutex> lock(closeMutex_);
                onUndeliveredElement_(element, closeCause_);
            }
            throw std::runtime_error("Channel is closed");
        }
        // State changed - retry
    }
}

template <typename E>
void BufferedChannel<E>::updateCellReceive(ChannelSegment<E>* segment, int index, long long r, void* waiter) {
    // This is a simplified version of the cell update logic
    // The full implementation would handle all the state machine transitions
    
    while (true) {
        auto currentState = segment->get_state(index);
        
        if (currentState == CellState::BUFFERED) {
            // Try to take the element from the buffered cell
            if (segment->cas_state(index, CellState::BUFFERED, CellState::EMPTY)) {
                auto elementPtr = static_cast<E*>(segment->get_data(index));
                delete elementPtr;
                segment->set_data(index, nullptr);
                return;
            }
        } else if (currentState == CellState::WAITER_SENDER) {
            // Rendezvous - try to complete with waiting sender
            auto senderWaiter = static_cast<Waiter*>(segment->get_data(index));
            if (segment->cas_state(index, CellState::WAITER_SENDER, CellState::EMPTY)) {
                // Complete the rendezvous
                if (senderWaiter) {
                    senderWaiter->resume();
                }
                // In a real implementation, we'd get the element from the sender
                throw std::runtime_error("Rendezvous with sender not fully implemented");
            }
        } else if (currentState == CellState::EMPTY) {
            // No element available - would suspend in real implementation
            throw std::runtime_error("Receive would suspend - not implemented in this stub");
        } else if (currentState == CellState::CHANNEL_CLOSED) {
            // Channel is closed
            throw std::runtime_error("Channel is closed");
        }
        // State changed - retry
    }
}

template <typename E>
ChannelSegment<E>* BufferedChannel<E>::findSegmentSend(long long id, ChannelSegment<E>* startFrom) {
    // Simplified segment finding - in real implementation this would be lock-free
    auto current = startFrom;
    while (current && current->id < id) {
        auto next = current->next.load(std::memory_order_acquire);
        if (!next) {
            // Create new segment
            next = createSegment(current->id + 1, current);
            ChannelSegment<E>* expected = nullptr;
            if (current->next.compare_exchange_strong(expected, next, std::memory_order_acq_rel, std::memory_order_acquire)) {
                // Successfully added new segment
                sendSegment.store(next, std::memory_order_release);
            } else {
                // Another thread added the segment - clean up ours
                delete next;
                next = current->next.load(std::memory_order_acquire);
            }
        }
        current = next;
    }
    return current;
}

template <typename E>
ChannelSegment<E>* BufferedChannel<E>::findSegmentReceive(long long id, ChannelSegment<E>* startFrom) {
    // Similar to findSegmentSend but for receive operations
    auto current = startFrom;
    while (current && current->id < id) {
        auto next = current->next.load(std::memory_order_acquire);
        if (!next) {
            // Create new segment
            next = createSegment(current->id + 1, current);
            ChannelSegment<E>* expected = nullptr;
            if (current->next.compare_exchange_strong(expected, next, std::memory_order_acq_rel, std::memory_order_acquire)) {
                // Successfully added new segment
                receiveSegment.store(next, std::memory_order_release);
            } else {
                // Another thread added the segment - clean up ours
                delete next;
                next = current->next.load(std::memory_order_acquire);
            }
        }
        current = next;
    }
    return current;
}

template <typename E>
ChannelSegment<E>* BufferedChannel<E>::createSegment(long long id, ChannelSegment<E>* prev) {
    return new ChannelSegment<E>(id, prev, this);
}

template <typename E>
void BufferedChannel<E>::completeClose(long long sendersCur) {
    // Simplified close completion - would handle all the cleanup in real implementation
    auto lastSegment = closeLinkedList();
    
    // Cancel suspended operations and clean up
    removeUnprocessedElements(lastSegment);
    cancelSuspendedReceiveRequests(lastSegment, sendersCur);
}

template <typename E>
ChannelSegment<E>* BufferedChannel<E>::closeLinkedList() {
    // Find the last segment and close the list
    auto lastSegment = bufferEndSegment.load(std::memory_order_acquire);
    auto sendSeg = sendSegment.load(std::memory_order_acquire);
    auto receiveSeg = receiveSegment.load(std::memory_order_acquire);
    
    if (sendSeg && sendSeg->id > lastSegment->id) {
        lastSegment = sendSeg;
    }
    if (receiveSeg && receiveSeg->id > lastSegment->id) {
        lastSegment = receiveSeg;
    }
    
    return lastSegment->close();
}

template <typename E>
void BufferedChannel<E>::removeUnprocessedElements(ChannelSegment<E>* lastSegment) {
    // Simplified cleanup - would handle undelivered elements in real implementation
    if (onUndeliveredElement_) {
        // Process segments in reverse order
        auto segment = lastSegment;
        while (segment) {
            for (int i = SEGMENT_SIZE - 1; i >= 0; --i) {
                auto state = segment->get_state(i);
                if (state == CellState::BUFFERED) {
                    auto elementPtr = static_cast<E*>(segment->get_data(i));
                    if (elementPtr) {
                        try {
                            std::lock_guard<std::mutex> lock(closeMutex_);
                            onUndeliveredElement_(*elementPtr, closeCause_);
                        } catch (...) {
                            // Ignore handler exceptions
                        }
                        delete elementPtr;
                    }
                }
            }
            segment = segment->prev.load(std::memory_order_acquire);
        }
    }
}

template <typename E>
void BufferedChannel<E>::cancelSuspendedReceiveRequests(ChannelSegment<E>* lastSegment, long long sendersCounter) {
    // Simplified cancellation - would handle suspended receivers in real implementation
    auto segment = lastSegment;
    while (segment) {
        for (int i = SEGMENT_SIZE - 1; i >= 0; --i) {
            auto globalIndex = segment->id * SEGMENT_SIZE + i;
            if (globalIndex < sendersCounter) {
                break; // All remaining cells are covered by senders
            }
            
            auto state = segment->get_state(i);
            if (state == CellState::WAITER_RECEIVER) {
                auto waiter = static_cast<Waiter*>(segment->get_data(i));
                if (waiter) {
                    waiter->resume_with_exception(std::make_exception_ptr(
                        std::runtime_error("Channel cancelled")));
                }
                segment->cas_state(i, CellState::WAITER_RECEIVER, CellState::CHANNEL_CLOSED);
            }
        }
        segment = segment->prev.load(std::memory_order_acquire);
    }
}

// Explicit instantiation for common types to ensure compilation validity and linkage.
template class BufferedChannel<int>;

} // namespace channels
} // namespace coroutines
} // namespace kotlinx