// Source contracts: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:90-103,159-162,191-202;
// kotlinx-coroutines-core/common/src/flow/Channels.kt:104-108,119-134.
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/flow/Channels.hpp"
#include "kotlinx/coroutines/flow/internal/Combine.hpp"
#include "kotlinx/coroutines/native/Exceptions.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include <deque>
#include <iostream>
#include <memory>
#include <vector>
#include <typeinfo>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::channels;

namespace {
void require(bool condition, int line) {
    if (!condition) throw std::runtime_error("channel consumption check at " + std::to_string(line));
}
#define CHECK(condition) require((condition), __LINE__)

class RecordingChannel final : public BufferedChannel<int> {
public:
    RecordingChannel() : BufferedChannel<int>(8) {}
    int cancellations = 0;
    std::exception_ptr cancellation_cause;
    std::exception_ptr cancellation_failure;

    void cancel(std::exception_ptr cause = nullptr) override {
        ++cancellations;
        cancellation_cause = cause;
        if (cancellation_failure) std::rethrow_exception(cancellation_failure);
        BufferedChannel<int>::cancel(cause);
    }
};

class Completion final : public Continuation<void*> {
public:
    int resumes = 0;
    void* value = nullptr;
    std::exception_ptr failure;
    std::shared_ptr<CoroutineContext> get_context() const override {
        return EmptyCoroutineContext::instance();
    }
    void resume_with(Result<void*> result) override {
        ++resumes;
        failure = result.exception_or_null();
        if (!failure) value = result.get_or_throw();
    }
};

void check_wrapped(std::exception_ptr actual, std::exception_ptr original) {
    CHECK(actual && actual != original);
    try {
        std::rethrow_exception(actual);
    } catch (const CancellationException& exception) {
        CHECK(exception.get_cause() == original);
        CHECK(exception.get_message() == "Channel was consumed, consumer had failed");
    }
}

// Native Exceptions.kt:27-29: identity, type, message, Job and cause equality
// execute in source order, including virtual equality on the other operands.
void job_cancellation_equality_contract() {
    auto job = std::make_shared<JobSupport>(true);
    auto other_job = std::make_shared<JobSupport>(true);
    auto cause = std::make_exception_ptr(std::runtime_error("cause"));
    JobCancellationException first("cancelled", cause, job.get());
    JobCancellationException same("cancelled", cause, job.get());
    JobCancellationException message("different", cause, job.get());
    JobCancellationException owner("cancelled", cause, other_job.get());
    JobCancellationException distinct_cause("cancelled", std::make_exception_ptr(std::runtime_error("cause")), job.get());
    JobCancellationException no_cause("cancelled", nullptr, job.get());
    CancellationException base("cancelled", cause);
    std::runtime_error unrelated("cancelled");
    CHECK(first.equals(&first) && first.equals(&same) && same.equals(&first));
    CHECK(!first.equals(nullptr) && !first.equals(&base) && !first.equals(&unrelated));
    CHECK(!first.equals(&message) && !first.equals(&owner) && !first.equals(&distinct_cause) && !first.equals(&no_cause));
    CHECK(base.equals(&base) && !base.equals(&first) && !base.equals(nullptr));
    JobCancellationException second_no_cause("cancelled", nullptr, job.get());
    CHECK(no_cause.equals(&second_no_cause));
    CHECK(first.get_job() == job.get() && first.get_cause() == cause);

    class EqualCause final : public CancellationException {
    public:
        explicit EqualCause(int key, std::shared_ptr<int> calls, std::exception_ptr failure = nullptr)
            : CancellationException("cause"), key_(key), calls_(std::move(calls)), failure_(failure) {}
        bool equals(const std::exception* other) const override {
            ++*calls_;
            if (failure_) std::rethrow_exception(failure_);
            auto* value = dynamic_cast<const EqualCause*>(other);
            return value && value->key_ == key_;
        }
    private:
        int key_;
        std::shared_ptr<int> calls_;
        std::exception_ptr failure_;
    };
    auto left_calls = std::make_shared<int>(0);
    auto right_calls = std::make_shared<int>(0);
    auto left_cause = std::make_exception_ptr(EqualCause(8, left_calls));
    auto right_cause = std::make_exception_ptr(EqualCause(8, right_calls));
    JobCancellationException left("equal", left_cause, job.get());
    JobCancellationException right("equal", right_cause, job.get());
    CHECK(left.equals(&right) && *right_calls == 1 && *left_calls == 0);
    JobCancellationException same_cause("equal", right_cause, job.get());
    CHECK(right.equals(&same_cause) && *right_calls == 2);
    CHECK(right.equals(&right) && *right_calls == 2);
    auto equality_failure = std::make_exception_ptr(std::runtime_error("equality failure"));
    auto throwing_cause = std::make_exception_ptr(EqualCause(8, right_calls, equality_failure));
    JobCancellationException throwing("equal", throwing_cause, job.get());
    try {
        left.equals(&throwing);
        CHECK(false);
    } catch (...) { CHECK(std::current_exception() == equality_failure); }

    class EqualJob final : public JobSupport {
    public:
        explicit EqualJob(int key) : JobSupport(true), key_(key) {}
        mutable int calls = 0;
        bool equals(const CoroutineContext* other) const override {
            ++calls;
            auto* job = dynamic_cast<const EqualJob*>(other);
            return job && job->key_ == key_;
        }
    private:
        int key_;
    };
    auto left_job = std::make_shared<EqualJob>(5);
    auto right_job = std::make_shared<EqualJob>(5);
    JobCancellationException left_owner("equal", nullptr, left_job.get());
    JobCancellationException right_owner("equal", nullptr, right_job.get());
    CHECK(left_owner.equals(&right_owner) && right_job->calls == 1 && left_job->calls == 0);
    JobCancellationException same_owner("equal", nullptr, right_job.get());
    CHECK(right_owner.equals(&same_owner) && right_job->calls == 2);
    JobCancellationException wrong_message("unequal", nullptr, right_job.get());
    CHECK(!left_owner.equals(&wrong_message) && right_job->calls == 2);
    CHECK(left_job.use_count() == 1 && right_job.use_count() == 1);

    // Distinct JobCancellationException causes recursively use source equality.
    auto nested_left = std::make_exception_ptr(JobCancellationException("nested", nullptr, job.get()));
    auto nested_right = std::make_exception_ptr(JobCancellationException("nested", nullptr, job.get()));
    JobCancellationException outer_left("outer", nested_left, job.get());
    JobCancellationException outer_right("outer", nested_right, job.get());
    CHECK(outer_left.equals(&outer_right));
}

// Native Exceptions.kt:30-31 and CoroutineContextImpl.kt:126,194.
void job_cancellation_hash_contract() {
    class HashedJob final : public JobSupport {
    public:
        HashedJob(int hash, std::vector<int>* order = nullptr, std::exception_ptr failure = nullptr)
            : JobSupport(true), hash_(hash), order_(order), failure_(failure) {}
        std::int32_t hash_code() const override {
            if (order_) order_->push_back(1);
            if (failure_) std::rethrow_exception(failure_);
            return hash_;
        }
        bool equals(const CoroutineContext* other) const override {
            auto* job = dynamic_cast<const HashedJob*>(other);
            return job && job->hash_ == hash_;
        }
    private:
        int hash_;
        std::vector<int>* order_;
        std::exception_ptr failure_;
    };
    HashedJob job(17);
    struct MessageCase { std::string message; std::int32_t expected; };
    // Golden values include a supplementary code point, embedded NUL and overflowing Int arithmetic.
    std::vector<MessageCase> messages{{"", 527}, {"abc", 92596721}, {"é", 224440},
        {"🙂", 1703819892}, {"é🙂", 1919000285}, {std::string("a\0b", 3), 89676242},
        {std::string(10000, 'z'), 1964547087}, {std::string(1, static_cast<char>(0xff)), 62977740}};
    for (const auto& test : messages) {
        JobCancellationException exception(test.message, nullptr, &job);
        CHECK(exception.hash_code() == test.expected);
    }
    class HashedCause final : public CancellationException {
    public:
        HashedCause(int hash, std::vector<int>* order, std::exception_ptr failure = nullptr)
            : CancellationException("cause"), hash_(hash), order_(order), failure_(failure) {}
        std::int32_t hash_code() const override {
            order_->push_back(2);
            if (failure_) std::rethrow_exception(failure_);
            return hash_;
        }
        bool equals(const std::exception* other) const override {
            auto* cause = dynamic_cast<const HashedCause*>(other);
            return cause && hash_ == cause->hash_;
        }
    private:
        int hash_;
        std::vector<int>* order_;
        std::exception_ptr failure_;
    };
    std::vector<int> order;
    HashedJob first_job(17, &order), second_job(17, &order);
    auto first_cause = std::make_exception_ptr(HashedCause(-50, &order));
    auto second_cause = std::make_exception_ptr(HashedCause(-50, &order));
    JobCancellationException first("abc", first_cause, &first_job);
    JobCancellationException second("abc", second_cause, &second_job);
    CHECK(first.equals(&second));
    CHECK(first.hash_code() == 92596671 && order == std::vector<int>({1, 2}));
    order.clear();
    CHECK(second.hash_code() == first.hash_code());
    CHECK(order == std::vector<int>({1, 2, 1, 2}));
    // Recursively hash a distinct but structurally equal JobCancellationException cause.
    auto nested_first = std::make_exception_ptr(first);
    auto nested_second = std::make_exception_ptr(second);
    JobCancellationException outer_first("nested", nested_first, &first_job);
    JobCancellationException outer_second("nested", nested_second, &second_job);
    CHECK(outer_first.equals(&outer_second) && outer_first.hash_code() == outer_second.hash_code());
    for (bool job_fails : {false, true}) {
        order.clear();
        auto failure = std::make_exception_ptr(std::runtime_error("actual hash failure"));
        HashedJob failing_job(17, &order, job_fails ? failure : nullptr);
        auto cause = std::make_exception_ptr(HashedCause(5, &order, job_fails ? nullptr : failure));
        JobCancellationException exception("abc", cause, &failing_job);
        std::exception_ptr observed;
        try { exception.hash_code(); } catch (...) { observed = std::current_exception(); }
        CHECK(observed == failure);
        CHECK(order == (job_fails ? std::vector<int>{1} : std::vector<int>{1, 2}));
    }
    auto identity_cause = std::make_exception_ptr(std::runtime_error("identity"));
    std::uint32_t cause_hash = 0;
    try { std::rethrow_exception(identity_cause); }
    catch (const std::exception& exception) { cause_hash = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&exception)); }
    CHECK(static_cast<std::uint32_t>(JobCancellationException("", identity_cause, &job).hash_code()) == 527 + cause_hash);
    CHECK(EmptyCoroutineContext::instance()->hash_code() == 0);
    auto identity_job = std::make_shared<JobSupport>(true);
    CHECK(static_cast<std::uint32_t>(identity_job->hash_code()) ==
          static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(static_cast<CoroutineContext*>(identity_job.get()))));
    class HashedElement final : public AbstractCoroutineContextElement {
    public:
        HashedElement(CoroutineContext::Key* key, int hash) : AbstractCoroutineContextElement(key), hash_(hash) {}
        std::int32_t hash_code() const override { return hash_; }
        bool equals(const CoroutineContext* other) const override {
            auto* element = dynamic_cast<const HashedElement*>(other);
            return element && element->key() == key() && element->hash_ == hash_;
        }
    private:
        int hash_;
    };
    CoroutineContext::Key left_key, right_key;
    auto left = std::make_shared<HashedElement>(&left_key, 2147483647);
    auto right = std::make_shared<HashedElement>(&right_key, 1);
    auto context = left->operator+(right);
    auto reverse = right->operator+(left);
    CHECK(context->equals(reverse.get()));
    CHECK(context->hash_code() == -2147483647 - 1 && context->hash_code() == reverse->hash_code());
    CHECK(context->minus_key(&left_key)->hash_code() == 1);
    CHECK(context->minus_key(&right_key)->hash_code() == 2147483647);
}

