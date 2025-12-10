/**
 * @file Dispatchers.cpp
 * @brief Implementation of Dispatchers (Default, IO, Main, Unconfined).
 */

#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/MainCoroutineDispatcher.hpp"
#include "kotlinx/coroutines/MultithreadedDispatchers.hpp"
#include <thread>
#include <algorithm>
#include <iostream>

namespace kotlinx {
namespace coroutines {

// Internal helpers
static CoroutineDispatcher& create_default_dispatcher_impl() {
    static ExecutorCoroutineDispatcherImpl instance(
        std::max(2u, std::thread::hardware_concurrency()), "Dispatchers.Default");
    return instance;
}

static CoroutineDispatcher& create_io_dispatcher_impl() {
    static ExecutorCoroutineDispatcherImpl instance(64, "Dispatchers.IO");
    return instance;
}

static MainCoroutineDispatcher& create_main_dispatcher_impl() {
    // Stub Main Dispatcher wrapping Default
    static class DefaultMain : public MainCoroutineDispatcher {
        CoroutineDispatcher& delegate;
    public:
        DefaultMain(CoroutineDispatcher& d) : delegate(d) {}
        
        bool is_dispatch_needed(const CoroutineContext& context) const override {
             return delegate.is_dispatch_needed(context);
        }
        
        void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
            delegate.dispatch(context, block);
        }
        
        void dispatch_yield(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
             delegate.dispatch_yield(context, block);
        }

        MainCoroutineDispatcher& get_immediate() override { return *this; }
        
        std::string to_string() const override { return "Dispatchers.Main[Default]"; }
    } instance(create_default_dispatcher_impl());
    
    return instance;
}

static CoroutineDispatcher& create_unconfined_dispatcher_impl() {
    static class UnconfinedDispatcher : public CoroutineDispatcher {
    public:
        void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
             block->run();
        }
        bool is_dispatch_needed(const CoroutineContext&) const override { return false; }
        std::string to_string() const override { return "Dispatchers.Unconfined"; }
    } instance;
    return instance;
}

// Implementations for Dispatchers class
CoroutineDispatcher& Dispatchers::get_default() {
    return create_default_dispatcher_impl();
}

CoroutineDispatcher& Dispatchers::get_io() {
    return create_io_dispatcher_impl();
}

MainCoroutineDispatcher& Dispatchers::get_main() {
    return create_main_dispatcher_impl();
}

CoroutineDispatcher& Dispatchers::get_unconfined() {
    return create_unconfined_dispatcher_impl();
}

// MainCoroutineDispatcher base implementation
std::string MainCoroutineDispatcher::to_string() const {
    return to_string_internal_impl();
}

std::string MainCoroutineDispatcher::to_string_internal_impl() const {
    return "MainCoroutineDispatcher";
}

std::shared_ptr<CoroutineDispatcher> MainCoroutineDispatcher::limited_parallelism(int parallelism, const std::string& name) {
    // Basic implementation that ignores parallelism limit for now in base
    // In real implementation it should wrap into LimitedDispatcher 
    return nullptr; 
}

} // namespace coroutines
} // namespace kotlinx
