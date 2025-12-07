// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/benchmarks/jvm/kotlin/kotlinx/coroutines/SemaphoreBenchmark.kt
// TODO: Resolve imports and dependencies
// TODO: Implement JMH benchmark annotations
// TODO: Handle suspend functions and coroutines
// TODO: Implement Semaphore and Channel

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlinx.coroutines.scheduling.*
// TODO: import kotlinx.coroutines.sync.*
// TODO: import org.openjdk.jmh.annotations.*
// TODO: import java.util.concurrent.*

// TODO: @Warmup(iterations = 3, time = 500, timeUnit = TimeUnit.MICROSECONDS)
// TODO: @Measurement(iterations = 10, time = 500, timeUnit = TimeUnit.MICROSECONDS)
// TODO: @Fork(value = 1)
// TODO: @BenchmarkMode(Mode.AverageTime)
// TODO: @OutputTimeUnit(TimeUnit.MILLISECONDS)
// TODO: @State(Scope.Benchmark)
class SemaphoreBenchmark {
private:
    // TODO: @Param annotation
    SemaphoreBenchDispatcherCreator _1_dispatcher_ = SemaphoreBenchDispatcherCreator::kDefault;

    // TODO: @Param("0", "1000")
    int _2_coroutines_ = 0;

    // TODO: @Param("1", "2", "4", "8", "32", "128", "100000")
    int _3_max_permits_ = 0;

    // TODO: @Param("1", "2", "4", "8", "16") // local machine
    // TODO: @Param("1", "2", "4", "8", "16", "32", "64", "128") // Server
    int _4_parallelism_ = 0;

    CoroutineDispatcher* dispatcher_ = nullptr;
    int coroutines_ = 0;

public:
    // TODO: @InternalCoroutinesApi annotation
    // TODO: @Setup annotation
    void setup() {
        dispatcher_ = _1_dispatcher_.create(_4_parallelism_);
        coroutines_ = (_2_coroutines_ == 0) ? _4_parallelism_ : _2_coroutines_;
    }

    // TODO: @Benchmark annotation
    void semaphore() {
        run_blocking([this]() {
            int n = kBatchSize / coroutines_;
            auto semaphore = Semaphore(_3_max_permits_);
            std::vector<Job> jobs;
            jobs.reserve(coroutines_);
            repeat(coroutines_, [&]() {
                jobs.push_back(GlobalScope::launch([&]() {
                    repeat(n, [&]() {
                        semaphore.with_permit([&]() {
                            do_geom_distr_work(kWorkInside);
                        });
                        do_geom_distr_work(kWorkOutside);
                    });
                }));
            });
            for (auto& job : jobs) {
                job.join();
            }
        });
    }

    // TODO: @Benchmark annotation
    void channel_as_semaphore() {
        run_blocking([this]() {
            int n = kBatchSize / coroutines_;
            auto semaphore = Channel<void>(_3_max_permits_);
            std::vector<Job> jobs;
            jobs.reserve(coroutines_);
            repeat(coroutines_, [&]() {
                jobs.push_back(GlobalScope::launch([&]() {
                    repeat(n, [&]() {
                        semaphore.send(); // acquire
                        do_geom_distr_work(kWorkInside);
                        semaphore.receive(); // release
                        do_geom_distr_work(kWorkOutside);
                    });
                }));
            });
            for (auto& job : jobs) {
                job.join();
            }
        });
    }
};

enum class SemaphoreBenchDispatcherCreator {
    kForkJoin,
    kDefault
};

// TODO: Implement enum with create function
// using SemaphoreBenchDispatcherCreatorFn = std::function<CoroutineDispatcher*(int)>;

static constexpr int kWorkInside = 50;
static constexpr int kWorkOutside = 50;
static constexpr int kBatchSize = 100000;

} // namespace coroutines
} // namespace kotlinx
