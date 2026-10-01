#pragma once

#include <utility>
#include <functional>

namespace kotlinx {
namespace coroutines {

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Runnable.common.kt
 *
 * Upstream:
 *   public expect fun interface Runnable { fun run() }
 */
struct Runnable {
    virtual ~Runnable() = default;
    virtual void run() = 0;
};

template <typename F>
class LambdaRunnable : public Runnable {
public:
    explicit LambdaRunnable(F block) : block_(std::move(block)) {}
    void run() override { block_(); }

private:
    F block_;
};

template <typename F>
Runnable* make_runnable(F block) {
    return new LambdaRunnable<F>(std::move(block));
}

} // namespace coroutines
} // namespace kotlinx
