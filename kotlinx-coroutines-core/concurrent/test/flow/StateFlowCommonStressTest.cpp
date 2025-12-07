// Original: kotlinx-coroutines-core/concurrent/test/flow/StateFlowCommonStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement MutableStateFlow
// TODO: Implement Random, onEach, take, map, sum

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.random.*
// TODO: import kotlin.test.*

// A simplified version of StateFlowStressTest
class StateFlowCommonStressTest : public TestBase {
private:
    MutableStateFlow<int64_t> state{0};

public:
    // @Test
    // TODO: Convert test annotation
    void test_single_emitter_and_collector() {
        runTest([&]() {
            // TODO: suspend function
            int64_t collected = 0;
            auto collector = launch(Dispatchers::Default, [&]() {
                // TODO: suspend function
                // collect, but abort and collect again after every 1000 values to stress allocation/deallocation
                int cnt;
                do {
                    int batch_size = Random::next_int(1, 1000);
                    int index = 0;
                    cnt = state.on_each([&](int64_t value) {
                        // the first value in batch is allowed to repeat, but cannot go back
                        bool ok = (index++ == 0) ? (value >= collected) : (value > collected);
                        if (!ok) {
                            throw std::runtime_error("Values must be monotonic, but " +
                                std::to_string(value) + " is not, was " + std::to_string(collected));
                        }
                        collected = value;
                    })
                    .take(batch_size)
                    .map([](int64_t) { return 1; })
                    .sum();
                } while (cnt == batch_size);
            });

            int64_t current = 1;
            auto emitter = launch([&]() {
                // TODO: suspend function
                while (true) {
                    state.value = current++;
                    if (current % 1000 == 0) yield(); // make it cancellable
                }
            });

            delay(3000);
            emitter.cancel_and_join();
            collector.cancel_and_join();
            assertTrue(current >= collected / 2);
        });
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
