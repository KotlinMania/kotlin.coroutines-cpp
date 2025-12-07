// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/benchmarks/jvm/kotlin/kotlinx/coroutines/channels/SimpleChannelBenchmark.kt
// TODO: Resolve imports and dependencies
// TODO: Implement JMH benchmark annotations
// TODO: Handle suspend functions and coroutines

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.*
// TODO: import org.openjdk.jmh.annotations.*
// TODO: import java.util.concurrent.*

// TODO: @Warmup(iterations = 5, time = 1, timeUnit = TimeUnit.SECONDS)
// TODO: @Measurement(iterations = 5, time = 1, timeUnit = TimeUnit.SECONDS)
// TODO: @Fork(value = 1)
// TODO: @BenchmarkMode(Mode.AverageTime)
// TODO: @OutputTimeUnit(TimeUnit.MICROSECONDS)
// TODO: @State(Scope.Benchmark)
class SimpleChannelBenchmark {
private:
    static constexpr int kIterations = 10000;

    // TODO: @Volatile annotation
    int sink_ = 0;

public:
    // TODO: @Benchmark annotation
    void cancellable() {
        run_blocking([this]() {
            auto ch = CancellableChannel();
            launch([&]() {
                repeat(kIterations, [&](int it) {
                    ch.send(it);
                });
            });

            launch([&]() {
                repeat(kIterations, [&]() {
                    sink_ = ch.receive();
                });
            });
        });
    }

    // TODO: @Benchmark annotation
    void cancellable_reusable() {
        run_blocking([this]() {
            auto ch = CancellableReusableChannel();
            launch([&]() {
                repeat(kIterations, [&](int it) {
                    ch.send(it);
                });
            });

            launch([&]() {
                repeat(kIterations, [&]() {
                    sink_ = ch.receive();
                });
            });
        });
    }

    // TODO: @Benchmark annotation
    void non_cancellable() {
        run_blocking([this]() {
            auto ch = NonCancellableChannel();
            launch([&]() {
                repeat(kIterations, [&](int it) {
                    ch.send(it);
                });
            });

            launch([&]() {
                repeat(kIterations, [&]() {
                    sink_ = ch.receive();
                });
            });
        });
    }
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
