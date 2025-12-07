// Transliterated from: test-utils/jvm/src/Threads.kt
// TODO: #include <kotlinx/coroutines/coroutines.hpp>
// TODO: #include <thread>
// TODO: #include <unordered_set>
// TODO: #include <vector>
// TODO: #include <string>
// TODO: #include <chrono>
// TODO: #include <iostream>

namespace kotlinx {
namespace coroutines {
namespace testing {

constexpr long kWaitLostThreads = 10000L; // 10s
std::unordered_set<std::string> ignore_lost_threads_set;

void ignore_lost_threads(const std::vector<std::string>& names) {
    for (const auto& name : names) {
        ignore_lost_threads_set.insert(name);
    }
}

void ignore_lost_threads(const std::string& name) {
    ignore_lost_threads_set.insert(name);
}

std::unordered_set<std::thread::id> current_threads() {
    // TODO: Implement Thread.enumerate equivalent
    // This is platform-specific and doesn't have a direct C++ standard library equivalent
    // On POSIX systems, would need to use platform-specific APIs
    // On Windows, would use Windows-specific thread enumeration

    std::unordered_set<std::thread::id> threads;
    // Placeholder implementation
    return threads;
}

void dump_threads(const std::vector<std::thread*>& thread_list, const std::string& header) {
    std::cout << "=== " << header << std::endl;

    for (const std::thread* thread : thread_list) {
        // TODO: Get thread name and state
        // std::cout << "Thread \"" << thread->name() << "\" " << thread->state() << std::endl;

        // TODO: Get stack trace
        // auto trace = thread->get_stack_trace();
        // for (const auto& frame : trace) {
        //     std::cout << "\tat " << frame.class_name() << "." << frame.method_name()
        //               << "(" << frame.file_name() << ":" << frame.line_number() << ")" << std::endl;
        // }
        std::cout << std::endl;
    }

    std::cout << "===" << std::endl;
}

class PoolThread : public std::thread {
public:
    // @JvmField
    ExecutorCoroutineDispatcher* dispatcher; // for debugging & tests

    template<typename Callable>
    PoolThread(ExecutorCoroutineDispatcher* dispatcher, Callable&& target, const std::string& name)
        : std::thread(std::forward<Callable>(target))
        , dispatcher(dispatcher) {
        // TODO: Set thread name (platform-specific)
        // TODO: Set daemon flag (doesn't exist in C++ std::thread)
    }
};

void dump_threads(ExecutorCoroutineDispatcher& dispatcher, const std::string& header) {
    auto all_threads = current_threads();
    std::vector<std::thread*> matching_threads;

    // TODO: Filter threads where thread is PoolThread && thread.dispatcher == &dispatcher
    // for (auto thread_id : all_threads) {
    //     // Get actual thread object from ID (platform-specific)
    //     // if (PoolThread* pool_thread = dynamic_cast<PoolThread*>(thread)) {
    //     //     if (pool_thread->dispatcher == &dispatcher) {
    //     //         matching_threads.push_back(pool_thread);
    //     //     }
    //     // }
    // }

    dump_threads(matching_threads, header);
}

void check_test_threads(const std::unordered_set<std::thread::id>& threads_before) {
    // give threads some time to shutdown
    auto wait_till = std::chrono::system_clock::now() + std::chrono::milliseconds(kWaitLostThreads);
    std::vector<std::thread*> diff;

    do {
        auto threads_after = current_threads();
        diff.clear();

        // TODO: Calculate diff = threads_after - threads_before
        // Filter out threads whose names start with any prefix in ignore_lost_threads_set
        // for (auto thread_id : threads_after) {
        //     if (threads_before.find(thread_id) == threads_before.end()) {
        //         // Get actual thread object (platform-specific)
        //         // std::thread* thread = get_thread_by_id(thread_id);
        //         // std::string thread_name = thread->name();
        //         // bool should_ignore = false;
        //         // for (const auto& prefix : ignore_lost_threads_set) {
        //         //     if (thread_name.starts_with(prefix)) {
        //         //         should_ignore = true;
        //         //         break;
        //         //     }
        //         // }
        //         // if (!should_ignore) {
        //         //     diff.push_back(thread);
        //         // }
        //     }
        // }

        if (diff.empty()) break;
    } while (std::chrono::system_clock::now() <= wait_till);

    ignore_lost_threads_set.clear();

    if (diff.empty()) return;

    // Build list of thread names
    std::vector<std::string> thread_names;
    for (const auto* thread : diff) {
        // TODO: thread_names.push_back(thread->name());
    }

    std::string message = "Lost threads: ";
    for (size_t i = 0; i < thread_names.size(); ++i) {
        if (i > 0) message += ", ";
        message += thread_names[i];
    }

    std::cout << "!!! " << message << std::endl;
    dump_threads(diff, "Dumping lost thread stack traces");
    throw std::runtime_error(message);
}

} // namespace testing
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement Thread enumeration (platform-specific)
// 2. Implement thread name getting/setting (platform-specific)
// 3. Implement thread state querying
// 4. Implement stack trace capture for threads
// 5. Implement daemon thread concept (or document it's not available in C++)
// 6. Implement thread ID to thread object mapping
// 7. Implement string starts_with if using C++17 or earlier
// 8. Consider using platform-specific APIs:
//    - POSIX: pthread_enumerate, pthread_getname_np
//    - Windows: CreateToolhelp32Snapshot, Thread32First/Next
// 9. Implement ExecutorCoroutineDispatcher interface
// 10. Add proper includes for all dependencies
// 11. Handle platform differences (POSIX vs Windows)
// 12. Document limitations compared to JVM thread introspection
