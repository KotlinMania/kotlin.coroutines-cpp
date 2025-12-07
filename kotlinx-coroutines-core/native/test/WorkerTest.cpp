// Original file: kotlinx-coroutines-core/native/test/WorkerTest.kt
// TODO: Remove or convert import statements
// TODO: Convert @Test annotation to appropriate test framework
// TODO: Convert suspend functions and coroutine builders
// TODO: Handle Worker and TransferMode APIs
// TODO: Handle TestBase inheritance

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.native.concurrent.*
// TODO: import kotlin.test.*

class WorkerTest : public TestBase {
public:
    // TODO: @Test
    void test_launch_in_worker() {
        auto worker = Worker::start();
        worker.execute(TransferMode::kSafe, []() {}, []() {
            // TODO: runBlocking is a suspend function
            // runBlocking {
            launch([]() {}).join();
            delay(1);
            // }
        }).result;
        worker.request_termination();
    }

    // TODO: @Test
    void test_launch_in_worker_through_global_scope() {
        auto worker = Worker::start();
        worker.execute(TransferMode::kSafe, []() {}, []() {
            // TODO: runBlocking is a suspend function
            // runBlocking {
            CoroutineScope(EmptyCoroutineContext).launch([&]() {
                delay(10);
            }).join();
            // }
        }).result;
        worker.request_termination();
    }

    /**
     * Test that [runBlocking] does not crash after [Worker.requestTermination] is called on the worker that runs it.
     */
    // TODO: @Test
    void test_run_blocking_in_terminated_worker() {
        Channel<void> worker_in_run_blocking;
        Channel<void> worker_terminated;
        Channel<void> check_resumption;
        Channel<void> finished;
        auto worker = Worker::start();
        worker.execute_after(0, [&]() {
            // TODO: runBlocking is a suspend function
            // runBlocking {
            worker_in_run_blocking.send();
            worker_terminated.receive();
            check_resumption.receive();
            finished.send();
            // }
        });
        // TODO: runBlocking is a suspend function
        // runBlocking {
        worker_in_run_blocking.receive();
        worker.request_termination();
        worker_terminated.send();
        check_resumption.send();
        finished.receive();
        // }
    }
};

} // namespace coroutines
} // namespace kotlinx
