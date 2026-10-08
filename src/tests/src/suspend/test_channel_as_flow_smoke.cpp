#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/CoroutineName.hpp"
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/flow/Channels.hpp"
#include "kotlinx/coroutines/flow/Collect.hpp"
#include "kotlinx/coroutines/flow/Emitters.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace {

int ownership_failure(int line) {
    std::cerr << "channel flow ownership check failed at " << line << "\n";
    return 1;
}

class QueueDispatcher : public kotlinx::coroutines::CoroutineDispatcher {
public:
    mutable std::deque<std::shared_ptr<kotlinx::coroutines::Runnable>> queue;

    void dispatch(const kotlin::coroutines::CoroutineContext&,
                  std::shared_ptr<kotlinx::coroutines::Runnable> block) const override {
        queue.push_back(std::move(block));
    }

    void drain() {
        while (!queue.empty()) {
            auto block = std::move(queue.front());
            queue.pop_front();
            block->run();
        }
    }
};

class NoopContinuation : public kotlin::coroutines::Continuation<void*> {
public:
    NoopContinuation() : ctx_(kotlin::coroutines::EmptyCoroutineContext::instance()) {}

    std::shared_ptr<kotlin::coroutines::CoroutineContext> get_context() const override { return ctx_; }
    void resume_with(kotlinx::coroutines::Result<void*> result) override {
        // No-op continuation - result indicates completion status
        // Could check result.is_success() to verify expected behavior
    }

private:
    std::shared_ptr<kotlin::coroutines::CoroutineContext> ctx_;
};

template <typename T>
class VectorCollector : public kotlinx::coroutines::flow::FlowCollector<T> {
public:
    explicit VectorCollector(std::vector<T>* out) : out_(out) {}

    void* emit(T value, kotlin::coroutines::Continuation<void*>* continuation) override {
        (void)continuation;
        out_->push_back(std::move(value));
        return nullptr;
    }

private:
    std::vector<T>* out_;
};

class RecordingContinuation : public kotlin::coroutines::Continuation<void*> {
public:
    bool completed = false;
    std::exception_ptr failure;
    std::shared_ptr<kotlin::coroutines::CoroutineContext> ctx_ = kotlin::coroutines::EmptyCoroutineContext::instance();

    std::shared_ptr<kotlin::coroutines::CoroutineContext> get_context() const override { return ctx_; }
    void resume_with(kotlinx::coroutines::Result<void*> result) override {
        try { (void)result.get_or_throw(); }
        catch (...) { failure = std::current_exception(); }
        completed = true;
    }
};

class MockSuspendingIterator : public kotlinx::coroutines::channels::ChannelIterator<int> {
public:
    int step = 0;
    kotlin::coroutines::Continuation<void*>* saved_cont = nullptr;

    void* has_next(kotlin::coroutines::Continuation<void*>* cont) override {
        if (step == 0) {
            step = 1;
            saved_cont = cont;
            return kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
        }
        return new bool(false);
    }

    int next() override {
        return 42;
    }
};

class MockSuspendingChannel : public kotlinx::coroutines::channels::ReceiveChannel<int> {
public:
    MockSuspendingIterator* it_ptr = nullptr;
    bool cancelled = false;
    int cancellations = 0;
    std::exception_ptr cancellation_cause;
    std::exception_ptr cancellation_failure;
    std::function<std::unique_ptr<kotlinx::coroutines::channels::ChannelIterator<int>>()> iterator_factory;

    std::unique_ptr<kotlinx::coroutines::channels::ChannelIterator<int>> iterator() override {
        if (iterator_factory) return iterator_factory();
        auto it = std::make_unique<MockSuspendingIterator>();
        it_ptr = it.get();
        return it;
    }

    void cancel(std::exception_ptr cause) override {
        ++cancellations;
        cancellation_cause = cause;
        cancelled = true;
        if (cancellation_failure) std::rethrow_exception(cancellation_failure);
    }

    bool is_closed_for_receive() const override { return cancelled; }
    bool is_empty() const override { return false; }
    kotlinx::coroutines::channels::ChannelResult<int> try_receive() override {
        return kotlinx::coroutines::channels::ChannelResult<int>::failure();
    }
    void* receive(kotlin::coroutines::Continuation<void*>* cont) override { (void)cont; return nullptr; }
    void* receive_catching(kotlin::coroutines::Continuation<void*>* cont) override { (void)cont; return nullptr; }
    kotlinx::coroutines::selects::SelectClause1<int>& on_receive() override { throw std::logic_error("not implemented"); }
    kotlinx::coroutines::selects::SelectClause1<kotlinx::coroutines::channels::ChannelResult<int>>& on_receive_catching() override {
        throw std::logic_error("not implemented");
    }
};

