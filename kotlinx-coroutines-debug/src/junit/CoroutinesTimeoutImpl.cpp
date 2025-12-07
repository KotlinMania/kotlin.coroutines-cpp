#include "kotlinx/coroutines/core_fwd.hpp"
// Original Kotlin package: kotlinx.coroutines.debug
// Line-by-line C++ transliteration from Kotlin
//
// TODO: inline function with crossinline lambda - translate to template with forwarding
// TODO: CountDownLatch - Java concurrency primitive, use C++ equivalent (condition_variable + mutex)
// TODO: FutureTask - Java concurrency, use C++ future/promise
// TODO: Thread - Java thread, use C++ std::thread
// TODO: TimeUnit - Java time unit, use C++ chrono
// TODO: TimeoutException, ExecutionException - Java exceptions, use C++ exception types
// TODO: System.err, System.out - Java I/O streams, use std::cerr, std::cout
// TODO: DebugProbes - needs implementation

#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <iostream>
#include <string>

namespace kotlinx {
namespace coroutines {
namespace debug {

// Forward declarations
class CoroutineInfo;
class Job;
namespace DebugProbes {
    void dump_coroutines(std::ostream& out);
    std::vector<CoroutineInfo> dump_coroutines_info();
}

// Run invocation in a separate thread with the given timeout in ms, after which the coroutines info is dumped and, if
// cancelOnTimeout is set, the execution is interrupted.
//
// Assumes that DebugProbes are installed. Does not deinstall them.
// TODO: inline function with crossinline lambda
template<typename T, typename F, typename E>
T run_with_timeout_dumping_coroutines(
    const std::string& method_name,
    long test_timeout_ms,
    bool cancel_on_timeout,
    E&& init_cancellation_exception,
    F&& invocation
) {
    // TODO: CountDownLatch -> condition_variable + mutex
    std::mutex mutex;
    std::condition_variable cv;
    bool test_started = false;

    // TODO: FutureTask -> std::packaged_task or std::promise/future
    auto test_result = std::async(:launch::async std, [&]() -> T {
        {
            std::lock_guard<std::mutex> lock(mutex);
            test_started = true;
        }
        cv.notify_all();
        return invocation();
    });

    // We are using hand-rolled thread instead of single thread executor
    // in order to be able to safely interrupt thread in the end of a test
    // TODO: Thread -> std::thread (already using std::async above)

    try {
        // Await until test is started to take only test execution time into account
        {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&]{ return test_started; });
        }

        // TODO: testResult.get(testTimeoutMs, TimeUnit.MILLISECONDS)
        auto status = test_result.wait_for(std::chrono::milliseconds(test_timeout_ms));
        if (status == std::future_status::timeout) {
            // TODO: handle_timeout - need thread handle to get stack trace
            throw std::runtime_error("Timeout"); // Simplified
        }
        return test_result.get();
    } catch (const std::runtime_error& e) {
        // TODO: distinguish between TimeoutException and ExecutionException
        // For ExecutionException, throw e.cause ?: e
        throw;
    }
}

// TODO: Private function
template<typename E>
[[noreturn]] void handle_timeout(
    std::thread::native_handle_type test_thread,
    const std::string& method_name,
    long test_timeout_ms,
    bool cancel_on_timeout,
    E&& cancellation_exception
) {
    std::string units;
    if (test_timeout_ms % 1000 == 0L) {
        units = std::to_string(test_timeout_ms / 1000) + " seconds";
    } else {
        units = std::to_string(test_timeout_ms) + " milliseconds";
    }

    std::cerr << "\nTest " << method_name << " timed out after " << units << "\n" << std::endl;
    std::cerr.flush();

    DebugProbes::dump_coroutines(std::cout);
    std::cout.flush(); // Synchronize serr/sout

    // Order is important:
    // 1) Create exception with a stacktrace of hang test
    // 2) Cancel all coroutines via debug agent API (changing system state!)
    // 3) Throw created exception

    // TODO: cancellation_exception.attach_stacktrace_from(test_thread);
    // TODO: test_thread.interrupt();
    cancel_if_necessary(cancel_on_timeout);
    // If timed out test throws an exception, we can't do much except ignoring it
    throw cancellation_exception;
}

inline void cancel_if_necessary(bool cancel_on_timeout) {
    if (cancel_on_timeout) {
        auto coroutines_info = DebugProbes::dump_coroutines_info();
        for (auto& info : coroutines_info) {
            // TODO: it.job*.cancel()
            // Job* job = info.job();
            // if (job != nullptr) {
            //     job->cancel();
            // }
        }
    }
}

// TODO: Extension function on Throwable
template<typename E>
void attach_stacktrace_from(E& exception, std::thread::native_handle_type thread) {
    // TODO: const auto stack_trace = thread.stackTrace;
    // TODO: exception.stack_trace = stack_trace;
}

} // namespace debug
} // namespace coroutines
} // namespace kotlinx
