#include <kotlinx/coroutines/channels/BufferedChannel.hpp>
#include <kotlinx/coroutines/Exceptions.hpp>
#include <algorithm>
#include <stdexcept>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace channels {

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
    // This is a simplified implementation - the real one would need more sophisticated
    // double-compare-and-swap logic or versioning to prevent ABA problems
    std::uintptr_t expected_state_val = static_cast<std::uintptr_t>(expected_state);
    if (!states[index].compare_exchange_strong(expected_state_val, 
                                              static_cast<std::uintptr_t>(desired_state),
                                              std::memory_order_acq_rel,
                                              std::memory_order_acquire)) {
        return false;
    }
    
    void* expected_data_val = expected_data;
    if (!data[index].compare_exchange_strong(expected_data_val, desired_data,
                                            std::memory_order_acq_rel,
                                            std::memory_order_acquire)) {
        // Rollback state change if data update failed
        states[index].store(static_cast<std::uintptr_t>(expected_state), std::memory_order_release);
        return false;
    }
    
    return true;
}

template <typename E>
void ChannelSegment<E>::clean_prev() {
    ChannelSegment<E>* prev_seg = prev.load(std::memory_order_acquire);
    if (prev_seg != nullptr) {
        prev_seg->mark_removed();
        // In a real implementation, this would coordinate with garbage collection
    }
}

template <typename E>
ChannelSegment<E>* ChannelSegment<E>::close() {
    // Mark the segment as closed and return the next segment
    ChannelSegment<E>* next_seg = next.load(std::memory_order_acquire);
    mark_removed();
    return next_seg;
}

// BufferedChannel implementation
template <typename E>
BufferedChannel<E>::BufferedChannel(int capacity, std::function<void(const E&, std::exception_ptr)> onUndeliveredElement)
    : capacity_(capacity), onUndeliveredElement_(onUndeliveredElement),
      sendersAndCloseStatus(0), receivers(0), bufferEnd(initialBufferEnd(capacity)),
      completedExpandBuffersAndPauseFlag(initialBufferEnd(capacity)),
      sendSegment(nullptr), receiveSegment(nullptr), bufferEndSegment(nullptr),
      closed_(false), closeCause_(nullptr) {
    
    // Create initial segment
    ChannelSegment<E>* initial = createSegment(0, nullptr);
    sendSegment.store(initial, std::memory_order_release);
    receiveSegment.store(initial, std::memory_order_release);
    bufferEndSegment.store(initial, std::memory_order_release);
}

template <typename E>
BufferedChannel<E>::~BufferedChannel() {
    // Clean up all segments
    ChannelSegment<E>* last = closeLinkedList();
    removeUnprocessedElements(last, sendersAndCloseStatus.load(std::memory_order_acquire));
}

template <typename E>
bool BufferedChannel<E>::is_closed_for_send() const {
    long long sendersAndStatus = sendersAndCloseStatus.load(std::memory_order_acquire);
    return extractClosedStatus(sendersAndStatus);
}

template <typename E>
bool BufferedChannel<E>::is_closed_for_receive() const {
    if (!closed_.load(std::memory_order_acquire)) {
        return false;
    }
    long long sendersAndStatus = sendersAndCloseStatus.load(std::memory_order_acquire);
    long long receivers_counter = receivers.load(std::memory_order_acquire);
    return extractSendersCounter(sendersAndStatus) <= receivers_counter;
}

template <typename E>
bool BufferedChannel<E>::is_empty() const {
    if (is_closed_for_receive()) {
        return true;
    }
    long long sendersAndStatus = sendersAndCloseStatus.load(std::memory_order_acquire);
    long long receivers_counter = receivers.load(std::memory_order_acquire);
    return extractSendersCounter(sendersAndStatus) <= receivers_counter;
}

template <typename E>
void BufferedChannel<E>::send(E element) {
    sendImpl(element);
}

template <typename E>
ChannelResult<void> BufferedChannel<E>::try_send(E element) {
    // Simplified implementation - real one would be more sophisticated
    if (is_closed_for_send()) {
        return ChannelResult<void>::closed_result(closeCause_);
    }
    
    long long sendersAndStatus = sendersAndCloseStatus.load(std::memory_order_acquire);
    if (shouldSendSuspend(sendersAndStatus)) {
        return ChannelResult<void>::failure();
    }
    
    sendImpl(element);
    return ChannelResult<void>::success(nullptr);
}

template <typename E>
E BufferedChannel<E>::receive() {
    return receiveImpl();
}

template <typename E>
ChannelResult<E> BufferedChannel<E>::receive_catching() {
    if (is_closed_for_receive() && is_empty()) {
        return ChannelResult<E>::closed_result(closeCause_);
    }
    
    try {
        E result = receiveImpl();
        return ChannelResult<E>::success(&result);
    } catch (const std::exception& e) {
        return ChannelResult<E>::failure();
    }
}

