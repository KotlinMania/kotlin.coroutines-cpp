/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt
 */
// port-lint: source kotlinx-coroutines-core/common/src/flow/internal/Combine.kt
#include "kotlinx/coroutines/flow/internal/Combine.hpp"
#include "kotlinx/coroutines/flow/internal/FlowCoroutine.hpp"
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include "kotlinx/coroutines/flow/internal/NullSurrogate.hpp"
#include "kotlinx/coroutines/internal/LocalAtomics.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include <algorithm>
#include <optional>

namespace kotlinx::coroutines::flow::internal {
namespace {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:9-9
// NOTE(port): The private IndexedValue specialization carries the erased source value.
struct Update {
    std::size_t index;
    std::any value;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:21-22
struct CombineState {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:21-22
    explicit CombineState(int size)
        : result_channel(channels::create_channel<Update>(size)), non_closed(size) {}
    std::shared_ptr<channels::Channel<Update>> result_channel;
    kotlinx::coroutines::internal::LocalAtomicInt non_closed;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
// NOTE(port): The compiler frame retains the actual channel and Update across send/yield.
class SendUpdateContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
    SendUpdateContinuation(std::shared_ptr<channels::Channel<Update>> channel,
                           Update update, Continuation<void*>* completion)
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
          channel_(std::move(channel)), update_(std::move(update)) {}
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
    void retain() { self_ref_ = shared_from_this(); }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        coroutine_yield(this, channel_->send(std::move(*update_), this));
        update_.reset();
        coroutine_yield(this, yield(shared_from_this()));
        coroutine_end(this)
    }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
    void release_intercepted() override {
        auto self = std::move(self_ref_);
        channel_.reset();
        update_.reset();
        ContinuationImpl::release_intercepted();
    }
private:
    void* _label = nullptr;
    std::shared_ptr<channels::Channel<Update>> channel_;
    std::optional<Update> update_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
class CombineCollector final : public FlowCollector<std::any> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
    CombineCollector(std::size_t index, std::shared_ptr<channels::Channel<Update>> channel)
        : index_(index), channel_(std::move(channel)) {}
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
    void* emit(std::any value, Continuation<void*>* completion) override {
        auto frame = std::make_shared<SendUpdateContinuation>(
            channel_, Update{index_, std::move(value)}, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    }
private:
    std::size_t index_;
    std::shared_ptr<channels::Channel<Update>> channel_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:27-40
class CollectSourceContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:27-40
    CollectSourceContinuation(std::shared_ptr<Flow<std::any>> flow, std::size_t index,
                              std::shared_ptr<CombineState> state, Continuation<void*>* completion)
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
          flow_(std::move(flow)), state_(std::move(state)),
          collector_(std::make_shared<CombineCollector>(index, state_->result_channel)) {}
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:27-40
    void retain() { self_ref_ = shared_from_this(); }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:27-40
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            coroutine_yield(this, flow_->collect(collector_.get(), this));
        } catch (...) {
            if (state_->non_closed.decrement_and_get() == 0) state_->result_channel->close();
            throw;
        }
        // Close the channel when there are no more flows.
        if (state_->non_closed.decrement_and_get() == 0) state_->result_channel->close();
        coroutine_end(this)
    }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:27-40
    void release_intercepted() override {
        auto self = std::move(self_ref_);
        flow_.reset();
        collector_.reset();
        state_.reset();
        ContinuationImpl::release_intercepted();
    }
private:
    void* _label = nullptr;
    std::shared_ptr<Flow<std::any>> flow_;
    std::shared_ptr<CombineState> state_;
    std::shared_ptr<CombineCollector> collector_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:17-80
class CombineContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:17-80
    CombineContinuation(CoroutineScope* scope, std::vector<std::shared_ptr<Flow<std::any>>> flows,
                        std::function<std::vector<std::any>*()> array_factory,
                        std::function<void*(const std::vector<std::any>&, Continuation<void*>*)> transform,
                        Continuation<void*>* completion)
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
          scope_(scope), flows_(std::move(flows)), array_factory_(std::move(array_factory)),
          transform_(std::move(transform)) {}
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:17-80
    void retain() { self_ref_ = shared_from_this(); }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:17-80
    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        if (flows_.empty()) return nullptr; // Bail out for empty input.
        latest_values_.resize(flows_.size());
        std::fill(latest_values_.begin(), latest_values_.end(), &UNINITIALIZED());
        state_ = std::make_shared<CombineState>(static_cast<int>(flows_.size()));
        remaining_absent_values_ = flows_.size();
        for (std::size_t i = 0; i < flows_.size(); ++i) {
            // Coroutine per flow that keeps track of its value and sends updates downstream.
            launch(scope_, [flow = flows_[i], i, state = state_](
                CoroutineScope*, std::shared_ptr<Continuation<void*>> completion) -> void* {
                auto frame = std::make_shared<CollectSourceContinuation>(flow, i, state, completion.get());
                frame->retain();
                return frame->start(Result<void*>::success(nullptr));
            });
        }
        last_received_epoch_.resize(flows_.size(), 0);
        while (true) {
            ++current_epoch_;
            // The very first receive in an epoch should suspend when no update is ready.
            coroutine_yield_value(this, result, state_->result_channel->receive_catching(this), received_box_);
            received_.reset(static_cast<channels::ChannelResult<Update>*>(received_box_));
            received_box_ = nullptr;
            element_ = received_->is_success() ? std::optional<Update>(received_->get_or_throw()) : std::nullopt;
            received_.reset();
            if (!element_) break;
            // Batch-receive, stopping at the second value from the same source.
            while (true) {
                index_ = element_->index;
                if (auto* previous = std::any_cast<kotlinx::coroutines::internal::Symbol*>(
                        &latest_values_[index_]); previous && *previous == &UNINITIALIZED())
                    --remaining_absent_values_;
                latest_values_[index_] = std::move(element_->value);
                if (last_received_epoch_[index_] == current_epoch_) break;
                last_received_epoch_[index_] = current_epoch_;
                {
                    auto next = state_->result_channel->try_receive();
                    element_ = next.is_success() ? std::optional<Update>(next.get_or_throw()) : std::nullopt;
                }
                if (!element_) break;
            }
            if (remaining_absent_values_ == 0) {
                results_.reset(array_factory_());
                // A null factory avoids copying for transformers that immediately deconstruct the array.
                if (!results_) {
                    coroutine_yield(this, std::function(transform_)(latest_values_, this));
                } else {
                    if (results_->size() < latest_values_.size())
                        throw std::out_of_range("copyInto destination is smaller than source");
                    std::copy(latest_values_.begin(), latest_values_.end(), results_->begin());
                    coroutine_yield(this, std::function(transform_)(*results_, this));
                }
                results_.reset();
            }
            element_.reset();
        }
        coroutine_end(this)
    }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:17-80
    void release_intercepted() override {
        auto self = std::move(self_ref_);
        flows_.clear();
        state_.reset();
        latest_values_.clear();
        results_.reset();
        element_.reset();
        received_.reset();
        array_factory_ = {};
        transform_ = {};
        ContinuationImpl::release_intercepted();
    }
private:
    void* _label = nullptr;
    CoroutineScope* scope_;
    std::vector<std::shared_ptr<Flow<std::any>>> flows_;
    std::shared_ptr<CombineState> state_;
    std::function<std::vector<std::any>*()> array_factory_;
    std::function<void*(const std::vector<std::any>&, Continuation<void*>*)> transform_;
    std::vector<std::any> latest_values_;
    std::size_t remaining_absent_values_ = 0;
    std::vector<std::uint8_t> last_received_epoch_;
    std::uint8_t current_epoch_ = 0;
    std::size_t index_ = 0;
    void* received_box_ = nullptr;
    std::unique_ptr<channels::ChannelResult<Update>> received_;
    std::optional<Update> element_;
    std::unique_ptr<std::vector<std::any>> results_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13,29-32
// NOTE(port): Only the typed forwarding closure requires header instantiation.
class AdapterContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
    AdapterContinuation(std::function<void*(Continuation<void*>*)> collect, Continuation<void*>* completion)
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
          collect_(std::move(collect)) {}
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
    void retain() { self_ref_ = shared_from_this(); }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        coroutine_yield(this, std::function(collect_)(this));
        coroutine_end(this)
    }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:29-32
    void release_intercepted() override {
        auto self = std::move(self_ref_);
        collect_ = {};
        ContinuationImpl::release_intercepted();
    }
