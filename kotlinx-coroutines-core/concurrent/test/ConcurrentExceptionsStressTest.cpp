// Original: kotlinx-coroutines-core/concurrent/test/ConcurrentExceptionsStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement async, launch, CoroutineStart
// TODO: Implement stressTestMultiplier, CloseableCoroutineDispatcher
// TODO: Implement exception handling and suppressed exceptions
// TODO: Handle @AfterTest, @Suppress annotations

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.exceptions.*
// TODO: import kotlinx.coroutines.internal.*
// TODO: import kotlin.test.*

class ConcurrentExceptionsStressTest : public TestBase {
private:
    static constexpr int n_workers = 4;
    int n_repeat = 1000 * stress_test_multiplier;

    std::vector<CloseableCoroutineDispatcher*> workers;

public:
    // @AfterTest
    // TODO: Convert test annotation
    void tear_down() {
        for (auto& worker : workers) {
            worker->close();
        }
    }

    // @Test
    // TODO: Convert test annotation
    void test_stress() {
        runTest([&]() {
            // TODO: suspend function
            workers.resize(n_workers);
            for (int index = 0; index < n_workers; ++index) {
                workers[index] = new_single_thread_context("JobExceptionsStressTest-" + std::to_string(index));
            }

            for (int i = 0; i < n_repeat; ++i) {
                test_once();
            }
        });
    }

    // @Suppress("SuspendFunctionOnCoroutineScope") // workaround native inline fun stacktraces
    // TODO: Implement suspend function
    void test_once(/* CoroutineScope scope */) {
        // TODO: suspend function
        auto deferred = async(NonCancellable, [&]() {
            // TODO: suspend function
            for (int index = 0; index < n_workers; ++index) {
                // Always launch a coroutine even if parent job was already cancelled (atomic start)
                launch(workers[index], CoroutineStart::kAtomic, [&, index]() {
                    // TODO: suspend function
                    random_wait();
                    throw StressException(index);
                });
            }
        });
        deferred.join();
        assertTrue(deferred.is_cancelled());
        auto completion_exception = deferred.get_completion_exception_or_null();
        auto* cause = dynamic_cast<StressException*>(completion_exception);
        if (!cause) {
            unexpected_exception("completion", completion_exception);
        }
        auto suppressed = cause->suppressed_exceptions();
        std::vector<int> indices;
        indices.push_back(cause->index);
        for (size_t index = 0; index < suppressed.size(); ++index) {
            auto* e = dynamic_cast<StressException*>(suppressed[index]);
            if (!e) {
                unexpected_exception("suppressed " + std::to_string(index), suppressed[index]);
            }
            indices.push_back(e->index);
        }
        for (int index = 0; index < n_workers; ++index) {
            bool found = std::find(indices.begin(), indices.end(), index) != indices.end();
            assertTrue(found, "Exception " + std::to_string(index) + " is missing: " + to_string(indices));
        }
        assertEquals(n_workers, static_cast<int>(indices.size()), "Duplicated exceptions in list: " + to_string(indices));
    }

private:
    [[noreturn]] void unexpected_exception(const std::string& msg, std::exception* e) {
        throw std::runtime_error("Unexpected " + msg + " exception");
    }

    class StressException : public std::exception {
    public:
        int index;

        explicit StressException(int index) : index(index) {}
    };
};

} // namespace coroutines
} // namespace kotlinx
