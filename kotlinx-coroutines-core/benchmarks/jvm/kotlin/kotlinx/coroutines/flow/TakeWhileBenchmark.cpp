// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/benchmarks/jvm/kotlin/kotlinx/coroutines/flow/TakeWhileBenchmark.kt
// TODO: Resolve imports and dependencies
// TODO: Implement JMH benchmark annotations
// TODO: Handle suspend functions and coroutines
// TODO: Implement Flow operations

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.flow.internal.*
// TODO: import kotlinx.coroutines.flow.internal.AbortFlowException
// TODO: import kotlinx.coroutines.flow.internal.unsafeFlow
// TODO: import org.openjdk.jmh.annotations.*
// TODO: import java.util.concurrent.*

// TODO: @Warmup(iterations = 5, time = 1, timeUnit = TimeUnit.SECONDS)
// TODO: @Measurement(iterations = 5, time = 1, timeUnit = TimeUnit.SECONDS)
// TODO: @Fork(value = 1)
// TODO: @BenchmarkMode(Mode.AverageTime)
// TODO: @OutputTimeUnit(TimeUnit.MICROSECONDS)
// TODO: @State(Scope.Benchmark)
class TakeWhileBenchmark {
private:
    // TODO: @Param("1", "10", "100", "1000")
    int size_ = 0;

    // TODO: suspend inline function
    template<typename FlowType>
    int consume(FlowType&& flow) {
        return flow.filter([](int64_t it) {
                return it % 2L != 0L;
            })
            .map([](int64_t it) {
                return it * it;
            })
            .count();
    }

public:
    // TODO: @Benchmark annotation
    int baseline() {
        return run_blocking<int>([this]() {
            // TODO: (0L until size).asFlow().consume()
            return 0;
        });
    }

    // TODO: @Benchmark annotation
    int take_while_direct() {
        return run_blocking<int>([this]() {
            // TODO: (0L..Long.MAX_VALUE).asFlow().takeWhileDirect { it < size }.consume()
            return 0;
        });
    }

    // TODO: @Benchmark annotation
    int take_while_via_collect_while() {
        return run_blocking<int>([this]() {
            // TODO: (0L..Long.MAX_VALUE).asFlow().takeWhileViaCollectWhile { it < size }.consume()
            return 0;
        });
    }

private:
    // Direct implementation by checking predicate and throwing AbortFlowException
    template<typename T, typename Predicate>
    Flow<T> take_while_direct(Flow<T> flow, Predicate predicate) {
        return unsafe_flow([=](auto collector) {
            try {
                flow.collect([&](T value) {
                    if (predicate(value)) {
                        collector.emit(value);
                    } else {
                        throw AbortFlowException(collector);
                    }
                });
            } catch (const AbortFlowException& e) {
                e.check_ownership(collector);
            }
        });
    }

    // Essentially the same code, but reusing the logic via collectWhile function
    template<typename T, typename Predicate>
    Flow<T> take_while_via_collect_while(Flow<T> flow, Predicate predicate) {
        return unsafe_flow([=](auto collector) {
            // This return is needed to work around a bug in JS BE: KT-39227
            return flow.collect_while([&](T value) {
                if (predicate(value)) {
                    collector.emit(value);
                    return true;
                } else {
                    return false;
                }
            });
        });
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
