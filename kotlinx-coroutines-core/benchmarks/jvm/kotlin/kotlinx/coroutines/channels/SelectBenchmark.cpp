// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/benchmarks/jvm/kotlin/kotlinx/coroutines/channels/SelectBenchmark.kt
// TODO: Resolve imports and dependencies
// TODO: Implement JMH benchmark annotations
// TODO: Handle suspend functions and coroutines
// TODO: Implement Channel and select

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlinx.coroutines.selects.*
// TODO: import org.openjdk.jmh.annotations.*
// TODO: import java.util.concurrent.*

// TODO: @Warmup(iterations = 8, time = 1, timeUnit = TimeUnit.SECONDS)
// TODO: @Measurement(iterations = 8, time = 1, timeUnit = TimeUnit.SECONDS)
// TODO: @Fork(value = 1)
// TODO: @BenchmarkMode(Mode.AverageTime)
// TODO: @OutputTimeUnit(TimeUnit.MICROSECONDS)
// TODO: @State(Scope.Benchmark)
class SelectBenchmark {
private:
    // 450
    static constexpr int kIterations = 1000;

public:
    // TODO: @Benchmark annotation
    void stress_select() {
        run_blocking([]() {
            auto ping_pong = Channel<void>();
            launch([&]() {
                repeat(kIterations, [&]() {
                    select<void>([&](auto selector) {
                        ping_pong.on_send([]() {});
                    });
                });
            });

            launch([&]() {
                repeat(kIterations, [&]() {
                    select<void>([&](auto selector) {
                        ping_pong.on_receive([]() {});
                    });
                });
            });
        });
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