// CoroutineContextImpl.kt:56-65,81-89,106-112; ContinuationInterceptor.kt:52-71;
// CoroutineDispatcher.kt:65-67. Exercise root/nested keys, source cast order and context identity.
void polymorphic_context_contract() {
    class BaseElement : public AbstractCoroutineContextElement {
    public:
        explicit BaseElement(CoroutineContext::Key* key) : AbstractCoroutineContextElement(key) {}
        std::shared_ptr<Element> get(CoroutineContext::Key* key) const override {
            return get_polymorphic_element(std::dynamic_pointer_cast<Element>(
                std::const_pointer_cast<CoroutineContext>(shared_from_this())), key);
        }
        std::shared_ptr<CoroutineContext> minus_key(CoroutineContext::Key* key) const override {
            return minus_polymorphic_key(std::dynamic_pointer_cast<Element>(
                std::const_pointer_cast<CoroutineContext>(shared_from_this())), key);
        }
    };
    class DerivedElement : public BaseElement { public: using BaseElement::BaseElement; };
    class LeafElement final : public DerivedElement { public: using DerivedElement::DerivedElement; };
    class DerivedKey final : public AbstractCoroutineContextKey<BaseElement, DerivedElement> {
    public:
        DerivedKey(CoroutineContext::KeyTyped<BaseElement>* base, int* calls, std::exception_ptr failure = nullptr)
            : AbstractCoroutineContextKey(base, [calls, failure](std::shared_ptr<CoroutineContext::Element> element) {
                ++*calls;
                if (failure) std::rethrow_exception(failure);
                return std::dynamic_pointer_cast<DerivedElement>(element);
            }) {}
    };
    class LeafKey final : public AbstractCoroutineContextKey<DerivedElement, LeafElement> {
    public:
        explicit LeafKey(CoroutineContext::KeyTyped<DerivedElement>* base)
            : AbstractCoroutineContextKey(base, [](std::shared_ptr<CoroutineContext::Element> element) {
                return std::dynamic_pointer_cast<LeafElement>(element);
            }) {}
    };
    CoroutineContext::KeyTyped<BaseElement> root, unrelated;
    int calls = 0;
    DerivedKey derived_key(&root, &calls);
    LeafKey leaf_key(&derived_key);
    CHECK(derived_key.is_sub_key(&root) && derived_key.is_sub_key(&derived_key));
    CHECK(leaf_key.is_sub_key(&root) && leaf_key.is_sub_key(&leaf_key));
    CHECK(!leaf_key.is_sub_key(&derived_key) && !derived_key.is_sub_key(&unrelated));
    auto base = std::make_shared<BaseElement>(&root);
    auto derived = std::make_shared<DerivedElement>(&root);
    auto leaf = std::make_shared<LeafElement>(&root);
    CHECK(get_polymorphic_element<BaseElement>(derived, &root) == derived);
    CHECK(get_polymorphic_element<DerivedElement>(derived, &derived_key) == derived && calls == 1);
    CHECK(get_polymorphic_element<LeafElement>(leaf, &leaf_key) == leaf);
    CHECK(!base->get(&derived_key) && calls == 2);
    CHECK(base->minus_key(&derived_key) == base && calls == 3);
    CHECK(derived->minus_key(&derived_key) == EmptyCoroutineContext::instance() && calls == 4);
    CHECK(leaf->minus_key(&leaf_key) == EmptyCoroutineContext::instance());
    auto foreign = std::make_shared<DerivedElement>(&unrelated);
    CHECK(!foreign->get(&derived_key) && foreign->minus_key(&derived_key) == foreign && calls == 4);
    CHECK(derived->get(&root) == derived && !derived->get(&unrelated));
    CHECK(derived->minus_key(&root) == EmptyCoroutineContext::instance());
    // The source also recognizes the polymorphic key itself, but not an intermediate key.
    auto own_key = std::make_shared<DerivedElement>(&derived_key);
    CHECK(own_key->get(&derived_key) == own_key && calls == 5);
    CHECK(own_key->minus_key(&derived_key) == EmptyCoroutineContext::instance() && calls == 6);
    auto intermediate = std::make_shared<LeafElement>(&derived_key);
    CHECK(!intermediate->get(&leaf_key) && intermediate->minus_key(&leaf_key) == intermediate);
    auto failure = std::make_exception_ptr(std::runtime_error("actual safe cast failure"));
    DerivedKey throwing(&root, &calls, failure);
    for (bool remove : {false, true}) {
        try {
            if (remove) derived->minus_key(&throwing); else derived->get(&throwing);
            CHECK(false);
        } catch (...) { CHECK(std::current_exception() == failure); }
    }
    CHECK(!foreign->get(&throwing) && foreign->minus_key(&throwing) == foreign && calls == 8);
    auto combined = foreign->operator+(leaf);
    CHECK(combined->get(&leaf_key) == leaf);
    CHECK(combined->minus_key(&leaf_key) == foreign);
    CHECK(combined->minus_key(&unrelated) == leaf);
    try { combined->minus_key(&throwing); CHECK(false); }
    catch (...) { CHECK(std::current_exception() == failure); }

    class Dispatcher final : public CoroutineDispatcher {
    public:
        void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> block) const override { block->run(); }
    };
    class DispatcherSubtypeKey final : public AbstractCoroutineContextKey<CoroutineDispatcher, Dispatcher> {
    public:
        DispatcherSubtypeKey() : AbstractCoroutineContextKey(&CoroutineDispatcher::KEY,
            [](std::shared_ptr<CoroutineContext::Element> element) { return std::dynamic_pointer_cast<Dispatcher>(element); }) {}
    };
    class IdentityInterceptor final : public ContinuationInterceptor {
    public:
        std::shared_ptr<Continuation<void*>> intercept_continuation(std::shared_ptr<Continuation<void*>> continuation) override {
            return continuation;
        }
    };
    auto dispatcher = std::make_shared<Dispatcher>();
    auto interceptor = std::make_shared<IdentityInterceptor>();
    IdentityInterceptor borrowed_interceptor;
    CHECK(!borrowed_interceptor.get(&unrelated));
    CHECK(!borrowed_interceptor.get(&derived_key));
    DispatcherSubtypeKey subtype;
    CHECK(dispatcher->get(ContinuationInterceptor::type_key) == dispatcher);
    CHECK(dispatcher->get(&CoroutineDispatcher::KEY) == dispatcher && dispatcher->get(&subtype) == dispatcher);
    CHECK(!interceptor->get(&CoroutineDispatcher::KEY));
    CHECK(interceptor->minus_key(&CoroutineDispatcher::KEY) == interceptor);
    CHECK(dispatcher->minus_key(&subtype) == EmptyCoroutineContext::instance());
    CHECK(dispatcher->minus_key(&CoroutineDispatcher::KEY) == EmptyCoroutineContext::instance());
    auto dispatcher_context = leaf->operator+(dispatcher);
    CHECK(dispatcher_context->get(&subtype) == dispatcher);
    CHECK(dispatcher_context->minus_key(&CoroutineDispatcher::KEY) == leaf);
    CHECK(dispatcher_context->minus_key(&leaf_key) == dispatcher);
    auto replacement = dispatcher_context->operator+(interceptor);
    CHECK(!replacement->get(&CoroutineDispatcher::KEY));
    CHECK(replacement->get(ContinuationInterceptor::type_key) == interceptor);
    CHECK(replacement->minus_key(ContinuationInterceptor::type_key) == leaf);
    // Ordinary Element retains its exact-key behavior unless it opts into the source helpers.
    auto exact = std::make_shared<AbstractCoroutineContextElement>(&root);
    CHECK(!exact->get(&derived_key) && exact->minus_key(&derived_key) == exact);

}

