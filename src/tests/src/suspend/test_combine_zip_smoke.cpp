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
        zip_success();
        zip_early_termination();
        zip_error();
        flow_exception_contract();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
