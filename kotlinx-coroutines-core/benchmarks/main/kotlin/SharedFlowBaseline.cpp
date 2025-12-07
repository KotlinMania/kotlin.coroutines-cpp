// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/benchmarks/main/kotlin/SharedFlowBaseline.kt
// TODO: Resolve imports and dependencies
// TODO: Implement kotlinx.benchmark annotations
// TODO: Handle suspend functions and coroutines
// TODO: Implement MutableSharedFlow

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.flow.*
// TODO: import kotlinx.benchmark.*

// Stresses out 'synchronized' codepath in MutableSharedFlow
// TODO: @State(Scope.Benchmark)
// TODO: @Measurement(iterations = 3, time = 1, timeUnit = BenchmarkTimeUnit.SECONDS)
// TODO: @OutputTimeUnit(BenchmarkTimeUnit.MICROSECONDS)
// TODO: @BenchmarkMode(Mode.AverageTime)
class SharedFlowBaseline {
private:
    int size_ = 10000;

public:
    // TODO: @Benchmark annotation
    void baseline() {
        run_blocking([this]() {
            auto flow = MutableSharedFlow<void>();
            launch([&]() {
                repeat(size_, [&]() {
                    flow.emit();
                });
            });

            flow.take(size_).collect([](auto) {});
        });
    }
};

} // namespace coroutines
} // namespace kotlinx
