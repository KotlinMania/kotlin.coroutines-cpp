#include "kotlinx/coroutines/core_fwd.hpp"
namespace kotlinx {namespace coroutines {namespace flow {namespace {
// import kotlinx.coroutines.*// import kotlinx.coroutines.channels.*// import kotlinx.coroutines.flow.*
/**
 * Collection that sends to channel
 * @suppress **This an API and should not be used from general code.**
 */
// @InternalCoroutinesApiclass SendingCollector<T>(
    SendChannel<T> channel
) : FlowCollector<T> {
    virtual auto  emit(T value): Unit { return channel.send(value); }
}

}}}} // namespace kotlinx::coroutines::flow::internal