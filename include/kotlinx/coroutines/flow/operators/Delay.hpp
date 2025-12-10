#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include <chrono>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * Returns a flow that mirrors the original flow, but filters out values
 * that are followed by the newer values within the given timeout.
 */
template<typename T>
std::shared_ptr<Flow<T>> debounce(std::shared_ptr<Flow<T>> flow, long timeout_millis) {
    if (timeout_millis < 0) throw std::invalid_argument("Debounce timeout should not be negative");
    if (timeout_millis == 0) return flow;
    
    return flow<T>([flow, timeout_millis](FlowCollector<T>* collector) {
        flow_scope<void>([&](CoroutineScope& scope){
            auto channel = std::make_shared<BufferedChannel<T>>(0); // Rendezvous
            
            // Upstream collector
            scope.launch([&](){
                 try {
                     flow->collect([&](T val) {
                          channel->send(val);
                     });
                     channel->close();
                 } catch (...) {
                     channel->close(std::current_exception());
                 }
            });
            
            T lastValue;
            bool hasValue = false;
            
            while(!channel->is_closed_for_receive() || hasValue) {
                ChannelResult<T> res;
                if (hasValue) {
                     res = channel->receive_with_timeout(timeout_millis);
                } else {
                     res = channel->receive_catching();
                }
                
                if (res.is_success()) {
                    lastValue = res.get_or_throw();
                    hasValue = true;
                } else {
                    if (res.is_closed()) {
                        if (hasValue) {
                            collector->emit(lastValue);
                            hasValue = false;
                        }
                        break; 
                    }
                    // Timeout
                    if (hasValue) {
                        collector->emit(lastValue);
                        hasValue = false;
                    }
                }
            }
        });
    });
}

template<typename T, typename Fn>
std::shared_ptr<Flow<T>> debounce(std::shared_ptr<Flow<T>> flow, Fn timeout_millis_selector) {
    // Dynamic debounce stub - requires refactor for per-item timeout
    return debounce(flow, 100); // Fallback stub
}

template<typename T>
std::shared_ptr<Flow<T>> sample(std::shared_ptr<Flow<T>> flow, long period_millis) {
    if (period_millis <= 0) throw std::invalid_argument("Sample period should be positive");
    
    return flow<T>([flow, period_millis](FlowCollector<T>* collector) {
        flow_scope<void>([&](CoroutineScope& scope){
            auto channel = std::make_shared<BufferedChannel<T>>(-1); // Conflated
            
            scope.launch([&](){
                 try {
                     flow->collect([&](T val) {
                          channel->send(val);
                     });
                     channel->close();
                 } catch(...) {
                     channel->close(std::current_exception());
                 }
            });
            
            while(true) {
                 auto res = channel->receive_with_timeout(period_millis);
                 if (res.is_closed()) break;
                 if (res.is_success()) {
                     collector->emit(res.get_or_throw());
                 }
            }
        });
    });
}

template<typename T>
std::shared_ptr<Flow<T>> timeout(std::shared_ptr<Flow<T>> flow, long timeout_millis) {
     return flow<T>([flow, timeout_millis](FlowCollector<T>* collector) {
        flow_scope<void>([&](CoroutineScope& scope){
            auto channel = std::make_shared<BufferedChannel<T>>(0);
            
            scope.launch([&](){
                 try {
                     flow->collect([&](T val) {
                          channel->send(val);
                     });
                     channel->close();
                 } catch(...) {
                     channel->close(std::current_exception());
                 }
            });
            
            while(true) {
                auto res = channel->receive_with_timeout(timeout_millis);
                if (res.is_success()) {
                    collector->emit(res.get_or_throw());
                } else if (res.is_closed()) {
                    break;
                } else {
                    throw std::runtime_error("Flow timeout"); // TimeoutCancellationException
                }
            }
        });
     });
}



} // namespace flow
} // namespace coroutines
} // namespace kotlinx