// SafeCollector.common.kt:22-97, Native SafeCollector.kt:16-24.
// Scope ancestry is a runtime type contract, not the JobSupport scoped flag.
void safe_collector_ancestry_contract() {
    using kotlinx::coroutines::flow::internal::transitive_coroutine_parent;
    class PretendScopedJob final : public JobSupport {
    public:
        explicit PretendScopedJob(std::shared_ptr<Job> parent) : JobSupport(true), parent_(std::move(parent)) {}
        mutable int parent_reads = 0;
        bool is_scoped_coroutine() const override { return true; }
        std::shared_ptr<Job> get_parent() const override { ++parent_reads; return parent_; }
    private:
        std::shared_ptr<Job> parent_;
    };
    auto root = std::make_shared<JobSupport>(true);
    auto impostor = std::make_shared<PretendScopedJob>(root);
    CHECK(transitive_coroutine_parent(impostor, root) == impostor);
    CHECK(impostor->parent_reads == 0);
    CHECK(transitive_coroutine_parent(impostor, impostor) == impostor && impostor->parent_reads == 0);
    CHECK(!transitive_coroutine_parent(nullptr, root));
    CHECK(transitive_coroutine_parent(root, nullptr) == root);

    auto completion = make_continuation<void*>(root, [](Result<void*>) {});
    auto outer = std::make_shared<kotlinx::coroutines::internal::ScopeCoroutine<void*>>(root, completion);
    outer->init_parent_job_if_needed();
    auto typed_completion = make_continuation<int>(outer->get_coroutine_context(), [](Result<int>) {});
    auto typed = std::make_shared<kotlinx::coroutines::internal::ScopeCoroutine<int>>(
        outer->get_coroutine_context(), typed_completion);
    typed->init_parent_job_if_needed();
    CHECK(typed->get_parent() == outer && outer->get_parent() == root);
    CHECK(transitive_coroutine_parent(typed, root) == root);
    CHECK(transitive_coroutine_parent(typed, outer) == outer);
    CHECK(transitive_coroutine_parent(typed, typed) == typed);
    CHECK(transitive_coroutine_parent(typed, nullptr) == root);
    auto unrelated = std::make_shared<JobSupport>(true);
    CHECK(transitive_coroutine_parent(typed, unrelated) == root);
    auto orphan_completion = make_continuation<void*>(EmptyCoroutineContext::instance(), [](Result<void*>) {});
    auto orphan = std::make_shared<kotlinx::coroutines::internal::ScopeCoroutine<void*>>(
        EmptyCoroutineContext::instance(), orphan_completion);
    orphan->init_parent_job_if_needed();
    CHECK(!transitive_coroutine_parent(orphan, nullptr));
    CHECK(!transitive_coroutine_parent(orphan, root));

    class EmissionCompletion final : public Continuation<void*> {
    public:
        std::shared_ptr<CoroutineContext> context;
        std::shared_ptr<CoroutineContext> get_context() const override { return context; }
        void resume_with(Result<void*>) override {}
    };
    class Collector final : public flow::FlowCollector<int> {
    public:
        int emits = 0;
        std::exception_ptr failure;
        void* emit(int value, Continuation<void*>*) override {
            if (failure) std::rethrow_exception(failure);
            emits += value;
            return nullptr;
        }
    };
    Collector downstream;
    flow::internal::SafeCollector<int> safe(&downstream, root);
    EmissionCompletion frame;
    frame.context = typed->get_coroutine_context();
    CHECK(safe.emit(1, &frame) == nullptr && downstream.emits == 1);
    CHECK(safe.emit(2, &frame) == nullptr && downstream.emits == 3);
    frame.context = impostor;
    bool rejected = false;
    try { safe.emit(4, &frame); }
    catch (const IllegalStateException& error) {
        rejected = std::string(error.what()).find("Emission from another coroutine is detected") != std::string::npos;
    }
    CHECK(rejected && downstream.emits == 3 && impostor->parent_reads == 0);
    frame.context = unrelated;
    rejected = false;
    try { safe.emit(8, &frame); } catch (const IllegalStateException&) { rejected = true; }
    CHECK(rejected && downstream.emits == 3);
    // Failed validation does not replace lastEmissionContext; the valid scope can emit again.
    frame.context = typed->get_coroutine_context();
    CHECK(safe.emit(16, &frame) == nullptr && downstream.emits == 19);
    auto failure = std::make_exception_ptr(std::runtime_error("actual downstream failure"));
    downstream.failure = failure;
    try { safe.emit(32, &frame); CHECK(false); }
    catch (...) { CHECK(std::current_exception() == failure); }
    downstream.failure = nullptr;
    flow::internal::SafeCollector<int> empty_safe(&downstream, EmptyCoroutineContext::instance());
    frame.context = orphan->get_coroutine_context();
    CHECK(empty_safe.emit(64, &frame) == nullptr && downstream.emits == 83);
    frame.context = root;
    rejected = false;
    try { empty_safe.emit(128, &frame); } catch (const IllegalStateException&) { rejected = true; }
    CHECK(rejected && downstream.emits == 83);
    // The actual scope Job relationships are disposed after these synchronous ancestry checks.
    typed->cancel();
    outer->cancel();
    orphan->cancel();
    root->cancel();
}

