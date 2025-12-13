// Kotlin-style syntax test
// Compare with actual Kotlin code side-by-side

#include <iostream>
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"

int main() {
    auto scope = kotlinx::coroutines::GlobalScope::instance();

    // Launch a fire-and-forget coroutine
    auto job = kotlinx::coroutines::launch(scope, nullptr, kotlinx::coroutines::CoroutineStart::DEFAULT, [](kotlinx::coroutines::CoroutineScope*) {
        std::cout << "Hello from launched coroutine!" << std::endl;
    });

    // Async coroutine that returns a value
    auto deferred = kotlinx::coroutines::async<int>(scope, nullptr, kotlinx::coroutines::CoroutineStart::DEFAULT, [](kotlinx::coroutines::CoroutineScope*) -> int {
        std::cout << "Computing in async coroutine..." << std::endl;
        return 42;
    });

    // Wait for the async result
    int result = deferred->await_blocking();
    std::cout << "Async result: " << result << std::endl;

    // Wait for the job to finish (though it's fire-and-forget, join to ensure completion)
    // Note: Job::join is suspend, so for blocking wait, we can use a loop or something, but for demo, just proceed.

    std::cout << "Main done." << std::endl;
    return 0;
}