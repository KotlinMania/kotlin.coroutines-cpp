/**
 * Transliterated from: kotlinx-coroutines-core/native/test/DelayExceptionTest.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *   imports: kotlinx.coroutines.testing.*, kotlin.coroutines.*, kotlin.test.*
 *
 * Verifies that a launch that delays for Long.MAX_VALUE can be cancelled cleanly without
 * the delay timer surfacing an exception into the cancelled scope. The Kotlin
 * `runBlocking { ... }` wrapper drives the suspending body to completion; the C++ port
 * uses `run_blocking(...)` which performs the same in-place suspension drive.
 */

#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/test/TestBase.hpp"

#include <climits>

namespace kotlinx::coroutines {

class DelayExceptionTest : public test::TestBase {
public:
    /**
     * Upstream:
     *   @Test fun testMaxDelay() = runBlocking {
     *       expect(1)
     *       val job = launch { expect(2); delay(Long.MAX_VALUE) }
     *       yield()
     *       job.cancel()
     *       finish(3)
     *   }
     */
    void test_max_delay() {
        run_blocking([this]() {
            expect(1);
            auto job = launch([this]() {
                expect(2);
                delay(LONG_MAX, nullptr);
            });
            yield(nullptr);
            job->cancel();
            finish(3);
        });
    }
};

} // namespace kotlinx::coroutines
