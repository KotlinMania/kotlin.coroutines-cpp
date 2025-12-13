// Open this in JetBrains CLion or VS Code to test IDE integration
// All of these should have: autocomplete, no red squiggles, go-to-definition

#include <kotlinx/coroutines.hpp>
#include <iostream>
#include <vector>

using namespace kotlinx::coroutines;

// =============================================================================
// Test: Basic suspend
// =============================================================================
void test_basic_suspend() {
    int x = 10;
    suspend();           // Basic
    suspend(42);         // With ID
    suspend("label_a");  // With string
    x = x * 2;
}

// =============================================================================
// Test: delay
// =============================================================================
void test_delay() {
    std::cout << "Starting..." << std::endl;
    delay(1000);  // 1 second
    delay(std::chrono::milliseconds(500));
    std::cout << "Done!" << std::endl;
}

// =============================================================================
// Test: launch and async
// =============================================================================
void test_launch_async() {
    // launch - fire and forget
    Job job = launch([]{
        std::cout << "Hello from coroutine!" << std::endl;
        delay(100);
    });

    // launch with dispatcher
    launch(Dispatchers::Default, []{
        std::cout << "On default dispatcher" << std::endl;
    });

    // launch with dispatcher and start mode
    launch(Dispatchers::IO, CoroutineStart::LAZY, []{
        std::cout << "Lazy IO coroutine" << std::endl;
    });

    // async - returns a value
    Deferred<int> deferred = async([]{
        delay(100);
        return 42;
    });

    int result = deferred.await();  // Suspend point
    std::cout << "Result: " << result << std::endl;

    // async with dispatcher
    auto d2 = async(Dispatchers::IO, []{
        return std::string("loaded from IO");
    });
}

// =============================================================================
// Test: runBlocking
// =============================================================================
void test_run_blocking() {
    int result = runBlocking([]{
        delay(100);
        return 42;
    });

    runBlocking(Dispatchers::Default, []{
        std::cout << "Blocking on default" << std::endl;
    });
}

// =============================================================================
// Test: withContext
// =============================================================================
void test_with_context() {
    runBlocking([]{
        std::cout << "On main" << std::endl;

        auto data = withContext(Dispatchers::IO, []{
            // Simulate loading data
            delay(100);
            return std::vector<int>{1, 2, 3, 4, 5};
        });

        std::cout << "Got " << data.size() << " items" << std::endl;
    });
}

// =============================================================================
// Test: coroutineScope and supervisorScope
// =============================================================================
void test_scopes() {
    runBlocking([]{
        coroutineScope([](CoroutineScope& scope){
            launch([]{
                delay(100);
                std::cout << "Child 1 done" << std::endl;
            });

            launch([]{
                delay(200);
                std::cout << "Child 2 done" << std::endl;
            });

            return 0;  // Wait for all children
        });

        supervisorScope([](CoroutineScope& scope){
            // Children can fail independently
            launch([]{
                throw std::runtime_error("oops");
            });

            launch([]{
                delay(100);
                std::cout << "This still runs!" << std::endl;
            });

            return 0;
        });
    });
}

// =============================================================================
// Test: Channels
// =============================================================================
void test_channels() {
    Channel<int> channel;

    // Producer
    launch([&channel]{
        for (int i = 0; i < 5; i++) {
            channel.send(i);
            delay(100);
        }
        channel.close();
    });

    // Consumer
    launch([&channel]{
        for (auto value : channel) {
            std::cout << "Received: " << value << std::endl;
        }
    });
}

// =============================================================================
// Test: Flow
// =============================================================================
void test_flow() {
    Flow<int> numbers;

    // Collect
    numbers.collect([](int n){
        std::cout << "Got: " << n << std::endl;
    });

    // Operators
    auto processed = numbers
        .filter([](int n){ return n % 2 == 0; })
        .map([](int n){ return n * 2; })
        .take(10);

    // Terminal operations
    int first = numbers.first();
    auto maybe = numbers.firstOrNull();
}

// =============================================================================
// Test: Mutex and Semaphore
// =============================================================================
void test_sync_primitives() {
    Mutex mutex;
    int counter = 0;

    launch([&]{
        mutex.withLock([&]{
            counter++;
        });
    });

    Semaphore semaphore(3);  // 3 permits

    semaphore.withPermit([]{
        std::cout << "Got permit!" << std::endl;
    });
}

// =============================================================================
// Test: Timeout
// =============================================================================
void test_timeout() {
    runBlocking([]{
        auto result = withTimeoutOrNull(std::chrono::milliseconds(1000), []{
            delay(500);
            return 42;
        });

        if (result) {
            std::cout << "Got: " << *result << std::endl;
        } else {
            std::cout << "Timed out!" << std::endl;
        }
    });
}

// =============================================================================
// Test: Cancellation
// =============================================================================
void test_cancellation() {
    runBlocking([]{
        Job job = launch([]{
            while (isActive()) {
                ensureActive();  // Throws if cancelled
                delay(100);
                std::cout << "Working..." << std::endl;
            }
        });

        delay(500);
        job.cancel();
        job.join();
    });
}

// =============================================================================
// Test: Job states
// =============================================================================
void test_job_states() {
    Job job = launch([]{
        delay(1000);
    });

    std::cout << "Active: " << job.isActive() << std::endl;
    std::cout << "Completed: " << job.isCompleted() << std::endl;
    std::cout << "Cancelled: " << job.isCancelled() << std::endl;
}

// =============================================================================
// Main
// =============================================================================
int main() {
    std::cout << "=== Kotlin Coroutines for C++ - IDE Test ===" << std::endl;

    test_basic_suspend();
    test_delay();
    test_launch_async();
    test_run_blocking();
    test_with_context();
    test_scopes();
    test_channels();
    test_flow();
    test_sync_primitives();
    test_timeout();
    test_cancellation();
    test_job_states();

    std::cout << "All tests compiled successfully!" << std::endl;
    return 0;
}
