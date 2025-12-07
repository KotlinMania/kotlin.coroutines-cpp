// Original: kotlinx-coroutines-core/common/test/LimitedParallelismSharedTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: Handle limited parallelism dispatchers
// TODO: Map test framework annotations to C++ test framework

#include <vector>

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.coroutines.CoroutineContext
// TODO: import kotlin.coroutines.EmptyCoroutineContext
// TODO: import kotlin.test.*

class LimitedParallelismSharedTest : public TestBase {
public:
    // TODO: @Test
    void test_limited_default() {
        // TODO: runTest {
        // Test that evaluates the very basic completion of tasks in limited dispatcher
        // for all supported platforms.
        // For more specific and concurrent tests, see 'concurrent' package.
        // TODO: const auto view = Dispatchers.Default.limited_parallelism(1);
        // TODO: const auto view2 = Dispatchers.Default.limited_parallelism(1);
        // TODO: const auto j1 = launch(view) {
        //     while (true) {
        //         yield();
        //     }
        // }
        // TODO: const auto j2 = launch(view2) { j1.cancel(); }
        // TODO: join_all(j1, j2);
        // TODO: }
    }

    // TODO: @Test
    void test_parallelism_spec() {
        // TODO: assertFailsWith<IllegalArgumentException>([&] { Dispatchers.Default.limited_parallelism(0); });
        // TODO: assertFailsWith<IllegalArgumentException>([&] { Dispatchers.Default.limited_parallelism(-1); });
        // TODO: assertFailsWith<IllegalArgumentException>([&] { Dispatchers.Default.limited_parallelism(INT_MIN); });
        // TODO: Dispatchers.Default.limited_parallelism(INT_MAX);
    }

    /**
     * Checks that even if the dispatcher sporadically fails, the limited dispatcher will still allow reaching the
     * target parallelism level.
     */
    // TODO: @Test
    void test_limited_parallelism_of_occasionally_failing_dispatcher() {
        const int limit = 5;
        bool do_fail = false;
        std::vector<Runnable*> worker_queue;
        // TODO: const auto limited = [&]() {
        //     struct : public CoroutineDispatcher {
        //         void dispatch(const CoroutineContext& context, Runnable* block) override {
        //             if (do_fail) throw TestException();
        //             worker_queue.push_back(block);
        //         }
        //     };
        // }().limited_parallelism(limit);
        for (int i = 0; i < 6 * limit; i++) {
            // TODO: try {
            //     limited.dispatch(EmptyCoroutineContext, new Runnable{[]{}});
            // } catch (const DispatchException&) {
            //     // ignore
            // }
            do_fail = !do_fail;
        }
        // TODO: assertEquals(limit, worker_queue.size());
    }
};

} // namespace coroutines
} // namespace kotlinx
