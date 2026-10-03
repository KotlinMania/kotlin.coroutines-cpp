#include "kotlinx/coroutines/flow/Share.hpp"
#include "kotlinx/coroutines/flow/SharedFlow.hpp"
#include "kotlinx/coroutines/flow/StateFlow.hpp"
#include "kotlinx/coroutines/flow/SharingStarted.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include <atomic>
#include "kotlinx/coroutines/testing/TestBase.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::flow;
using namespace kotlinx::coroutines::testing;
using flow_abort = kotlinx::coroutines::flow::internal::AbortFlowException;

void test_subscription_count_tracking() {
    auto shared = make_mutable_shared_flow<int>(/*replay=*/1, /*extra_buffer_capacity=*/0);
    assert_equals(0, shared->subscription_count()->value());
    assert_equals(0, shared->get_subscription_count()->value());

    std::atomic<bool> stop_sub1{false};
    std::thread t1([&]() {
        class CancellableCollector : public FlowCollector<int> {
            std::atomic<bool>& stop_;
        public:
            explicit CancellableCollector(std::atomic<bool>& stop) : stop_(stop) {}
            void* emit(int, Continuation<void*>*) override {
                if (stop_.load()) throw flow_abort(this);
                return nullptr;
            }
        };
        CancellableCollector col(stop_sub1);
        try {
            shared->collect(&col, nullptr);
        } catch (const flow_abort&) {}
    });

    for (int i = 0; i < 50 && shared->subscription_count()->value() == 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    assert_equals(1, shared->subscription_count()->value());

    std::atomic<bool> stop_sub2{false};
    std::thread t2([&]() {
        class CancellableCollector : public FlowCollector<int> {
            std::atomic<bool>& stop_;
        public:
            explicit CancellableCollector(std::atomic<bool>& stop) : stop_(stop) {}
            void* emit(int, Continuation<void*>*) override {
                if (stop_.load()) throw flow_abort(this);
                return nullptr;
            }
        };
        CancellableCollector col(stop_sub2);
        try {
            shared->collect(&col, nullptr);
        } catch (const flow_abort&) {}
    });

    for (int i = 0; i < 50 && shared->subscription_count()->value() < 2; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    assert_equals(2, shared->subscription_count()->value());

    stop_sub1 = true;
    shared->try_emit(999); // wake up collector 1
    if (t1.joinable()) t1.join();

    for (int i = 0; i < 50 && shared->subscription_count()->value() > 1; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    assert_equals(1, shared->subscription_count()->value());

    stop_sub2 = true;
    shared->try_emit(999); // wake up collector 2
    if (t2.joinable()) t2.join();

    for (int i = 0; i < 50 && shared->subscription_count()->value() > 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    assert_equals(0, shared->subscription_count()->value());

    // Also verify StateFlow tracking
    auto state = make_mutable_state_flow<int>(42);
    assert_equals(0, state->subscription_count()->value());
    assert_equals(42, state->value());

    std::cout << "test_subscription_count_tracking passed" << std::endl;
}

inline std::shared_ptr<CoroutineScope> make_test_scope() {
    return create_coroutine_scope(std::dynamic_pointer_cast<CoroutineContext>(make_job()));
}

void test_share_in_eagerly() {
    auto scope = make_test_scope();
    std::atomic<int> emissions{0};

    auto cold = flow::flow<int>([&emissions](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        col->emit(10, c);
        emissions++;
        col->emit(20, c);
        emissions++;
        return nullptr;
    });

    auto shared = share_in(cold, scope.get(), SharingStarted::eagerly(), /*replay=*/2);
    for (int i = 0; i < 50 && emissions.load() < 2; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    assert_equals(2, emissions.load());

    const auto& cache = shared->replay_cache();
    assert_equals(static_cast<size_t>(2), cache.size());
    assert_equals(10, cache[0]);
    assert_equals(20, cache[1]);

    cancel(*scope);
    std::cout << "test_share_in_eagerly passed" << std::endl;
}

void test_share_in_lazily() {
    auto scope = make_test_scope();
    std::atomic<bool> upstream_started{false};

    auto cold = flow::flow<int>([&upstream_started](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        upstream_started = true;
        col->emit(100, c);
        return nullptr;
    });

    auto shared = share_in(cold, scope.get(), SharingStarted::lazily(), /*replay=*/1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    assert_false(upstream_started.load());

    std::atomic<int> received{0};
    std::thread sub([&]() {
        class OnceCollector : public FlowCollector<int> {
            std::atomic<int>& rec_;
        public:
            explicit OnceCollector(std::atomic<int>& r) : rec_(r) {}
            void* emit(int v, Continuation<void*>*) override {
                rec_ = v;
                throw flow_abort(this);
            }
        };
        OnceCollector col(received);
        try {
            shared->collect(&col, nullptr);
        } catch (const flow_abort&) {}
    });

    for (int i = 0; i < 50 && received.load() == 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    assert_true(upstream_started.load());
    assert_equals(100, received.load());

    if (sub.joinable()) sub.join();
    cancel(*scope);
    std::cout << "test_share_in_lazily passed" << std::endl;
}

void test_share_in_while_subscribed_restart_and_cache_reset() {
    auto scope = make_test_scope();
    std::atomic<int> upstream_runs{0};

    auto cold = flow::flow<int>([&upstream_runs](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        upstream_runs++;
        col->emit(upstream_runs.load() * 10, c);
        // keep alive until cancelled
        while (c && c->get_context()) {
            auto job = std::dynamic_pointer_cast<Job>(c->get_context()->get(Job::type_key));
            if (job && !job->is_active()) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return nullptr;
    });

    auto shared = share_in(
        cold,
        scope.get(),
        SharingStarted::while_subscribed(/*stop_timeout_millis=*/50, /*replay_expiration_millis=*/100),
        /*replay=*/1
    );

    assert_equals(0, upstream_runs.load());

    // Subscriber 1
    std::atomic<bool> sub1_stop{false};
    std::atomic<int> sub1_val{0};
    std::thread t1([&]() {
        class StoppableCollector : public FlowCollector<int> {
            std::atomic<bool>& stop_;
            std::atomic<int>& val_;
        public:
            StoppableCollector(std::atomic<bool>& s, std::atomic<int>& v) : stop_(s), val_(v) {}
            void* emit(int v, Continuation<void*>*) override {
                val_ = v;
                while (!stop_.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                throw flow_abort(this);
            }
        };
        StoppableCollector col(sub1_stop, sub1_val);
        try {
            shared->collect(&col, nullptr);
        } catch (const flow_abort&) {}
    });

    for (int i = 0; i < 50 && sub1_val.load() == 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    assert_equals(1, upstream_runs.load());
    assert_equals(10, sub1_val.load());
    assert_false(shared->replay_cache().empty());

    // Unsubscribe
    sub1_stop = true;
    if (t1.joinable()) t1.join();

    // After stop_timeout (50ms) + replay_expiration (100ms), cache should be reset
    for (int i = 0; i < 50 && !shared->replay_cache().empty(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    assert_true(shared->replay_cache().empty());

    cancel(*scope);
    std::cout << "test_share_in_while_subscribed_restart_and_cache_reset passed" << std::endl;
}

void test_state_in_while_subscribed_reset_to_initial() {
    auto scope = make_test_scope();
    std::atomic<int> upstream_runs{0};

    auto cold = flow::flow<int>([&upstream_runs](FlowCollector<int>* col, Continuation<void*>* c) -> void* {
        upstream_runs++;
        col->emit(77, c);
        while (c && c->get_context()) {
            auto job = std::dynamic_pointer_cast<Job>(c->get_context()->get(Job::type_key));
            if (job && !job->is_active()) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return nullptr;
    });

    auto state = state_in(
        cold,
        scope.get(),
        SharingStarted::while_subscribed(/*stop_timeout_millis=*/50, /*replay_expiration_millis=*/100),
        /*initial_value=*/-1
    );

    assert_equals(-1, state->value());

    // Subscriber 1
    std::atomic<bool> sub1_stop{false};
    std::atomic<int> sub1_val{0};
    std::thread t1([&]() {
        class StoppableCollector : public FlowCollector<int> {
            std::atomic<bool>& stop_;
            std::atomic<int>& val_;
        public:
            StoppableCollector(std::atomic<bool>& s, std::atomic<int>& v) : stop_(s), val_(v) {}
            void* emit(int v, Continuation<void*>*) override {
                val_ = v;
                while (!stop_.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                throw flow_abort(this);
            }
        };
        StoppableCollector col(sub1_stop, sub1_val);
        try {
            state->collect(&col, nullptr);
        } catch (const flow_abort&) {}
    });

    for (int i = 0; i < 50 && sub1_val.load() != 77; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    assert_equals(77, state->value());

    // Unsubscribe
    sub1_stop = true;
    if (t1.joinable()) t1.join();

    // After expiration, value resets to initial_value (-1)
    for (int i = 0; i < 50 && state->value() != -1; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    assert_equals(-1, state->value());

    cancel(*scope);
    std::cout << "test_state_in_while_subscribed_reset_to_initial passed" << std::endl;
}

int main() {
    test_subscription_count_tracking();
    test_share_in_eagerly();
    test_share_in_lazily();
    test_share_in_while_subscribed_restart_and_cache_reset();
    test_state_in_while_subscribed_reset_to_initial();
    std::cout << "All SharedFlow/StateFlow/SharingStarted tests passed!" << std::endl;
    return 0;
}
