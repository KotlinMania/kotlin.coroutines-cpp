/**
 * Transliterated from: kotlinx-coroutines-core/native/test/ConcurrentTestUtilities.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.exceptions
 *   imports: platform.posix.*, kotlin.native.concurrent.*
 *
 * Upstream:
 *   actual inline fun yieldThread() { sched_yield() }
 *   actual fun currentThreadName(): String = Worker.current.name
 *
 * `sched_yield(3)` is a POSIX yield that asks the scheduler to favour another runnable
 * thread. `Worker.current.name` returns the K/N worker's debug name; the C++ port uses
 * `pthread_getname_np` where available, falling back to a stringified thread id.
 */

#include <pthread.h>
#include <sched.h>

#include <sstream>
#include <string>
#include <thread>

namespace kotlinx::coroutines::exceptions {

inline void yield_thread() {
    ::sched_yield();
}

std::string current_thread_name() {
    char name[64] = {};
#if defined(__APPLE__) || defined(__linux__)
    if (::pthread_getname_np(::pthread_self(), name, sizeof(name)) == 0 && name[0] != '\0') {
        return std::string(name);
    }
#endif
    std::ostringstream out;
    out << "thread-" << std::this_thread::get_id();
    return out.str();
}

} // namespace kotlinx::coroutines::exceptions
