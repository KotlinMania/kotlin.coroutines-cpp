#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace kotlinx {
namespace coroutines {

class CloseableCoroutineDispatcher : public CoroutineDispatcher {
public:
    virtual void close() = 0;
};

class MultithreadedDispatcher : public CloseableCoroutineDispatcher {
public:
    virtual void close() override = 0;
};

CloseableCoroutineDispatcher* new_fixed_thread_pool_context(int n_threads, const std::string& name);

} // namespace coroutines
} // namespace kotlinx
