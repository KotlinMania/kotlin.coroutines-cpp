#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++ (first-pass, mechanical syntax mapping)
// Original: kotlinx-coroutines-core/common/src/NonCancellable.kt
//
// TODO:
// - class declaration needs singleton pattern
// - Deprecated annotation handling
// - Multiple struct inheritance (AbstractCoroutineContextElement, Job)
// - Sequence<Job> needs iteration infrastructure
// - SelectClause0 needs select infrastructure

#include <stdexcept>
#include <string>

namespace kotlinx {
namespace coroutines {

class Job;
class CoroutineContext;
class CancellationException;
class DisposableHandle;
class CompletionHandler;
class SelectClause0;
class ChildJob;
class ChildHandle;
template<typename T> class Sequence;

/**
 * A non-cancelable job that is always [active][Job.isActive]. It is designed for [withContext] function
 * to prevent cancellation of code blocks that need to be executed without cancellation.
 *
 * Use it like this:
 * ```
 * withContext(NonCancellable) {
 *     // this code will not be cancelled
 * }
 * ```
 *
 * **WARNING**: This class is not designed to be used with [launch], [async], and other coroutine builders.
 * if you write `launch(NonCancellable) { ... }` then not only the newly launched job will not be cancelled
 * when the parent is cancelled, the whole parent-child relation between parent and child is severed.
 * The parent will not wait for the child's completion, nor will be cancelled when the child crashed.
 */
// @OptIn(InternalForInheritanceCoroutinesApi::class)
// @Suppress("DeprecatedCallableAddReplaceWith")
// TODO: class declaration - needs singleton implementation
class NonCancellable /* : AbstractCoroutineContextElement(Job), Job */ {
private:
    static constexpr const char* kMessage =
        "NonCancellable can be used only as an argument for 'withContext', direct usages of its API are prohibited";

    NonCancellable() = default; // Private constructor for singleton

public:
    static NonCancellable& instance();

    /**
     * Always returns `nullptr`.
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    Job* get_parent() const { return nullptr; }

    /**
     * Always returns `true`.
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    bool is_active() const { return true; }

    /**
     * Always returns `false`.
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    bool is_completed() const { return false; }

    /**
     * Always returns `false`.
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    bool is_cancelled() const { return false; }

    /**
     * Always returns `false`.
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    bool start() { return false; }

    /**
     * Always throws [UnsupportedOperationException].
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    // TODO: suspend function - coroutine semantics not implemented
    void join() {
        throw std::logic_error("This job is always active");
    }

    /**
     * Always throws [UnsupportedOperationException].
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    SelectClause0& get_on_join() {
        throw std::logic_error("This job is always active");
    }

    /**
     * Always throws [IllegalStateException].
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    CancellationException* get_cancellation_exception() {
        throw std::logic_error("This job is always active");
    }

    /**
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    DisposableHandle* invoke_on_completion(CompletionHandler* handler);

    /**
     * Always returns no-op handle.
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    DisposableHandle* invoke_on_completion(bool on_cancelling, bool invoke_immediately, CompletionHandler* handler);

    /**
     * Does nothing.
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    void cancel(CancellationException* cause) {}

    /**
     * Always returns `false`.
     * @suppress This method has bad semantics when cause is not a [CancellationException]. Use [cancel].
     */
    // @Deprecated(level = DeprecationLevel.HIDDEN, message = "Since 1.2.0, binary compatibility with versions <= 1.1.x")
    bool cancel(std::exception_ptr cause) { return false; } // never handles exceptions

    /**
     * Always returns [emptySequence].
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    Sequence<Job*>& get_children();

    /**
     * Always returns [NonDisposableHandle] and does not do anything.
     * @suppress **This an API and should not be used from general code.**
     */
    // @Deprecated(level = DeprecationLevel.WARNING, message = message)
    ChildHandle* attach_child(ChildJob* child);

    /** @suppress */
    std::string to_string() const {
        return "NonCancellable";
    }
};

} // namespace coroutines
} // namespace kotlinx
