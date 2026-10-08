/**
 * @file test_sync.cpp
 * @brief Tests for Mutex and Semaphore implementations.
 *
 * Tests the lock-free segment-based implementations transliterated from
 * kotlinx-coroutines-core/common/src/sync/Mutex.kt and Semaphore.kt.
 */

#include <iostream>
#include "kotlinx/coroutines/testing/TestBase.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <deque>
#include "kotlinx/coroutines/JobImpl.hpp"

#include "kotlinx/coroutines/sync/Mutex.hpp"
#include "kotlinx/coroutines/sync/Semaphore.hpp"

using namespace kotlinx::coroutines::sync;
using namespace kotlinx::coroutines::testing;

// Test basic mutex lock/unlock
void test_mutex_basic() {
    std::cout << "test_mutex_basic... ";

    auto mutex = make_mutex(false);

    assert_false(mutex->is_locked());
    assert_true(mutex->try_lock());
    assert_true(mutex->is_locked());
    mutex->unlock();
    assert_false(mutex->is_locked());

    std::cout << "completed\n";
}

// Test mutex owner tracking
void test_mutex_owner() {
    std::cout << "test_mutex_owner... ";

    auto mutex = make_mutex(false);
    void* owner1 = reinterpret_cast<void*>(1);
    void* owner2 = reinterpret_cast<void*>(2);

    assert_true(mutex->try_lock(owner1));
    assert_true(mutex->holds_lock(owner1));
    assert_false(mutex->holds_lock(owner2));
    mutex->unlock(owner1);
    assert_false(mutex->holds_lock(owner1));

    std::cout << "completed\n";
}

// Test mutex reentrant check (should throw)
void test_mutex_reentrant() {
    std::cout << "test_mutex_reentrant... ";

    auto mutex = make_mutex(false);
    void* owner = reinterpret_cast<void*>(1);

    mutex->try_lock(owner);

    bool threw = false;
    try {
        mutex->try_lock(owner); // Should throw
    } catch (const std::logic_error& e) {
        threw = true;
    }
    assert_true(threw);

    mutex->unlock(owner);
    std::cout << "completed\n";
}

// Test mutex created locked
void test_mutex_created_locked() {
    std::cout << "test_mutex_created_locked... ";

    auto mutex = make_mutex(true);
    assert_true(mutex->is_locked());
    assert_false(mutex->try_lock());

    mutex->unlock();
    assert_false(mutex->is_locked());

    std::cout << "completed\n";
}

// Test basic semaphore
void test_semaphore_basic() {
    std::cout << "test_semaphore_basic... ";

    auto sem = create_semaphore(2);

    assert_equals(2, sem->available_permits());
    assert_true(sem->try_acquire());
    assert_equals(1, sem->available_permits());
    assert_true(sem->try_acquire());
    assert_equals(0, sem->available_permits());
    assert_false(sem->try_acquire());

    sem->release();
    assert_equals(1, sem->available_permits());
    sem->release();
    assert_equals(2, sem->available_permits());

    std::cout << "completed\n";
}

// Test semaphore with acquired permits
void test_semaphore_acquired() {
    std::cout << "test_semaphore_acquired... ";

    auto sem = create_semaphore(3, 2);

    assert_equals(1, sem->available_permits());
    assert_true(sem->try_acquire());
    assert_equals(0, sem->available_permits());
    assert_false(sem->try_acquire());

    std::cout << "completed\n";
}

// Test semaphore release overflow (should throw)
void test_semaphore_overflow() {
    std::cout << "test_semaphore_overflow... ";

    auto sem = create_semaphore(1);

    bool threw = false;
    try {
        sem->release(); // More releases than acquires
    } catch (const std::logic_error& e) {
        threw = true;
    }
    assert_true(threw);

    std::cout << "completed\n";
}

