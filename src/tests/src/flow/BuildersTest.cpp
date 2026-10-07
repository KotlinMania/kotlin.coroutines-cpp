/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/BuildersTest.kt
 */
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Collect.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/CoroutineName.hpp"
#include "kotlinx/coroutines/flow/internal/Merge.hpp"
#include <array>
#include <climits>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::flow;

namespace {

void require(bool condition, int line = __builtin_LINE()) {
    if (!condition) throw std::runtime_error("flow builder assertion failed at line " + std::to_string(line));
}

struct Completion : Continuation<void*> {
    int resumes = 0;
    Result<void*> result = Result<void*>::success(nullptr);
    std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance();
    std::shared_ptr<CoroutineContext> get_context() const override { return context; }
    void resume_with(Result<void*> outcome) override { ++resumes; result = std::move(outcome); }
};

template <typename T>
struct Collector : FlowCollector<T> {
    std::vector<T> values;
    bool pause = false;
    Continuation<void*>* pending = nullptr;
    void* emit(T value, Continuation<void*>* continuation) override {
        values.push_back(std::move(value));
        if (!pause) return nullptr;
        pending = continuation;
        return intrinsics::get_COROUTINE_SUSPENDED();
    }
    void resume(Result<void*> result = Result<void*>::success(nullptr)) {
        auto continuation = std::exchange(pending, nullptr);
        require(continuation != nullptr);
        continuation->resume_with(std::move(result));
    }
};

template <typename T>
std::vector<T> collect_now(std::shared_ptr<Flow<T>> flow) {
    Collector<T> collector;
    Completion completion;
    require(flow->collect(&collector, &completion) == nullptr);
    require(completion.resumes == 0);
    return collector.values;
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/BuildersTest.kt:10-13
void test_suspend_lambda_as_flow() {
    auto lambda = std::function<void*(Continuation<void*>*)>([](Continuation<void*>*) -> void* { return new int(42); });
    require(collect_now(as_flow<int>(std::move(lambda))) == std::vector<int>{42});
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/BuildersTest.kt:16-22
void test_range_as_flow() {
    const std::vector<int> ints{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    const std::vector<long> longs{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    require(collect_now(as_flow_range(0, 9)) == ints);
    require(collect_now(as_flow_range(0, -1)).empty());
    require(collect_now(as_flow_range(0L, 9L)) == longs);
    require(collect_now(as_flow_range(0L, -1L)).empty());
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/BuildersTest.kt:25-31
void test_array_as_flow() {
    std::array<int, 10> ints{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::array<long, 10> longs{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::array<int, 0> empty_ints;
    std::array<long, 0> empty_longs;
    require(collect_now(as_flow(ints)) == std::vector<int>(ints.begin(), ints.end()));
    require(collect_now(as_flow(empty_ints)).empty());
    require(collect_now(as_flow(longs)) == std::vector<long>(longs.begin(), longs.end()));
    require(collect_now(as_flow(empty_longs)).empty());
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/BuildersTest.kt:34-38
void test_sequence() {
    const std::vector<int> expected{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto iterator_flow = as_flow(expected.begin(), expected.end());
    require(collect_now(iterator_flow) == expected);
    require(collect_now(as_flow(expected)) == expected);
    // Iterator conversion consumes its cursor, unlike the repeatable iterable conversion.
    require(collect_now(iterator_flow).empty());
}

void test_range_and_array_suspension() {
    Completion completion;
    Collector<int> collector;
    collector.pause = true;
    auto flow = as_flow_range(INT_MAX - 1, INT_MAX);
    require(intrinsics::is_coroutine_suspended(flow->collect(&collector, &completion)));
    require(collector.values == std::vector<int>{INT_MAX - 1});
    collector.resume();
    require(completion.resumes == 0);
    require(collector.values == std::vector<int>({INT_MAX - 1, INT_MAX}));
    collector.resume();
    require(completion.resumes == 1 && completion.result.is_success());

    std::array<int, 2> array{1, 2};
    auto array_flow = as_flow(array);
    require(collect_now(array_flow) == std::vector<int>({1, 2}));
    array[0] = 3;
    require(collect_now(array_flow) == std::vector<int>({3, 2}));
}

void test_two_suspensions_and_resumed_failure() {
    for (bool fail : {false, true}) {
        Continuation<void*>* pending = nullptr;
        auto flow = as_flow<int>(std::function<void*(Continuation<void*>*)>(
            [&](Continuation<void*>* continuation) -> void* {
                pending = continuation;
                return intrinsics::get_COROUTINE_SUSPENDED();
            }));
        Completion completion;
        Collector<int> collector;
        collector.pause = true;
        require(intrinsics::is_coroutine_suspended(flow->collect(&collector, &completion)));
        require(collector.values.empty() && completion.resumes == 0);
        auto failure = std::make_exception_ptr(std::runtime_error("suspended producer failed"));
        pending->resume_with(fail ? Result<void*>::failure(failure) : Result<void*>::success(new int(42)));
        if (fail) {
            require(completion.resumes == 1 && completion.result.exception_or_null() == failure);
            require(collector.values.empty());
        } else {
            require(completion.resumes == 0 && collector.values == std::vector<int>{42});
            collector.resume();
            require(completion.resumes == 1 && completion.result.is_success());
        }
    }
}

void test_undispatched_context_and_resume() {
    Completion completion;
    auto upstream_context = std::make_shared<CoroutineName>("upstream");
    Continuation<void*>* pending = nullptr;
    void* suspended = kotlinx::coroutines::flow::internal::with_context_undispatched<void*>(upstream_context, 42,
        [&](int value, Continuation<void*>* continuation) -> void* {
            require(value == 42 && continuation->get_context() == upstream_context);
            pending = continuation;
            return intrinsics::get_COROUTINE_SUSPENDED();
        }, &completion);
    require(intrinsics::is_coroutine_suspended(suspended) && completion.resumes == 0);
    auto failure = std::make_exception_ptr(std::runtime_error("undispatched callee failed"));
    pending->resume_with(Result<void*>::failure(failure));
    require(completion.resumes == 1 && completion.result.exception_or_null() == failure);
}

void test_callback_checks_after_suspension() {
    for (bool close_channel : {false, true}) {
        auto channel = channels::create_channel<int>(channels::Channel<int>::BUFFERED);
        auto scope = std::make_shared<channels::ProducerCoroutine<int>>(EmptyCoroutineContext::instance(), channel);
        auto completion = std::make_shared<Completion>();
        Continuation<void*>* pending = nullptr;
        kotlinx::coroutines::flow::internal::CallbackFlowBuilder<int> builder(
            [&](channels::ProducerScope<int>*, std::shared_ptr<Continuation<void*>> continuation) -> void* {
                pending = continuation.get();
                return intrinsics::get_COROUTINE_SUSPENDED();
            });
        require(intrinsics::is_coroutine_suspended(builder.collect_to(scope.get(), completion)));
        require(completion->resumes == 0);
        if (close_channel) channel->close(nullptr);
        pending->resume_with(Result<void*>::success(nullptr));
        require(completion->resumes == 1);
        require(completion->result.is_success() == close_channel);
        if (!close_channel) {
            try { completion->result.get_or_throw(); require(false); }
            catch (const std::logic_error& error) {
                require(std::string(error.what()).find("awaitClose") != std::string::npos);
            }
            channel->close(nullptr);
        }
        scope->resume_with(Result<Unit>::success(Unit{}));
    }
}

void test_merge_permit_remains_held_during_suspension() {
    auto dispatcher = std::shared_ptr<CoroutineContext>(&Dispatchers::get_unconfined(), [](CoroutineContext*) {});
    auto channel = channels::create_channel<int>(channels::Channel<int>::BUFFERED);
    auto scope = std::make_shared<channels::ProducerCoroutine<int>>(dispatcher, channel);
    auto completion = std::make_shared<Completion>();
    completion->context = scope->get_coroutine_context();
    std::vector<Continuation<void*>*> pending;
    std::vector<std::shared_ptr<Flow<int>>> inners;
    for (int i = 0; i < 3; ++i) {
        inners.push_back(kotlinx::coroutines::flow::flow<int>([&](FlowCollector<int>*, Continuation<void*>* continuation) -> void* {
            pending.push_back(continuation);
            return intrinsics::get_COROUTINE_SUSPENDED();
        }));
    }
    auto upstream = as_flow(inners);
    kotlinx::coroutines::flow::internal::ChannelFlowMerge<int> merged(upstream, 1);
    require(intrinsics::is_coroutine_suspended(merged.collect_to(scope.get(), completion)));
    require(pending.size() == 1 && completion->resumes == 0);
    pending[0]->resume_with(Result<void*>::success(nullptr));
    require(pending.size() == 2 && completion->resumes == 0);
    pending[1]->resume_with(Result<void*>::success(nullptr));
    require(pending.size() == 3 && completion->resumes == 1 && completion->result.is_success());
    pending[2]->resume_with(Result<void*>::success(nullptr));
    scope->resume_with(Result<Unit>::success(Unit{}));
    require(scope->is_completed());
}

void test_flow_scope_executes_and_waits_for_children() {
    Completion completion;
    int calls = 0;
    void* direct = kotlinx::coroutines::flow::internal::flow_scope(
        [&](CoroutineScope*, Continuation<void*>*) -> void* { ++calls; return new int(42); }, &completion);
    std::unique_ptr<int> value(static_cast<int*>(direct));
    require(*value == 42 && calls == 1 && completion.resumes == 0);

    completion.context = std::shared_ptr<CoroutineContext>(&Dispatchers::get_unconfined(), [](CoroutineContext*) {});
    std::shared_ptr<Continuation<void*>> child_completion;
    std::shared_ptr<Job> child;
    require(intrinsics::is_coroutine_suspended(kotlinx::coroutines::flow::internal::flow_scope(
        [&](CoroutineScope* scope, Continuation<void*>*) -> void* {
            child = launch(scope, EmptyCoroutineContext::instance(), CoroutineStart::UNDISPATCHED,
                [&](CoroutineScope*, std::shared_ptr<Continuation<void*>> continuation) -> void* {
                    child_completion = std::move(continuation);
                    return intrinsics::get_COROUTINE_SUSPENDED();
                });
            return nullptr;
        }, &completion)));
    require(completion.resumes == 0 && !child->is_completed());
    child_completion->resume_with(Result<void*>::success(nullptr));
    require(child->is_completed() && completion.resumes == 1 && completion.result.is_success());
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/internal/FlowScopeTest.kt:63-74
void test_nested_flow_scope_propagates_child_cancellation() {
    Completion completion;
    completion.context = std::shared_ptr<CoroutineContext>(&Dispatchers::get_unconfined(), [](CoroutineContext*) {});
    bool threw = false;
    try {
        kotlinx::coroutines::flow::internal::flow_scope(
            [&](CoroutineScope*, Continuation<void*>* outer) -> void* {
                return kotlinx::coroutines::flow::internal::flow_scope(
                    [&](CoroutineScope* inner, Continuation<void*>*) -> void* {
                        launch(inner, EmptyCoroutineContext::instance(), CoroutineStart::UNDISPATCHED,
                            [](CoroutineScope*, std::shared_ptr<Continuation<void*>>) -> void* {
                                throw CancellationException("");
                            });
                        return nullptr;
                    }, outer);
            }, &completion);
    } catch (const CancellationException&) { threw = true; }
    require(threw && completion.resumes == 0);
}

// Source contract: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:55-59,110-113.
void test_terminal_collectors_survive_repeated_suspension() {
    for (bool indexed : {false, true}) for (bool fail : {false, true}) {
        Completion completion;
        std::shared_ptr<Flow<int>> upstream = as_flow_range(4, 5);
        std::weak_ptr<Flow<int>> lifetime = upstream;
        auto capture = std::make_shared<int>(42);
        std::weak_ptr<int> action_lifetime = capture;
        Continuation<void*>* pending = nullptr;
        std::vector<int> values;
        int next_index = 0;
        void* result;
        if (indexed) {
            result = collect_indexed<int>(upstream,
                std::function<void*(int, int, Continuation<void*>*)>(
                    [&, capture](int index, int value, Continuation<void*>* continuation) -> void* {
                        require(index == next_index++ && *capture == 42);
                        values.push_back(value);
                        pending = continuation;
                        return intrinsics::get_COROUTINE_SUSPENDED();
                    }), &completion);
        } else {
            result = collect<int>(upstream,
                std::function<void*(int, Continuation<void*>*)>(
                    [&, capture](int value, Continuation<void*>* continuation) -> void* {
                        require(*capture == 42);
                        values.push_back(value);
                        pending = continuation;
                        return intrinsics::get_COROUTINE_SUSPENDED();
                    }), &completion);
        }
        require(intrinsics::is_coroutine_suspended(result));
        upstream.reset();
        capture.reset();
        require(!lifetime.expired() && !action_lifetime.expired());
        require(values == std::vector<int>{4} && completion.resumes == 0);
        auto failure = std::make_exception_ptr(std::runtime_error("terminal action failed"));
        pending->resume_with(fail ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
        if (!fail) {
            require(values == std::vector<int>({4, 5}) && completion.resumes == 0);
            require(!lifetime.expired() && !action_lifetime.expired());
            pending->resume_with(Result<void*>::success(nullptr));
        }
        require(completion.resumes == 1);
        require(completion.result.exception_or_null() == (fail ? failure : nullptr));
        require(lifetime.expired() && action_lifetime.expired());
    }
}

// Source contract: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:26,45-47,103-106.
void test_terminal_collection_retains_upstream_and_launched_job() {
    Completion completion;
    Continuation<void*>* pending = nullptr;
    auto upstream = kotlinx::coroutines::flow::flow<int>(
        [&](FlowCollector<int>*, Continuation<void*>* continuation) -> void* {
            pending = continuation;
            return intrinsics::get_COROUTINE_SUSPENDED();
        });
    std::weak_ptr<Flow<int>> lifetime = upstream;
    require(intrinsics::is_coroutine_suspended(collect<int>(upstream, &completion)));
    upstream.reset();
    require(!lifetime.expired() && completion.resumes == 0);
    pending->resume_with(Result<void*>::success(nullptr));
    require(lifetime.expired() && completion.resumes == 1);

    Collector<int> collector;
    collector.pause = true;
    upstream = as_flow_range(1, 2);
    lifetime = upstream;
    require(intrinsics::is_coroutine_suspended(emit_all<int>(&collector, upstream, &completion)));
    upstream.reset();
    require(!lifetime.expired());
    collector.resume();
    require(collector.values == std::vector<int>({1, 2}));
    require(!lifetime.expired());
    collector.resume();
    require(lifetime.expired() && completion.resumes == 2);

    class Scope final : public CoroutineScope {
        std::shared_ptr<CoroutineContext> context_;
    public:
        explicit Scope(std::shared_ptr<CoroutineContext> context) : context_(std::move(context)) {}
        std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return context_; }
    };
    auto root = JobImpl::create(nullptr);
    auto unconfined = std::shared_ptr<CoroutineContext>(&Dispatchers::get_unconfined(), [](CoroutineContext*) {});
    Scope scope(root->operator+(unconfined));
    upstream = kotlinx::coroutines::flow::flow<int>(
        [&](FlowCollector<int>*, Continuation<void*>* continuation) -> void* {
            pending = continuation;
            return intrinsics::get_COROUTINE_SUSPENDED();
        });
    auto job = launch_in(upstream, scope);
    require(job->is_active() && !job->is_completed());
    pending->resume_with(Result<void*>::success(nullptr));
    require(job->is_completed() && !job->is_cancelled());
}

// Source contract: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/ContinuationImpl.kt:21-45.
void test_completion_chain_preserves_each_frame_until_resumed() {
    for (bool fail : {false, true}) {
        auto completion = std::make_shared<Completion>();
        int calls = 0;
        int destroyed = 0;
        class Frame final : public ContinuationImpl {
            int& calls_;
            int& destroyed_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        public:
            Frame(std::shared_ptr<Continuation<void*>> completion, int& calls, int& destroyed)
                : ContinuationImpl(std::move(completion)), calls_(calls), destroyed_(destroyed) {}
            ~Frame() override { ++destroyed_; }
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                ++calls_;
                self_ref_.reset();
                return result.get_or_throw();
            }
        };
        auto parent = std::make_shared<Frame>(completion, calls, destroyed);
        auto child = std::make_shared<Frame>(parent, calls, destroyed);
        auto leaf = std::make_shared<Frame>(child, calls, destroyed);
        auto* pending = leaf.get();
        std::weak_ptr<BaseContinuationImpl> parent_lifetime = parent;
        std::weak_ptr<BaseContinuationImpl> child_lifetime = child;
        std::weak_ptr<BaseContinuationImpl> leaf_lifetime = leaf;
        leaf->retain();
        parent.reset();
        child.reset();
        leaf.reset();
        auto failure = std::make_exception_ptr(std::runtime_error("leaf failed"));
        pending->resume_with(fail ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
        require(calls == 3 && destroyed == 3);
        require(parent_lifetime.expired() && child_lifetime.expired() && leaf_lifetime.expired());
        require(completion->resumes == 1);
        require(completion->result.exception_or_null() == (fail ? failure : nullptr));
    }
}

// Source contract: kotlinx-coroutines-core/native/src/Exceptions.kt:13-14.
void test_native_cancellation_factory_retains_cause() {
    auto cause = std::make_exception_ptr(std::runtime_error("original failure"));
    std::unique_ptr<CancellationException> cancellation(cancellation_exception("cancelled", cause));
    require(cancellation->get_cause() == cause);
    require(std::string(cancellation->what()) == "cancelled");
    std::unique_ptr<CancellationException> normal(cancellation_exception("normal cancellation", nullptr));
    require(normal->get_cause() == nullptr);
    // CancellationException.kt:13-15; Throwable.kt:26-35: null differs from empty.
    CancellationException default_message;
    CancellationException empty_message("");
    require(!default_message.get_message());
    require(empty_message.get_message() && empty_message.get_message()->empty());
    for (auto message : {std::optional<std::string>{}, std::optional<std::string>{""},
                         std::optional<std::string>{"cancelled"}}) {
        std::unique_ptr<CancellationException> nullable(cancellation_exception(message, cause));
        require(nullable->get_message() == message && nullable->get_cause() == cause);
        try { throw *nullable; }
        catch (const IllegalStateException& error) {
            require(std::string(error.what()) == message.value_or(""));
        }
    }
    std::unique_ptr<CancellationException> null_pointer(cancellation_exception(nullptr, cause));
    require(!null_pointer->get_message() && null_pointer->get_cause() == cause);
    class ResourceCause final : public std::runtime_error {
    public:
        std::shared_ptr<int> resource;
        explicit ResourceCause(std::shared_ptr<int> value)
            : std::runtime_error("resource cause"), resource(std::move(value)) {}
    };
    auto resource = std::make_shared<int>(42);
    std::weak_ptr<int> lifetime = resource;
    auto resource_cause = std::make_exception_ptr(ResourceCause(resource));
    resource.reset();
    {
        std::unique_ptr<CancellationException> owning(cancellation_exception(std::nullopt, resource_cause));
        resource_cause = nullptr;
        require(!lifetime.expired());
        auto copy = *owning;
        owning.reset();
        require(!lifetime.expired() && copy.get_cause());
    }
    require(lifetime.expired());
}

class QueueDispatcher final : public CoroutineDispatcher {
    mutable std::vector<std::shared_ptr<Runnable>> tasks_;
public:
    bool is_dispatch_needed(const CoroutineContext&) const override { return true; }
    void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> task) const override {
        tasks_.push_back(std::move(task));
    }
    std::size_t size() const { return tasks_.size(); }
    void run_next() {
        require(!tasks_.empty());
        auto task = std::move(tasks_.front());
        tasks_.erase(tasks_.begin());
        task->run();
    }
};

class CallerFrame : public ContinuationImpl {
public:
    explicit CallerFrame(std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)) {}
    void* invoke_suspend(Result<void*> result) override { return result.get_or_throw(); }
};

// Source contract: libraries/stdlib/src/kotlin/coroutines/CoroutineContext.kt:30-43,72-73;
// CoroutineContextImpl.kt:140-192.
void test_context_order_removal_and_structural_equality() {
    auto empty = EmptyCoroutineContext::instance();
    auto name = std::make_shared<CoroutineName>("name");
    auto root = JobImpl::create(nullptr);
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto context = dispatcher->operator+(name)->operator+(root);
    std::vector<CoroutineContext::Key*> keys;
    context->for_each([&](std::shared_ptr<CoroutineContext::Element> element) { keys.push_back(element->key()); });
    require(keys == std::vector<CoroutineContext::Key*>({CoroutineName::type_key, Job::type_key, ContinuationInterceptor::type_key}));
    require(context->get(ContinuationInterceptor::type_key).get() == dispatcher.get());
    require(context->operator+(empty) == context);
    require(empty->operator+(name) == name);
    require(empty->minus_key(Job::type_key) == empty);
    require(name->minus_key(CoroutineName::type_key) == empty);
    require(context->minus_key(ContinuationInterceptor::type_key)->minus_key(Job::type_key)->minus_key(CoroutineName::type_key) == empty);
    auto equal_context = root->operator+(std::make_shared<CoroutineName>("name"))->operator+(dispatcher);
    require(context->equals(equal_context.get()) && equal_context->equals(context.get()));
    require(!context->equals(root->operator+(std::make_shared<CoroutineName>("other"))->operator+(dispatcher).get()));
    require(!coroutine_name(context).has_value());
}

// Source contract: CoroutineScope.kt:279-288; Supervisor.kt:50-69.
void test_public_scopes_wait_for_real_children() {
    for (bool supervisor : {false, true}) {
        auto completion = std::make_shared<Completion>();
        auto caller = std::make_shared<QueueDispatcher>();
        completion->context = caller;
        auto frame = std::make_shared<CallerFrame>(completion);
        std::shared_ptr<Continuation<void*>> child_completion;
        std::shared_ptr<Job> child;
        auto block = [&](CoroutineScope* scope, std::shared_ptr<Continuation<void*>>) -> void* {
            child = launch(scope, EmptyCoroutineContext::instance(), CoroutineStart::UNDISPATCHED,
                [&](CoroutineScope*, std::shared_ptr<Continuation<void*>> continuation) -> void* {
                    child_completion = std::move(continuation);
                    return intrinsics::get_COROUTINE_SUSPENDED();
                });
            return new int(42);
        };
        void* result = supervisor ? supervisor_scope<int>(block, frame) : coroutine_scope<int>(block, frame);
        require(intrinsics::is_coroutine_suspended(result));
        require(completion->resumes == 0 && !child->is_completed());
        child_completion->resume_with(Result<void*>::success(nullptr));
        child_completion.reset();
        require(completion->resumes == 0 && caller->size() == 1);
        caller->run_next();
        require(completion->resumes == 1 && child->is_completed());
        std::unique_ptr<int> value(static_cast<int*>(completion->result.get_or_throw()));
        require(*value == 42);
    }
}

// Source contract: CoroutineScope.kt:250-288; Supervisor.kt:35-69.
void test_public_scopes_child_failure_and_block_failure() {
    for (bool supervisor : {false, true}) {
        auto completion = std::make_shared<Completion>();
        completion->context = std::shared_ptr<CoroutineContext>(&Dispatchers::get_unconfined(), [](CoroutineContext*) {});
        std::shared_ptr<Continuation<void*>> sibling_completion;
        std::shared_ptr<Job> sibling;
        auto cause = std::make_exception_ptr(std::runtime_error("child failed"));
        auto block = [&](CoroutineScope* scope, std::shared_ptr<Continuation<void*>>) -> void* {
            sibling = launch(scope, EmptyCoroutineContext::instance(), CoroutineStart::UNDISPATCHED,
                [&](CoroutineScope*, std::shared_ptr<Continuation<void*>> continuation) -> void* {
                    sibling_completion = std::move(continuation);
                    return intrinsics::get_COROUTINE_SUSPENDED();
                });
            auto failed = async<int>(scope, EmptyCoroutineContext::instance(), CoroutineStart::UNDISPATCHED,
                std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
                    [cause](CoroutineScope*, std::shared_ptr<Continuation<void*>>) -> void* { std::rethrow_exception(cause); }));
            require(failed->is_cancelled());
            require(sibling->is_cancelled() == !supervisor);
            return new int(42);
        };
        require(intrinsics::is_coroutine_suspended(supervisor
            ? supervisor_scope<int>(block, completion) : coroutine_scope<int>(block, completion)));
        require(completion->resumes == 0);
        sibling_completion->resume_with(Result<void*>::success(nullptr));
        sibling_completion.reset();
        require(completion->resumes == 1 && sibling->is_completed());
        if (supervisor) {
            std::unique_ptr<int> value(static_cast<int*>(completion->result.get_or_throw()));
            require(*value == 42);
        } else require(completion->result.exception_or_null() == cause);

        auto failing_block = [cause](CoroutineScope*, std::shared_ptr<Continuation<void*>>) -> void* {
            std::rethrow_exception(cause);
        };
        bool caught = false;
        try {
            if (supervisor) supervisor_scope<int>(failing_block, completion);
            else coroutine_scope<int>(failing_block, completion);
        } catch (...) { caught = std::current_exception() == cause; }
        require(caught);
    }
}

// Source contract: Builders.common.kt:147-173,219-267; native/CoroutineContext.kt:48-53.
void test_with_context_fast_paths_and_dispatch_cancellation() {
    auto completion = std::make_shared<Completion>();
    std::unique_ptr<int> immediate(static_cast<int*>(with_context<int>(EmptyCoroutineContext::instance(),
        [](CoroutineScope* scope, std::shared_ptr<Continuation<void*>> continuation) -> void* {
            require(scope->get_job().get() == continuation->get_context()->get(Job::type_key).get());
            return new int(7);
        }, completion)));
    require(*immediate == 7 && completion->resumes == 0);
    std::shared_ptr<CoroutineDispatcher> unconfined(&Dispatchers::get_unconfined(), [](CoroutineDispatcher*) {});
    std::unique_ptr<int> invoked(static_cast<int*>(invoke<int>(unconfined,
        [](CoroutineScope*, std::shared_ptr<Continuation<void*>>) -> void* { return new int(9); }, completion)));
    require(*invoked == 9 && completion->resumes == 0);
    auto unit_block = [](CoroutineScope*, std::shared_ptr<Continuation<void*>>) -> void* { return nullptr; };
    require(with_context<void>(EmptyCoroutineContext::instance(), unit_block, completion) == nullptr);
    require(coroutine_scope<void>(unit_block, completion) == nullptr);
    require(supervisor_scope<void>(unit_block, completion) == nullptr);
    require(completion->resumes == 0);

    auto cancelled_job = JobImpl::create(nullptr);
    cancelled_job->cancel();
    auto cancelled_completion = std::make_shared<Completion>();
    cancelled_completion->context = cancelled_job;
    bool entered = false;
    bool cancelled = false;
    try {
        with_context<void>(EmptyCoroutineContext::instance(),
            [&](CoroutineScope*, std::shared_ptr<Continuation<void*>>) -> void* { entered = true; return nullptr; },
            cancelled_completion);
    } catch (const CancellationException&) { cancelled = true; }
    require(cancelled && !entered && cancelled_completion->resumes == 0);

    std::shared_ptr<Continuation<void*>> pending;
    auto changed = std::make_shared<CoroutineName>("inner");
    require(intrinsics::is_coroutine_suspended(with_context<int>(changed,
        [&](CoroutineScope* scope, std::shared_ptr<Continuation<void*>> continuation) -> void* {
            auto first = scope->get_coroutine_context();
            require(first == scope->get_coroutine_context());
            require(first->get(CoroutineName::type_key) == changed);
            pending = std::move(continuation);
            return intrinsics::get_COROUTINE_SUSPENDED();
        }, completion)));
    pending->resume_with(Result<void*>::success(new int(8)));
    pending.reset();
    require(completion->resumes == 1);
    std::unique_ptr<int> resumed(static_cast<int*>(completion->result.get_or_throw()));
    require(*resumed == 8);

    for (bool cancel_before_return : {false, true}) {
        auto caller = std::make_shared<QueueDispatcher>();
        auto worker = std::make_shared<QueueDispatcher>();
        auto job = JobImpl::create(nullptr);
        auto completion = std::make_shared<Completion>();
        completion->context = job->operator+(caller);
        auto frame = std::make_shared<CallerFrame>(completion);
        bool ran = false;
        auto payload = std::make_shared<int>(42);
        std::weak_ptr<int> lifetime = payload;
        require(intrinsics::is_coroutine_suspended(with_context<std::shared_ptr<int>>(worker,
            [&, payload](CoroutineScope*, std::shared_ptr<Continuation<void*>>) -> void* {
                ran = true;
                return new std::shared_ptr<int>(payload);
            }, frame)));
        payload.reset();
        require(!ran && completion->resumes == 0 && worker->size() == 1);
        worker->run_next();
        require(ran && completion->resumes == 0 && caller->size() == 1 && !lifetime.expired());
        if (cancel_before_return) job->cancel(std::make_exception_ptr(CancellationException("caller cancelled")));
        caller->run_next();
        require(completion->resumes == 1);
        if (cancel_before_return) {
            require(completion->result.is_failure() && lifetime.expired());
        } else {
            std::unique_ptr<std::shared_ptr<int>> result(static_cast<std::shared_ptr<int>*>(completion->result.get_or_throw()));
            require(**result == 42 && result->get() == lifetime.lock().get());
            result.reset();
            require(lifetime.expired());
        }
    }
}

} // namespace

int main() {
    try {
        test_suspend_lambda_as_flow();
        test_range_as_flow();
        test_array_as_flow();
        test_sequence();
        test_range_and_array_suspension();
        test_two_suspensions_and_resumed_failure();
        test_undispatched_context_and_resume();
        test_callback_checks_after_suspension();
        test_merge_permit_remains_held_during_suspension();
        test_flow_scope_executes_and_waits_for_children();
        test_nested_flow_scope_propagates_child_cancellation();
        test_terminal_collectors_survive_repeated_suspension();
        test_terminal_collection_retains_upstream_and_launched_job();
        test_completion_chain_preserves_each_frame_until_resumed();
        test_native_cancellation_factory_retains_cause();
        test_context_order_removal_and_structural_equality();
        test_public_scopes_wait_for_real_children();
        test_public_scopes_child_failure_and_block_failure();
        test_with_context_fast_paths_and_dispatch_cancellation();
        std::cout << "Builder, suspension, Native cause, context and public scope cases completed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
