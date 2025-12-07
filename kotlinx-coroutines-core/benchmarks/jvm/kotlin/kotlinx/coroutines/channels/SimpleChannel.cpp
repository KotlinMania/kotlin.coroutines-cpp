// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/benchmarks/jvm/kotlin/kotlinx/coroutines/channels/SimpleChannel.kt
// TODO: Resolve imports and dependencies
// TODO: Handle suspend functions and coroutines
// TODO: Implement continuation and intrinsics

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.coroutines.intrinsics.*

class SimpleChannel {
public:
    static constexpr int kNullSurrogate = -1;

protected:
    Continuation<void>* producer_ = nullptr;
    int enqueued_value_ = kNullSurrogate;
    Continuation<int>* consumer_ = nullptr;

public:
    virtual ~SimpleChannel() = default;

    // TODO: suspend function
    void send(int element) {
        // TODO: require(element != NULL_SURROGATE)
        if (offer(element)) {
            return;
        }

        return suspend_send(element);
    }

private:
    bool offer(int element) {
        if (consumer_ == nullptr) {
            return false;
        }

        consumer_->resume(element);
        consumer_ = nullptr;
        return true;
    }

public:
    // TODO: suspend function
    int receive() {
        // Cached value
        if (enqueued_value_ != kNullSurrogate) {
            int result = enqueued_value_;
            enqueued_value_ = kNullSurrogate;
            producer_->resume();
            return result;
        }

        return suspend_receive();
    }

    // TODO: suspend function
    virtual int suspend_receive() = 0;

    // TODO: suspend function
    virtual void suspend_send(int element) = 0;
};

class NonCancellableChannel : public SimpleChannel {
public:
    // TODO: suspend function - suspendCoroutineUninterceptedOrReturn
    int suspend_receive() override {
        // TODO: suspendCoroutineUninterceptedOrReturn implementation
        // consumer = it.intercepted()
        // COROUTINE_SUSPENDED
        return 0;
    }

    // TODO: suspend function - suspendCoroutineUninterceptedOrReturn
    void suspend_send(int element) override {
        // TODO: suspendCoroutineUninterceptedOrReturn implementation
        // enqueuedValue = element
        // producer = it.intercepted()
        // COROUTINE_SUSPENDED
    }
};

class CancellableChannel : public SimpleChannel {
public:
    // TODO: suspend function - suspendCancellableCoroutine
    int suspend_receive() override {
        // TODO: suspendCancellableCoroutine implementation
        // consumer = it.intercepted()
        // COROUTINE_SUSPENDED
        return 0;
    }

    // TODO: suspend function - suspendCancellableCoroutine
    void suspend_send(int element) override {
        // TODO: suspendCancellableCoroutine implementation
        // enqueuedValue = element
        // producer = it.intercepted()
        // COROUTINE_SUSPENDED
    }
};

class CancellableReusableChannel : public SimpleChannel {
public:
    // TODO: suspend function - suspendCancellableCoroutineReusable
    int suspend_receive() override {
        // TODO: suspendCancellableCoroutineReusable implementation
        // consumer = it.intercepted()
        // COROUTINE_SUSPENDED
        return 0;
    }

    // TODO: suspend function - suspendCancellableCoroutineReusable
    void suspend_send(int element) override {
        // TODO: suspendCancellableCoroutineReusable implementation
        // enqueuedValue = element
        // producer = it.intercepted()
        // COROUTINE_SUSPENDED
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
