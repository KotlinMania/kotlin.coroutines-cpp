#pragma once
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/internal/SendingCollector.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include <functional>
#include <thread>
#include <atomic>
#include <exception>

namespace kotlinx::coroutines::flow::internal {

    template <typename T1, typename T2, typename R>
    class ZipFlow : public Flow<R> {
        Flow<T1>* flow1_;
        Flow<T2>* flow2_;
        std::function<R(T1, T2)> transform_;

    public:
        ZipFlow(Flow<T1>* f1, Flow<T2>* f2, std::function<R(T1, T2)> t)
            : flow1_(f1), flow2_(f2), transform_(t) {}

        void collect(FlowCollector<R>* collector) override {
            // Create channels to decouple producers from consumer
            auto c1 = channels::createChannel<T1>(channels::Channel<T1>::BUFFERED);
            auto c2 = channels::createChannel<T2>(channels::Channel<T2>::BUFFERED);

            // Threads for concurrent collection
            // TODO: Use CoroutineScope.launch when available
            std::thread t1([&](){
                try {
                    SendingCollector<T1> sc(c1.get());
                    flow1_->collect(&sc);
                    c1->close();
                } catch (...) {
                    c1->close(std::current_exception());
                }
            });

            std::thread t2([&](){
                try {
                    SendingCollector<T2> sc(c2.get());
                    flow2_->collect(&sc);
                    c2->close();
                } catch (...) {
                    c2->close(std::current_exception());
                }
            });

            // Zip loop
            try {
                while (true) {
                    // Wait for element from first
                    auto r1 = c1->receive_catching();
                    if (r1.is_closed()) {
                        // If flow1 finishes, zip finishes
                        // Need to cancel flow2?
                        c2->cancel();
                        break;
                    }

                    // Wait for element from second
                    auto r2 = c2->receive_catching();
                    if (r2.is_closed()) {
                        c1->cancel();
                        break;
                    }

                    if (r1.is_success() && r2.is_success()) {
                        R result = transform_(r1.get_or_throw(), r2.get_or_throw());
                        collector->emit(result);
                    } else {
                        break;
                    }
                }
            } catch (...) {
                c1->cancel();
                c2->cancel();
                // rethrow or handle?
            }

            if (t1.joinable()) t1.join();
            if (t2.joinable()) t2.join();
        }
    };

    template <typename T1, typename T2, typename R>
    Flow<R>* zip_impl(Flow<T1>* flow1, Flow<T2>* flow2, std::function<R(T1, T2)> transform) {
        return new ZipFlow<T1, T2, R>(flow1, flow2, transform);
    }

}
