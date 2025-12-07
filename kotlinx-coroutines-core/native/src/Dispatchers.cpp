#include <string>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/Dispatchers.kt
//
// TODO: actual keyword - platform-specific implementation marker
// TODO: class keyword - singleton pattern
// TODO: expect function declarations
// TODO: Extension property for Dispatchers.IO
// TODO: @PublishedApi annotation

namespace kotlinx {
namespace coroutines {

// TODO: Remove imports, fully qualify or add includes:
// import kotlinx.coroutines.internal.*
// import kotlin.coroutines.*

// Forward declarations
class DefaultIoScheduler;
CoroutineDispatcher* create_default_dispatcher();
MainCoroutineDispatcher* create_main_dispatcher(CoroutineDispatcher* default_dispatcher);

// TODO: actual class -> singleton
class Dispatchers {
private:
    CoroutineDispatcher* main_dispatcher;
    MainCoroutineDispatcher* injected_main_dispatcher;

    Dispatchers()
        : main_dispatcher(nullptr)
        , injected_main_dispatcher(nullptr)
    {
        main_dispatcher = create_main_dispatcher(kDefault);
    }

public:
    static Dispatchers& instance() {
        static Dispatchers instance;
        return instance;
    }

    static CoroutineDispatcher* kDefault;

    MainCoroutineDispatcher* main() {
        return injected_main_dispatcher != nullptr ? injected_main_dispatcher : main_dispatcher;
    }

    CoroutineDispatcher* unconfined() {
        return &kotlinx::coroutines::Unconfined; // Avoid freezing
    }

    // TODO: @PublishedApi internal
    void inject_main(MainCoroutineDispatcher* dispatcher) {
        injected_main_dispatcher = dispatcher;
    }

    static CoroutineDispatcher* kIo;
};

// Initialize static members
CoroutineDispatcher* Dispatchers::kDefault = create_default_dispatcher();
CoroutineDispatcher* Dispatchers::kIo = /* DefaultIoScheduler */nullptr;

// TODO: class -> singleton or namespace
class DefaultIoScheduler : CoroutineDispatcher {
private:
    // 2048 is an arbitrary KMP-friendly constant
    CoroutineDispatcher* unlimited_pool;
    CoroutineDispatcher* io;

    DefaultIoScheduler() {
        unlimited_pool = new_fixed_thread_pool_context(2048, "Dispatchers.IO");
        io = unlimited_pool->limited_parallelism(64); // Default JVM size
    }

public:
    static DefaultIoScheduler& instance() {
        static DefaultIoScheduler instance;
        return instance;
    }

    CoroutineDispatcher* limited_parallelism(int parallelism, std::string* name = nullptr) override {
        // See documentation to Dispatchers.IO for the rationale
        return unlimited_pool->limited_parallelism(parallelism, name);
    }

    void dispatch(CoroutineContext context, Runnable block) override {
        io->dispatch(context, block);
    }

    // TODO: @InternalCoroutinesApi annotation
    void dispatch_yield(CoroutineContext context, Runnable block) override {
        io->dispatch_yield(context, block);
    }

    std::string to_string() const override {
        return "Dispatchers.IO";
    }
};

// TODO: @Suppress("EXTENSION_SHADOWED_BY_MEMBER")
// TODO: Extension property - free function or namespace
CoroutineDispatcher* get_io(Dispatchers& dispatchers) {
    return Dispatchers::kIo;
}

// TODO: expect function - platform-specific declaration
MainCoroutineDispatcher* create_main_dispatcher(CoroutineDispatcher* default_dispatcher);

} // namespace coroutines
} // namespace kotlinx
