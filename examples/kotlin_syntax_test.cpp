// Kotlin-style syntax test
// Compare with actual Kotlin code side-by-side

#include <coroutine>

#include "kotlinx/coroutines.hpp"
#include <iostream>
#include <experimental/coroutine>
#include "kotlinx/coroutines/Delay.hpp"

struct ReturnObject {
    struct promise_type {
        ReturnObject get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void unhandled_exception() {}
    };
};

main{
    auto pp = co_await GetPromise<ReturnObject3::promise_type>{};
}