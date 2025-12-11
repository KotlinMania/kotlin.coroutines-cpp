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
#include <atomic>

namespace kotlinx {
namespace coroutines {

// Internal helpers
static std::atomic<ExecutorCoroutineDispatcherImpl*> default_dispatcher_ptr{nullptr};
static std::atomic<ExecutorCoroutineDispatcherImpl*> io_dispatcher_ptr{nullptr};

static CoroutineDispatcher& create_default_dispatcher_impl() {
    ExecutorCoroutineDispatcherImpl* ptr = default_dispatcher_ptr.load();
    if (ptr == nullptr) {
        auto* new_dispatcher = new ExecutorCoroutineDispatcherImpl(
            std::max(2u, std::thread::hardware_concurrency()), "Dispatchers.Default");
        ExecutorCoroutineDispatcherImpl* expected = nullptr;
        if (default_dispatcher_ptr.compare_exchange_strong(expected, new_dispatcher)) {
            ptr = new_dispatcher;
        } else {
            delete new_dispatcher; // lost race
            ptr = expected;
        }
    }
    return *ptr;
}

static CoroutineDispatcher& create_io_dispatcher_impl() {
    ExecutorCoroutineDispatcherImpl* ptr = io_dispatcher_ptr.load();
    if (ptr == nullptr) {
        auto* new_dispatcher = new ExecutorCoroutineDispatcherImpl(64, "Dispatchers.IO");
        ExecutorCoroutineDispatcherImpl* expected = nullptr;
        if (io_dispatcher_ptr.compare_exchange_strong(expected, new_dispatcher)) {
            ptr = new_dispatcher;
        } else {
            delete new_dispatcher; // lost race
            ptr = expected;
        }
    }
    return *ptr;
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

        std::shared_ptr<CoroutineDispatcher> limited_parallelism(int parallelism, const std::string& name) override {
             return delegate.limited_parallelism(parallelism, name);
        }
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

void Dispatchers::shutdown() {
    if (auto* ptr = default_dispatcher_ptr.exchange(nullptr)) {
        ptr->close();
        delete ptr;
    }
    if (auto* ptr = io_dispatcher_ptr.exchange(nullptr)) {
        ptr->close();
        delete ptr;
    }
}

// MainCoroutineDispatcher base implementation
std::string MainCoroutineDispatcher::to_string() const {
    return to_string_internal_impl();
}

std::string MainCoroutineDispatcher::to_string_internal_impl() const {
    return "MainCoroutineDispatcher";
}

std::shared_ptr<CoroutineDispatcher> MainCoroutineDispatcher::limited_parallelism(int parallelism, const std::string& name) {
     return CoroutineDispatcher::limited_parallelism(parallelism, name);
}

} // namespace coroutines
} // namespace kotlinx