// Source contract: SafeCollector.common.kt:32-33 uses checked Job casts,
// while Native SafeCollector.kt:16-24 validates before downstream emission.
void safe_collector_checked_job_cast_contract() {
    class NonJob final : public AbstractCoroutineContextElement {
    public:
        NonJob() : AbstractCoroutineContextElement(Job::type_key) {}
    };
    class Collector final : public flow::FlowCollector<int> {
    public:
        int emits = 0;
        void* emit(int value, Continuation<void*>*) override { emits += value; return nullptr; }
    };
    auto empty = EmptyCoroutineContext::instance();
    auto non_job = std::make_shared<NonJob>();
    auto job = std::make_shared<JobSupport>(true);
    Collector downstream;
    // A failed non-null cast must not become a missing Job, including when
    // both contexts return the same malformed element for the Job key.
    for (const auto& collect : std::vector<std::shared_ptr<CoroutineContext>>{empty, non_job, job}) {
        flow::internal::SafeCollector<int> safe(&downstream, collect);
        CHECK(safe.get_collector() == &downstream && safe.get_collect_context() == collect);
        CHECK(safe.get_collect_context_size() == (collect == empty ? 0 : 1));
        auto malformed_completion = make_continuation<void*>(non_job, [](Result<void*>) {});
        for (int attempt = 0; attempt < 2; ++attempt) {
            bool cast_rejected = false;
            try { safe.emit(1, malformed_completion.get()); }
            catch (const std::bad_cast&) { cast_rejected = true; }
            CHECK(cast_rejected && downstream.emits == 0);
        }
    }
    // The nullable collectJob cast accepts null, but rejects a non-Job object
    // even when the emitting element is an actual Job.
    flow::internal::SafeCollector<int> invalid_collect(&downstream, non_job);
    auto job_completion = make_continuation<void*>(job, [](Result<void*>) {});
    bool cast_rejected = false;
    try { invalid_collect.emit(1, job_completion.get()); }
    catch (const std::bad_cast&) { cast_rejected = true; }
    CHECK(cast_rejected && downstream.emits == 0);
    flow::internal::SafeCollector<int> valid_collect(&downstream, job);
    CHECK(valid_collect.emit(2, job_completion.get()) == nullptr && downstream.emits == 2);
    auto empty_completion = make_continuation<void*>(empty, [](Result<void*>) {});
    flow::internal::SafeCollector<int> empty_collect(&downstream, empty);
    CHECK(empty_collect.emit(4, empty_completion.get()) == nullptr && downstream.emits == 6);
}

