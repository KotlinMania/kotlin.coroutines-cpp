// port-lint: source Runnable.common.kt
/**
 * Transliterated from: kotlinx-coroutines-core/native/src/Runnable.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *
 * Upstream:
 *   public actual fun interface Runnable { fun run() }
 *   @Deprecated(...) public inline fun Runnable(crossinline block: () -> Unit): Runnable =
 *       object : Runnable { override fun run() { block() } }
 *
 * Kotlin's `fun interface` is SAM-convertible; in C++ that role is filled by a virtual
 * interface plus a templated factory `make_runnable` that captures any callable. The
 * upstream `Runnable(block)` constructor is marked HIDDEN for binary compatibility — we
 * keep it as the templated factory so it does not pollute the public API namespace but
 * remains callable from binary-compat-sensitive call sites.
 */

#include <utility>

namespace kotlinx::coroutines {

class Runnable {
public:
    virtual ~Runnable() = default;
    virtual void run() = 0;
};

template <typename F>
Runnable* make_runnable(F block) {
    class RunnableImpl : public Runnable {
    public:
        explicit RunnableImpl(F block) : block_(std::move(block)) {}
        void run() override { block_(); }

    private:
        F block_;
    };
    return new RunnableImpl(std::move(block));
}

} // namespace kotlinx::coroutines
