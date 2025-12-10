#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/MultithreadedDispatchers.hpp"
#include "kotlinx/coroutines/MainCoroutineDispatcher.hpp"
#include <algorithm>
#include <stdexcept>
#include <mutex>

namespace kotlinx {
namespace coroutines {

namespace {
    // Unconfined Implementation
    class UnconfinedDispatcherImpl : public CoroutineDispatcher {
    public:
        void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
            // "Artisan" simplified Unconfined: just run it.
            // In full Kotlin this involves thread-local event loops to avoid stack overflow.
            block->run();
        }
        std::string to_string() const override { return "Dispatchers.Unconfined"; }
    };

    // Main Placeholder
    class MissingMainCoroutineDispatcher : public MainCoroutineDispatcher {
    public:
        MainCoroutineDispatcher& get_immediate() override {
            return *this;
        }

        void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
             throw std::logic_error("MainDispatcher is not implemented on this platform");
        }
        
        bool is_dispatch_needed(const CoroutineContext& context) const override {
            return true;
        }

        std::shared_ptr<CoroutineDispatcher> limited_parallelism(int parallelism, const std::string& name = "") override {
             throw std::logic_error("MainDispatcher is not implemented on this platform");
        }

        std::string to_string() const override {
            return "Dispatchers.Main[Missing]";
        }
    };
}

CoroutineDispatcher& Dispatchers::get_default() {
    static CloseableCoroutineDispatcher* instance = []{
        int threads = std::max(2u, std::thread::hardware_concurrency());
        return new_fixed_thread_pool_context(threads, "Dispatchers.Default");
    }();
    return *instance;
}

CoroutineDispatcher& Dispatchers::get_io() {
    static CloseableCoroutineDispatcher* instance = []{
        int threads = std::max(64u, std::thread::hardware_concurrency());
        return new_fixed_thread_pool_context(threads, "Dispatchers.IO");
    }();
    return *instance;
}

CoroutineDispatcher& Dispatchers::get_unconfined() {
    static UnconfinedDispatcherImpl instance;
    return instance;
}

MainCoroutineDispatcher& Dispatchers::get_main() {
    static MissingMainCoroutineDispatcher instance;
    return instance;
}

} // namespace coroutines
} // namespace kotlinx
