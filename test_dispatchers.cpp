#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/MainCoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include <iostream>
#include <cassert>
#include <atomic>
#include <chrono>
#include <thread>

using namespace kotlinx::coroutines;

class SimpleRunnable : public Runnable {
    std::atomic<bool>& flag;
public:
    SimpleRunnable(std::atomic<bool>& f) : flag(f) {}
    void run() override {
        flag = true;
    }
};

void test_default() {
    std::cout << "Testing Dispatchers::get_default()..." << std::endl;
    std::atomic<bool> executed{false};
    auto& dispatcher = Dispatchers::get_default();
    // Use empty context (stub needed?)
    // Dispatchers::dispatch requires context (const CoroutineContext&)
    // We need a stub context.
    // Assuming CoroutineContext is defined or fwd declared successfully.
    // include/kotlinx/coroutines/CoroutineContext.hpp must be included implicitly or explicitly?
    // Dispatchers.hpp includes CoroutineDispatcher.hpp which includes CoroutineContext.hpp
    
    // We need a concrete context or empty one.
    // Only interfaces are defined so far in CoroutineContext.hpp? 
    // Wait, CoroutineContext is a struct.
    
    // Let's create a dummy context since it's pure virtual?
    class EmptyContext : public CoroutineContext {
    public:
        // No elements, stub pure virtuals if any (get, fold, etc are defined in base or declared?)
        // CoroutineContext declares: virtual std::shared_ptr<Element> get(Key* key) const { return nullptr; } (defined inline in header)
        // Others? operator+, fold, minusKey declared but not defined.
        // We assume we link against CoroutineContext.cpp which defines them?
        // Actually CoroutineContext.cpp defines them. So we inherit them.
    };
    EmptyContext ctx;

    std::cout << "Dispatching to Default..." << std::endl;
    dispatcher.dispatch(ctx, std::make_shared<SimpleRunnable>(executed));
    
    // Wait for execution
    int retries = 0;
    while (!executed && retries < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        retries++;
    }
    
    if (executed) {
        std::cout << "Default Dispatcher: SUCCESS" << std::endl;
    } else {
        std::cout << "Default Dispatcher: FAILED (Timeout)" << std::endl;
        assert(false);
    }
}

void test_io() {
    std::cout << "Testing Dispatchers::get_io()..." << std::endl;
    std::atomic<bool> executed{false};
    auto& dispatcher = Dispatchers::get_io();
    class EmptyContext : public CoroutineContext {};
    EmptyContext ctx;

    dispatcher.dispatch(ctx, std::make_shared<SimpleRunnable>(executed));
    
    int retries = 0;
    while (!executed && retries < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        retries++;
    }
    
    if (executed) {
        std::cout << "IO Dispatcher: SUCCESS" << std::endl;
    } else {
        std::cout << "IO Dispatcher: FAILED (Timeout)" << std::endl;
        assert(false);
    }
}

void test_unconfined() {
    std::cout << "Testing Dispatchers::get_unconfined()..." << std::endl;
    std::atomic<bool> executed{false};
    auto& dispatcher = Dispatchers::get_unconfined();
    class EmptyContext : public CoroutineContext {};
    EmptyContext ctx;
    
    dispatcher.dispatch(ctx, std::make_shared<SimpleRunnable>(executed));
    
    if (executed) {
        std::cout << "Unconfined Dispatcher: SUCCESS (Immediate)" << std::endl;
    } else {
        std::cout << "Unconfined Dispatcher: FAILED (Not Immediate)" << std::endl;
        assert(false);
    }
}

void test_main() {
    std::cout << "Testing Dispatchers::get_main()..." << std::endl;
    auto& dispatcher = Dispatchers::get_main();
    class EmptyContext : public CoroutineContext {};
    EmptyContext ctx;
    std::atomic<bool> executed{false};
    
    try {
        dispatcher.dispatch(ctx, std::make_shared<SimpleRunnable>(executed));
        std::cout << "Main Dispatcher: FAILED (Should throw)" << std::endl;
        assert(false);
    } catch (const std::exception& e) {
        std::cout << "Main Dispatcher: SUCCESS (Caught expected exception: " << e.what() << ")" << std::endl;
    }
}

int main() {
    test_default();
    test_io();
    test_unconfined();
    test_main();
    return 0;
}