// Test concurrent mutex access
void test_mutex_concurrent() {
    std::cout << "test_mutex_concurrent... ";

    auto mutex = make_mutex(false);
    std::atomic<int> counter{0};
    constexpr int iterations = 1000;
    constexpr int num_threads = 4;

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&mutex, &counter]() {
            for (int i = 0; i < iterations; ++i) {
                mutex->lock();
                int prev = counter.load();
                counter.store(prev + 1);
                mutex->unlock();
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    assert_equals(num_threads * iterations, counter.load());
    std::cout << "completed\n";
}

// Test concurrent semaphore access
void test_semaphore_concurrent() {
    std::cout << "test_semaphore_concurrent... ";

    auto sem = create_semaphore(2);
    std::atomic<int> active{0};
    std::atomic<int> max_active{0};
    constexpr int iterations = 100;
    constexpr int num_threads = 4;

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&sem, &active, &max_active]() {
            for (int i = 0; i < iterations; ++i) {
                sem->acquire(); // blocking acquire
                int cur = ++active;
                // Track max concurrent
                int prev_max = max_active.load();
                while (cur > prev_max && !max_active.compare_exchange_weak(prev_max, cur));

                // Simulate work
                std::this_thread::yield();

                --active;
                sem->release();
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Max active should never exceed semaphore permits
    assert_true(max_active.load() <= 2);
    std::cout << "completed (max_active=" << max_active.load() << ")\n";
}

// Transliterated from: kotlinx-coroutines-core/common/test/sync/SemaphoreTest.kt:60-98
void test_with_permit_suspend_and_finally() {
    using namespace kotlinx::coroutines;
    for (bool fail : {false, true}) {
        auto semaphore = create_semaphore(1, 1);
        int calls = 0;
        int resumes = 0;
        Result<void*> outcome = Result<void*>::success(nullptr);
        auto failure = std::make_exception_ptr(std::runtime_error("action failure"));
        FunctionalContinuation<void*> completion(EmptyCoroutineContext::instance(), [&](Result<void*> result) {
            ++resumes;
            outcome = std::move(result);
        });
        void* result = with_permit(*semaphore, [&]() -> int {
            ++calls;
            assert_equals(0, semaphore->available_permits());
            if (fail) std::rethrow_exception(failure);
            return 42;
        }, &completion);
        assert_true(intrinsics::is_coroutine_suspended(result));
        assert_equals(0, calls);
        assert_equals(0, resumes);
        semaphore->release();
        assert_equals(1, calls);
        assert_equals(1, resumes);
        assert_equals(1, semaphore->available_permits());
        if (fail) assert_true(outcome.exception_or_null() == failure);
        else {
            std::unique_ptr<int> value(static_cast<int*>(outcome.get_or_throw()));
            assert_equals(42, *value);
        }
    }
    auto semaphore = create_semaphore(1);
    FunctionalContinuation<void*> completion(kotlin::coroutines::EmptyCoroutineContext::instance(),
        [](kotlinx::coroutines::Result<void*>) { throw std::runtime_error("direct completion resumed"); });
    assert_true(with_permit(*semaphore, [] {}, &completion) == nullptr);
    assert_equals(1, semaphore->available_permits());
}

// Transliterated from: kotlinx-coroutines-core/common/test/CancellableResumeOldTest.kt:27-44,197-232
// NOTE(port): Exercise Kotlin's ordering through the port's erased suspension ABI.
class PermitQueueDispatcher final : public kotlinx::coroutines::CoroutineDispatcher {
public:
    mutable std::deque<std::shared_ptr<kotlinx::coroutines::Runnable>> queue;
    bool is_dispatch_needed(const kotlin::coroutines::CoroutineContext&) const override { return true; }
    void dispatch(const kotlin::coroutines::CoroutineContext&,
                  std::shared_ptr<kotlinx::coroutines::Runnable> runnable) const override {
        queue.push_back(std::move(runnable));
    }
    void drain() {
        while (!queue.empty()) {
            auto runnable = std::move(queue.front());
            queue.pop_front();
            runnable->run();
        }
    }
};

void test_cancelled_unit_resume_and_permit_return() {
    using namespace kotlinx::coroutines;
    // An installed cancellation handler must not consume the first resume attempt.
    for (bool exceptional : {false, true}) {
        int handlers = 0;
        int cancelled_values = 0;
        auto cause = std::make_exception_ptr(CancellationException("cancelled first"));
        auto completion = std::make_shared<FunctionalContinuation<void>>(
            EmptyCoroutineContext::instance(), [](Result<void>) { throw std::logic_error("direct path resumed"); });
        auto continuation = std::make_shared<CancellableContinuationImpl<void>>(completion, MODE_CANCELLABLE);
        continuation->invoke_on_cancellation([&](std::exception_ptr failure) {
            assert_true(failure == cause);
            ++handlers;
        });
        assert_true(continuation->cancel(cause));
        if (exceptional) continuation->resume_with(Result<void>::failure(std::make_exception_ptr(std::runtime_error("late failure"))));
        else continuation->resume([&](std::exception_ptr failure) {
            assert_true(failure == cause);
            ++cancelled_values;
        });
        assert_equals(1, handlers);
        assert_equals(exceptional ? 0 : 1, cancelled_values);
        bool second_resume_rejected = false;
        try { continuation->resume(nullptr); }
        catch (const std::logic_error&) { second_resume_rejected = true; }
        assert_true(second_resume_rejected);
        try { continuation->get_result(); throw std::logic_error("cancelled result succeeded"); }
        catch (const CancellationException&) {}
    }

    // A permit accepted into the dispatch queue is returned if the acquiring job
    // is cancelled before its continuation runs.
    auto dispatcher = std::make_shared<PermitQueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    auto context = job->operator+(dispatcher);
    int resumes = 0;
    Result<void*> outcome = Result<void*>::success(nullptr);
    auto completion = std::make_shared<FunctionalContinuation<void*>>(context, [&](Result<void*> result) {
        ++resumes;
        outcome = std::move(result);
    });
    auto semaphore = create_semaphore(1, 1);
    auto dispatched = dispatcher->intercept_continuation<void*>(completion);
    auto result = semaphore->acquire(dispatched.get());
    assert_true(intrinsics::is_coroutine_suspended(result));
    semaphore->release();
    assert_equals(0, semaphore->available_permits());
    assert_equals(0, resumes);
    assert_equals(size_t(1), dispatcher->queue.size());
    job->cancel();
    dispatcher->drain();
    assert_equals(1, resumes);
    assert_false(outcome.is_success());
    assert_equals(1, semaphore->available_permits());

    // Immediate Unit resume must call get_result once, rather than deciding twice.
    FunctionalContinuation<void*> direct(EmptyCoroutineContext::instance(),
        [](Result<void*>) { throw std::logic_error("direct path resumed"); });
    assert_true(suspend_cancellable_coroutine<void>([](CancellableContinuation<void>& continuation) {
        continuation.resume(nullptr);
    }, &direct) == nullptr);
}

// Source contract: CancellableContinuationImpl.kt:201-217,399-458,493-523.
// Race cancellation, resume and registration rather than copying the state algorithm.
void test_continuation_state_publication_races() {
    using namespace kotlinx::coroutines;
    for (int iteration = 0; iteration < 200; ++iteration) {
        std::atomic<int> resumes{0};
        std::atomic<int> handlers{0};
        std::atomic<int> cancelled_values{0};
        std::exception_ptr outcome;
        auto cause = std::make_exception_ptr(CancellationException("racing cancellation"));
        auto completion = std::make_shared<FunctionalContinuation<void>>(EmptyCoroutineContext::instance(),
            [&](Result<void> result) { outcome = result.exception_or_null(); ++resumes; });
        auto continuation = std::make_shared<CancellableContinuationImpl<void>>(completion, MODE_CANCELLABLE);
        assert_true(intrinsics::is_coroutine_suspended(continuation->get_result()));
        std::thread resume([&] { continuation->resume([&](std::exception_ptr failure) {
            assert_true(failure == cause);
            ++cancelled_values;
        }); });
        std::thread cancel([&] { continuation->cancel(cause); });
        std::thread install([&] { continuation->invoke_on_cancellation([&](std::exception_ptr failure) {
            assert_true(failure == cause);
            ++handlers;
        }); });
        resume.join();
        cancel.join();
        install.join();
        assert_equals(1, resumes.load());
        assert_true(outcome == nullptr || outcome == cause);
        assert_equals(outcome ? 1 : 0, handlers.load());
        assert_equals(outcome ? 1 : 0, cancelled_values.load());
    }
}

// Source contract: CancellableContinuationImpl.kt:595-603.
void test_unit_undispatched_resumes_require_dispatcher_identity() {
    using namespace kotlinx::coroutines;
    for (bool matching : {false, true}) for (bool exceptional : {false, true}) {
        auto dispatcher = std::make_shared<PermitQueueDispatcher>();
        auto other = std::make_shared<PermitQueueDispatcher>();
        int resumes = 0;
        auto cause = std::make_exception_ptr(std::runtime_error("resume failure"));
        Result<void> outcome = Result<void>::success();
        auto completion = std::make_shared<FunctionalContinuation<void>>(dispatcher, [&](Result<void> result) {
            ++resumes;
            outcome = result;
        });
        auto delegate = dispatcher->intercept_continuation<void>(completion);
        auto continuation = std::make_shared<CancellableContinuationImpl<void>>(delegate, MODE_CANCELLABLE);
        assert_true(intrinsics::is_coroutine_suspended(continuation->get_result()));
        if (exceptional) continuation->resume_undispatched_with_exception(matching ? dispatcher.get() : other.get(), cause);
        else continuation->resume_undispatched(matching ? dispatcher.get() : other.get());
        assert_equals(matching ? 1 : 0, resumes);
        assert_equals(size_t(matching ? 0 : 1), dispatcher->queue.size());
        dispatcher->drain();
        assert_equals(1, resumes);
        assert_true(outcome.exception_or_null() == (exceptional ? cause : nullptr));
    }
}

// Source contract: CancellableContinuation.kt:265-320 and CancellableContinuationImpl.kt:365-383.
void test_resume_callback_receives_the_actual_value_and_context() {
    using namespace kotlinx::coroutines;
    for (bool cancelled : {false, true}) {
        auto context = EmptyCoroutineContext::instance();
        auto completion = std::make_shared<FunctionalContinuation<std::shared_ptr<int>>>(context,
            [](Result<std::shared_ptr<int>>) { throw std::logic_error("direct path resumed"); });
        auto implementation = std::make_shared<CancellableContinuationImpl<std::shared_ptr<int>>>(completion, MODE_CANCELLABLE);
        CancellableContinuation<std::shared_ptr<int>>& continuation = *implementation;
        auto resource = std::make_shared<int>(42);
        auto cause = std::make_exception_ptr(CancellationException("resource cancelled"));
        if (cancelled) assert_true(continuation.cancel(cause));
        int callbacks = 0;
        continuation.resume(resource, [&](std::exception_ptr failure, std::shared_ptr<int> value,
                                          std::shared_ptr<CoroutineContext> received_context) {
            assert_true(failure == cause);
            assert_true(value.get() == resource.get());
            assert_true(received_context == context);
            ++callbacks;
        });
        assert_equals(cancelled ? 1 : 0, callbacks);
        if (cancelled) {
            try { implementation->get_result(); throw std::logic_error("cancelled result succeeded"); }
            catch (const CancellationException&) {}
        } else {
            std::unique_ptr<std::shared_ptr<int>> result(static_cast<std::shared_ptr<int>*>(implementation->get_result()));
            assert_true(result->get() == resource.get());
        }
    }
}

int main() {
    std::cout << "=== Sync Module Tests ===\n";

    test_mutex_basic();
    test_mutex_owner();
    test_mutex_reentrant();
    test_mutex_created_locked();
    test_semaphore_basic();
    test_semaphore_acquired();
    test_semaphore_overflow();
    test_mutex_concurrent();
    test_semaphore_concurrent();
    test_with_permit_suspend_and_finally();
    test_cancelled_unit_resume_and_permit_return();
    test_continuation_state_publication_races();
    test_unit_undispatched_resumes_require_dispatcher_identity();
    test_resume_callback_receives_the_actual_value_and_context();

    std::cout << "\nSync cases completed\n";
    return 0;
}
