// Original: kotlinx-coroutines-core/concurrent/test/flow/CombineStressTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement flow, combine, flatMapLatest
// TODO: Implement withContext, CoroutineExceptionHandler
// TODO: Implement stressTestMultiplierSqrt

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.test.*

class CombineStressTest : public TestBase {
public:
    // @Test
    // TODO: Convert test annotation
    void test_cancellation() {
        runTest([&]() {
            // TODO: suspend function
            with_context(Dispatchers::Default + CoroutineExceptionHandler([&](auto&, auto&) {
                expectUnreached();
            }), [&]() {
                // TODO: suspend function
                flow([&](auto& emitter) {
                    // TODO: suspend function
                    expect(1);
                    for (int i = 0; i < 1'000 * stress_test_multiplier; ++i) {
                        emitter.emit(i);
                    }
                })
                .flat_map_latest([&](int it) {
                    return combine(flow_of(it), flow_of(it), [](auto arr) {
                        return arr[0];
                    });
                })
                .collect([](int) {});
                finish(2);
                reset();
            });
        });
    }

    // @Test
    // TODO: Convert test annotation
    void test_failure() {
        runTest([&]() {
            // TODO: suspend function
            int inner_iterations = 100 * stress_test_multiplier_sqrt;
            int outer_iterations = 10 * stress_test_multiplier_sqrt;
            with_context(Dispatchers::Default + CoroutineExceptionHandler([&](auto&, auto&) {
                expectUnreached();
            }), [&]() {
                // TODO: suspend function
                for (int i = 0; i < outer_iterations; ++i) {
                    try {
                        flow([&](auto& emitter) {
                            // TODO: suspend function
                            expect(1);
                            for (int j = 0; j < inner_iterations; ++j) {
                                emitter.emit(j);
                            }
                        })
                        .flat_map_latest([&](int it) {
                            return combine(flow_of(it), flow_of(it), [](auto arr) {
                                return arr[0];
                            });
                        })
                        .on_each([&](int it) {
                            if (it >= inner_iterations / 2) throw TestException();
                        })
                        .collect([](int) {});
                    } catch (const TestException& e) {
                        expect(2);
                    }
                    finish(3);
                    reset();
                }
            });
        });
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
