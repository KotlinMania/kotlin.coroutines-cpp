#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/NamedDispatchers.kt
 */

#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <memory>

namespace kotlinx::coroutines {

/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/NamedDispatchers.kt:31-62
 */
class ArrayStack {
private:
    std::vector<std::string> elements_;

public:
    void push(const std::string& value) {
        elements_.push_back(value);
    }

    std::optional<std::string> peek() const {
        if (elements_.empty()) return std::nullopt;
        return elements_.back();
    }

    std::optional<std::string> pop() {
        if (elements_.empty()) return std::nullopt;
        auto val = elements_.back();
        elements_.pop_back();
        return val;
    }
};

/**
 * Test dispatchers that emulate multiplatform context tracking.
 *
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/NamedDispatchers.kt:8-29
 */
class NamedDispatchers {
private:
    static ArrayStack& stack_instance() {
        thread_local ArrayStack stack;
        return stack;
    }

    class NamedDispatcher : public CoroutineDispatcher {
    private:
        std::string name_;

    public:
        explicit NamedDispatcher(std::string name) : name_(std::move(name)) {}

        bool is_dispatch_needed(const CoroutineContext&) const override {
            return true;
        }

        void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> block) const override {
            stack_instance().push(name_);
            try {
                if (block) {
                    block->run();
                }
            } catch (...) {
                auto last = stack_instance().pop();
                if (!last.has_value() || *last != name_) {
                    // stack mismatch on throw
                }
                throw;
            }
            auto last = stack_instance().pop();
            if (!last.has_value() || *last != name_) {
                throw std::runtime_error("Inconsistent stack: expected " + name_ + ", but had " + last.value_or("none"));
            }
        }
    };

    std::shared_ptr<CoroutineDispatcher> dispatcher_;

public:
    explicit NamedDispatchers(const std::string& name)
        : dispatcher_(std::make_shared<NamedDispatcher>(name)) {}

    operator std::shared_ptr<CoroutineDispatcher>() const { return dispatcher_; }
    operator std::shared_ptr<CoroutineContext>() const { return dispatcher_; }
    CoroutineDispatcher* get() const { return dispatcher_.get(); }

    static std::string name() {
        auto result = stack_instance().peek();
        if (!result.has_value()) {
            throw std::runtime_error("No names on stack");
        }
        return *result;
    }

    static std::string name_or(const std::string& default_value) {
        auto result = stack_instance().peek();
        return result.value_or(default_value);
    }
};

inline std::shared_ptr<CoroutineScope> operator+(
    const CoroutineScope& scope,
    const NamedDispatchers& nd
) {
    return operator+(scope, static_cast<std::shared_ptr<CoroutineContext>>(nd));
}

inline std::shared_ptr<CoroutineScope> operator+(
    CoroutineScope* scope,
    const NamedDispatchers& nd
) {
    return operator+(scope, static_cast<std::shared_ptr<CoroutineContext>>(nd));
}

} // namespace kotlinx::coroutines
