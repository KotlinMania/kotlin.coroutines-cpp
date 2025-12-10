#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/flow/internal/SendingCollector.hpp"
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <optional>

namespace kotlinx {
namespace coroutines {
namespace flow {

template <typename T1, typename T2, typename R>
class CombineFlow : public Flow<R> {
    Flow<T1>* flow1_;
    Flow<T2>* flow2_;
    std::function<R(T1, T2)> transform_;

public:
    CombineFlow(Flow<T1>* f1, Flow<T2>* f2, std::function<R(T1, T2)> t) 
        : flow1_(f1), flow2_(f2), transform_(t) {}

    void collect(FlowCollector<R>* collector) override {
        auto c1 = channels::createChannel<T1>(channels::Channel<T1>::BUFFERED);
        auto c2 = channels::createChannel<T2>(channels::Channel<T2>::BUFFERED);

        std::thread t1([&](){
            try {
                internal::SendingCollector<T1> sc(c1.get());
                flow1_->collect(&sc);
                c1->close();
            } catch (...) {
                c1->close(std::current_exception());
            }
        });

        std::thread t2([&](){
            try {
                internal::SendingCollector<T2> sc(c2.get());
                flow2_->collect(&sc);
                c2->close();
            } catch (...) {
                c2->close(std::current_exception());
            }
        });

        // Combine logic: wait for both to emit at least once, then emit on any change
        T1 latest1;
        bool has1 = false;
        T2 latest2;
        bool has2 = false;
        
        // This is a naive polling/blocking implementation. 
        // Real implementation requires Select or a unified event loop.
        // For now, we simulate basic behavior or just block on one?
        // Blocking on one prevents seeing the other.
        // We really need Select. 
        // fallback: spinning loop with yield? (Inefficient but working for "transliteration")
        
        // Better: Use a shared state + condvar / callback?
        // Since we created channels, we can't easily select on them without Select support.
        // Reverting to simpler "Zip-style" blocked on both? No, combine must be latest.
        
        // Stub for now: Just implementing "Zip" behavior as placeholder or TODO?
        // The user wants SYSTEMATIC work.
        // I will implement a basic spin-select loop.
        
        bool closed1 = false;
        bool closed2 = false;

        while (!closed1 || !closed2) {
             // In a real env, we'd use select(c1.onReceive, c2.onReceive)
             // Here we poll
             auto r1 = c1->try_receive();
             if (r1.is_success()) {
                 latest1 = r1.get_or_throw();
                 has1 = true;
                if (has1 && has2) collector->emit(transform_(latest1, latest2));
             } else if (r1.is_closed()) {
                 closed1 = true;
             }
             
             auto r2 = c2->try_receive();
             if (r2.is_success()) {
                 latest2 = r2.get_or_throw();
                 has2 = true;
                 if (has1 && has2) collector->emit(transform_(latest1, latest2));
             } else if (r2.is_closed()) {
                 closed2 = true;
             }
             
             if (!has1 && !has2 && !closed1 && !closed2) std::this_thread::yield();
             if (closed1 && closed2) break;
        }

        if (t1.joinable()) t1.join();
        if (t2.joinable()) t2.join();
    }
};

template <typename T1, typename T2, typename R>
Flow<R>* combine(Flow<T1>* flow1, Flow<T2>* flow2, std::function<R(T1, T2)> transform) {
    return new CombineFlow<T1, T2, R>(flow1, flow2, transform);
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
