#pragma once
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"

namespace kotlinx {
namespace coroutines {
namespace channels {

template <typename E>
struct ProducerScope : public virtual CoroutineScope, public virtual SendChannel<E> {
    virtual ~ProducerScope() = default;

    virtual SendChannel<E>* get_channel() = 0;
};

} // namespace channels
} // namespace coroutines
} // namespace kotlinx
