#pragma once
// port-lint: source flow/internal/Combine.kt
/**
 * @file Combine.hpp
 * @brief Internal primitives for combine and zip operators.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/SendingCollector.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include <any>
#include <atomic>
#include <exception>
#include <functional>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::flow::internal {

template <typename T>
inline std::shared_ptr<Flow<std::any>> as_any_flow(std::shared_ptr<Flow<T>> flow) {
    class AnyFlow : public Flow<std::any> {
        std::shared_ptr<Flow<T>> upstream_;
    public:
        explicit AnyFlow(std::shared_ptr<Flow<T>> u) : upstream_(std::move(u)) {}
        void* collect(FlowCollector<std::any>* collector, Continuation<void*>* cont) override {
            class AnyCollector : public FlowCollector<T> {
                FlowCollector<std::any>* down_;
            public:
                explicit AnyCollector(FlowCollector<std::any>* d) : down_(d) {}
                void* emit(T value, Continuation<void*>* c) override {
                    return down_->emit(std::any(std::move(value)), c);
                }
            };
            AnyCollector ac(collector);
            return upstream_->collect(&ac, cont);
        }
    };
    return std::make_shared<AnyFlow>(std::move(flow));
}

template <typename R>
inline void* combine_internal(
    FlowCollector<R>* collector,
    const std::vector<std::shared_ptr<Flow<std::any>>>& flows,
    std::function<void*(FlowCollector<R>*, const std::vector<std::any>&, Continuation<void*>*)> transform,
    std::shared_ptr<Continuation<void*>> completion = nullptr) {
    const size_t size = flows.size();
    if (size == 0) return nullptr;

    struct Update {
        size_t index;
        std::any value;
    };

    auto result_channel = channels::create_channel<Update>(channels::Channel<Update>::BUFFERED);
    auto non_closed = std::make_shared<std::atomic<size_t>>(size);
    std::vector<std::thread> threads;
    threads.reserve(size);

    for (size_t i = 0; i < size; ++i) {
        threads.emplace_back([i, &flows, result_channel, non_closed]() {
            try {
                class ThreadCollector : public FlowCollector<std::any> {
                    size_t idx_;
                    std::shared_ptr<channels::Channel<Update>> ch_;
                public:
                    ThreadCollector(size_t idx, std::shared_ptr<channels::Channel<Update>> ch)
                        : idx_(idx), ch_(ch) {}
                    void* emit(std::any val, Continuation<void*>*) override {
                        ch_->send(Update{idx_, std::move(val)});
                        return nullptr;
                    }
                };
                ThreadCollector tc(i, result_channel);
                flows[i]->collect(&tc, nullptr);
            } catch (...) {}
            if (non_closed->fetch_sub(1) == 1) {
                result_channel->close();
            }
        });
    }

    std::vector<std::any> latest_values(size);
    std::vector<bool> has_value(size, false);
    size_t remaining = size;

    try {
        while (true) {
            auto res = result_channel->receive_catching();
            if (res.is_closed()) break;
            if (res.is_success()) {
                auto update = res.get_or_throw();
                if (!has_value[update.index]) {
                    has_value[update.index] = true;
                    --remaining;
                }
                latest_values[update.index] = std::move(update.value);
                if (remaining == 0) {
                    transform(collector, latest_values, completion ? completion.get() : nullptr);
                }
            }
        }
    } catch (...) {
        result_channel->cancel();
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    return nullptr;
}

template <typename R>
inline std::shared_ptr<Flow<R>> combine_transform_unsafe(
    std::vector<std::shared_ptr<Flow<std::any>>> flows,
    std::function<void*(FlowCollector<R>*, const std::vector<std::any>&, Continuation<void*>*)> transform) {
    return flow<R>([flows = std::move(flows), transform = std::move(transform)](
        FlowCollector<R>* collector, Continuation<void*>* cont) -> void* {
        return combine_internal<R>(
            collector, flows,
            [transform](FlowCollector<R>* sink, const std::vector<std::any>& vals, Continuation<void*>* c) -> void* {
                return transform(sink, vals, c);
            },
            nullptr);
    });
}

template <typename R>
inline std::shared_ptr<Flow<R>> combine_unsafe(
    std::vector<std::shared_ptr<Flow<std::any>>> flows,
    std::function<R(const std::vector<std::any>&)> transform) {
    return combine_transform_unsafe<R>(
        std::move(flows),
        [transform = std::move(transform)](
            FlowCollector<R>* sink, const std::vector<std::any>& vals, Continuation<void*>* c) -> void* {
            return sink->emit(transform(vals), c);
        });
}

template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> zip_impl(
    std::shared_ptr<Flow<T1>> flow1,
    std::shared_ptr<Flow<T2>> flow2,
    std::function<R(T1, T2)> transform) {
    class ZipFlowImpl : public Flow<R> {
        std::shared_ptr<Flow<T1>> flow1_;
        std::shared_ptr<Flow<T2>> flow2_;
        std::function<R(T1, T2)> transform_;
    public:
        ZipFlowImpl(std::shared_ptr<Flow<T1>> f1, std::shared_ptr<Flow<T2>> f2, std::function<R(T1, T2)> t)
            : flow1_(std::move(f1)), flow2_(std::move(f2)), transform_(std::move(t)) {}

        void* collect(FlowCollector<R>* collector, Continuation<void*>* cont) override {
            auto c1 = channels::create_channel<T1>(channels::Channel<T1>::BUFFERED);
            auto c2 = channels::create_channel<T2>(channels::Channel<T2>::BUFFERED);

            std::thread t1([this, c1]() {
                try {
                    class ZipSendingCollector : public FlowCollector<T1> {
                        std::shared_ptr<channels::Channel<T1>> ch_;
                    public:
                        explicit ZipSendingCollector(std::shared_ptr<channels::Channel<T1>> ch) : ch_(ch) {}
                        void* emit(T1 value, Continuation<void*>*) override {
                            ch_->send(std::move(value));
                            return nullptr;
                        }
                    };
                    ZipSendingCollector sc(c1);
                    flow1_->collect(&sc, nullptr);
                    c1->close();
                } catch (...) {
                    c1->close(std::current_exception());
                }
            });

            std::thread t2([this, c2]() {
                try {
                    class ZipSendingCollector2 : public FlowCollector<T2> {
                        std::shared_ptr<channels::Channel<T2>> ch_;
                    public:
                        explicit ZipSendingCollector2(std::shared_ptr<channels::Channel<T2>> ch) : ch_(ch) {}
                        void* emit(T2 value, Continuation<void*>*) override {
                            ch_->send(std::move(value));
                            return nullptr;
                        }
                    };
                    ZipSendingCollector2 sc(c2);
                    flow2_->collect(&sc, nullptr);
                    c2->close();
                } catch (...) {
                    c2->close(std::current_exception());
                }
            });

            try {
                while (true) {
                    auto r1 = c1->receive_catching();
                    if (r1.is_closed()) {
                        c2->cancel();
                        break;
                    }
                    auto r2 = c2->receive_catching();
                    if (r2.is_closed()) {
                        c1->cancel();
                        break;
                    }
                    if (r1.is_success() && r2.is_success()) {
                        R res = transform_(r1.get_or_throw(), r2.get_or_throw());
                        collector->emit(std::move(res), cont);
                    } else {
                        break;
                    }
                }
            } catch (...) {
                c1->cancel();
                c2->cancel();
            }

            if (t1.joinable()) t1.join();
            if (t2.joinable()) t2.join();
            return nullptr;
        }
    };

    return std::make_shared<ZipFlowImpl>(std::move(flow1), std::move(flow2), std::move(transform));
}

} // namespace kotlinx::coroutines::flow::internal
