#include "kotlinx/coroutines/JobSupport.hpp"
#include <iostream>
#include <cassert>

using namespace kotlinx::coroutines;

// Simple concrete Job implementation
class SimpleJob : public JobSupport {
public:
    SimpleJob(bool active = false) : JobSupport(active) {}
    
    // Helper to start if lazy
    bool start_job() { return start(); }
    
    // Helper to complete
    bool complete() {
        return make_completing(nullptr);
    }
    
    bool complete_exceptionally(std::exception_ptr ex) {
        return make_completing(ex);
    }
};

int main() {
    std::cout << "Testing JobSupport..." << std::endl;
    
    // Test 1: Start and Active check
    {
        auto job = std::make_shared<SimpleJob>(false); // New
        assert(!job->is_active());
        assert(!job->is_completed());
        
        bool started = job->start();
        assert(started);
        assert(job->is_active());
        
        bool started_again = job->start();
        assert(!started_again); // Already started
    }
    std::cout << "Test 1 Passed: Start/Active" << std::endl;

    // Test 2: Completion Listener
    {
        auto job = std::make_shared<SimpleJob>(true); // Active
        bool invoked = false;
        
        job->invoke_on_completion([&](std::exception_ptr cause) {
            invoked = true;
            assert(cause == nullptr);
        });
        
        bool completed = job->complete();
        assert(completed);
        assert(!job->is_active());
        assert(job->is_completed());
        assert(invoked);
    }
    std::cout << "Test 2 Passed: Completion" << std::endl;
    
    // Test 3: Exception Propagation
    {
        auto job = std::make_shared<SimpleJob>(true);
        bool invoked = false;
        
        job->invoke_on_completion([&](std::exception_ptr cause) {
            invoked = true;
            assert(cause != nullptr);
            try {
                std::rethrow_exception(cause);
            } catch (const std::runtime_error& e) {
                assert(std::string(e.what()) == "Test error");
            } catch (...) {
                assert(false);
            }
        });
        
        job->complete_exceptionally(std::make_exception_ptr(std::runtime_error("Test error")));
        assert(invoked);
    }
    std::cout << "Test 3 Passed: Exception" << std::endl;

    return 0;
}
