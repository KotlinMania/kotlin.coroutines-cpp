// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/benchmarks/jvm/kotlin/kotlinx/coroutines/channels/ChannelProducerConsumerBenchmark.kt
// TODO: Resolve imports and dependencies
// TODO: Implement JMH benchmark annotations
// TODO: Handle suspend functions and coroutines
// TODO: Implement Channel and select

namespace kotlinx {
namespace coroutines {
namespace channels {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlinx.coroutines.channels.Channel
// TODO: import kotlinx.coroutines.scheduling.*
// TODO: import kotlinx.coroutines.selects.select
// TODO: import org.openjdk.jmh.annotations.*
// TODO: import java.lang.Integer.max
// TODO: import java.util.concurrent.ForkJoinPool
// TODO: import java.util.concurrent.Phaser
// TODO: import java.util.concurrent.TimeUnit

// Benchmark to measure channel algorithm performance in terms of average time per `send-receive` pair;
// actually, it measures the time for a batch of such operations separated into the specified number of consumers/producers.
// It uses different channels (rendezvous, buffered, unlimited; see [ChannelCreator]) and different dispatchers
// (see [DispatcherCreator]). If the [_3_withSelect] property is set, it invokes `send` and
// `receive` via [select], waiting on a local dummy channel simultaneously, simulating a "cancellation" channel.
//
// Please, be patient, this benchmark takes quite a lot of time to complete.

// TODO: @Warmup(iterations = 3, time = 500, timeUnit = TimeUnit.MICROSECONDS)
// TODO: @Measurement(iterations = 20, time = 500, timeUnit = TimeUnit.MICROSECONDS)
// TODO: @Fork(value = 1)
// TODO: @BenchmarkMode(Mode.Throughput)
// TODO: @OutputTimeUnit(TimeUnit.MILLISECONDS)
// TODO: @State(Scope.Benchmark)
class ChannelProducerConsumerBenchmark {
private:
    // TODO: @Param annotation
    DispatcherCreator _0_dispatcher_ = DispatcherCreator::kDefault;

    // TODO: @Param annotation
    ChannelCreator _1_channel_ = ChannelCreator::kRendezvous;

    // TODO: @Param("0", "1000")
    int _2_coroutines_ = 0;

    // TODO: @Param("false", "true")
    bool _3_with_select_ = false;

    // TODO: @Param("1", "2", "4", "8", "16") // local machine
    // TODO: @Param("1", "2", "4", "8", "16", "32", "64", "128") // Server
    int _4_parallelism_ = 0;

    // TODO: @Param("50")
    int _5_work_size_ = 0;

    CoroutineDispatcher* dispatcher_ = nullptr;
    Channel<int>* channel_ = nullptr;

public:
    // TODO: @InternalCoroutinesApi annotation
    // TODO: @Setup annotation
    void setup() {
        dispatcher_ = _0_dispatcher_.create(_4_parallelism_);
        channel_ = _1_channel_.create();
    }

    // TODO: @Benchmark annotation
    void mcsp() {
        if (_2_coroutines_ != 0) return;
        int producers = std::max(1, _4_parallelism_ - 1);
        int consumers = 1;
        run(producers, consumers);
    }

    // TODO: @Benchmark annotation
    void spmc() {
        if (_2_coroutines_ != 0) return;
        int producers = 1;
        int consumers = std::max(1, _4_parallelism_ - 1);
        run(producers, consumers);
    }

    // TODO: @Benchmark annotation
    void mpmc() {
        int producers = (_2_coroutines_ == 0) ? (_4_parallelism_ + 1) / 2 : _2_coroutines_ / 2;
        int consumers = producers;
        run(producers, consumers);
    }

private:
    void run(int producers, int consumers) {
        int n = (kApproxBatchSize / producers * producers) / consumers * consumers;
        Phaser phaser(producers + consumers + 1);
        // Run producers
        repeat(producers, [&]() {
            GlobalScope::launch(*dispatcher_, [&]() {
                auto dummy = _3_with_select_ ? _1_channel_.create() : nullptr;
                repeat(n / producers, [&](int it) {
                    produce(it, dummy);
                });
                phaser.arrive();
            });
        });
        // Run consumers
        repeat(consumers, [&]() {
            GlobalScope::launch(*dispatcher_, [&]() {
                auto dummy = _3_with_select_ ? _1_channel_.create() : nullptr;
                repeat(n / consumers, [&]() {
                    consume(dummy);
                });
                phaser.arrive();
            });
        });
        // Wait until work is done
        phaser.arrive_and_await_advance();
    }

    // TODO: suspend function
    void produce(int element, Channel<int>* dummy) {
        if (_3_with_select_) {
            select<void>([&](auto selector) {
                channel_->on_send(element, []() {});
                dummy->on_receive([]() {});
            });
        } else {
            channel_->send(element);
        }
        do_work(_5_work_size_);
    }

    // TODO: suspend function
    void consume(Channel<int>* dummy) {
        if (_3_with_select_) {
            select<void>([&](auto selector) {
                channel_->on_receive([]() {});
                dummy->on_receive([]() {});
            });
        } else {
            channel_->receive();
        }
        do_work(_5_work_size_);
    }
};

enum class DispatcherCreator {
    kForkJoin,
    kDefault
};

enum class ChannelCreator {
    kRendezvous,      // Channel::RENDEZVOUS
    kBuffered16,      // 16
    kBuffered64,      // 64
    kBufferedUnlimited // Channel::UNLIMITED
};
// TODO  figure out why     fun create(): Channel<Int> = Channel(capacity) is not next
void do_work(int work_size) {
    do_geom_distr_work(work_size);
}

static constexpr int kApproxBatchSize = 100000;

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
