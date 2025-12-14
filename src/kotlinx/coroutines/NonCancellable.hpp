/**
 * @file NonCancellable.hpp
 * @brief Non-cancellable job for use with withContext
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/NonCancellable.kt
 *
 * A non-cancelable job that is always [active][Job.isActive]. It is designed for withContext() function
 * to prevent cancellation of code blocks that need to be executed without cancellation.
 */

#pragma once

#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <functional>

namespace kotlinx {
namespace coroutines {

/**
 * A non-cancelable job that is always active. It is designed for withContext() function
 * to prevent cancellation of code blocks that need to be executed without cancellation.
 *
 * Use it like this:
 * ```cpp
 * with_context(non_cancellable(), [](CoroutineScope*) {
 *     // this code will not be cancelled
 * });
 * ```
 *
 * **WARNING**: This class is not designed to be used with launch(), async(), and other coroutine builders.
 * if you write `launch(scope, non_cancellable(), ...)` then not only the newly launched job will not be cancelled
 * when the parent is cancelled, the whole parent-child relation between parent and child is severed.
 * The parent will not wait for the child's completion, nor will be cancelled when the child crashed.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/NonCancellable.kt
 */
class NonCancellable : public AbstractCoroutineContextElement, public Job {
private:
    static constexpr const char* MESSAGE =
        "NonCancellable can be used only as an argument for 'withContext', direct usages of its API are prohibited";

    NonCancellable() : AbstractCoroutineContextElement(Job::type_key) {}

public:
    static std::shared_ptr<NonCancellable> instance() {
        static auto inst = std::shared_ptr<NonCancellable>(new NonCancellable());
        return inst;
    }

    // ==================== Job interface ====================

    /**
     * Always returns nullptr.
     * @suppress **This an API and should not be used from general code.**
     */
    std::shared_ptr<Job> get_parent() const override { return nullptr; }

    /**
     * Always returns true.
     * @suppress **This an API and should not be used from general code.**
     */
    bool is_active() const override { return true; }

    /**
     * Always returns false.
     * @suppress **This an API and should not be used from general code.**
     */
    bool is_completed() const override { return false; }

    /**
     * Always returns false.
     * @suppress **This an API and should not be used from general code.**
     */
    bool is_cancelled() const override { return false; }

    /**
     * Always throws std::logic_error.
     * @suppress **This an API and should not be used from general code.**
     */
    std::exception_ptr get_cancellation_exception() override {
        throw std::logic_error("This job is always active");
    }

    /**
     * Always returns false.
     * @suppress **This an API and should not be used from general code.**
     */
    bool start() override { return false; }

    /**
     * Does nothing.
     * @suppress **This an API and should not be used from general code.**
     */
    void cancel(std::exception_ptr cause = nullptr) override {
        // Does nothing - job is never cancelled
    }

    /**
     * Always returns empty vector.
     * @suppress **This an API and should not be used from general code.**
     */
    std::vector<std::shared_ptr<Job>> get_children() const override {
        return {};
    }

    /**
     * Always returns NonDisposableHandle and does not do anything.
     * @suppress **This an API and should not be used from general code.**
     */
    std::shared_ptr<ChildHandle> attach_child(std::shared_ptr<ChildJob> child) override {
        // Return a shared_ptr to the singleton NonDisposableHandle
        return std::shared_ptr<ChildHandle>(&NonDisposableHandle::instance(), [](ChildHandle*){});
    }

    /**
     * Always throws std::logic_error.
     * @suppress **This an API and should not be used from general code.**
     */
    void* join(Continuation<void*>* continuation) override {
        throw std::logic_error("This job is always active");
    }

    /**
     * Always throws std::logic_error.
     * @suppress **This an API and should not be used from general code.**
     */
    void join_blocking() override {
        throw std::logic_error("This job is always active");
    }

    /**
     * Always returns no-op handle.
     * @suppress **This an API and should not be used from general code.**
     */
    std::shared_ptr<DisposableHandle> invoke_on_completion(
        std::function<void(std::exception_ptr)> handler) override {
        return non_disposable_handle();
    }

    /**
     * Always returns no-op handle.
     * @suppress **This an API and should not be used from general code.**
     */
    std::shared_ptr<DisposableHandle> invoke_on_completion(
        bool on_cancelling,
        bool invoke_immediately,
        std::function<void(std::exception_ptr)> handler) override {
        return non_disposable_handle();
    }

    // ==================== CoroutineContext::Element interface ====================

    CoroutineContext::Key* key() const override { return Job::type_key; }

    std::string to_string() const {
        return "NonCancellable";
    }
};

/**
 * Convenience function to get NonCancellable as a context element.
 * Use: with_context(non_cancellable(), ...)
 */
inline std::shared_ptr<CoroutineContext> non_cancellable() {
    return NonCancellable::instance();
}

} // namespace coroutines
} // namespace kotlinx
