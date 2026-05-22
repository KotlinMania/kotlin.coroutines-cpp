/**
 * Transliterated from: kotlinx-coroutines-core/nativeDarwin/test/MainDispatcherTest.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *   imports: kotlinx.coroutines.testing.*, kotlinx.cinterop.*,
 *            platform.CoreFoundation.*, platform.darwin.*, kotlin.coroutines.*,
 *            kotlin.test.*
 *
 * Upstream:
 *   class MainDispatcherTest : MainDispatcherTestBase.WithRealTimeDelay() {
 *       override fun isMainThread(): Boolean =
 *           CFRunLoopGetCurrent() == CFRunLoopGetMain()
 *       override fun shouldSkipTesting(): Boolean = isMainThread()
 *       override fun scheduleOnMainQueue(block: () -> Unit) {
 *           autoreleasepool {
 *               dispatch_async(dispatch_get_main_queue()) { block() }
 *           }
 *       }
 *   }
 *
 * Darwin-only: bridges the cross-platform MainDispatcherTestBase contract onto CFRunLoop
 * and the GCD main queue. The C++ port matches the upstream behavior exactly when
 * compiled for Apple; on other platforms the class is omitted entirely so the test suite
 * compiles cleanly.
 */

#if defined(__APPLE__)

#include "kotlinx/coroutines/test/MainDispatcherTestBase.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <dispatch/dispatch.h>

#include <functional>

namespace kotlinx::coroutines {

class MainDispatcherTest : public test::MainDispatcherTestBase::WithRealTimeDelay {
public:
    bool is_main_thread() override {
        return CFRunLoopGetCurrent() == CFRunLoopGetMain();
    }

    // Skip if already on the main thread — runBlocking doesn't play well with that.
    bool should_skip_testing() override { return is_main_thread(); }

    void schedule_on_main_queue(std::function<void()> block) override {
        @autoreleasepool {
            dispatch_async(dispatch_get_main_queue(), ^{ block(); });
        }
    }
};

} // namespace kotlinx::coroutines

#endif // __APPLE__
