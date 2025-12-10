#pragma once
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/DispatchedContinuation.hpp"
#include <atomic>
#include <mutex>
#include <memory>

namespace kotlinx {
namespace coroutines {

// Stub base classes
// DispatchedTask removed (using internal/DispatchedTask.hpp)

struct CoroutineStackFrame {};
struct Waiter {};

    // State Hierarchy for RTTI logic (mirrors Kotlin's Any + is checks)
    struct State { 
        virtual ~State() = default; 
        virtual bool is_active_state() const { return false; }
    };
    
    struct Active : State {
        static Active instance;
        bool is_active_state() const override { return true; }
        std::string to_string() const { return "Active"; }
    };
    inline Active Active::instance;

    struct ActiveHandler : State {
        std::function<void(std::exception_ptr)> handler;
        explicit ActiveHandler(std::function<void(std::exception_ptr)> h) : handler(h) {}
        bool is_active_state() const override { return true; }
    };

    struct CancelledContinuation : State {
        std::exception_ptr cause;
        bool handled;
        explicit CancelledContinuation(std::exception_ptr cause, bool handled = false) 
            : cause(cause), handled(handled) {}
    };

    struct CompletedContinuation : State {
        std::shared_ptr<void> result; // Type erased result
        explicit CompletedContinuation(std::shared_ptr<void> r) : result(r) {}
    };

    template <typename T>
    class CancellableContinuationImpl : public DispatchedTask<T>, 
                                        public CancellableContinuation<T>, 
                                        public CoroutineStackFrame, 
                                        public Waiter,
                                        public std::enable_shared_from_this<CancellableContinuationImpl<T>> {
    public:
    CancellableContinuationImpl(std::shared_ptr<Continuation<T>> delegate, int resume_mode)
        : DispatchedTask<T>(resume_mode), delegate(delegate), resume_mode(resume_mode) {
        state_.store(&Active::instance, std::memory_order_release);
    }

    std::shared_ptr<Continuation<T>> get_delegate() override { return delegate; }
    
    Result<T> take_state() override {
        State* s = state_.load(std::memory_order_acquire);
        if (auto* c = dynamic_cast<CompletedContinuation*>(s)) {
             return Result<T>::success(*static_cast<T*>(c->result.get()));
        }
        if (auto* c = dynamic_cast<CancelledContinuation*>(s)) {
             return Result<T>::failure(c->cause);
        }
        throw std::runtime_error("Invalid state in take_state");
    }

    bool is_active() const override { 
        return state_.load(std::memory_order_acquire)->is_active_state();
    }

    void resume(T value, std::function<void(std::exception_ptr)> on_cancellation) override {
        // Keep alive during resumption
        auto self = this->shared_from_this();
        // TODO: Handle on_cancellation
        void* token = try_resume(value);
        if (token) {
            complete_resume(token);
        }
    }

    std::shared_ptr<CoroutineContext> get_context() const override { return delegate->get_context(); }

    void resume_with(Result<T> result) override {
        delegate->resume_with(result);
    }

    bool is_completed() const override { 
        State* s = state_.load(std::memory_order_acquire);
        return s != &Active::instance && !is_cancelled_state(s);
    }
    
    bool is_cancelled() const override { 
        return is_cancelled_state(state_.load(std::memory_order_acquire));
    }

    void complete_resume(void* token) override {
        dispatch_resume(token);
    }
    
    void init_cancellability() override {
        auto ctx = get_context();
        if (!ctx) return;
        
        auto job_element = ctx->get(Job::type_key);
        auto job = std::dynamic_pointer_cast<struct Job>(job_element);
        if (job) {
             // Kotlin: parentHandle = parent.invokeOnCompletion(onCancelling = true, handler = ChildContinuation(this))
             auto new_handle = job->invoke_on_completion(true, true, [self = this->weak_from_this()](std::exception_ptr cause){
                 if (auto p = self.lock()) {
                     p->cancel(cause);
                 }
             });
             // TODO: Use explicit ChildContinuation class for full fidelity?
             // Lambda is equivalent for now, but explicit class allows RTTI/debugging checks similar to Kotlin.
             
             {
                 std::lock_guard<std::mutex> lock(parent_handle_mutex_);
                 parentHandle_ = new_handle;
                 // If already completed, dispose immediately
                 if (is_completed()) {
                     new_handle->dispose();
                     parentHandle_ = nullptr;
                 }
             }
        }
    }
    

    
    bool cancel(std::exception_ptr cause = nullptr) override {
        while(true) {
            State* expected = state_.load(std::memory_order_acquire);
            if (expected->is_active_state()) {
                 auto* res = new CancelledContinuation(cause);
                 if (state_.compare_exchange_strong(expected, res, std::memory_order_release, std::memory_order_acquire)) {
                     if (auto* ah = dynamic_cast<ActiveHandler*>(expected)) {
                         ah->handler(cause);
                         delete ah;
                     }
                     detach_child();
                     dispatch_resume(res); // Important!
                     return true;
                 }
                 delete res;
            } else {
                return false;
            }
        }
    }
    
    void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) override {
        while(true) {
             State* expected = state_.load(std::memory_order_acquire);
             if (expected == &Active::instance) {
                 auto* ah = new ActiveHandler(handler);
                 if (state_.compare_exchange_strong(expected, ah, std::memory_order_release, std::memory_order_acquire)) {
                     return;
                 }
                 delete ah;
             } else if (auto* c = dynamic_cast<CancelledContinuation*>(expected)) {
                 handler(c->cause);
                 return;
             } else if (dynamic_cast<ActiveHandler*>(expected)) {
                 // Already has handler... multiple handlers TODO
                 // For now, ignore or throw?
                 // Kotlin appends. We can make ActiveHandler hold a list or chain.
                 // Simplest: chain.
                 // let's just abort for MVP (One handler supported)
                 return;
             } else {
                 // Completed
                 return;
             }
        }
    }

