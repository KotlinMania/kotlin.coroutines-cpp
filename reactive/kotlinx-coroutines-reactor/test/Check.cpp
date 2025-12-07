// Transliterated from: reactive/kotlinx-coroutines-reactor/test/Check.cpp

// TODO: #include <reactor/core/publisher/flux.hpp>
// TODO: #include <reactor/core/publisher/mono.hpp>

namespace kotlinx { namespace coroutines { namespace reactor {

template<typename T>
void check_mono_value(
        Mono<T>* mono,
        std::function<void(T)> checker
) {
    T mono_value = mono->block();
    checker(mono_value);
}

void check_erroneous(
        Mono<void>* mono,
        std::function<void(Throwable*)> checker
) {
    try {
        mono->block();
        throw std::runtime_error("Should have failed");
    } catch (Throwable* e) {
        checker(e);
    }
}

template<typename T>
void check_single_value(
        Flux<T>* flux,
        std::function<void(T)> checker
) {
    T single_value = flux->to_iterable().single();
    checker(single_value);
}

void check_erroneous(
        Flux<void>* flux,
        std::function<void(Throwable*)> checker
) {
    auto single_notification = flux->materialize().to_iterable().single();
    checker(single_notification.get_throwable());
}

} } } // namespace kotlinx::coroutines::reactor

// TODO: Semantic implementation tasks:
// 1. Implement Mono<T>::block() method
// 2. Implement Flux<T>::to_iterable() method
// 3. Implement Flux<T>::materialize() method
// 4. Implement single() on iterable
// 5. Implement Throwable exception type
// 6. Implement get_throwable() method
// 7. Implement std::function wrappers
// 8. Handle void template specialization
