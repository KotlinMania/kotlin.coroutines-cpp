// Transliterated from: test-utils/common/src/LaunchFlow.kt
// TODO: #include equivalent headers
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <kotlinx/coroutines/flow/flow.hpp>
// TODO: #include <map>
// TODO: #include <functional>

namespace kotlinx {
namespace coroutines {
namespace testing {
namespace flow {

// TODO: #include kotlin::jvm equivalents
// TODO: #include kotlin::reflect equivalents

template<typename T>
using Handler = std::function<void(CoroutineScope&, T)>; // TODO: implement coroutine suspension

/*
 * Design of this builder is not yet stable, so leaving it as is.
 */
template<typename T>
class LaunchFlowBuilder {
public:
    /*
     * NB: this implementation is a temporary ad-hoc (and slightly incorrect)
     * solution until coroutine-builders are ready
     *
     * NB 2: this internal stuff is required to workaround KT-30795
     */
    // @PublishedApi
    Handler<T>* on_each = nullptr;

    // @PublishedApi
    Handler<Throwable*>* finally_handler = nullptr;

    // @PublishedApi
    std::map<void*, Handler<Throwable>> exception_handlers; // TODO: KClass replacement

    void on_each(std::function<void(CoroutineScope&, T)> action) { // TODO: implement coroutine suspension
        if (on_each != nullptr) {
            throw std::runtime_error("onEach block is already registered");
        }
        if (!exception_handlers.empty()) {
            throw std::runtime_error("onEach block should be registered before exceptionHandlers block");
        }
        if (finally_handler != nullptr) {
            throw std::runtime_error("onEach block should be registered before finally block");
        }
        on_each = new Handler<T>(action);
    }

    template<typename ExceptionT>
    void catch_handler(std::function<void(CoroutineScope&, ExceptionT)> action) { // TODO: implement coroutine suspension
        if (on_each == nullptr) {
            throw std::runtime_error("onEach block should be registered first");
        }
        if (finally_handler != nullptr) {
            throw std::runtime_error("exceptionHandlers block should be registered before finally block");
        }
        // @Suppress("UNCHECKED_CAST")
        exception_handlers[typeid(ExceptionT).name()] = reinterpret_cast<Handler<Throwable>>(action);
    }

    void finally(std::function<void(CoroutineScope&, Throwable*)> action) { // TODO: implement coroutine suspension
        if (finally_handler != nullptr) {
            throw std::runtime_error("Finally block is already registered");
        }
        if (on_each == nullptr) {
            throw std::runtime_error("onEach block should be registered before finally block");
        }
        if (finally_handler == nullptr) {
            finally_handler = new Handler<Throwable*>(action);
        }
    }

    Handlers<T> build() {
        if (on_each == nullptr) {
            throw std::runtime_error("onEach is not registered");
        }
        return Handlers<T>(*on_each, exception_handlers, finally_handler);
    }
};

template<typename T>
class Handlers {
public:
    // @JvmField
    Handler<T> on_each;

    // @JvmField
    std::map<void*, Handler<Throwable>> exception_handlers;

    // @JvmField
    Handler<Throwable*>* finally_handler;

    Handlers(Handler<T> on_each,
             std::map<void*, Handler<Throwable>> exception_handlers,
             Handler<Throwable*>* finally_handler)
        : on_each(on_each)
        , exception_handlers(exception_handlers)
        , finally_handler(finally_handler) {}
};

template<typename T>
Job launch_flow(
    CoroutineScope& scope,
    Flow<T>& flow,
    std::function<void(LaunchFlowBuilder<T>&)> builder
) {
    LaunchFlowBuilder<T> launch_flow_builder;
    builder(launch_flow_builder);
    Handlers<T> handlers = launch_flow_builder.build();

    return scope.launch([&]() { // TODO: implement coroutine suspension
        Throwable* caught = nullptr;
        try {
            scope.coroutine_scope([&]() { // TODO: implement coroutine suspension
                flow.collect([&](T value) { // TODO: implement coroutine suspension
                    handlers.on_each(scope, value);
                });
            });
        } catch (Throwable& e) {
            for (auto& [key, value] : handlers.exception_handlers) {
                // TODO: implement KClass::isInstance equivalent
                // if (key.is_instance(e)) {
                    caught = &e;
                    value(scope, e);
                    break;
                // }
            }
            if (caught == nullptr) {
                caught = &e;
                throw e;
            }
        }
        // finally block
        scope.cancel(); // TODO discuss
        if (handlers.finally_handler != nullptr) {
            CoroutineScope new_scope(scope.coroutine_context() + NonCancellable);
            (*handlers.finally_handler)(new_scope, caught);
        }
    });
}

template<typename T>
Job launch_in(
    Flow<T>& flow,
    CoroutineScope& scope,
    std::function<void(LaunchFlowBuilder<T>&)> builder
) {
    return launch_flow(scope, flow, builder);
}

} // namespace flow
} // namespace testing
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement suspend function mechanics (coroutine suspension)
// 2. Replace void* in exception_handlers map with proper type reflection
// 3. Implement KClass::isInstance equivalent for exception matching
// 4. Implement Flow<T>::collect properly
// 5. Implement CoroutineScope operations (launch, coroutineScope, cancel)
// 6. Implement Job return type and behavior
// 7. Implement NonCancellable context element
// 8. Handle memory management for Handler pointers
// 9. Implement proper exception handling and rethrowing
// 10. Add proper includes for all dependencies