    // Update try_resume to use is_active_state()
    void* try_resume(T value, void* idempotent = nullptr) override {
        while(true) {
            State* expected = state_.load(std::memory_order_acquire);
            if (expected->is_active_state()) {
                 auto* res = new CompletedContinuation(std::make_shared<T>(value));
                 if (state_.compare_exchange_strong(expected, res, std::memory_order_release, std::memory_order_acquire)) {
                     // If expected was ActiveHandler, delete it
                     if (auto* ah = dynamic_cast<ActiveHandler*>(expected)) {
                         delete ah; // Handler is now obsolete
                     }
                     detach_child();
                     return res; 
                 }
                 delete res;
            } else {
                return nullptr;
            }
        }
    }

    void* try_resume_with_exception(std::exception_ptr exception) override {
        while(true) {
            State* expected = state_.load(std::memory_order_acquire);
            if (expected->is_active_state()) {
                 auto* res = new CancelledContinuation(exception); 
                 if (state_.compare_exchange_strong(expected, res, std::memory_order_release, std::memory_order_acquire)) {
                     if (auto* ah = dynamic_cast<ActiveHandler*>(expected)) {
                         delete ah; 
                     }
                     detach_child();
                     return res;
                 }
                 delete res;
            } else {
                return nullptr; 
            }
        }
    }

    void resume_undispatched(CoroutineDispatcher* dispatcher, T value) override {
        auto dc = std::dynamic_pointer_cast<DispatchedContinuation<T>>(delegate);
        if (dc && dc->dispatcher.get() == dispatcher) {
             void* token = try_resume(value);
             if (token) {
                 detach_child();
                 dc->continuation->resume_with(Result<T>::success(value)); // Bypass dispatch
             }
        } else {
             resume(value, nullptr);
        }
    }

    void resume_undispatched_with_exception(CoroutineDispatcher* dispatcher, std::exception_ptr exception) override {
        auto dc = std::dynamic_pointer_cast<DispatchedContinuation<T>>(delegate);
        if (dc && dc->dispatcher.get() == dispatcher) {
             void* token = try_resume_with_exception(exception);
             if (token) {
                 detach_child();
                 dc->continuation->resume_with(Result<T>::failure(exception));
             }
        } else {
             // resume uses callbacks, passing exception logic not directly exposed in basic resume() override?
             // Helper needed.
             // Our basic resume() takes value.
             // We need resume_with(Result).
             resume_with(Result<T>::failure(exception));
        }
    }

    // Suspension Point logic
    T get_result() {
        State* s = state_.load(std::memory_order_acquire);
        if (s == &Active::instance) {
             // We return a specific token to say "Suspended"
             // But T is the return type.
             // For standard threads, this might block?
             // Or C++20 await_resume should check?
             // If we are here via await_resume, it means handle.resume() was called.
             // If handle.resume() was called, state MUST be completed/cancelled.
             // So if Active, it's an error in logic (resumed prematurely?).
             // UNLESS we are checking *immediate* availability before suspend?
             // But get_result is called by await_resume which is after resumption.
             // So state should not be Active.
        }
        
        if (auto* c = dynamic_cast<CancelledContinuation*>(s)) {
            std::rethrow_exception(c->cause);
        }
        if (auto* c = dynamic_cast<CompletedContinuation*>(s)) {
             return *static_cast<T*>(c->result.get());
        }
        throw std::runtime_error("Invalid state in get_result");
    }

private:
   bool is_cancelled_state(State* s) const {
       return dynamic_cast<CancelledContinuation*>(s) != nullptr;
   }
   
   std::atomic<State*> state_; // Typed!
    
    std::shared_ptr<DisposableHandle> parentHandle_;
    std::mutex parent_handle_mutex_;

