// port-lint: source internal/NamedDispatcher.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/NamedDispatcher.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream:
 *   internal class NamedDispatcher(
 *       private val dispatcher: CoroutineDispatcher,
 *       private val name: String
 *   ) : CoroutineDispatcher(), Delay by (dispatcher as? Delay ?: defaultDelay) {
 *       override fun dispatch(context, block) = dispatcher.dispatch(context, block)
 *       override fun toString(): String = name
 *   }
 *
 * A wrapping dispatcher carrying a user-supplied name. The Kotlin `Delay by` delegation
 * picks up the underlying dispatcher's Delay implementation when it implements it,
 * otherwise falls back to DefaultDelay. In C++ this is expressed as explicit
 * pass-through overrides.
 */

#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/Runnable.hpp"

#include <memory>
#include <string>
#include <utility>

namespace kotlinx::coroutines::internal {

class NamedDispatcher : public CoroutineDispatcher, public Delay {
public:
    NamedDispatcher(std::shared_ptr<CoroutineDispatcher> dispatcher, std::string name)
        : dispatcher_(std::move(dispatcher)), name_(std::move(name)) {}

    void dispatch(const CoroutineContext& context,
                  std::shared_ptr<Runnable> block) const override {
        dispatcher_->dispatch(context, std::move(block));
    }

    std::string to_string() const { return name_; }

    // Delay by dispatcher: forward to underlying when it implements Delay, otherwise
    // delegate to DefaultDelay via the project-wide get_default_delay() shim.
    void schedule_resume_after_delay(long long time_millis,
                                     CancellableContinuation<void>& continuation) override {
        auto* delay_impl = dynamic_cast<Delay*>(dispatcher_.get());
        if (delay_impl) {
            delay_impl->schedule_resume_after_delay(time_millis, continuation);
            return;
        }
        get_default_delay().schedule_resume_after_delay(time_millis, continuation);
    }

private:
    std::shared_ptr<CoroutineDispatcher> dispatcher_;
    std::string name_;
};

} // namespace kotlinx::coroutines::internal