private:
    void* _label = nullptr;
    std::function<void*(Continuation<void*>*)> collect_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:124-131
class ZipEmitContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:124-131
    ZipEmitContinuation(std::any value, std::shared_ptr<channels::ReceiveChannel<std::any>> second,
        std::shared_ptr<CompletableJob> collect_job,
        std::function<void*(std::any, std::any, Continuation<void*>*)> transform,
        std::function<void*(void*, Continuation<void*>*)> emit,
        std::function<void(void*)> delete_result, Continuation<void*>* completion)
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
          value_(std::move(value)), second_(std::move(second)), collect_job_(std::move(collect_job)),
          transform_(std::move(transform)), emit_(std::move(emit)), delete_result_(std::move(delete_result)) {}
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:124-131
    void retain() { self_ref_ = shared_from_this(); }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:124-131
    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        coroutine_yield_value(this, result, second_->receive_catching(this), received_box_);
        received_.reset(static_cast<channels::ChannelResult<std::any>*>(received_box_));
        received_box_ = nullptr;
        if (!received_->is_success()) {
            if (auto cause = received_->exception_or_null()) std::rethrow_exception(cause);
            throw AbortFlowException(collect_job_.get());
        }
        other_ = received_->get_or_throw();
        received_.reset();
        // NOTE(port): std::any inside tagged ChannelResult retains nullable payloads without a GC surrogate.
        coroutine_yield_value(this, result, std::function(transform_)(value_, other_, this), result_box_);
        coroutine_yield(this, emit_result());
        coroutine_end(this)
    }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:124-131
    void release_intercepted() override {
        auto self = std::move(self_ref_);
        if (result_box_) delete_result_(std::exchange(result_box_, nullptr));
        value_.reset();
        other_.reset();
        received_.reset();
        second_.reset();
        collect_job_.reset();
        transform_ = {};
        emit_ = {};
        delete_result_ = {};
        ContinuationImpl::release_intercepted();
    }
private:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:124-131
    void* emit_result() { return std::function(emit_)(std::exchange(result_box_, nullptr), this); }
    void* _label = nullptr;
    std::any value_, other_;
    std::shared_ptr<channels::ReceiveChannel<std::any>> second_;
    std::shared_ptr<CompletableJob> collect_job_;
    std::function<void*(std::any, std::any, Continuation<void*>*)> transform_;
    std::function<void*(void*, Continuation<void*>*)> emit_;
    std::function<void(void*)> delete_result_;
    void* received_box_ = nullptr;
    std::unique_ptr<channels::ChannelResult<std::any>> received_;
    void* result_box_ = nullptr;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:123-132
class ZipCollector final : public FlowCollector<std::any> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:123-132
    ZipCollector(std::shared_ptr<channels::ReceiveChannel<std::any>> second,
        std::shared_ptr<CompletableJob> collect_job, std::shared_ptr<CoroutineContext> scope_context,
        std::function<void*(std::any, std::any, Continuation<void*>*)> transform,
        std::function<void*(void*, Continuation<void*>*)> emit, std::function<void(void*)> delete_result)
        : second_(std::move(second)), collect_job_(std::move(collect_job)),
          scope_context_(std::move(scope_context)),
          count_(kotlinx::coroutines::internal::thread_context_elements(*scope_context_)),
          transform_(std::move(transform)), emit_(std::move(emit)), delete_result_(std::move(delete_result)) {}
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:123-132
    void* emit(std::any value, Continuation<void*>* completion) override {
        return with_context_undispatched<void*>(scope_context_, std::move(value), count_,
            [this](std::any value, Continuation<void*>* continuation) {
                auto frame = std::make_shared<ZipEmitContinuation>(std::move(value), second_, collect_job_,
                    transform_, emit_, delete_result_, continuation);
                frame->retain();
                return frame->start(Result<void*>::success(nullptr));
            }, completion);
    }
private:
    std::shared_ptr<channels::ReceiveChannel<std::any>> second_;
    std::shared_ptr<CompletableJob> collect_job_;
    std::shared_ptr<CoroutineContext> scope_context_;
    void* count_;
    std::function<void*(std::any, std::any, Continuation<void*>*)> transform_;
    std::function<void*(void*, Continuation<void*>*)> emit_;
    std::function<void(void*)> delete_result_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:84-139
class ZipContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:84-139
    ZipContinuation(CoroutineScope* scope, std::shared_ptr<Flow<std::any>> first,
        std::shared_ptr<Flow<std::any>> second,
        std::function<void*(std::any, std::any, Continuation<void*>*)> transform,
        std::function<void*(void*, Continuation<void*>*)> emit,
        std::function<void(void*)> delete_result, Continuation<void*>* completion)
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
          scope_(scope), first_(std::move(first)), source_second_(std::move(second)),
          transform_(std::move(transform)), emit_(std::move(emit)), delete_result_(std::move(delete_result)) {}
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:84-139
    void retain() { self_ref_ = shared_from_this(); }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:84-139
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            // The second producer uses a rendezvous channel, as required by the close invariant.
            second_ = channels::produce<std::any>(scope_, EmptyCoroutineContext::instance(), 0,
                channels::BufferOverflow::SUSPEND, CoroutineStart::DEFAULT,
                [source = source_second_](channels::ProducerScope<std::any>* receiver,
                                          std::shared_ptr<Continuation<void*>> continuation) {
                    auto collector = std::make_shared<SendingCollector<std::any>>(receiver);
                    return collect_combine_adapter([source, collector](Continuation<void*>* frame) {
                        return source->collect(collector.get(), frame);
                    }, continuation.get());
                });
            collect_job_ = make_job();
            std::dynamic_pointer_cast<channels::SendChannel<std::any>>(second_)->invoke_on_close(
                [job = collect_job_](std::exception_ptr) {
                    if (job->is_active()) job->cancel(std::make_exception_ptr(AbortFlowException(job.get())));
                });
            collector_ = std::make_shared<ZipCollector>(second_, collect_job_, get_context(),
                transform_, emit_, delete_result_);
            coroutine_yield(this, collect_first());
        } catch (AbortFlowException& exception) {
            try { exception.check_ownership(collect_job_.get()); }
            catch (...) {
                if (second_) second_->cancel(nullptr);
                throw;
            }
        } catch (...) {
            if (second_) second_->cancel(nullptr);
            throw;
        }
        second_->cancel(nullptr);
        coroutine_end(this)
    }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:84-139
    void release_intercepted() override {
        auto self = std::move(self_ref_);
        first_.reset();
        source_second_.reset();
        second_.reset();
        collect_job_.reset();
        collector_.reset();
        transform_ = {};
        emit_ = {};
        delete_result_ = {};
        ContinuationImpl::release_intercepted();
    }
private:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:84-139
    void* collect_first() {
        return with_context_undispatched<void*>(get_context()->operator+(collect_job_), Unit{},
            [this](Unit, Continuation<void*>* continuation) {
                return first_->collect(collector_.get(), continuation);
            }, this);
    }
    void* _label = nullptr;
    CoroutineScope* scope_;
    std::shared_ptr<Flow<std::any>> first_, source_second_;
    std::shared_ptr<channels::ReceiveChannel<std::any>> second_;
    std::shared_ptr<CompletableJob> collect_job_;
    std::shared_ptr<ZipCollector> collector_;
    std::function<void*(std::any, std::any, Continuation<void*>*)> transform_;
    std::function<void*(void*, Continuation<void*>*)> emit_;
    std::function<void(void*)> delete_result_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};
} // namespace

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:11-80
void* combine_internal_erased(
    std::vector<std::shared_ptr<Flow<std::any>>> flows,
    std::function<std::vector<std::any>*()> array_factory,
    std::function<void*(const std::vector<std::any>&, Continuation<void*>*)> transform,
    Continuation<void*>* completion) {
    return flow_scope([flows = std::move(flows), array_factory = std::move(array_factory),
                       transform = std::move(transform)](CoroutineScope* scope, Continuation<void*>* frame) {
        auto body = std::make_shared<CombineContinuation>(scope, flows, array_factory, transform, frame);
        body->retain();
        return body->start(Result<void*>::success(nullptr));
    }, completion);
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13,29-32
void* collect_combine_adapter(std::function<void*(Continuation<void*>*)> collect,
                             Continuation<void*>* completion) {
    auto frame = std::make_shared<AdapterContinuation>(std::move(collect), completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}
} // namespace kotlinx::coroutines::flow::internal

namespace kotlinx::coroutines::flow::internal {
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:82-139
void* zip_internal_erased(
    std::shared_ptr<Flow<std::any>> first, std::shared_ptr<Flow<std::any>> second,
    std::function<void*(std::any, std::any, Continuation<void*>*)> transform,
    std::function<void*(void*, Continuation<void*>*)> emit,
    std::function<void(void*)> delete_result, Continuation<void*>* completion) {
    return collect_in_scope([first = std::move(first), second = std::move(second),
        transform = std::move(transform), emit = std::move(emit), delete_result = std::move(delete_result)](
            CoroutineScope* scope, std::shared_ptr<Continuation<void*>> continuation) {
        auto frame = std::make_shared<ZipContinuation>(scope, first, second, transform, emit,
                                                       delete_result, continuation.get());
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    }, completion);
}
} // namespace kotlinx::coroutines::flow::internal
