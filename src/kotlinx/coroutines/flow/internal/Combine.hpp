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
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/internal/SendingCollector.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <any>
#include <atomic>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::flow::internal {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:273
template <typename T>
inline auto null_array_factory() {
    return []() -> std::vector<T>* { return nullptr; };
}

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

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:11-80
template <typename R>
inline void* combine_internal(
    FlowCollector<R>* collector,
    const std::vector<std::shared_ptr<Flow<std::any>>>& flows,
    std::function<void*(FlowCollector<R>*, const std::vector<std::any>&, Continuation<void*>*)> transform,
    Continuation<void*>* completion = nullptr) {
    const size_t size = flows.size();
    if (size == 0) return nullptr;

    struct Update {
        size_t index;
        std::any value;
    };

    auto result_channel = channels::create_channel<Update>(static_cast<int>(size));
    auto non_closed = std::make_shared<std::atomic<size_t>>(size);
    auto cancelled = std::make_shared<std::atomic<bool>>(false);

    std::mutex exception_mutex;
    std::exception_ptr failure = nullptr;

    auto record_exception = [&](std::exception_ptr e) {
        if (!e) return;
        std::lock_guard<std::mutex> lock(exception_mutex);
        if (!failure) {
            failure = e;
            cancelled->store(true, std::memory_order_relaxed);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(size);

    for (size_t i = 0; i < size; ++i) {
        threads.emplace_back([i, &flows, result_channel, non_closed, cancelled, &record_exception]() {
            try {
                class ThreadCollector : public FlowCollector<std::any> {
                    size_t idx_;
                    std::shared_ptr<channels::Channel<Update>> ch_;
                    std::shared_ptr<std::atomic<bool>> canc_;
                public:
                    ThreadCollector(
                        size_t idx,
                        std::shared_ptr<channels::Channel<Update>> ch,
                        std::shared_ptr<std::atomic<bool>> canc)
                        : idx_(idx), ch_(std::move(ch)), canc_(std::move(canc)) {}

                    void* emit(std::any val, Continuation<void*>*) override {
                        while (!canc_->load(std::memory_order_relaxed)) {
                            if (ch_->is_closed_for_send()) {
                                throw CancellationException("Flow cancelled due to closed channel");
                            }
                            auto send_res = ch_->try_send(Update{idx_, val});
                            if (send_res.is_success()) break;
                            std::this_thread::yield();
                        }
                        return nullptr;
                    }
                };
                ThreadCollector tc(i, result_channel, cancelled);
                flows[i]->collect(&tc, nullptr);
            } catch (...) {
                record_exception(std::current_exception());
                result_channel->cancel(std::current_exception());
            }
            if (non_closed->fetch_sub(1) == 1) {
                result_channel->close();
            }
        });
    }

    std::vector<std::any> latest_values(size);
    std::vector<bool> has_value(size, false);
    size_t remaining_absent_values = size;

    std::vector<uint8_t> last_received_epoch(size, 0);
    uint8_t current_epoch = 0;

    void* suspended_result = nullptr;

    try {
        while (true) {
            ++current_epoch;
            channels::ChannelResult<Update> res = channels::ChannelResult<Update>::failure();
            while (true) {
                if (cancelled->load(std::memory_order_relaxed)) break;
                res = result_channel->try_receive();
                if (res.is_success() || res.is_closed()) break;
                std::this_thread::yield();
            }
            if (res.is_closed()) {
                auto ex = res.exception_or_null();
                if (ex) std::rethrow_exception(ex);
                break;
            }
            if (!res.is_success()) break;

            Update element = res.get_or_throw();
            while (true) {
                size_t index = element.index;
                if (!has_value[index]) {
                    has_value[index] = true;
                    --remaining_absent_values;
                }
                latest_values[index] = std::move(element.value);

                if (last_received_epoch[index] == current_epoch) break;
                last_received_epoch[index] = current_epoch;

                auto try_res = result_channel->try_receive();
                if (!try_res.is_success()) break;
                element = try_res.get_or_throw();
            }

            if (remaining_absent_values == 0) {
                void* r = transform(collector, latest_values, completion);
                if (intrinsics::is_coroutine_suspended(r)) {
                    suspended_result = r;
                    return r;
                }
            }
        }
    } catch (...) {
        record_exception(std::current_exception());
        result_channel->cancel(std::current_exception());
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    {
        std::lock_guard<std::mutex> lock(exception_mutex);
        if (failure) {
            std::rethrow_exception(failure);
        }
    }

    return suspended_result;
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:265-270
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
            cont);
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:254-260
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

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:82-139
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
            auto second = channels::create_channel<T2>(1);
            auto collect_job = JobImpl::create(nullptr);

            std::exception_ptr second_exception = nullptr;
            std::mutex exception_mutex;

            std::thread t2([this, second, &second_exception, &exception_mutex]() {
                try {
                    class ZipSecondCollector : public FlowCollector<T2> {
                        std::shared_ptr<channels::Channel<T2>> ch_;
                    public:
                        explicit ZipSecondCollector(std::shared_ptr<channels::Channel<T2>> ch)
                            : ch_(std::move(ch)) {}
                        void* emit(T2 value, Continuation<void*>*) override {
                            while (!ch_->is_closed_for_send()) {
                                auto send_res = ch_->try_send(value);
                                if (send_res.is_success()) break;
                                std::this_thread::yield();
                            }
                            return nullptr;
                        }
                    };
                    ZipSecondCollector sc(second);
                    flow2_->collect(&sc, nullptr);
                    second->close();
                } catch (...) {
                    {
                        std::lock_guard<std::mutex> lock(exception_mutex);
                        second_exception = std::current_exception();
                    }
                    second->close(std::current_exception());
                }
            });

            second->invoke_on_close([collect_job](std::exception_ptr) {
                if (collect_job->is_active()) {
                    collect_job->cancel(std::make_exception_ptr(AbortFlowException(collect_job.get())));
                }
            });

            class ZipFirstCollector : public FlowCollector<T1> {
                FlowCollector<R>* collector_;
                std::shared_ptr<channels::Channel<T2>> second_;
                std::function<R(T1, T2)> transform_;
                std::shared_ptr<JobImpl> collect_job_;
                Continuation<void*>* cont_;
            public:
                ZipFirstCollector(
                    FlowCollector<R>* collector,
                    std::shared_ptr<channels::Channel<T2>> second,
                    std::function<R(T1, T2)> transform,
                    std::shared_ptr<JobImpl> collect_job,
                    Continuation<void*>* cont)
                    : collector_(collector),
                      second_(std::move(second)),
                      transform_(std::move(transform)),
                      collect_job_(std::move(collect_job)),
                      cont_(cont) {}

                void* emit(T1 value, Continuation<void*>* c) override {
                    if (!collect_job_->is_active()) {
                        throw AbortFlowException(collect_job_.get());
                    }
                    channels::ChannelResult<T2> other_val_res = channels::ChannelResult<T2>::failure();
                    while (collect_job_->is_active()) {
                        other_val_res = second_->try_receive();
                        if (other_val_res.is_success() || other_val_res.is_closed()) break;
                        std::this_thread::yield();
                    }
                    if (!other_val_res.is_success()) {
                        if (other_val_res.is_closed()) {
                            auto ex = other_val_res.exception_or_null();
                            if (ex) std::rethrow_exception(ex);
                        }
                        throw AbortFlowException(collect_job_.get());
                    }
                    T2 other_value = other_val_res.get_or_throw();
                    R transformed = transform_(std::move(value), std::move(other_value));
                    return collector_->emit(std::move(transformed), c ? c : cont_);
                }
            };

            ZipFirstCollector first_collector(collector, second, transform_, collect_job, cont);

            void* res = nullptr;
            try {
                res = flow1_->collect(&first_collector, cont);
            } catch (const AbortFlowException& e) {
                const_cast<AbortFlowException&>(e).check_ownership(collect_job.get());
            } catch (...) {
                second->cancel(std::current_exception());
                if (t2.joinable()) t2.join();
                throw;
            }

            second->cancel(nullptr);
            if (t2.joinable()) t2.join();

            {
                std::lock_guard<std::mutex> lock(exception_mutex);
                if (second_exception) {
                    std::rethrow_exception(second_exception);
                }
            }

            return res;
        }
    };

    return std::make_shared<ZipFlowImpl>(std::move(flow1), std::move(flow2), std::move(transform));
}

} // namespace kotlinx::coroutines::flow::internal
