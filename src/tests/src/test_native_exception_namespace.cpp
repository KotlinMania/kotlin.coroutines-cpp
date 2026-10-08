// Source contracts: native/src/Exceptions.kt:9-14 and
// libraries/stdlib/common-non-jvm/src/kotlin/coroutines/cancellation/CancellationException.kt:11-16.
#include "kotlin/coroutines/cancellation/CancellationException.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include <memory>
#include <stdexcept>
#include <type_traits>

using ActualCancellation = kotlin::coroutines::cancellation::CancellationException;
static_assert(std::is_same_v<ActualCancellation, kotlinx::coroutines::CancellationException>);
static_assert(std::is_base_of_v<ActualCancellation, kotlinx::coroutines::JobCancellationException>);

namespace {
void require(bool value) {
    if (!value) throw std::runtime_error("Native cancellation namespace contract");
}
struct ResourceCause final : std::runtime_error {
    std::shared_ptr<int> resource;
    explicit ResourceCause(std::shared_ptr<int> value)
        : std::runtime_error("original cause"), resource(std::move(value)) {}
};

class FinishingJob final : public kotlinx::coroutines::JobSupport {
public:
    FinishingJob() : JobSupport(true) {}
    bool finish() { return make_completing(nullptr); }
};

class JobContinuation final : public kotlinx::coroutines::Continuation<void*> {
public:
    explicit JobContinuation(std::shared_ptr<kotlinx::coroutines::Job> job) : context_(std::move(job)) {}
    std::shared_ptr<kotlinx::coroutines::CoroutineContext> get_context() const override { return context_; }
    void resume_with(kotlinx::coroutines::Result<void*> result) override {
        ++completions;
        failure = result.exception_or_null();
    }
    int completions = 0;
    std::exception_ptr failure;
private:
    std::shared_ptr<kotlinx::coroutines::CoroutineContext> context_;
};

// Job.kt:509-512. Both the extension and this caller use CMake's Clang/LLVM
// lowering; execution after the call must wait for real join completion.
[[clang::annotate("suspend")]]
void* authored_cancel_and_join(kotlinx::coroutines::Job& target, int* after,
                              std::shared_ptr<kotlinx::coroutines::Continuation<void*>> completion) {
    kotlinx::coroutines::cancel_and_join(target);
    ++*after;
    return nullptr;
}
}

int main() {
    using namespace kotlinx::coroutines;
    for (int outcome : {0, 1, 2, 3}) {
        auto target = std::make_shared<FinishingJob>();
        auto caller = make_job();
        auto completion = std::make_shared<JobContinuation>(caller);
        auto cancellation = std::make_exception_ptr(CancellationException("caller cancelled"));
        if (outcome == 0) require(target->finish());
        if (outcome == 2) caller->cancel(cancellation);
        int after = 0;
        if (outcome == 2) {
            try {
                authored_cancel_and_join(*target, &after, completion);
                require(false);
            } catch (...) {
                require(std::current_exception() == cancellation);
            }
            require(after == 0 && completion->completions == 0 && target->is_cancelled());
            require(target->finish());
        } else {
            auto result = authored_cancel_and_join(*target, &after, completion);
            if (outcome == 0) {
                require(result == nullptr && after == 1 && completion->completions == 0);
            } else {
                require(intrinsics::is_coroutine_suspended(result));
                require(after == 0 && completion->completions == 0 &&
                        target->is_cancelled() && !target->is_completed());
                if (outcome == 3) {
                    caller->cancel(cancellation);
                    require(after == 0 && completion->completions == 1 && completion->failure == cancellation);
                }
                require(target->finish());
                require(completion->completions == 1 && after == (outcome == 1 ? 1 : 0));
                require(completion->failure == (outcome == 3 ? cancellation : nullptr));
            }
        }
        if (caller->is_active()) caller->complete();
    }

    auto empty = EmptyCoroutineContext::instance();
    require(is_active(*empty));
    ensure_active(*empty);
    cancel(*empty);
    cancel_children(*empty);
    try {
        get_job(*empty);
        require(false);
    } catch (const IllegalStateException& exception) {
        require(std::string(exception.what()) == "Current context doesn't contain Job in it: " + empty->to_string());
    }
    auto diagnostic_job = make_job();
    auto original_cause = std::make_exception_ptr(std::runtime_error("diagnostic cause"));
    cancel(*diagnostic_job, "diagnostic cancellation", original_cause);
    require(!is_active(*diagnostic_job) && get_job(*diagnostic_job).get() == diagnostic_job.get());
    try {
        ensure_active(*diagnostic_job);
        require(false);
    } catch (const CancellationException& exception) {
        require(exception.get_message() == "diagnostic cancellation" && exception.get_cause() == original_cause);
    }

    auto resource = std::make_shared<int>(101);
    std::weak_ptr<int> lifetime = resource;
    auto cause = std::make_exception_ptr(ResourceCause(resource));
    resource.reset();
    {
        // Factory construction occurs in the library translation unit; the result
        // has the actual stdlib class identity in this consumer translation unit.
        std::unique_ptr<ActualCancellation> exception(
            kotlinx::coroutines::cancellation_exception(std::nullopt, cause));
        cause = nullptr;
        require(!exception->get_message() && exception->get_cause());
        require(!lifetime.expired());
        auto original = exception->get_cause();
        try { throw *exception; }
        catch (const kotlinx::coroutines::CancellationException& caught) {
            require(caught.get_cause() == original && !caught.get_message());
            require(kotlinx::coroutines::is_cancellation_exception(std::current_exception()));
        }
        try { throw kotlinx::coroutines::CancellationException(""); }
        catch (const ActualCancellation& caught) {
            require(caught.get_message() && caught.get_message()->empty());
            require(!caught.get_cause());
        }
        auto copy = *exception;
        exception.reset();
        require(!lifetime.expired() && copy.get_cause() == original);
        require(copy.equals(&copy));
    }
    require(lifetime.expired());
    ActualCancellation first("same"), second("same");
    require(!first.equals(&second));
    require(first.hash_code() == first.hash_code());
    require(!ActualCancellation{}.get_message());
}