class SuspendingCollector : public kotlinx::coroutines::flow::FlowCollector<int> {
public:
    std::vector<int>* out_;
    kotlin::coroutines::Continuation<void*>* saved_emit_cont = nullptr;

    explicit SuspendingCollector(std::vector<int>* out) : out_(out) {}

    void* emit(int value, kotlin::coroutines::Continuation<void*>* continuation) override {
        out_->push_back(value);
        saved_emit_cont = continuation;
        return kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
    }
};

} // namespace

int main() {
    using kotlinx::coroutines::channels::Channel;
    using kotlinx::coroutines::channels::create_channel;
    using kotlinx::coroutines::flow::consume_as_flow;
    using kotlinx::coroutines::flow::receive_as_flow;

    // ChannelFlow.kt:155-170: equal but distinct interceptors use the
    // undispatched path. An added name forces the full-context check to differ.
    for (bool equal : {true, false}) {
        class EqualDispatcher final : public QueueDispatcher {
        public:
            explicit EqualDispatcher(int value) : value_(value) {}
            bool equals(const kotlin::coroutines::CoroutineContext* other) const override {
                auto dispatcher = dynamic_cast<const EqualDispatcher*>(other);
                return dispatcher && dispatcher->value_ == value_;
            }
        private:
            int value_;
        };
        auto caller_dispatcher = std::make_shared<EqualDispatcher>(1);
        auto upstream_dispatcher = std::make_shared<EqualDispatcher>(equal ? 1 : 2);
        auto upstream_context = upstream_dispatcher->operator+(
            std::make_shared<kotlinx::coroutines::CoroutineName>("upstream"));
        int collections = 0;
        bool undispatched_collector = false;
        bool upstream_context_seen = false;
        auto source = kotlinx::coroutines::flow::unsafe_flow<int>(
            [&](kotlinx::coroutines::flow::FlowCollector<int>* collector,
                kotlin::coroutines::Continuation<void*>* continuation) -> void* {
                ++collections;
                undispatched_collector = dynamic_cast<kotlinx::coroutines::flow::internal::
                    UndispatchedContextCollector<int>*>(collector) != nullptr;
                upstream_context_seen = continuation->get_context()->get(
                    kotlin::coroutines::ContinuationInterceptor::type_key).get() == upstream_dispatcher.get();
                return nullptr;
            });
        class ObservedOperator final : public kotlinx::coroutines::flow::internal::ChannelFlowOperatorImpl<int> {
        public:
            using ChannelFlowOperatorImpl<int>::ChannelFlowOperatorImpl;
            int producers = 0;
            std::shared_ptr<kotlinx::coroutines::channels::ReceiveChannel<int>> produce_impl(
                kotlinx::coroutines::CoroutineScope* scope) override {
                ++producers;
                return ChannelFlowOperatorImpl<int>::produce_impl(scope);
            }
        };
        auto operated = std::make_shared<ObservedOperator>(source, upstream_context);
        RecordingContinuation completion;
        completion.ctx_ = caller_dispatcher;
        std::vector<int> values;
        VectorCollector<int> collector(&values);
        auto result = operated->collect(&collector, &completion);
        upstream_dispatcher->drain();
        caller_dispatcher->drain();
        if (collections != 1 || !upstream_context_seen || undispatched_collector != equal ||
            operated->producers != (equal ? 0 : 1) || !values.empty())
            return ownership_failure(__LINE__);
        if (equal) {
            if (result != nullptr || completion.completed) return ownership_failure(__LINE__);
        } else if (result != kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED() ||
                   !completion.completed || completion.failure) return ownership_failure(__LINE__);
    }

    // Build a channel with some buffered values.
    auto ch = create_channel<int>(Channel<int>::BUFFERED);
    if (!ch->try_send(1).is_success()) return 1;
    if (!ch->try_send(2).is_success()) return 1;
    ch->close(nullptr);

    // Channel<E> inherits from ReceiveChannel<E>
    std::shared_ptr<kotlinx::coroutines::channels::ReceiveChannel<int>> recv = ch;
    if (!recv) return 1;

    // receive_as_flow: multiple collections are allowed (channel fan-out).
    {
        auto f = receive_as_flow<int>(recv);
        std::vector<int> out;
        VectorCollector<int> collector(&out);
        NoopContinuation cont;
        void* r = f->collect(&collector, &cont);
        if (r != nullptr) return 1;
        std::sort(out.begin(), out.end());
        if (out != std::vector<int>{1, 2}) return 1;
    }

    // consume_as_flow: only a single collection is allowed.
    {
        auto f = consume_as_flow<int>(recv);
        std::vector<int> out;
        VectorCollector<int> collector(&out);
        NoopContinuation cont;
        (void)f->collect(&collector, &cont);

        bool threw = false;
        try {
            std::vector<int> out2;
            VectorCollector<int> collector2(&out2);
            (void)f->collect(&collector2, &cont);
        } catch (const std::logic_error&) {
            threw = true;
        }
        if (!threw) return 1;
    }

    // emit_all: normal completion and channel consumption
    {
        auto ch1 = create_channel<int>(Channel<int>::BUFFERED);
        ch1->try_send(10);
        ch1->try_send(20);
        ch1->close(nullptr);

        std::vector<int> out;
        VectorCollector<int> collector(&out);
        NoopContinuation cont;
        void* r = kotlinx::coroutines::flow::emit_all(&collector, ch1.get(), &cont);
        if (r != nullptr) return 1;
        if (out != std::vector<int>{10, 20}) return 1;
        if (!ch1->is_closed_for_receive()) return 1;
    }

    // emit_all: exception during emit cancels channel and rethrows
    {
        auto ch2 = create_channel<int>(Channel<int>::BUFFERED);
        ch2->try_send(100);
        ch2->try_send(200);

        class ThrowingCollector : public kotlinx::coroutines::flow::FlowCollector<int> {
        public:
            void* emit(int value, kotlin::coroutines::Continuation<void*>* continuation) override {
                (void)value;
                (void)continuation;
                throw std::runtime_error("emission failure");
            }
        };

        ThrowingCollector collector;
        NoopContinuation cont;
        bool caught = false;
        try {
            kotlinx::coroutines::flow::emit_all(&collector, ch2.get(), &cont);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        if (!caught) return 1;
        if (!ch2->is_closed_for_receive()) return 1;
    }

    // Emitters.kt:193-205 and Channels.kt:28-41: the failure collector must
    // reject collection before reading even an empty, open channel. The check
    // precedes the consume/finally region, so the source remains usable.
    {
        auto channel = create_channel<int>(Channel<int>::BUFFERED);
        auto failure = std::make_exception_ptr(std::runtime_error("original completion failure"));
        kotlinx::coroutines::flow::ThrowingCollector<int> collector(failure);
        RecordingContinuation completion;
        bool caught_original = false;
        try {
            (void)kotlinx::coroutines::flow::emit_all(&collector, channel.get(), &completion);
        } catch (...) {
            caught_original = std::current_exception() == failure;
        }
        if (!caught_original || completion.completed) return 1;
        if (channel->is_closed_for_receive()) return 1;
        if (!channel->try_send(73).is_success()) return 1;
        auto received = channel->try_receive();
        if (!received.is_success() || received.get_or_throw() != 73) return 1;
        channel->close(nullptr);

        auto owned_channel = create_channel<int>(Channel<int>::BUFFERED);
        std::shared_ptr<kotlinx::coroutines::flow::FlowCollector<int>> owned_collector =
            std::make_shared<kotlinx::coroutines::flow::ThrowingCollector<int>>(failure);
        caught_original = false;
        try {
            (void)kotlinx::coroutines::flow::emit_all_impl(
                owned_collector, owned_channel.get(), true, &completion);
        } catch (...) {
            caught_original = std::current_exception() == failure;
        }
        if (!caught_original || owned_channel->is_closed_for_receive()) return 1;
        owned_channel->close(nullptr);

        bool source_collected = false;
        auto source = kotlinx::coroutines::flow::flow<int>(
            [&source_collected](kotlinx::coroutines::flow::FlowCollector<int>*,
                               kotlin::coroutines::Continuation<void*>*) -> void* {
                source_collected = true;
                return nullptr;
            });
        caught_original = false;
        try {
            (void)kotlinx::coroutines::flow::emit_all(&collector, source, &completion);
        } catch (...) {
            caught_original = std::current_exception() == failure;
        }
        if (!caught_original || source_collected) return 1;
        caught_original = false;
        try {
            (void)collector.emit(91, &completion);
        } catch (...) {
            caught_original = std::current_exception() == failure;
        }
        if (!caught_original) return 1;
    }

    // emit_all: async suspension and resumption at has_next
    {
        MockSuspendingChannel mock_ch;
        std::vector<int> out;
        VectorCollector<int> collector(&out);
        RecordingContinuation completion;

        void* r = kotlinx::coroutines::flow::emit_all(&collector, &mock_ch, &completion);
        if (r != kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED()) return 1;
        if (completion.completed) return 1;
        if (mock_ch.it_ptr == nullptr || mock_ch.it_ptr->saved_cont == nullptr) return 1;

        // Resume with true -> should emit 42 and complete
        mock_ch.it_ptr->saved_cont->resume_with(
            kotlinx::coroutines::Result<void*>::success(new bool(true)));

        if (!completion.completed) return 1;
        if (out != std::vector<int>{42}) return 1;
        if (!mock_ch.cancelled) return 1;
    }

    // emit_all: async suspension and resumption at collector emit
    {
        auto ch3 = create_channel<int>(Channel<int>::BUFFERED);
        ch3->try_send(999);
        ch3->close(nullptr);

        std::vector<int> out;
        SuspendingCollector collector(&out);
        RecordingContinuation completion;

        void* r = kotlinx::coroutines::flow::emit_all(&collector, ch3.get(), &completion);
        if (r != kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED()) return 1;
        if (completion.completed) return 1;
        if (collector.saved_emit_cont == nullptr) return 1;
        if (out != std::vector<int>{999}) return 1;

        // Resume emit continuation
        collector.saved_emit_cont->resume_with(kotlinx::coroutines::Result<void*>::success(nullptr));

        if (!completion.completed) return 1;
    }

    // Channels.kt:127-134,28-41: collection retains the actual channel across
    // repeated emission suspension, including a failed resumed emission.
    for (bool consume : {false, true}) for (bool fail : {false, true}) {
        auto channel = create_channel<int>(Channel<int>::BUFFERED);
        channel->try_send(8);
        channel->try_send(9);
        channel->close(nullptr);
        std::shared_ptr<kotlinx::coroutines::channels::ReceiveChannel<int>> receiver = channel;
        std::weak_ptr<kotlinx::coroutines::channels::ReceiveChannel<int>> lifetime = receiver;
        auto flow = consume ? consume_as_flow<int>(receiver) : receive_as_flow<int>(receiver);
        std::vector<int> values;
        SuspendingCollector collector(&values);
        RecordingContinuation completion;
        if (flow->collect(&collector, &completion) !=
            kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED()) return 1;
        flow.reset();
        receiver.reset();
        channel.reset();
        if (lifetime.expired() || values != std::vector<int>{8}) return 1;
        auto failure = std::make_exception_ptr(std::runtime_error("resumed emission failed"));
        collector.saved_emit_cont->resume_with(fail ?
            kotlinx::coroutines::Result<void*>::failure(failure) :
            kotlinx::coroutines::Result<void*>::success(nullptr));
        if (!fail) {
            if (lifetime.expired() || completion.completed || values != std::vector<int>({8, 9})) return 1;
            collector.saved_emit_cont->resume_with(kotlinx::coroutines::Result<void*>::success(nullptr));
        }
        if (!completion.completed || completion.failure != (fail ? failure : nullptr)) return 1;
        if (!lifetime.expired()) return 1;
    }

    // Channels.kt:28-41: creation, hasNext and next failures enter the same
    // catch/finally boundary; a thrown cleanup exception supersedes the body.
    for (int phase : {0, 1, 2}) for (bool consume : {false, true})
        for (bool cleanup_fails : {false, true}) {
        class FailingIterator final : public kotlinx::coroutines::channels::ChannelIterator<int> {
        public:
            FailingIterator(int phase, std::exception_ptr failure) : phase_(phase), failure_(failure) {}
            void* has_next(kotlin::coroutines::Continuation<void*>*) override {
                if (phase_ == 1) std::rethrow_exception(failure_);
                return new bool(true);
            }
            int next() override { std::rethrow_exception(failure_); }
        private:
            int phase_;
            std::exception_ptr failure_;
        };
        auto failure = std::make_exception_ptr(std::runtime_error("iterator failure"));
        auto cleanup_failure = std::make_exception_ptr(std::runtime_error("cancel failure"));
        MockSuspendingChannel channel;
        channel.iterator_factory = [phase, failure]() -> std::unique_ptr<kotlinx::coroutines::channels::ChannelIterator<int>> {
            if (phase == 0) std::rethrow_exception(failure);
            return std::make_unique<FailingIterator>(phase, failure);
        };
        if (cleanup_fails) channel.cancellation_failure = cleanup_failure;
        std::vector<int> values;
        VectorCollector<int> collector(&values);
        RecordingContinuation completion;
        try {
            kotlinx::coroutines::flow::emit_all_impl(&collector, &channel, consume, &completion);
            return ownership_failure(__LINE__);
        } catch (...) {
            auto expected = consume && cleanup_fails ? cleanup_failure : failure;
            if (std::current_exception() != expected) return ownership_failure(__LINE__);
        }
        if (completion.completed || !values.empty() || channel.cancellations != (consume ? 1 : 0))
            return ownership_failure(__LINE__);
        if (consume) {
            try {
                std::rethrow_exception(channel.cancellation_cause);
            } catch (const kotlinx::coroutines::CancellationException& cancellation) {
                if (cancellation.get_cause() != failure) return ownership_failure(__LINE__);
            }
        }
    }

    // Owned collector/channel bindings survive both emits and release their
    // owners on termination even while the completed continuation is retained.
    for (bool fail : {false, true}) {
        auto channel = create_channel<int>(Channel<int>::BUFFERED);
        channel->try_send(61);
        channel->try_send(62);
        channel->close(nullptr);
        std::shared_ptr<kotlinx::coroutines::channels::ReceiveChannel<int>> receiver = channel;
        std::weak_ptr<kotlinx::coroutines::channels::ReceiveChannel<int>> channel_lifetime = receiver;
        std::vector<int> values;
        auto collector = std::make_shared<SuspendingCollector>(&values);
        std::weak_ptr<SuspendingCollector> collector_lifetime = collector;
        auto* collector_ptr = collector.get();
        std::shared_ptr<kotlinx::coroutines::flow::FlowCollector<int>> collector_owner = collector;
        auto completion = std::make_shared<RecordingContinuation>();
        if (kotlinx::coroutines::flow::emit_all_impl<int>(collector_owner, receiver, true, completion) !=
            kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED()) return ownership_failure(__LINE__);
        auto frame = dynamic_cast<kotlin::coroutines::native::internal::BaseContinuationImpl*>(collector_ptr->saved_emit_cont)->shared_from_this();
        collector.reset();
        collector_owner.reset();
        receiver.reset();
        channel.reset();
        if (channel_lifetime.expired() || collector_lifetime.expired() || values != std::vector<int>{61})
            return ownership_failure(__LINE__);
        frame->resume_with(kotlinx::coroutines::Result<void*>::success(nullptr));
        if (channel_lifetime.expired() || collector_lifetime.expired() || values != std::vector<int>({61, 62}))
            return ownership_failure(__LINE__);
        class HoldingFailure final : public std::runtime_error {
        public:
            explicit HoldingFailure(std::shared_ptr<int> resource)
                : std::runtime_error("owned collector failure"), resource_(std::move(resource)) {}
        private:
            std::shared_ptr<int> resource_;
        };
        auto resource = std::make_shared<int>(63);
        std::weak_ptr<int> resource_lifetime = resource;
        auto failure = std::make_exception_ptr(HoldingFailure(resource));
        frame->resume_with(fail ? kotlinx::coroutines::Result<void*>::failure(failure) :
            kotlinx::coroutines::Result<void*>::success(nullptr));
        if (!completion->completed || completion->failure != (fail ? failure : nullptr) ||
            !channel_lifetime.expired() || !collector_lifetime.expired()) return ownership_failure(__LINE__);
        resource.reset();
        failure = nullptr;
        completion->failure = nullptr;
        if (!resource_lifetime.expired()) return ownership_failure(__LINE__);
    }

    // Emitters.kt:70-81: the start action must finish before upstream starts,
    // including when both the action and upstream collection suspend.
    {
        kotlin::coroutines::Continuation<void*>* action_continuation = nullptr;
        kotlin::coroutines::Continuation<void*>* upstream_continuation = nullptr;
        int upstream_starts = 0;
        auto source = kotlinx::coroutines::flow::unsafe_flow<int>(
            [&](kotlinx::coroutines::flow::FlowCollector<int>*, kotlin::coroutines::Continuation<void*>* continuation) -> void* {
                ++upstream_starts;
                upstream_continuation = continuation;
                return kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
            });
        auto started = kotlinx::coroutines::flow::on_start<int>(source,
            [&](kotlinx::coroutines::flow::FlowCollector<int>* collector, kotlin::coroutines::Continuation<void*>* continuation) -> void* {
                if (!dynamic_cast<kotlinx::coroutines::flow::internal::SafeCollector<int>*>(collector))
                    throw std::logic_error("start action lacks SafeCollector");
                action_continuation = continuation;
                return kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
            });
        std::vector<int> out;
        VectorCollector<int> collector(&out);
        RecordingContinuation completion;
        if (started->collect(&collector, &completion) != kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED()) return 1;
        if (upstream_starts || completion.completed || !action_continuation) return 1;
        started.reset();
        source.reset();
        action_continuation->resume_with(kotlinx::coroutines::Result<void*>::success(nullptr));
        if (upstream_starts != 1 || completion.completed || !upstream_continuation) return 1;
        upstream_continuation->resume_with(kotlinx::coroutines::Result<void*>::success(nullptr));
        if (!completion.completed || upstream_starts != 1) return 1;
    }

    // Failure when the start action resumes must reach the caller without
    // starting upstream collection.
    {
        kotlin::coroutines::Continuation<void*>* action_continuation = nullptr;
        bool upstream_started = false;
        auto source = kotlinx::coroutines::flow::unsafe_flow<int>(
            [&](kotlinx::coroutines::flow::FlowCollector<int>*,
                kotlin::coroutines::Continuation<void*>*) -> void* {
                upstream_started = true;
                return nullptr;
            });
        auto started = kotlinx::coroutines::flow::on_start<int>(source,
            [&](kotlinx::coroutines::flow::FlowCollector<int>*,
                kotlin::coroutines::Continuation<void*>* continuation) -> void* {
                action_continuation = continuation;
                return kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
            });
        std::vector<int> out;
        VectorCollector<int> collector(&out);
        RecordingContinuation completion;
        auto failure = std::make_exception_ptr(std::runtime_error("start action failure"));
        if (started->collect(&collector, &completion) !=
            kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED()) return 1;
        action_continuation->resume_with(kotlinx::coroutines::Result<void*>::failure(failure));
        if (upstream_started || !completion.completed || completion.failure != failure) return 1;
    }

    // ChannelFlow.kt:54-56,114-120: the producer's collectToFun retains its
    // actual receiver before dispatch and throughout a suspended collectTo.
    for (bool channel_backed : {false, true}) for (bool fail : {false, true}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        auto scope = kotlinx::coroutines::create_coroutine_scope(dispatcher);
        kotlin::coroutines::Continuation<void*>* paused = nullptr;
        auto resource = std::make_shared<int>(71);
        auto identity = resource.get();
        std::weak_ptr<int> resource_lifetime = resource;
        std::shared_ptr<kotlinx::coroutines::flow::Flow<int>> source;
        if (channel_backed) source = kotlinx::coroutines::flow::channel_flow<int>(
            [resource, identity, &paused](
                kotlinx::coroutines::channels::ProducerScope<int>*,
                std::shared_ptr<kotlin::coroutines::Continuation<void*>> continuation) -> void* {
                if (resource.get() != identity || *resource != 71)
                    throw std::logic_error("producer resource identity changed");
                paused = continuation.get();
                return kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
            });
        else source = kotlinx::coroutines::flow::unsafe_flow<int>(
            [resource, identity, &paused](
                kotlinx::coroutines::flow::FlowCollector<int>*,
                kotlin::coroutines::Continuation<void*>* continuation) -> void* {
                if (resource.get() != identity || *resource != 71)
                    throw std::logic_error("producer resource identity changed");
                paused = continuation;
                return kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
            });
        std::weak_ptr<kotlinx::coroutines::flow::Flow<int>> lifetime = source;
        auto produced = kotlinx::coroutines::flow::produce_in<int>(source, scope.get());
        source.reset();
        resource.reset();
        if (lifetime.expired() || resource_lifetime.expired() || paused) return ownership_failure(__LINE__);
        dispatcher->drain();
        if (!paused || lifetime.expired() || resource_lifetime.expired()) return ownership_failure(__LINE__);
        // A retained completed frame must release its actual source/captures.
        auto retained_frame = dynamic_cast<kotlin::coroutines::native::internal::BaseContinuationImpl*>(paused)->shared_from_this();
        auto failure = std::make_exception_ptr(std::runtime_error("producer resumed failure"));
        paused->resume_with(fail ? kotlinx::coroutines::Result<void*>::failure(failure) :
            kotlinx::coroutines::Result<void*>::success(nullptr));
        paused = nullptr;
        dispatcher->drain();
        if (!lifetime.expired() || !resource_lifetime.expired()) return ownership_failure(__LINE__);
        auto closed = produced->try_receive();
        if (!closed.is_closed() || closed.exception_or_null() != (fail ? failure : nullptr)) return ownership_failure(__LINE__);
    }

    // ChannelFlow.kt:117-120, Channels.kt:127-134: buffered collection keeps
    // the fused flow alive across repeated downstream suspension and failure.
    for (bool fail : {false, true}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        auto channel = create_channel<int>(Channel<int>::BUFFERED);
        channel->try_send(31);
        channel->try_send(32);
        channel->close(nullptr);
        auto source = consume_as_flow<int>(channel);
        auto channel_flow = std::dynamic_pointer_cast<
            kotlinx::coroutines::flow::internal::ChannelFlow<int>>(source);
        std::shared_ptr<kotlinx::coroutines::flow::Flow<int>> buffered(
            channel_flow->fuse(kotlin::coroutines::EmptyCoroutineContext::instance(), 2,
                              kotlinx::coroutines::channels::BufferOverflow::SUSPEND));
        std::weak_ptr<kotlinx::coroutines::flow::Flow<int>> lifetime = buffered;
        std::vector<int> values;
        SuspendingCollector collector(&values);
        RecordingContinuation completion;
        completion.ctx_ = dispatcher;
        if (buffered->collect(&collector, &completion) !=
            kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED()) return ownership_failure(__LINE__);
        buffered.reset();
        source.reset();
        channel_flow.reset();
        channel.reset();
        if (lifetime.expired()) return ownership_failure(__LINE__);
        dispatcher->drain();
        if (values != std::vector<int>{31} || !collector.saved_emit_cont) return ownership_failure(__LINE__);
        auto failure = std::make_exception_ptr(std::runtime_error("buffered emission failure"));
        collector.saved_emit_cont->resume_with(fail ?
            kotlinx::coroutines::Result<void*>::failure(failure) :
            kotlinx::coroutines::Result<void*>::success(nullptr));
        dispatcher->drain();
        if (!fail) {
            if (lifetime.expired() || completion.completed || values != std::vector<int>({31, 32})) return ownership_failure(__LINE__);
            collector.saved_emit_cont->resume_with(kotlinx::coroutines::Result<void*>::success(nullptr));
            dispatcher->drain();
        }
        if (!completion.completed || completion.failure != (fail ? failure : nullptr)) return ownership_failure(__LINE__);
        if (!lifetime.expired()) return ownership_failure(__LINE__);
    }

    // ChannelFlow.kt:54-56,114-120 and Channels.kt:28-41: cancelling the
    // produced channel resumes the actual waiting iterator and releases the flow.
    {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        auto scope = kotlinx::coroutines::create_coroutine_scope(dispatcher);
        auto channel = create_channel<int>(Channel<int>::BUFFERED);
        auto source = consume_as_flow<int>(channel);
        auto channel_flow = std::dynamic_pointer_cast<
            kotlinx::coroutines::flow::internal::ChannelFlow<int>>(source);
        std::shared_ptr<kotlinx::coroutines::flow::Flow<int>> buffered(
            channel_flow->fuse(kotlin::coroutines::EmptyCoroutineContext::instance(), 2,
                              kotlinx::coroutines::channels::BufferOverflow::SUSPEND));
        std::weak_ptr<kotlinx::coroutines::flow::Flow<int>> lifetime = buffered;
        std::weak_ptr<kotlinx::coroutines::channels::Channel<int>> channel_lifetime = channel;
        auto produced = kotlinx::coroutines::flow::produce_in<int>(buffered, scope.get());
        buffered.reset();
        source.reset();
        channel_flow.reset();
        channel.reset();
        dispatcher->drain();
        if (lifetime.expired() || channel_lifetime.expired()) return ownership_failure(__LINE__);
        produced->cancel(std::make_exception_ptr(kotlinx::coroutines::CancellationException("producer cancelled")));
        dispatcher->drain();
        if (!lifetime.expired() || !channel_lifetime.expired()) {
            std::cerr << "cancelled flow owners=" << lifetime.use_count()
                      << ", channel owners=" << channel_lifetime.use_count() << "\n";
            return ownership_failure(__LINE__);
        }
    }

    // Flow.kt:223-230: SafeCollector collection retains its receiver and block
    // captures until the suspended collectSafely returns or throws.
    for (int outcome : {0, 1, 2}) {
        kotlin::coroutines::Continuation<void*>* paused = nullptr;
        auto resource = std::make_shared<int>(81);
        std::weak_ptr<int> resource_lifetime = resource;
        auto source = kotlinx::coroutines::flow::flow<int>(
            [resource, &paused](kotlinx::coroutines::flow::FlowCollector<int>*,
                                kotlin::coroutines::Continuation<void*>* continuation) -> void* {
                if (*resource != 81) throw std::logic_error("flow resource changed");
                paused = continuation;
                return kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED();
            });
        std::weak_ptr<kotlinx::coroutines::flow::Flow<int>> lifetime = source;
        std::vector<int> values;
        VectorCollector<int> collector(&values);
        RecordingContinuation completion;
        if (source->collect(&collector, &completion) !=
            kotlin::coroutines::intrinsics::get_COROUTINE_SUSPENDED()) return ownership_failure(__LINE__);
        source.reset();
        resource.reset();
        if (lifetime.expired() || resource_lifetime.expired() || !paused) return ownership_failure(__LINE__);
        // Keep the terminated continuation alive independently: its spills must
        // stop owning the flow and captures when collection finishes.
        auto retained_frame = dynamic_cast<kotlin::coroutines::native::internal::BaseContinuationImpl*>(paused)->shared_from_this();
        auto failure = outcome == 2 ?
            std::make_exception_ptr(kotlinx::coroutines::CancellationException("safe flow cancelled")) :
            std::make_exception_ptr(std::runtime_error("safe flow resumed failure"));
        paused->resume_with(outcome != 0 ? kotlinx::coroutines::Result<void*>::failure(failure) :
            kotlinx::coroutines::Result<void*>::success(nullptr));
        paused = nullptr;
        if (!completion.completed || completion.failure != (outcome != 0 ? failure : nullptr)) return ownership_failure(__LINE__);
        if (!lifetime.expired() || !resource_lifetime.expired()) return ownership_failure(__LINE__);
    }

    // A stack AbstractFlow remains borrowed on immediate success and failure.
    // The entry returns/throws directly, without resuming its completion.
    {
        class StackFlow final : public kotlinx::coroutines::flow::AbstractFlow<int> {
        public:
            explicit StackFlow(int* destructions) : destructions_(destructions) {}
            ~StackFlow() override { ++*destructions_; }
            std::exception_ptr failure;
            int collections = 0;
            void* collect_safely(kotlinx::coroutines::flow::FlowCollector<int>* collector,
                                kotlin::coroutines::Continuation<void*>* completion) override {
                ++collections;
                collector->emit(91, completion);
                if (failure) std::rethrow_exception(failure);
                return nullptr;
            }
        private:
            int* destructions_;
        };
        int destructions = 0;
        {
            StackFlow source(&destructions);
            std::vector<int> values;
            VectorCollector<int> collector(&values);
            RecordingContinuation completion;
            if (source.collect(&collector, &completion) != nullptr || completion.completed || destructions)
                return ownership_failure(__LINE__);
            source.failure = std::make_exception_ptr(std::runtime_error("immediate collect failure"));
            try {
                source.collect(&collector, &completion);
                return ownership_failure(__LINE__);
            } catch (...) {
                if (std::current_exception() != source.failure) return ownership_failure(__LINE__);
            }
            if (values != std::vector<int>({91, 91}) || source.collections != 2 ||
                completion.completed || destructions) return ownership_failure(__LINE__);
        }
        if (destructions != 1) return ownership_failure(__LINE__);
    }

    return 0;
}