    void detach_child() {
        std::lock_guard<std::mutex> lock(parent_handle_mutex_);
        if (parentHandle_) {
            parentHandle_->dispose();
            parentHandle_ = nullptr;
        }
    }

    void dispatch_resume(void* state) {
         if (auto* c = dynamic_cast<CompletedContinuation*>((State*)state)) {
             delegate->resume_with(Result<T>::success(*static_cast<T*>(c->result.get())));
         } else if (auto* c = dynamic_cast<CancelledContinuation*>((State*)state)) {
             delegate->resume_with(Result<T>::failure(c->cause));
         }
   }
   std::shared_ptr<Continuation<T>> delegate;
   int resume_mode;
};

// Void Specialization of Impl
template <>
class CancellableContinuationImpl<void> : public DispatchedTask<void>, 
                                    public CancellableContinuation<void>, 
                                    public CoroutineStackFrame, 
                                    public Waiter,
                                    public std::enable_shared_from_this<CancellableContinuationImpl<void>> {
public:
    CancellableContinuationImpl(std::shared_ptr<Continuation<void>> delegate, int resume_mode)
        : DispatchedTask<void>(resume_mode), delegate(delegate), resume_mode(resume_mode) {
        state_.store(&Active::instance, std::memory_order_release);
    }

    std::shared_ptr<Continuation<void>> get_delegate() override { return delegate; }
    
    Result<void> take_state() override {
        State* s = state_.load(std::memory_order_acquire);
        if (dynamic_cast<CompletedContinuation*>(s)) {
             return Result<void>::success();
        }
        if (auto* c = dynamic_cast<CancelledContinuation*>(s)) {
             return Result<void>::failure(c->cause);
        }
        throw std::runtime_error("Invalid state in take_state");
    }

    bool is_active() const override { 
        return state_.load(std::memory_order_acquire)->is_active_state();
    }

    void resume(std::function<void(std::exception_ptr)> on_cancellation) override {
        auto self = this->shared_from_this();
        void* token = try_resume();
        if (token) {
            complete_resume(token);
        }
    }

    std::shared_ptr<CoroutineContext> get_context() const override { return delegate->get_context(); }

    void resume_with(Result<void> result) override {
        delegate->resume_with(result);
    }

    bool is_completed() const override { 
        State* s = state_.load(std::memory_order_acquire);
        return s != &Active::instance && !is_cancelled_state(s);
    }
    
    bool is_cancelled() const override { 
        return is_cancelled_state(state_.load(std::memory_order_acquire));
    }

    void complete_resume(void* token) override {
        dispatch_resume(token);
    }
    
    void init_cancellability() override {
        auto ctx = get_context();
        if (!ctx) return;
        
        auto job_element = ctx->get(Job::type_key);
        auto job = std::dynamic_pointer_cast<struct Job>(job_element);
        if (job) {
             auto new_handle = job->invoke_on_completion(true, true, [self = this->weak_from_this()](std::exception_ptr cause){
                 if (auto p = self.lock()) {
                     p->cancel(cause);
                 }
             });
             {
                 std::lock_guard<std::mutex> lock(parent_handle_mutex_);
                 parentHandle_ = new_handle;
                 if (is_completed()) {
                     new_handle->dispose();
                     parentHandle_ = nullptr;
                 }
             }
        }
    }
    
    bool cancel(std::exception_ptr cause = nullptr) override {
        while(true) {
            State* expected = state_.load(std::memory_order_acquire);
            if (expected->is_active_state()) {
                 auto* res = new CancelledContinuation(cause);
                 if (state_.compare_exchange_strong(expected, res, std::memory_order_release, std::memory_order_acquire)) {
                     if (auto* ah = dynamic_cast<ActiveHandler*>(expected)) {
                         ah->handler(cause);
                         delete ah;
                     }
                     detach_child();
                     dispatch_resume(res); 
                     return true;
                 }
                 delete res;
            } else {
                return false;
            }
        }
    }
    
    void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) override {
        while(true) {
             State* expected = state_.load(std::memory_order_acquire);
             if (expected == &Active::instance) {
                 auto* ah = new ActiveHandler(handler);
                 if (state_.compare_exchange_strong(expected, ah, std::memory_order_release, std::memory_order_acquire)) {
                     return;
                 }
                 delete ah;
             } else if (auto* c = dynamic_cast<CancelledContinuation*>(expected)) {
                 handler(c->cause);
                 return;
             } else if (dynamic_cast<ActiveHandler*>(expected)) {
                 return;
             } else {
                 return;
             }
        }
    }

