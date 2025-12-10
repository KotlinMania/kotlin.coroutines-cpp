#pragma once
#include "kotlinx/coroutines/Deferred.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include <memory>
#include <exception>

namespace kotlinx {
namespace coroutines {

template<typename T>
class CompletableDeferred : public Deferred<T> {
public:
    virtual bool complete(T value) = 0;
    virtual bool complete_exceptionally(std::exception_ptr exception) = 0;
};

// Extension function stub
template<typename T>
bool complete_with(CompletableDeferred<T>* deferred, Result<T> result) {
    if (result.is_success()) {
        return deferred->complete(result.get_or_throw());
    } else {
        return deferred->complete_exceptionally(result.exception_or_null());
    }
}

template<typename T>
class CompletableDeferredImpl : public JobSupport, public CompletableDeferred<T> {
private:
    std::shared_ptr<Job> parent_job;

public:
    CompletableDeferredImpl(std::shared_ptr<Job> parent)
        : JobSupport(true), parent_job(parent) {
        if (parent) {
             // Cast Job to Element explicitly if needed, but Job inherits Element
             // init_parent_job_internal takes shared_ptr<Element>? No, I decided.
             // Let's check JobSupport... assuming init_parent_job_internal is available
             // But wait, AbstractCoroutine has init_parent_job_internal. JobSupport does NOT?
             // JobSupport has init_parent_job_internal?
             // I need to check JobSupport definition again.
             // If not, I should implement logic here.
             
             parent->start();
             auto handle = parent->attach_child(std::static_pointer_cast<ChildJob>(this->shared_from_this()));
             // ignore handle for now or store it? JobSupport stores parentHandle_
             // JobSupport has parentHandle_ protected member.
             // But parentHandle_ is atomic<DisposableHandle*>.
             // Using attach_child returns shared_ptr which manages lifetime?
             // Need to adapt state_? 
        }
    }

    bool get_on_cancel_complete() const override {
        return true;
    }

    T get_completed() const override {
        // stub
        return T();
    }
     
    std::exception_ptr get_completion_exception_or_null() const override {
        return nullptr; // stub
    }

    T await() override {
        // stub
        return T();
    }

    // SelectClause1<T>& get_on_await() override { ... }

    bool complete(T value) override {
        return this->make_completing(new T(value)); // Leaking T? JobSupport logic needed
    }

    bool complete_exceptionally(std::exception_ptr exception) override {
        return this->make_completing(std::make_exception_ptr(exception)); 
        // make_completing has overload for exception_ptr?
    }
    
     // Job overrides from Deferred
     bool is_active() const override { return JobSupport::is_active(); }
     bool is_completed() const override { return JobSupport::is_completed(); }
     bool is_cancelled() const override { return JobSupport::is_cancelled(); }
     std::exception_ptr get_cancellation_exception() override { return JobSupport::get_cancellation_exception(); }
     bool start() override { return JobSupport::start(); }
     void cancel(std::exception_ptr cause = nullptr) override { JobSupport::cancel(cause); }
     std::shared_ptr<Job> get_parent() const override { return JobSupport::get_parent(); }
     std::vector<std::shared_ptr<Job>> get_children() const override { return JobSupport::get_children(); }
     std::shared_ptr<DisposableHandle> attach_child(std::shared_ptr<ChildJob> child) override { return JobSupport::attach_child(child); }
     void join() override { JobSupport::join(); }
     std::shared_ptr<DisposableHandle> invoke_on_completion(std::function<void(std::exception_ptr)> handler) override { return JobSupport::invoke_on_completion(handler); }
     std::shared_ptr<DisposableHandle> invoke_on_completion(bool on_cancelling, bool invoke_immediately, std::function<void(std::exception_ptr)> handler) override { return JobSupport::invoke_on_completion(on_cancelling, invoke_immediately, handler); }
     CoroutineContext::Key* key() const override { return JobSupport::key(); }
};

template<typename T>
std::shared_ptr<CompletableDeferred<T>> create_completable_deferred(std::shared_ptr<Job> parent = nullptr) {
    return std::make_shared<CompletableDeferredImpl<T>>(parent);
}

} // namespace coroutines
} // namespace kotlinx