// ChannelFlow.kt:118-121 uses CoroutineScope.kt:279-288 directly: the
// scoped continuation's caller frame is the actual caller, with no intermediary.
void channel_scope_contract() {
    class FrameCompletion final : public Continuation<void*>, public kotlinx::coroutines::internal::CoroutineStackFrame {
    public:
        int resumes = 0;
        std::exception_ptr failure;
        std::shared_ptr<CoroutineContext> get_context() const override { return EmptyCoroutineContext::instance(); }
        void resume_with(Result<void*> result) override {
            ++resumes;
            failure = result.exception_or_null();
            if (!failure) CHECK(result.get_or_throw() == nullptr);
        }
        kotlinx::coroutines::internal::CoroutineStackFrame* get_caller_frame() const override { return nullptr; }
        kotlinx::coroutines::internal::StackTraceElement* get_stack_trace_element() const override { return nullptr; }
    };
    for (bool suspended : {false, true}) for (bool fails : {false, true}) {
        FrameCompletion caller;
        std::shared_ptr<Continuation<void*>> paused;
        auto failure = std::make_exception_ptr(std::runtime_error("scope source failure"));
        int starts = 0;
        std::exception_ptr observed;
        void* outcome = nullptr;
        try {
            outcome = flow::internal::collect_in_scope(
                [&](CoroutineScope* receiver, std::shared_ptr<Continuation<void*>> continuation) -> void* {
                    ++starts;
                    auto* scope = dynamic_cast<kotlinx::coroutines::internal::ScopeCoroutine<void*>*>(receiver);
                    CHECK(scope && scope == dynamic_cast<kotlinx::coroutines::internal::ScopeCoroutine<void*>*>(continuation.get()));
                    CHECK(scope->get_caller_frame() == &caller);
                    CHECK(scope->u_cont.get() == &caller);
                    CHECK(scope->get_coroutine_context()->get(Job::type_key).get() ==
                          static_cast<CoroutineContext::Element*>(scope));
                    if (suspended) {
                        paused = std::move(continuation);
                        return intrinsics::get_COROUTINE_SUSPENDED();
                    }
                    if (fails) std::rethrow_exception(failure);
                    return nullptr;
                }, &caller);
        } catch (...) { observed = std::current_exception(); }
        CHECK(starts == 1 && caller.resumes == 0);
        if (suspended) {
            CHECK(!observed && intrinsics::is_coroutine_suspended(outcome) && paused);
            paused->resume_with(fails ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
            CHECK(caller.resumes == 1 && caller.failure == (fails ? failure : nullptr));
        } else CHECK(observed == (fails ? failure : nullptr) && outcome == nullptr);
    }

    // A successful body still waits for its actual attached child before
    // returning to the original caller. Child failure becomes scope failure.
    for (bool fails : {false, true}) {
        FrameCompletion caller;
        std::shared_ptr<CompletableJob> child;
        auto failure = std::make_exception_ptr(std::runtime_error("scope child failure"));
        auto outcome = flow::internal::collect_in_scope(
            [&](CoroutineScope* receiver, std::shared_ptr<Continuation<void*>>) -> void* {
                child = make_job(receiver->get_job());
                return nullptr;
            }, &caller);
        CHECK(intrinsics::is_coroutine_suspended(outcome) && caller.resumes == 0 && child->is_active());
        if (fails) child->complete_exceptionally(failure);
        else child->complete();
        CHECK(caller.resumes == 1 && caller.failure == (fails ? failure : nullptr));
    }
    // Child completion must use the actual compiler caller's interceptor.
    // Wrapping that caller in a non-frame completion silently bypasses it.
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
    class CallerFrame final : public ContinuationImpl {
    public:
        CallerFrame(std::shared_ptr<Continuation<void*>> completion, std::shared_ptr<CoroutineContext> context)
            : ContinuationImpl(std::move(completion), std::move(context)) {}
        int resumes = 0;
        void* invoke_suspend(Result<void*> result) override {
            ++resumes;
            return result.get_or_throw();
        }
    };
    for (bool fails : {false, true}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        auto outer = std::make_shared<Completion>();
        auto caller = std::make_shared<CallerFrame>(outer, dispatcher);
        std::shared_ptr<CompletableJob> child;
        auto failure = std::make_exception_ptr(std::runtime_error("intercepted child failure"));
        auto outcome = flow::internal::collect_in_scope(
            [&](CoroutineScope* receiver, std::shared_ptr<Continuation<void*>>) -> void* {
                child = make_job(receiver->get_job());
                return nullptr;
            }, caller.get());
        CHECK(intrinsics::is_coroutine_suspended(outcome) && caller->resumes == 0 && outer->resumes == 0);
        if (fails) child->complete_exceptionally(failure);
        else child->complete();
        CHECK(caller->resumes == 0 && outer->resumes == 0 && dispatcher->queue.size() == 1);
        dispatcher->drain();
        CHECK(caller->resumes == 1 && outer->resumes == 1 && outer->failure == (fails ? failure : nullptr));
    }
}

// SendingCollector.kt:12-15 delegates directly to the actual channel send.
void sending_collector_contract() {
    for (bool cancelled : {false, true}) {
        auto channel = create_channel<int>(0);
        auto failure = cancelled
            ? std::make_exception_ptr(CancellationException("original send cancellation"))
            : std::make_exception_ptr(std::runtime_error("original send failure"));
        if (cancelled) channel->cancel(failure);
        else channel->close(failure);
        flow::internal::SendingCollector<int> collector(channel.get());
        Completion completion;
        std::exception_ptr observed;
        try { collector.emit(17, &completion); }
        catch (...) { observed = std::current_exception(); }
        CHECK((observed == failure && completion.resumes == 0) ||
              (!observed && completion.resumes == 1 && completion.failure == failure));
    }

    auto channel = create_channel<int>(0);
    flow::internal::SendingCollector<int> collector(channel.get());
    for (int value : {19, 23}) {
        Completion completion;
        CHECK(intrinsics::is_coroutine_suspended(collector.emit(value, &completion)));
        CHECK(completion.resumes == 0);
        auto received = channel->try_receive();
        CHECK(received.is_success() && received.get_or_throw() == value);
        CHECK(completion.resumes == 1 && !completion.failure && !completion.value);
    }
    Completion cancelled;
    CHECK(intrinsics::is_coroutine_suspended(collector.emit(29, &cancelled)));
    auto failure = std::make_exception_ptr(CancellationException("waiting send cancelled"));
    channel->cancel(failure);
    CHECK(cancelled.resumes == 1 && cancelled.failure == failure);
    CHECK(channel->try_receive().is_closed());

    // The collector borrows the channel and forwards the actual value object.
    BufferedChannel<std::shared_ptr<int>> borrowed(1);
    flow::internal::SendingCollector<std::shared_ptr<int>> forwarding(&borrowed);
    auto resource = std::make_shared<int>(31);
    auto identity = resource.get();
    std::weak_ptr<int> lifetime = resource;
    Completion done;
    CHECK(forwarding.emit(std::move(resource), &done) == nullptr);
    CHECK(!resource && !lifetime.expired() && done.resumes == 0);
    {
        auto received = borrowed.try_receive();
        CHECK(received.is_success() && received.get_or_throw().get() == identity);
    }
    CHECK(lifetime.expired());
}

// Combine.kt:17-80: real child collection, suspending receive/transform and batching.
void combine_contract() {
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
    class ContextCompletion final : public Continuation<void*> {
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
    for (bool copy_array : {false, true}) for (bool fails : {false, true}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        ContextCompletion completion;
        completion.context = dispatcher;
        auto left = create_channel<std::any>(8);
        auto right = create_channel<std::any>(8);
        std::vector<std::shared_ptr<flow::Flow<std::any>>> sources{
            flow::receive_as_flow<std::any>(left), flow::receive_as_flow<std::any>(right)};
        std::vector<std::vector<int>> batches;
        std::shared_ptr<Continuation<void*>> paused;
        const std::vector<std::any>* suspended_values = nullptr;
        auto resource = std::make_shared<int>(43);
        std::weak_ptr<int> lifetime = resource;
        int factories = 0;
        auto outcome = flow::internal::combine_internal<int>(nullptr, sources,
            [&]() -> std::vector<std::any>* {
                ++factories;
                return copy_array ? new std::vector<std::any>(2) : nullptr;
            },
            [&, resource](flow::FlowCollector<int>*, const std::vector<std::any>& values,
                          Continuation<void*>* frame) -> void* {
                CHECK(*resource == 43);
                batches.push_back({std::any_cast<int>(values[0]), std::any_cast<int>(values[1])});
                if (batches.size() == 1) {
                    paused = kotlinx::coroutines::internal::retain_continuation(frame);
                    suspended_values = &values;
                    return intrinsics::get_COROUTINE_SUSPENDED();
                }
                return nullptr;
            }, &completion);
        resource.reset();
        CHECK(intrinsics::is_coroutine_suspended(outcome));
        dispatcher->drain();
        CHECK(batches.empty() && completion.resumes == 0);
        CHECK(left->try_send(1).is_success());
        dispatcher->drain();
        CHECK(batches.empty());
        CHECK(right->try_send(10).is_success());
        dispatcher->drain();
        CHECK(batches == std::vector<std::vector<int>>({{1, 10}}) && paused && factories == 1);
        CHECK(left->try_send(2).is_success() && right->try_send(20).is_success());
        dispatcher->drain();
        CHECK(std::any_cast<int>((*suspended_values)[0]) == 1);
        CHECK(std::any_cast<int>((*suspended_values)[1]) == 10);
        CHECK(!lifetime.expired() && completion.resumes == 0);
        auto failure = std::make_exception_ptr(std::runtime_error("resumed combine transform failure"));
        paused->resume_with(fails ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
        dispatcher->drain();
        if (!fails) {
            CHECK(batches == std::vector<std::vector<int>>({{1, 10}, {2, 20}}) && factories == 2);
            // Exercise Byte epoch wrap while both source coroutines remain active.
            for (int epoch = 0; epoch < 260; ++epoch) {
                CHECK(left->try_send(100 + epoch).is_success());
                CHECK(right->try_send(1000 + epoch).is_success());
                dispatcher->drain();
                CHECK(batches.back() == std::vector<int>({100 + epoch, 1000 + epoch}));
                CHECK(completion.resumes == 0);
            }
            left->close();
            right->close();
            dispatcher->drain();
        }
        CHECK(completion.resumes == 1 && completion.failure == (fails ? failure : nullptr));
        CHECK(lifetime.expired()); // Even while the completed frame is independently retained.
    }
}

// Combine.kt:82-139: second completion cancels first collection, but not downstream.
void zip_contract() {
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
    class ContextCompletion final : public Continuation<void*> {
    public:
        std::shared_ptr<CoroutineContext> context;
        int resumes = 0;
        std::exception_ptr failure;
        std::shared_ptr<CoroutineContext> get_context() const override { return context; }
        void resume_with(Result<void*> result) override {
            ++resumes;
            failure = result.exception_or_null();
        }
    };
    class PausedCollector final : public flow::FlowCollector<std::shared_ptr<int>> {
    public:
        std::shared_ptr<Continuation<void*>> paused;
        std::shared_ptr<Job> emission_job;
        std::shared_ptr<int> value;
        void* emit(std::shared_ptr<int> received, Continuation<void*>* frame) override {
            value = std::move(received);
            emission_job = std::dynamic_pointer_cast<Job>(frame->get_context()->get(Job::type_key));
            paused = kotlinx::coroutines::internal::retain_continuation(frame);
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
    };
    for (int failure_point : {0, 1, 2}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        ContextCompletion completion;
        completion.context = dispatcher;
        auto left = create_channel<int>(8);
        auto right = create_channel<int>(8);
        std::shared_ptr<Continuation<void*>> transform_frame;
        std::shared_ptr<Job> transform_job;
        auto resource = std::make_shared<int>(47);
        auto identity = resource.get();
        std::weak_ptr<int> lifetime = resource;
        auto zipped = flow::internal::zip_impl<int, int, std::shared_ptr<int>>(
            flow::receive_as_flow<int>(left), flow::receive_as_flow<int>(right),
            std::function<void*(int, int, Continuation<void*>*)>(
                [&, resource](int first, int second, Continuation<void*>* frame) -> void* {
                    CHECK(first == 1 && second == 10 && resource.get() == identity);
                    transform_job = std::dynamic_pointer_cast<Job>(frame->get_context()->get(Job::type_key));
                    transform_frame = kotlinx::coroutines::internal::retain_continuation(frame);
                    return intrinsics::get_COROUTINE_SUSPENDED();
                }));
        PausedCollector collector;
        CHECK(intrinsics::is_coroutine_suspended(zipped->collect(&collector, &completion)));
        resource.reset();
        zipped.reset();
        dispatcher->drain();
        CHECK(left->try_send(1).is_success() && right->try_send(10).is_success());
        right->close();
        dispatcher->drain();
        CHECK(transform_frame && transform_job->is_active() && completion.resumes == 0);
        CHECK(!lifetime.expired());
        auto failure = std::make_exception_ptr(std::runtime_error("zip transform failure"));
        if (failure_point == 1) transform_frame->resume_with(Result<void*>::failure(failure));
        else transform_frame->resume_with(Result<void*>::success(new std::shared_ptr<int>(lifetime.lock())));
        dispatcher->drain();
        if (failure_point != 1) {
            CHECK(collector.paused && collector.value.get() == identity);
            CHECK(collector.emission_job == transform_job && transform_job->is_active());
            CHECK(completion.resumes == 0);
            collector.paused->resume_with(failure_point == 2
                ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
            dispatcher->drain();
            collector.value.reset();
        }
        CHECK(completion.resumes == 1 && completion.failure == (failure_point ? failure : nullptr));
        CHECK(lifetime.expired());
    }
}

void cancellation_contract() {
    for (int kind : {0, 1, 2}) {
        RecordingChannel channel;
        std::exception_ptr cause;
        if (kind == 1) cause = std::make_exception_ptr(std::runtime_error("consumer failure"));
        if (kind == 2) cause = std::make_exception_ptr(CancellationException("already cancelled"));
        cancel_consumed(&channel, cause);
        CHECK(channel.cancellations == 1);
        if (kind == 1) check_wrapped(channel.cancellation_cause, cause);
        else CHECK(channel.cancellation_cause == cause);
    }

    RecordingChannel channel;
    int value = 71;
    int& reference = consume<int, int&>(&channel, [&value](ReceiveChannel<int>*) -> int& { return value; });
    CHECK(&reference == &value && channel.cancellations == 1);
    RecordingChannel move_channel;
    auto owned = consume<int, std::unique_ptr<int>>(&move_channel,
        [](ReceiveChannel<int>*) { return std::make_unique<int>(93); });
    CHECK(*owned == 93 && move_channel.cancellations == 1);

    for (bool returns_value : {false, true}) for (bool block_fails : {false, true}) {
        RecordingChannel failing;
        auto block_failure = std::make_exception_ptr(std::runtime_error("block failure"));
        auto finally_failure = std::make_exception_ptr(std::runtime_error("finally failure"));
        failing.cancellation_failure = finally_failure;
        std::exception_ptr observed;
        try {
            if (returns_value) {
                (void)consume<int, int>(&failing, [&](ReceiveChannel<int>*) {
                    if (block_fails) std::rethrow_exception(block_failure);
                    return 7;
                });
            } else {
                consume<int>(&failing, std::function<void(ReceiveChannel<int>*)>([&](ReceiveChannel<int>*) {
                    if (block_fails) std::rethrow_exception(block_failure);
                }));
            }
        } catch (...) { observed = std::current_exception(); }
        CHECK(observed == finally_failure && failing.cancellations == 1);
        if (block_fails) check_wrapped(failing.cancellation_cause, block_failure);
        else CHECK(!failing.cancellation_cause);
    }
}

void iteration_contract() {
    auto channel = std::make_shared<RecordingChannel>();
    auto completion = std::make_shared<Completion>();
    std::vector<int> values;
    auto capture = std::make_shared<int>(41);
    auto identity = capture.get();
    std::weak_ptr<int> lifetime = capture;
    auto result = consume_each<int>(channel.get(), [capture, identity, &values](int value) {
        CHECK(capture.get() == identity && *capture == 41);
        values.push_back(value);
    }, completion.get());
    capture.reset();
    CHECK(intrinsics::is_coroutine_suspended(result));
    CHECK(!completion->resumes && !channel->cancellations && !lifetime.expired());
    CHECK(channel->try_send(1).is_success());
    CHECK(values == std::vector<int>{1} && !completion->resumes);
    CHECK(channel->try_send(2).is_success());
    CHECK(values == std::vector<int>({1, 2}) && !completion->resumes);
    channel->close(nullptr);
    CHECK(completion->resumes == 1 && !completion->failure && !completion->value);
    CHECK(channel->cancellations == 1 && !channel->cancellation_cause && lifetime.expired());

    for (bool action_fails : {false, true}) {
        auto failed = std::make_shared<RecordingChannel>();
        auto done = std::make_shared<Completion>();
        auto failure = std::make_exception_ptr(std::runtime_error("iteration failure"));
        CHECK(intrinsics::is_coroutine_suspended(consume_each<int>(failed.get(),
            [failure, action_fails](int) { if (action_fails) std::rethrow_exception(failure); }, done.get())));
        if (action_fails) CHECK(failed->try_send(5).is_success());
        else failed->close(failure);
        CHECK(done->resumes == 1 && done->failure == failure);
        CHECK(failed->cancellations == 1);
        check_wrapped(failed->cancellation_cause, failure);
    }
}

// ChannelFlow.kt:26-30,55-56,69-100,188: inherited defaults, public
// operator removal and the shared producer lambda are callable on the real types.
void channel_flow_surface_contract() {
    auto upstream = flow::unsafe_flow<int>(std::function<void(flow::FlowCollector<int>*)>(
        [](flow::FlowCollector<int>*) {}));
    auto operation = std::make_shared<flow::internal::ChannelFlowOperatorImpl<int>>(upstream);
    CHECK(operation->fuse() == operation.get());
    CHECK(operation->fuse(EmptyCoroutineContext::instance()) == operation.get());
    std::unique_ptr<flow::Flow<int>> buffered(operation->fuse(EmptyCoroutineContext::instance(), 3));
    auto* buffered_channel_flow = dynamic_cast<flow::internal::ChannelFlow<int>*>(buffered.get());
    CHECK(buffered_channel_flow && buffered_channel_flow->capacity() == 3);
    CHECK(buffered_channel_flow->on_buffer_overflow() == BufferOverflow::SUSPEND);
    CHECK(operation->drop_channel_operators() == upstream.get());
    auto collect = operation->get_collect_to_fun();
    CHECK(static_cast<bool>(collect));
    std::weak_ptr<flow::Flow<int>> lifetime = operation;
    operation.reset();
    CHECK(!lifetime.expired());
    collect = {};
    CHECK(lifetime.expired());
}

// ChannelFlow.kt:54-56,151-152,190-191: the source lambda forwards to
// the real producer scope, retains its receiver across send suspension, and
// releases its captures on success, failure and cancellation.
void channel_flow_collect_lambda_contract() {
    for (int outcome : {0, 1, 2}) {
        auto resource = std::make_shared<int>(117);
        std::weak_ptr<int> resource_lifetime = resource;
        int calls = 0;
        auto upstream = flow::unsafe_flow<int>(
            std::function<void*(flow::FlowCollector<int>*, Continuation<void*>*)>(
                [resource, &calls](flow::FlowCollector<int>* collector, Continuation<void*>* continuation) -> void* {
                    CHECK(*resource == 117);
                    ++calls;
                    return collector->emit(*resource, continuation);
                }));
        auto operation = std::make_shared<flow::internal::ChannelFlowOperatorImpl<int>>(upstream);
        std::weak_ptr<flow::Flow<int>> lifetime = operation;
        auto collect = operation->get_collect_to_fun();
        auto channel = create_channel<int>(0);
        auto producer = std::make_shared<ProducerCoroutine<int>>(EmptyCoroutineContext::instance(), channel);
        auto completion = std::make_shared<Completion>();
        CHECK(intrinsics::is_coroutine_suspended(collect(producer.get(), completion)));
        CHECK(calls == 1 && !completion->resumes && channel->is_empty());
        operation.reset();
        upstream.reset();
        resource.reset();
        collect = {};
        CHECK(!lifetime.expired() && !resource_lifetime.expired());
        std::exception_ptr failure;
        if (outcome == 0) {
            auto value = channel->try_receive();
            CHECK(value.is_success() && value.get_or_throw() == 117);
        } else {
            failure = outcome == 1
                ? std::make_exception_ptr(std::runtime_error("collect lambda send failed"))
                : std::make_exception_ptr(CancellationException("collect lambda cancelled"));
            if (outcome == 1) channel->close(failure);
            else channel->cancel(failure);
        }
        CHECK(completion->resumes == 1 && completion->failure == failure && calls == 1);
        CHECK(lifetime.expired() && resource_lifetime.expired());
    }
}

void list_contract() {
    for (bool suspended : {false, true}) {
        auto channel = std::make_shared<RecordingChannel>();
        auto completion = std::make_shared<Completion>();
        channel->try_send(3);
        if (!suspended) { channel->try_send(4); channel->close(nullptr); }
        auto result = to_list<int>(channel.get(), completion.get());
        if (suspended) {
            CHECK(intrinsics::is_coroutine_suspended(result));
            CHECK(!completion->resumes && !channel->cancellations);
            CHECK(channel->try_send(4).is_success());
            CHECK(!completion->resumes);
            channel->close(nullptr);
            CHECK(completion->resumes == 1 && !completion->failure);
            result = completion->value;
        } else CHECK(!completion->resumes);
        std::unique_ptr<std::vector<int>> values(static_cast<std::vector<int>*>(result));
        CHECK(*values == std::vector<int>({3, 4}));
        CHECK(channel->cancellations == 1);
    }
    auto channel = std::make_shared<RecordingChannel>();
    auto completion = std::make_shared<Completion>();
    CHECK(intrinsics::is_coroutine_suspended(to_list<int>(channel.get(), completion.get())));
    auto failure = std::make_exception_ptr(std::runtime_error("closed list failure"));
    channel->close(failure);
    CHECK(completion->resumes == 1 && completion->failure == failure && !completion->value);
    check_wrapped(channel->cancellation_cause, failure);

    auto source = flow::consume_as_flow<int>(channel);
    auto scope = create_coroutine_scope(EmptyCoroutineContext::instance());
    auto first = flow::produce_in<int>(source, scope.get());
    CHECK(first.get() == channel.get());
    bool caught = false;
    try { (void)flow::produce_in<int>(source, scope.get()); }
    catch (const IllegalStateException& exception) {
        caught = std::string(exception.what()) == "ReceiveChannel.consumeAsFlow can be collected just once";
    }
    CHECK(caught);
}

void suspended_action_contract() {
    for (bool fails : {false, true}) {
        auto channel = std::make_shared<RecordingChannel>();
        auto completion = std::make_shared<Completion>();
        channel->try_send(6);
        channel->try_send(7);
        channel->close(nullptr);
        Continuation<void*>* paused = nullptr;
        std::vector<int> values;
        auto capture = std::make_shared<int>(19);
        std::weak_ptr<int> lifetime = capture;
        auto result = consume_each<int>(channel.get(),
            [capture, &paused, &values](int value, Continuation<void*>* continuation) -> void* {
                CHECK(*capture == 19);
                values.push_back(value);
                paused = continuation;
                return intrinsics::get_COROUTINE_SUSPENDED();
            }, completion.get());
        capture.reset();
        CHECK(intrinsics::is_coroutine_suspended(result));
        CHECK(values == std::vector<int>{6} && !lifetime.expired());
        CHECK(!completion->resumes && !channel->cancellations);
        auto failure = std::make_exception_ptr(std::runtime_error("resumed action failure"));
        paused->resume_with(fails ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
        if (!fails) {
            CHECK(values == std::vector<int>({6, 7}) && !completion->resumes);
            CHECK(!channel->cancellations && !lifetime.expired());
            paused->resume_with(Result<void*>::success(nullptr));
        }
        CHECK(completion->resumes == 1 && completion->failure == (fails ? failure : nullptr));
        CHECK(channel->cancellations == 1 && lifetime.expired());
        if (fails) check_wrapped(channel->cancellation_cause, failure);
    }
}

void list_resource_contract() {
    for (bool fails : {false, true}) {
        auto channel = create_channel<std::shared_ptr<int>>(8);
        auto completion = std::make_shared<Completion>();
        auto resource = std::make_shared<int>(29);
        auto identity = resource.get();
        std::weak_ptr<int> lifetime = resource;
        CHECK(channel->try_send(resource).is_success());
        resource.reset();
        CHECK(intrinsics::is_coroutine_suspended(to_list<std::shared_ptr<int>>(channel.get(), completion.get())));
        CHECK(!lifetime.expired() && !completion->resumes);
        auto failure = std::make_exception_ptr(std::runtime_error("resource list failure"));
        channel->close(fails ? failure : nullptr);
        CHECK(completion->resumes == 1 && completion->failure == (fails ? failure : nullptr));
        if (fails) CHECK(lifetime.expired() && !completion->value);
        else {
            std::unique_ptr<std::vector<std::shared_ptr<int>>> values(
                static_cast<std::vector<std::shared_ptr<int>>*>(completion->value));
            CHECK(values->size() == 1 && values->front().get() == identity && *values->front() == 29);
            CHECK(!lifetime.expired());
            values.reset();
            CHECK(lifetime.expired());
        }
    }
}
}

int main() {
    try {
        job_cancellation_equality_contract();
        job_cancellation_hash_contract();
        polymorphic_context_contract();
        safe_collector_ancestry_contract();
        safe_collector_checked_job_cast_contract();
        channel_scope_contract();
        channel_flow_surface_contract();
        channel_flow_collect_lambda_contract();
        sending_collector_contract();
        combine_contract();
        zip_contract();
        cancellation_contract();
        iteration_contract();
        list_contract();
        suspended_action_contract();
        list_resource_contract();
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}