template <typename E>
ChannelResult<E> BufferedChannel<E>::try_receive() {
    // Simplified implementation
    if (is_empty()) {
        if (is_closed_for_receive()) {
            return ChannelResult<E>::closed_result(closeCause_);
        }
        return ChannelResult<E>::failure();
    }
    
    try {
        E result = receiveImpl();
        return ChannelResult<E>::success(&result);
    } catch (const std::exception& e) {
        return ChannelResult<E>::failure();
    }
}

template <typename E>
bool BufferedChannel<E>::close(std::exception_ptr cause) {
    std::lock_guard<std::mutex> lock(closeMutex_);
    if (closed_.load(std::memory_order_acquire)) {
        return false;
    }
    
    closed_.store(true, std::memory_order_release);
    closeCause_ = cause;
    
    long long sendersCur = sendersAndCloseStatus.load(std::memory_order_acquire);
    completeClose(sendersCur);
    
    // Invoke close handlers
    for (auto& handler : closeHandlers_) {
        handler(cause);
    }
    closeHandlers_.clear();
    
    return true;
}

template <typename E>
void BufferedChannel<E>::invoke_on_close(std::function<void(std::exception_ptr)> handler) {
    std::lock_guard<std::mutex> lock(closeMutex_);
    if (closed_.load(std::memory_order_acquire)) {
        handler(closeCause_);
    } else {
        closeHandlers_.push_back(handler);
    }
}

template <typename E>
void BufferedChannel<E>::cancel(std::exception_ptr cause) {
    close(cause);
}

template <typename E>
std::shared_ptr<ChannelIterator<E>> BufferedChannel<E>::iterator() {
    // Simplified iterator implementation
    return nullptr; // Would return actual iterator
}

// Helper methods
template <typename E>
long long BufferedChannel<E>::initialBufferEnd(int capacity) {
    if (capacity == 0) return BUFFER_END_RENDEZVOUS;
    if (capacity == Channel<E>::UNLIMITED) return BUFFER_END_UNLIMITED;
    return capacity;
}

template <typename E>
long long BufferedChannel<E>::extractSendersCounter(long long sendersAndCloseStatus) {
    return sendersAndCloseStatus & 0x7FFFFFFFFFFFFFFFLL; // Clear high bit
}

template <typename E>
bool BufferedChannel<E>::extractClosedStatus(long long sendersAndCloseStatus) {
    return (sendersAndCloseStatus & 0x8000000000000000LL) != 0;
}

template <typename E>
long long BufferedChannel<E>::packSendersAndCloseStatus(long long senders, bool closed) {
    return senders | (closed ? 0x8000000000000000LL : 0);
}

template <typename E>
bool BufferedChannel<E>::shouldSendSuspend(long long sendersAndCloseStatus) const {
    if (extractClosedStatus(sendersAndCloseStatus)) return false;
    long long senders = extractSendersCounter(sendersAndCloseStatus);
    return !bufferOrRendezvousSend(senders);
}

template <typename E>
bool BufferedChannel<E>::bufferOrRendezvousSend(long long senders) const {
    if (isRendezvousOrUnlimited()) return false;
    long long buffer_end = bufferEnd.load(std::memory_order_acquire);
    return senders < buffer_end;
}

template <typename E>
bool BufferedChannel<E>::isRendezvousOrUnlimited() const {
    return capacity_ == 0 || capacity_ == Channel<E>::UNLIMITED;
}

// Core algorithms (simplified implementations)
template <typename E>
void BufferedChannel<E>::sendImpl(const E& element) {
    while (true) {
        long long sendersAndStatus = sendersAndCloseStatus.load(std::memory_order_acquire);
        long long senders = extractSendersCounter(sendersAndStatus);
        
        // Try to increment senders counter
        long long newSendersAndStatus = packSendersAndCloseStatus(senders + 1, extractClosedStatus(sendersAndStatus));
        if (sendersAndCloseStatus.compare_exchange_strong(sendersAndStatus, newSendersAndStatus,
                                                         std::memory_order_acq_rel, std::memory_order_acquire)) {
            // Successfully reserved a cell
            ChannelSegment<E>* segment = findSegmentSend(senders, sendSegment.load(std::memory_order_acquire));
            int index = static_cast<int>(senders % SEGMENT_SIZE);
            
            updateCellSend(segment, index, element, senders, nullptr, extractClosedStatus(sendersAndStatus));
            return;
        }
        // Retry if CAS failed
    }
}

template <typename E>
E BufferedChannel<E>::receiveImpl() {
    while (true) {
        long long receivers_counter = receivers.load(std::memory_order_acquire);
        
        // Try to increment receivers counter
        if (receivers.compare_exchange_strong(receivers_counter, receivers_counter + 1,
                                             std::memory_order_acq_rel, std::memory_order_acquire)) {
            // Successfully reserved a cell
            ChannelSegment<E>* segment = findSegmentReceive(receivers_counter, receiveSegment.load(std::memory_order_acquire));
            int index = static_cast<int>(receivers_counter % SEGMENT_SIZE);
            
            return receiveImplSuspend(segment, index, receivers_counter);
        }
        // Retry if CAS failed
    }
}