    void* try_resume(void* idempotent = nullptr) override {
        while(true) {
            State* expected = state_.load(std::memory_order_acquire);
            if (expected->is_active_state()) {
                 auto* res = new CompletedContinuation(nullptr); // No result for void
                 if (state_.compare_exchange_strong(expected, res, std::memory_order_release, std::memory_order_acquire)) {
                     if (auto* ah = dynamic_cast<ActiveHandler*>(expected)) {
                         delete ah; 
                     }
                     detach_child();
                     return res; 
                 }
                 delete res;
            } else {
                return nullptr;
            }
        }
    }

    void* try_resume_with_exception(std::exception_ptr exception) override {
        while(true) {
            State* expected = state_.load(std::memory_order_acquire);
            if (expected->is_active_state()) {
                 auto* res = new CancelledContinuation(exception); 
                 if (state_.compare_exchange_strong(expected, res, std::memory_order_release, std::memory_order_acquire)) {
                     if (auto* ah = dynamic_cast<ActiveHandler*>(expected)) {
                         delete ah; 
                     }
                     detach_child();
                     return res;
                 }
                 delete res;
            } else {
                return nullptr; 
            }
        }
    }

    void resume_undispatched(CoroutineDispatcher* dispatcher) override {
        auto dc = std::dynamic_pointer_cast<DispatchedContinuation<void>>(delegate);
        if (dc && dc->dispatcher.get() == dispatcher) {
             void* token = try_resume();
             if (token) {
                 detach_child();
                 dc->continuation->resume_with(Result<void>::success()); 
             }
        } else {
             resume(nullptr);
        }
    }

    void resume_undispatched_with_exception(CoroutineDispatcher* dispatcher, std::exception_ptr exception) override {
        auto dc = std::dynamic_pointer_cast<DispatchedContinuation<void>>(delegate);
        if (dc && dc->dispatcher.get() == dispatcher) {
             void* token = try_resume_with_exception(exception);
             if (token) {
                 detach_child();
                 dc->continuation->resume_with(Result<void>::failure(exception));
             }
        } else {
             resume_with(Result<void>::failure(exception));
        }
    }

    void get_result() {
        State* s = state_.load(std::memory_order_acquire);
        if (auto* c = dynamic_cast<CancelledContinuation*>(s)) {
            std::rethrow_exception(c->cause);
        }
        if (dynamic_cast<CompletedContinuation*>(s)) {
             return; // Void result
        }
        if (s == &Active::instance) return; // Suspended?
        throw std::runtime_error("Invalid state in get_result");
    }

private:
   bool is_cancelled_state(State* s) const {
       return dynamic_cast<CancelledContinuation*>(s) != nullptr;
   }
   
   std::atomic<State*> state_;
    
    std::shared_ptr<DisposableHandle> parentHandle_;
    std::mutex parent_handle_mutex_;

    void detach_child() {
        std::lock_guard<std::mutex> lock(parent_handle_mutex_);
        if (parentHandle_) {
            parentHandle_->dispose();
            parentHandle_ = nullptr;
        }
    }

    void dispatch_resume(void* state) {
         if (auto* c = dynamic_cast<CompletedContinuation*>((State*)state)) {
             delegate->resume_with(Result<void>::success());
         } else if (auto* c = dynamic_cast<CancelledContinuation*>((State*)state)) {
             delegate->resume_with(Result<void>::failure(c->cause));
         }
   }
   std::shared_ptr<Continuation<void>> delegate;
   int resume_mode;
};


// ------------------------------------------------------------------
// SuspendingCancellableCoroutine Implementation
// ------------------------------------------------------------------

// Adapter for coroutine_handle to Continuation<T>
template <typename T>
struct HandleContinuation : public Continuation<T> {
    std::coroutine_handle<> handle;
    std::shared_ptr<CoroutineContext> context; // Captured context?
    
    HandleContinuation(std::coroutine_handle<> h) : handle(h) {
         // In a real implementation we should extract context from handle via promise?
         // Or empty context if raw.
         // Coroutine traits?
         // For now use Empty stub or TODO
         // We need context to be non-null usually.
         // Let's create an empty context stub or use a global one.
    }
    
    std::shared_ptr<CoroutineContext> get_context() const override { return nullptr; } 
    
    void resume_with(Result<T> result) override {
        // Resume handle
        handle.resume();
    }
};

template <typename T>
void SuspendingCancellableCoroutine<T>::await_suspend(std::coroutine_handle<> h) {
   auto delegate_ptr = std::shared_ptr<Continuation<T>>(std::make_shared<HandleContinuation<T>>(h));
   impl = std::shared_ptr<CancellableContinuationImpl<T>>(new CancellableContinuationImpl<T>(delegate_ptr, 1)); // MODE_CANCELLABLE
   impl->init_cancellability();
   block(*impl);
}

template <typename T>
T SuspendingCancellableCoroutine<T>::await_resume() {
   if (!impl) throw std::runtime_error("Implementation missing");
   return impl->get_result();
}

} // namespace coroutines
} // namespace kotlinx

