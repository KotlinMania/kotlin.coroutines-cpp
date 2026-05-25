/**
 * Transliterated from: kotlinx-coroutines-core/native/test/WorkerTest.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *   imports: kotlinx.coroutines.testing.*, kotlinx.coroutines.channels.*,
 *            kotlin.coroutines.*, kotlin.native.concurrent.*, kotlin.test.*
 *
 * Tests that runBlocking works correctly when invoked on a Kotlin/Native Worker, and that
 * it does not crash after the worker is requested to terminate. The Kotlin
 * `runBlocking { ... }` wrapper drives the suspending body to completion; the C++ port
 * uses `run_blocking(...)` which performs the same in-place suspension drive.
 */

#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/test/TestBase.hpp"

#include <future>

namespace kotlinx::coroutines {

class WorkerTest : public test::TestBase {
public:
    /**
     * Upstream:
     *   @Test fun testLaunchInWorker() {
     *       val worker = Worker.start()
     *       worker.execute(TransferMode.SAFE, {}) {
     *           runBlocking { launch { }.join(); delay(1) }
     *       }.result
     *       worker.requestTermination()
     *   }
     */
    static void test_launch_in_worker() {
        auto worker = Worker::start();
        worker.execute(TransferMode::SAFE, []() {}, []() {
            run_blocking([]() {
                launch([]() {})->join();
                delay(1, nullptr);
            });
        }).result();
        worker.request_termination();
    }

    /**
     * Upstream:
     *   @Test fun testLaunchInWorkerThroughGlobalScope() { ... }
     */
    static void test_launch_in_worker_through_global_scope() {
        auto worker = Worker::start();
        worker.execute(TransferMode::SAFE, []() {}, []() {
            run_blocking([]() {
                CoroutineScope(EmptyCoroutineContext::instance())
                    .launch([&]() { delay(10, nullptr); })
                    ->join();
            });
        }).result();
        worker.request_termination();
    }

    /**
     * Upstream:
     *   @Test fun testRunBlockingInTerminatedWorker() { ... }
     *
     * Verifies that runBlocking does not crash when Worker.requestTermination is called
     * on the worker hosting the runBlocking call.
     */
    static void test_run_blocking_in_terminated_worker() {
        auto worker_in_run_blocking = channels::make_channel<Unit>();
        auto worker_terminated = channels::make_channel<Unit>();
        auto check_resumption = channels::make_channel<Unit>();
        auto finished = channels::make_channel<Unit>();
        auto worker = Worker::start();
        worker.execute_after(0, [&]() {
            run_blocking([&]() {
                worker_in_run_blocking->send(Unit{}, nullptr);
                worker_terminated->receive(nullptr);
                check_resumption->receive(nullptr);
                finished->send(Unit{}, nullptr);
            });
        });
        run_blocking([&]() {
            worker_in_run_blocking->receive(nullptr);
            worker.request_termination();
            worker_terminated->send(Unit{}, nullptr);
            check_resumption->send(Unit{}, nullptr);
            finished->receive(nullptr);
        });
    }
};

} // namespace kotlinx::coroutines