template <typename E>
void BufferedChannel<E>::sendImplSuspend(ChannelSegment<E>* segment, int index, const E& element, long long s, void* waiter) {
    // Simplified suspension logic
    updateCellSend(segment, index, element, s, waiter, false);
}

template <typename E>
E BufferedChannel<E>::receiveImplSuspend(ChannelSegment<E>* segment, int index, long long r) {
    // Simplified receive logic
    CellState state = segment->get_state(index);
    if (state == CellState::BUFFERED) {
        E* element = static_cast<E*>(segment->get_data(index));
        E result = *element;
        delete element;
        segment->cas_state(index, CellState::BUFFERED, CellState::EMPTY);
        return result;
    }
    
    // Handle other states (rendezvous, closed, etc.)
    throw std::runtime_error("Channel receive failed");
}

template <typename E>
void BufferedChannel<E>::updateCellSend(ChannelSegment<E>* segment, int index, const E& element, long long s, void* waiter, bool closed) {
    if (closed) {
        segment->cas_state(index, CellState::EMPTY, CellState::CHANNEL_CLOSED);
        return;
    }
    
    // Store element in cell
    E* element_copy = new E(element);
    segment->set_data(index, element_copy);
    segment->cas_state(index, CellState::EMPTY, CellState::BUFFERED);
}

template <typename E>
void BufferedChannel<E>::updateCellReceive(ChannelSegment<E>* segment, int index, long long r, void* waiter) {
    // Simplified receive update
    CellState state = segment->get_state(index);
    if (state == CellState::BUFFERED) {
        segment->cas_state(index, CellState::BUFFERED, CellState::EMPTY);
    }
}

// Segment management (simplified)
template <typename E>
ChannelSegment<E>* BufferedChannel<E>::findSegmentSend(long long id, ChannelSegment<E>* startFrom) {
    ChannelSegment<E>* current = startFrom;
    while (current != nullptr && current->id <= id / SEGMENT_SIZE) {
        if (current->id == id / SEGMENT_SIZE) {
            return current;
        }
        ChannelSegment<E>* next = current->next.load(std::memory_order_acquire);
        if (next == nullptr) {
            // Create new segment
            next = createSegment(current->id + 1, current);
            ChannelSegment<E>* expected_null = nullptr;
            if (current->next.compare_exchange_strong(expected_null, next, std::memory_order_acq_rel, std::memory_order_acquire)) {
                return next;
            }
            // Another thread created the segment, use it
            next = current->next.load(std::memory_order_acquire);
        }
        current = next;
    }
    return current;
}

template <typename E>
ChannelSegment<E>* BufferedChannel<E>::findSegmentReceive(long long id, ChannelSegment<E>* startFrom) {
    return findSegmentSend(id, startFrom); // Same logic for both
}

template <typename E>
ChannelSegment<E>* BufferedChannel<E>::createSegment(long long id, ChannelSegment<E>* prev) {
    return new ChannelSegment<E>(id, prev, this);
}

template <typename E>
void BufferedChannel<E>::completeClose(long long sendersCur) {
    // Update close status in atomic counter
    long long current = sendersAndCloseStatus.load(std::memory_order_acquire);
    while (!sendersAndCloseStatus.compare_exchange_strong(current, packSendersAndCloseStatus(extractSendersCounter(current), true),
                                                         std::memory_order_acq_rel, std::memory_order_acquire)) {
        // Retry until successful
    }
}

template <typename E>
ChannelSegment<E>* BufferedChannel<E>::closeLinkedList() {
    ChannelSegment<E>* last = bufferEndSegment.load(std::memory_order_acquire);
    // Mark all segments as closed
    ChannelSegment<E>* current = sendSegment.load(std::memory_order_acquire);
    while (current != nullptr) {
        ChannelSegment<E>* next = current->close();
        current = next;
    }
    return last;
}

template <typename E>
void BufferedChannel<E>::removeUnprocessedElements(ChannelSegment<E>* lastSegment, long long sendersCounter) {
    // Clean up any remaining elements in the buffer
    ChannelSegment<E>* current = sendSegment.load(std::memory_order_acquire);
    while (current != nullptr && current != lastSegment) {
        for (int i = 0; i < SEGMENT_SIZE; ++i) {
            if (current->get_state(i) == CellState::BUFFERED) {
                E* element = static_cast<E*>(current->get_data(i));
                if (onUndeliveredElement_) {
                    onUndeliveredElement_(*element, closeCause_);
                }
                delete element;
            }
        }
        current = current->next.load(std::memory_order_acquire);
    }
}

template <typename E>
void BufferedChannel<E>::cancelSuspendedReceiveRequests(ChannelSegment<E>* lastSegment, long long sendersCounter) {
    // Cancel any suspended receivers
    // This would involve resuming suspended coroutines with cancellation exceptions
}

// Explicit template instantiations
template class ChannelSegment<int>;
template class BufferedChannel<int>;

} // namespace channels
} // namespace coroutines
} // namespace kotlinx