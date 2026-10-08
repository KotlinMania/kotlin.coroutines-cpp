// port-lint: source selects/OnTimeout.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt
 */
#include "kotlinx/coroutines/selects/OnTimeout.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/Waiter.hpp"

namespace kotlinx::coroutines::selects {

// Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt:34-61
class OnTimeout : public std::enable_shared_from_this<OnTimeout> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt:34-36
    explicit OnTimeout(std::int64_t time_millis) : time_millis_(time_millis) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt:38-42
    std::unique_ptr<SelectClause0> select_clause() {
        auto self = shared_from_this();
        return std::make_unique<SelectClause0Impl>(this,
            [self](void*, void* select, void* param) {
                self->register_(*static_cast<SelectInstanceBase*>(select), param);
            });
    }

private:
    // Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt:45-60
    // NOTE(port): register is a C++ keyword; the suffix preserves the source name.
    void register_(SelectInstanceBase& select, void*) {
        if (time_millis_ <= 0) {
            select.select_in_registration_phase(nullptr);
            return;
        }
        auto self = shared_from_this();
        // NOTE(port): Retain the existing SelectImplementation owner just as the
        // source Runnable captures select. A borrowed waiter stays borrowed.
        std::shared_ptr<Waiter> owner;
        if (auto* waiter = dynamic_cast<Waiter*>(&select)) {
            owner = waiter->shared_from_this_waiter();
        }
        auto action = std::shared_ptr<Runnable>(make_runnable(
            [select = &select, self, owner = std::move(owner)] {
                select->try_select(self.get(), nullptr);
            }));
        auto context = select.get_context();
        auto handle = get_delay(*context).invoke_on_timeout(time_millis_, action, *context);
        select.dispose_on_completion(std::move(handle));
    }

    std::int64_t time_millis_;
};

namespace detail {
// Transliterated from: kotlinx-coroutines-core/common/src/selects/OnTimeout.kt:16-17,34-42
std::unique_ptr<SelectClause0> make_on_timeout_clause(std::int64_t time_millis) {
    return std::make_shared<OnTimeout>(time_millis)->select_clause();
}
}
} // namespace kotlinx::coroutines::selects
