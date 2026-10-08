// Source contracts: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:11-139.
// NOTE(port): Executable ABI regressions use real channels and a deterministic dispatcher.
#include "kotlinx/coroutines/flow/Zip.hpp"
#include "kotlinx/coroutines/flow/Channels.hpp"
#include <deque>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::flow;

namespace {
void require(bool value, int line) {
    if (!value) throw std::runtime_error("combine/zip check at " + std::to_string(line));
}
#define CHECK(value) require((value), __LINE__)
class QueueDispatcher final : public CoroutineDispatcher {
public:
    mutable std::deque<std::shared_ptr<Runnable>> queue;
    void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> task) const override {
        queue.push_back(std::move(task));
    }
    void drain() {
        while (!queue.empty()) {
            auto task = std::move(queue.front());
            queue.pop_front();
            task->run();
        }
    }
};
class Completion final : public Continuation<void*> {
public:
    std::shared_ptr<CoroutineContext> context;
    int resumes = 0;
    std::exception_ptr failure;
    std::shared_ptr<CoroutineContext> get_context() const override { return context; }
    void resume_with(Result<void*> result) override {
        ++resumes;
        failure = result.exception_or_null();
        if (!failure) CHECK(result.get_or_throw() == nullptr);
    }
};
template <typename T>
class AccumulatorCollector final : public FlowCollector<T> {
public:
    std::vector<T> items;
    void* emit(T value, Continuation<void*>*) override {
        items.push_back(std::move(value));
        return nullptr;
    }
};
template <typename T>
std::shared_ptr<Flow<T>> values(std::vector<T> input) {
    auto channel = channels::create_channel<T>(static_cast<int>(input.size()) + 1);
    for (auto& value : input) CHECK(channel->try_send(std::move(value)).is_success());
    channel->close();
    return receive_as_flow<T>(channel);
}
template <typename T>
std::vector<T> collect(std::shared_ptr<Flow<T>> source) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    Completion completion;
    completion.context = dispatcher;
    AccumulatorCollector<T> collector;
    auto outcome = source->collect(&collector, &completion);
    dispatcher->drain();
    if (intrinsics::is_coroutine_suspended(outcome)) {
        CHECK(completion.resumes == 1);
        if (completion.failure) std::rethrow_exception(completion.failure);
    } else CHECK(completion.resumes == 0);
    return collector.items;
}
void combine_success() {
    auto result = collect(combine<int, std::string, std::string>(
        values<int>({1, 2}), values<std::string>({"a", "b"}),
        [](int n, std::string value) { return std::to_string(n) + value; }));
    CHECK(!result.empty() && result.back() == "2b");
}
void combine_error() {
    auto failure = std::make_exception_ptr(std::runtime_error("combine original failure"));
    auto failed = flow::internal::unsafe_flow<int>([failure](FlowCollector<int>*, Continuation<void*>*) -> void* {
        std::rethrow_exception(failure);
    });
    std::exception_ptr observed;
    try { collect(combine<int, int, int>(failed, values<int>({1}), [](int a, int b) { return a + b; })); }
    catch (...) { observed = std::current_exception(); }
    CHECK(observed == failure);
}
// Zip.kt:28-30,114-126,146-160,186-202: transform and emit are separate suspend calls.
void combine_suspend_transform() {
    class PausedCollector final : public FlowCollector<std::shared_ptr<int>> {
    public:
        std::shared_ptr<Continuation<void*>> frame;
        std::shared_ptr<int> value;
        void* emit(std::shared_ptr<int> received, Continuation<void*>* continuation) override {
            CHECK(!value);
            value = std::move(received);
            frame = kotlinx::coroutines::internal::retain_continuation(continuation);
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
    };
    for (int arity : {2, 3, 4, 5, 6}) for (int failure_point : {0, 1, 2}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        Completion completion;
        completion.context = dispatcher;
        auto resource = std::make_shared<int>(79);
        auto* identity = resource.get();
        std::weak_ptr<int> lifetime = resource;
        std::shared_ptr<Continuation<void*>> transform_frame;
        std::function<void*(int, Continuation<void*>*)> suspend_transform = [&, resource](int sum, Continuation<void*>* continuation) -> void* {
            CHECK(sum == arity * (arity + 1) / 2 && resource.get() == identity);
            CHECK(!transform_frame);
            transform_frame = kotlinx::coroutines::internal::retain_continuation(continuation);
            return intrinsics::get_COROUTINE_SUSPENDED();
        };
        std::shared_ptr<Flow<std::shared_ptr<int>>> source;
        if (arity == 2) source = combine<int, int, std::shared_ptr<int>>(
            values<int>({1}), values<int>({2}),
            [suspend_transform](int a, int b, Continuation<void*>* c) { return suspend_transform(a + b, c); });
        if (arity == 3) source = combine<int, int, int, std::shared_ptr<int>>(
            values<int>({1}), values<int>({2}), values<int>({3}),
            [suspend_transform](int a, int b, int d, Continuation<void*>* c) { return suspend_transform(a + b + d, c); });
        if (arity == 4) source = combine<int, int, int, int, std::shared_ptr<int>>(
            values<int>({1}), values<int>({2}), values<int>({3}), values<int>({4}),
            [suspend_transform](int a, int b, int d, int e, Continuation<void*>* c) { return suspend_transform(a + b + d + e, c); });
        if (arity == 5) source = combine<int, int, int, int, int, std::shared_ptr<int>>(
            values<int>({1}), values<int>({2}), values<int>({3}), values<int>({4}), values<int>({5}),
            [suspend_transform](int a, int b, int d, int e, int f, Continuation<void*>* c) { return suspend_transform(a + b + d + e + f, c); });
        if (arity == 6) source = combine<int, std::shared_ptr<int>>(
            {values<int>({1}), values<int>({2}), values<int>({3}),
             values<int>({4}), values<int>({5}), values<int>({6})},
            [suspend_transform](std::vector<int> input, Continuation<void*>* c) {
                CHECK(input == std::vector<int>({1, 2, 3, 4, 5, 6}));
                int sum = 0;
                for (int value : input) sum += value;
                return suspend_transform(sum, c);
            });
        // Release the local callback too, so the actual suspended library frames are the owners.
        suspend_transform = {};
        PausedCollector collector;
        CHECK(intrinsics::is_coroutine_suspended(source->collect(&collector, &completion)));
        source.reset();
        resource.reset();
        dispatcher->drain();
        CHECK(transform_frame && !collector.frame && completion.resumes == 0 && !lifetime.expired());
        auto failure = std::make_exception_ptr(std::runtime_error("public combine resumed failure"));
        transform_frame->resume_with(failure_point == 1
            ? Result<void*>::failure(failure) : Result<void*>::success(new std::shared_ptr<int>(lifetime.lock())));
        dispatcher->drain();
        if (failure_point != 1) {
            CHECK(collector.frame && collector.value.get() == identity && completion.resumes == 0);
            collector.frame->resume_with(failure_point == 2
                ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
            dispatcher->drain();
            collector.value.reset();
        }
        CHECK(completion.resumes == 1 && completion.failure == (failure_point ? failure : nullptr));
        CHECK(lifetime.expired()); // Keep both completed frames independently held.
    }
}

// The existing erased Unit ABI returns nullptr, including on resumed transforms.
void unit_transform_contract() {
    CHECK(collect(combine<int, int, Unit>(values<int>({1}), values<int>({2}),
        [](int a, int b) { CHECK(a == 1 && b == 2); return Unit{}; })).size() == 1);
    CHECK(collect(zip<int, int, Unit>(values<int>({1}), values<int>({2}),
        [](int a, int b) { CHECK(a == 1 && b == 2); return Unit{}; })).size() == 1);
    for (bool zipped : {false, true}) for (bool suspended : {false, true}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        Completion completion;
        completion.context = dispatcher;
        std::shared_ptr<Continuation<void*>> frame;
        std::function<void*(int, int, Continuation<void*>*)> transform =
            [&](int a, int b, Continuation<void*>* continuation) -> void* {
                CHECK(a == 1 && b == 2);
                if (!suspended) return nullptr;
                frame = kotlinx::coroutines::internal::retain_continuation(continuation);
                return intrinsics::get_COROUTINE_SUSPENDED();
            };
        auto source = zipped ? zip<int, int, Unit>(values<int>({1}), values<int>({2}), transform)
                             : combine<int, int, Unit>(values<int>({1}), values<int>({2}), transform);
        AccumulatorCollector<Unit> collector;
        CHECK(intrinsics::is_coroutine_suspended(source->collect(&collector, &completion)));
        dispatcher->drain();
        if (suspended) {
            CHECK(frame && collector.items.empty() && completion.resumes == 0);
            frame->resume_with(Result<void*>::success(nullptr));
            dispatcher->drain();
        }
        CHECK(completion.resumes == 1 && !completion.failure && collector.items.size() == 1);
    }
}

// Zip.kt:229-250,280-310: array/Iterable inputs copy arrays and empty input never transforms.
void combine_array_contract() {
    std::vector<std::shared_ptr<Flow<int>>> sources{values<int>({3}), values<int>({7})};
    CHECK(collect(combine_all<int, int>(sources,
        [](std::vector<int> input) { CHECK(input == std::vector<int>({3, 7})); return input[0] + input[1]; }))
        == std::vector<int>{10});
    CHECK(collect(combine_transform<int, int>({values<int>({3}), values<int>({7})},
        [](FlowCollector<int>* sink, std::vector<int> input, Continuation<void*>* c) {
            CHECK(input == std::vector<int>({3, 7}));
            return sink->emit(input[0] * input[1], c);
        })) == std::vector<int>{21});
    CHECK(collect(combine<int, int>({}, [](std::vector<int>) -> int { CHECK(false); return 0; })).empty());
    CHECK(collect(combine_transform<int, int>({},
        [](FlowCollector<int>*, std::vector<int>, Continuation<void*>*) -> void* { CHECK(false); return nullptr; })).empty());
}

void zip_success() {
    auto result = collect(zip<int, std::string, std::string>(
        values<int>({1, 2, 3}), values<std::string>({"a", "b", "c", "d"}),
        [](int n, std::string value) { return std::to_string(n) + value; }));
    CHECK(result == std::vector<std::string>({"1a", "2b", "3c"}));
}
void zip_early_termination() {
    std::vector<int> long_input;
    for (int value = 1; value <= 100; ++value) long_input.push_back(value);
    auto result = collect(zip<int, int, int>(values<int>({10, 20}), values<int>(long_input),
                                            [](int a, int b) { return a + b; }));
    CHECK(result == std::vector<int>({11, 22}));
    CHECK(collect(zip<int, int, int>(values<int>({1, 2, 3}), values<int>({10}),
                                    [](int a, int b) { return a + b; })) == std::vector<int>{11});
}
void zip_error() {
    for (bool first_fails : {false, true}) {
        auto failure = std::make_exception_ptr(std::runtime_error("zip original failure"));
        auto failed = flow::internal::unsafe_flow<int>([failure](FlowCollector<int>*, Continuation<void*>*) -> void* {
            std::rethrow_exception(failure);
        });
        auto normal = values<int>({1, 2});
        std::exception_ptr observed;
        try { collect(zip<int, int, int>(first_fails ? failed : normal, first_fails ? normal : failed,
                                        [](int a, int b) { return a + b; })); }
        catch (...) { observed = std::current_exception(); }
        CHECK(observed == failure);
    }
    int other_owner = 0;
    auto failure = std::make_exception_ptr(flow::internal::AbortFlowException(&other_owner));
    std::exception_ptr observed;
    try {
        collect(zip<int, int, int>(values<int>({1}), values<int>({10}),
            [failure](int, int) -> int { std::rethrow_exception(failure); }));
    } catch (...) { observed = std::current_exception(); }
    CHECK(observed == failure);
}
void flow_exception_contract() {
    int owner = 0;
    int other = 0;
    flow::internal::AbortFlowException value(&owner);
    CHECK(value.get_message() == "Flow was aborted, no more elements needed");
    value.check_ownership(&owner);
    auto failure = std::make_exception_ptr(value);
    std::exception_ptr observed;
    try {
        try { std::rethrow_exception(failure); }
        catch (flow::internal::AbortFlowException& raised) {
            raised.check_ownership(&other);
        }
    } catch (...) { observed = std::current_exception(); }
    CHECK(observed == failure);
    // An unrelated active exception must not replace the receiver being thrown.
    try {
        try { throw std::runtime_error("unrelated"); }
        catch (...) { value.check_ownership(&other); }
    } catch (const flow::internal::AbortFlowException& raised) {
        CHECK(raised.owner == &owner && raised.get_message() == value.get_message());
    }
    flow::internal::ChildCancelledException child;
    CHECK(child.get_message() == "Child of the scoped flow was cancelled");
}
} // namespace
int main() {
    try {
        combine_success();
        combine_error();
        combine_suspend_transform();
        unit_transform_contract();
        combine_array_contract();
        zip_success();
        zip_early_termination();
        zip_error();
        flow_exception_contract();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
