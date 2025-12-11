#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include <iostream>
#include <cassert>
#include <coroutine>
#include <thread>
#include <chrono>

using namespace kotlinx::coroutines;

// Minimal Coroutine Infrastructure for testing co_await
struct TestTask {
    struct promise_type {
        int result = 0;
        std::exception_ptr exception;
        TestTask get_return_object();
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { exception = std::current_exception(); }
    };
    std::coroutine_handle<promise_type> handle;
    
    explicit TestTask(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~TestTask() { 
        if (handle) {
             std::cout << "TestTask: Destroying handle..." << std::endl;
             handle.destroy(); 
             std::cout << "TestTask: Handle destroyed." << std::endl;
        }
    }
    TestTask(const TestTask&) = delete;
    TestTask& operator=(const TestTask&) = delete;
    TestTask(TestTask&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
    TestTask& operator=(TestTask&& other) noexcept {
        if (this != &other) {
            if (handle) {
                 std::cout << "TestTask: Move Assign destroying old handle..." << std::endl;
                 handle.destroy();
            }
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }
};

inline TestTask TestTask::promise_type::get_return_object() { 
    return TestTask{std::coroutine_handle<promise_type>::from_promise(*this)}; 
}

// Test 1: Successful Resume
TestTask test_successful_resume() {
    std::cout << "Test 1: Successful Resume..." << std::endl;
    int value = co_await suspend_cancellable_coroutine<int>([](CancellableContinuation<int>& cont) {
        cont.resume(42, nullptr); 
    });
    
    if (value == 42) {
        std::cout << "SUCCESS: Got 42" << std::endl;
    } else {
        std::cout << "FAILED: Got " << value << std::endl;
        assert(false);
    }
    co_return;
}

// Test 2: Cancellation
TestTask test_cancellation() {
    std::cout << "Test 2: Cancellation..." << std::endl;
    bool handler_invoked = false;
    
    try {
        co_await suspend_cancellable_coroutine<int>([&](CancellableContinuation<int>& cont) {
            cont.invoke_on_cancellation([&](std::exception_ptr) {
                handler_invoked = true;
            });
            cont.cancel(std::make_exception_ptr(std::runtime_error("Cancelled")));
        });
        std::cout << "FAILED: Should have thrown" << std::endl;
        assert(false);
    } catch (const std::runtime_error& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
        if (std::string(e.what()) == "Cancelled") {
             if (handler_invoked) {
                 std::cout << "SUCCESS: Handler invoked and Exception caught" << std::endl;
             } else {
                 std::cout << "FAILED: Handler NOT invoked" << std::endl;
                 assert(false);
             }
        } else {
            std::cout << "FAILED: Wrong exception message" << std::endl;
            assert(false);
        }
    } catch (...) {
        std::cout << "FAILED: Wrong exception type" << std::endl;
        assert(false);
    }
    co_return;
}

// Test 3: Resume vs Cancel Race (Simulated)
TestTask test_race() {
    std::cout << "Test 3: Race (Resume wins)..." << std::endl;
    int value = co_await suspend_cancellable_coroutine<int>([](CancellableContinuation<int>& cont) {
        // Safe casting to Impl to hold reference!
        auto& impl = dynamic_cast<CancellableContinuationImpl<int>&>(cont);
        auto shared_cont = impl.shared_from_this();
        
        cont.resume(100, nullptr); // This might destroy internal impl ref
        // But shared_cont keeps it alive!
        
        bool cancelled = shared_cont->cancel(std::make_exception_ptr(std::runtime_error("Too late")));
        assert(!cancelled); // Resume should win
    });
    
    if (value == 100) std::cout << "SUCCESS: Resume won" << std::endl;
    else {
        std::cout << "FAILED: Value " << value << std::endl; 
        assert(false);
    }
    co_return;
}

// Test 4: Parent Cancellation Propagation
struct MockContinuation : public Continuation<int> {
    std::shared_ptr<Job> job;
    MockContinuation(std::shared_ptr<Job> j) : job(j) {}
    
    std::shared_ptr<CoroutineContext> get_context() const override { return job; }
    void resume_with(Result<int> result) override {}
};

void test_parent_cancellation() {
    std::cout << "Test 4: Parent Cancellation Propagation..." << std::endl;
    // Uses JobImpl
    auto parent = JobImpl::create(nullptr);
    auto mock = std::make_shared<MockContinuation>(parent);
    auto impl = std::make_shared<CancellableContinuationImpl<int>>(mock, 1);
    
    impl->init_cancellability();
    
    if(!impl->is_active()) {
         std::cout << "FAILED: Impl should be active initially." << std::endl;
         assert(false);
    }
    
    // Cancel parent
    parent->cancel(std::make_exception_ptr(std::runtime_error("Parent Cancelled")));
    
    if (impl->is_cancelled()) {
        std::cout << "SUCCESS: Impl cancelled by parent." << std::endl;
    } else {
        std::cout << "FAILED: Impl NOT cancelled." << std::endl;
        assert(false);
    }
}

int main() {
    test_successful_resume();
    test_cancellation();
    test_race();
    test_parent_cancellation();
    return 0;
}
