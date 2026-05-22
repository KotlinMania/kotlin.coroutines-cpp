/**
 * Transliterated from: kotlinx-coroutines-core/nativeDarwin/test/Launcher.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *   imports: platform.CoreFoundation.*, kotlin.native.concurrent.*,
 *            kotlin.native.internal.test.*, kotlin.system.*
 *
 * Upstream:
 *   fun mainBackground(args: Array<String>) {
 *       val worker = Worker.start(name = "main-background")
 *       worker.execute(TransferMode.SAFE, { args }) {
 *           val result = testLauncherEntryPoint(it)
 *           exitProcess(result)
 *       }
 *       CFRunLoopRun()
 *       error("CFRunLoopRun should never return")
 *   }
 *
 * Darwin-only: a separate test entry that pins the actual test runner onto a worker
 * thread while CFRunLoopRun owns the main thread. The C++ port goes through the same
 * shape using a std::thread for the worker and CFRunLoopRun for the main thread; the
 * test entry point is `test_launcher_entry_point`, defined by the test framework
 * integration.
 */

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace kotlinx::coroutines {

int test_launcher_entry_point(const std::vector<std::string>& args);

[[noreturn]] void main_background(const std::vector<std::string>& args) {
#if defined(__APPLE__)
    std::thread worker([args]() {
        const int result = test_launcher_entry_point(args);
        std::exit(result);
    });
    worker.detach();
    CFRunLoopRun();
    throw std::logic_error("CFRunLoopRun should never return");
#else
    (void)args;
    throw std::logic_error("main_background is Darwin-only");
#endif
}

} // namespace kotlinx::coroutines
