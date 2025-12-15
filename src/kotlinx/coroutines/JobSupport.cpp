/**
 * @file JobSupport.cpp
 * @brief Implementation of JobSupport and all internal helper classes
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt
 *
 * All internal state machine classes are defined here, not exposed in headers.
 */

#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/Result.hpp"
// kotlinx.coroutines.internal.* (from Kotlin)
#include "kotlinx/coroutines/internal/LockFreeLinkedList.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/ConcurrentLinkedList.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
// kotlinx.coroutines.selects.* (from Kotlin)
#include "kotlinx/coroutines/selects/Select.hpp"
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <cassert>

namespace kotlinx {
    namespace coroutines {
        // ============================================================================
        // Internal Constants (private in Kotlin)
        // ============================================================================

        namespace {
            struct Symbol {
                const char *name;
                std::string to_string() const { return name; }
            };

            Symbol COMPLETING_ALREADY_SYMBOL{"COMPLETING_ALREADY"};
            Symbol COMPLETING_WAITING_CHILDREN_SYMBOL{"COMPLETING_WAITING_CHILDREN"};
            Symbol COMPLETING_RETRY_SYMBOL{"COMPLETING_RETRY"};
            Symbol TOO_LATE_TO_CANCEL_SYMBOL{"TOO_LATE_TO_CANCEL"};
            Symbol SEALED_SYMBOL{"SEALED"};

            // Typed sentinel pointers
            const auto COMPLETING_ALREADY = reinterpret_cast<JobState *>(&COMPLETING_ALREADY_SYMBOL);
            const auto COMPLETING_WAITING_CHILDREN = reinterpret_cast<JobState *>(&
                COMPLETING_WAITING_CHILDREN_SYMBOL);
            const auto COMPLETING_RETRY = reinterpret_cast<JobState *>(&COMPLETING_RETRY_SYMBOL);
            const auto TOO_LATE_TO_CANCEL = reinterpret_cast<JobState *>(&TOO_LATE_TO_CANCEL_SYMBOL);
            const auto SEALED = reinterpret_cast<JobState *>(&SEALED_SYMBOL); // TODO: Find out why SEALED is unused

            constexpr int RETRY = -1;
            constexpr int FALSE = 0;
            constexpr int TRUE = 1;

            constexpr int LIST_ON_COMPLETION_PERMISSION = 1; // TODO: Find out why this is unused
            constexpr int LIST_CHILD_PERMISSION = 2;
            constexpr int LIST_CANCELLATION_PERMISSION = 4; // TODO: Find out why this is unused
        } // anonymous namespace

        // ============================================================================
        // Internal State Classes (private in Kotlin - not exposed in header)
        // ============================================================================

        /**
 * Empty state - either New (inactive) or Active.
 * Transliterated from: private class Empty (JobSupport.kt)
 */
        class Empty : public Incomplete {
            bool is_active_;

        public:
            explicit Empty(bool active) : is_active_(active) {
            }

            bool is_active() const override { return is_active_; }
            NodeList *get_list() const override { return nullptr; }
        };

        // Static instances
        static Empty EMPTY_NEW(false);
        static Empty EMPTY_ACTIVE(true);

        /**
 * Inactive state with a list of handlers waiting to be activated.
 * Transliterated from: private class InactiveNodeList (JobSupport.kt)
 */
        class InactiveNodeList : public Incomplete {
            NodeList *list_;

        public:
            explicit InactiveNodeList(NodeList *list) : list_(list) {
            }

            bool is_active() const override { return false; }
            NodeList *get_list() const override { return list_; }
        };

        /**
 * State during job completion/cancellation.
 * Transliterated from: private class Finishing
 */
        class Finishing : public Incomplete {
        public:
            NodeList *list;
            std::atomic<bool> is_completing{false};
            std::atomic<std::exception_ptr *> root_cause_{nullptr};
            std::atomic<void *> exceptions_holder{nullptr}; // null | exception_ptr* | vector* | SEALED
            std::recursive_mutex mutex;

            Finishing(NodeList *list, bool completing, std::exception_ptr root_cause)
                : list(list) {
                is_completing.store(completing);
                if (root_cause) {
                    root_cause_.store(new std::exception_ptr(root_cause));
                }
            }

            ~Finishing() {
                auto *root = root_cause_.load();
                if (root) delete root;
                // TODO: Clean up exceptions_holder
            }

            bool is_active() const override { return root_cause_.load() == nullptr; }
            NodeList *get_list() const override { return list; }

            bool is_cancelling() const { return root_cause_.load() != nullptr; }
            bool is_sealed() const { return exceptions_holder.load() == &SEALED_SYMBOL; }

            std::exception_ptr get_root_cause() const {
                auto *ptr = root_cause_.load();
                return ptr ? *ptr : nullptr;
            }

            void set_root_cause(std::exception_ptr cause) {
                auto *old = root_cause_.exchange(new std::exception_ptr(cause));
                if (old) delete old;
            }

            std::vector<std::exception_ptr> seal_locked(std::exception_ptr proposed);

            void add_exception_locked(std::exception_ptr exception);
        };

        // ============================================================================
        // Handler Node Types (private in Kotlin)
        // ============================================================================

        class InvokeOnCompletion : public JobNode {
            std::function<void(std::exception_ptr)> handler_;

        public:
            explicit InvokeOnCompletion(std::function<void(std::exception_ptr)> h) : handler_(std::move(h)) {
            }

            bool get_on_cancelling() const override { return false; }
            void invoke(std::exception_ptr cause) override { handler_(cause); }
        };

        class InvokeOnCancelling : public JobNode {
            std::function<void(std::exception_ptr)> handler_;
            std::atomic<bool> invoked_{false};

        public:
            explicit InvokeOnCancelling(std::function<void(std::exception_ptr)> h) : handler_(std::move(h)) {
            }

            bool get_on_cancelling() const override { return true; }

            void invoke(std::exception_ptr cause) override {
                bool expected = false;
                if (invoked_.compare_exchange_strong(expected, true)) {
                    handler_(cause);
                }
            }
        };

        class ChildHandleNode : public JobNode, public ChildHandle {
        public:
            std::shared_ptr<ChildJob> child_job;

            explicit ChildHandleNode(std::shared_ptr<ChildJob> child) : child_job(std::move(child)) {
            }

            bool get_on_cancelling() const override { return true; }

            void invoke(std::exception_ptr cause) override {
                if (child_job && job) {
                    child_job->parent_cancelled(dynamic_cast<ParentJob *>(job));
                }
            }

            bool child_cancelled(std::exception_ptr cause) override {
                return job ? job->child_cancelled(cause) : false;
            }

            std::shared_ptr<Job> get_parent() const override {
                return job ? std::dynamic_pointer_cast<Job>(job->shared_from_this()) : nullptr;
            }

            void dispose() override {
                JobNode::dispose();
            }
        };

        class ChildCompletion : public JobNode {
        public:
            JobSupport *parent;
            Finishing *state;
            ChildHandleNode *child;
            JobState *proposed_update;

            ChildCompletion(JobSupport *p, Finishing *s, ChildHandleNode *c, JobState *u)
                : parent(p), state(s), child(c), proposed_update(u) {
            }

            bool get_on_cancelling() const override { return false; }

            void invoke(std::exception_ptr cause) override;
        };

        /**
 * ResumeOnCompletion - resumes a continuation when job completes
 * Transliterated from: private class ResumeOnCompletion in JobSupport.kt
 * Used by join() - resumes with Unit (nullptr)
 */
        class ResumeOnCompletion : public JobNode {
            Continuation<void *> *continuation_;

        public:
            explicit ResumeOnCompletion(Continuation<void *> *cont) : continuation_(cont) {
            }

            bool get_on_cancelling() const override { return false; }

            void invoke(std::exception_ptr cause) override {
                continuation_->resume_with(Result<void *>::success(nullptr));
            }
        };

        /**
 * ResumeAwaitOnCompletion - resumes a continuation with the deferred result
 * Transliterated from: private class ResumeAwaitOnCompletion<T> in JobSupport.kt
 * Used by await() - resumes with result value or exception
 */
        class ResumeAwaitOnCompletion : public JobNode {
            Continuation<void *> *continuation_;

        public:
            explicit ResumeAwaitOnCompletion(Continuation<void *> *cont) : continuation_(cont) {
            }

            bool get_on_cancelling() const override { return false; }

            void invoke(std::exception_ptr cause) override {
                auto *state = job->get_state_for_await();
                if (auto *ex = dynamic_cast<CompletedExceptionally *>(state)) {
                    continuation_->resume_with(Result<void *>::failure(ex->cause));
                } else {
                    continuation_->resume_with(Result<void *>::success(state));
                }
            }
        };

        // ============================================================================
        // JobSupport::Impl - Private Implementation
        // ============================================================================

        class JobSupport::Impl {
        public:
            std::atomic<JobState *> state;
            std::shared_ptr<Job> parent;
            std::atomic<DisposableHandle *> parent_handle{nullptr};

            explicit Impl(bool active) {
                state.store(active
                                ? static_cast<JobState *>(&EMPTY_ACTIVE)
                                : static_cast<JobState *>(&EMPTY_NEW));
            }

            // State query helpers
            bool is_active() const {
                auto *s = state.load(std::memory_order_acquire);
                if (auto *incomplete = dynamic_cast<Incomplete *>(s)) {
                    return incomplete->is_active();
                }
                return false;
            }

            bool is_completed() const {
                auto *s = state.load(std::memory_order_acquire);
                return dynamic_cast<Incomplete *>(s) == nullptr;
            }

            bool is_cancelled() const {
                auto *s = state.load(std::memory_order_acquire);
                if (dynamic_cast<CompletedExceptionally *>(s)) return true;
                if (auto *finishing = dynamic_cast<Finishing *>(s)) {
                    return finishing->is_cancelling();
                }
                return false;
            }

            // State transition helpers
            int start_internal(JobSupport *job);

            JobState *make_cancelling(JobSupport *job, std::exception_ptr cause);

            bool try_make_cancelling(JobSupport *job, Incomplete *state, std::exception_ptr root_cause);

            NodeList *get_or_promote_cancelling_list(Incomplete *state);

            void promote_empty_to_node_list(Empty *empty);

            void promote_single_to_node_list(JobNode *node);

            void notify_cancelling(JobSupport *job, NodeList *list, std::exception_ptr cause);

            bool cancel_parent(std::exception_ptr cause);

            bool try_put_node_into_list(JobSupport *job, JobNode *node,
                                        std::function<bool(Incomplete *, NodeList *)> try_add);

            void remove_node(JobNode *node);

            // Completion
            bool make_completing(JobSupport *job, JobState *proposed);

            JobState *try_make_completing(JobSupport *job, JobState *state, JobState *proposed);

            JobState *try_make_completing_slow_path(JobSupport *job, Incomplete *state, JobState *proposed);

            JobState *finalize_finishing_state(JobSupport *job, Finishing *state, JobState *proposed);

            std::exception_ptr get_final_root_cause(Finishing *state,
                                                    const std::vector<std::exception_ptr> &exceptions);

            bool try_finalize_simple_state(JobSupport *job, Incomplete *state, JobState *update);

            void complete_state_finalization(JobSupport *job, Incomplete *state, JobState *update);

            // Child iteration
            ChildHandleNode *next_child(internal::LockFreeLinkedListNode *node);

            bool try_wait_for_child(JobSupport *job, Finishing *state, ChildHandleNode *child, JobState *proposed);

            void continue_completing(JobSupport *job, Finishing *state, ChildHandleNode *last_child,
                                     JobState *proposed);

            std::exception_ptr create_cause_exception(std::exception_ptr cause);

            std::exception_ptr default_cancellation_exception(const char *message);
        };

        // ============================================================================
        // JobSupport Public Methods
        // ============================================================================

        JobSupport::JobSupport(bool active) : impl_(std::make_unique<Impl>(active)) {
        }

        JobSupport::~JobSupport() = default;

        std::shared_ptr<Job> JobSupport::get_parent() const {
            return impl_->parent;
        }

        bool JobSupport::is_active() const {
            return impl_->is_active();
        }

        bool JobSupport::is_completed() const {
            return impl_->is_completed();
        }

        bool JobSupport::is_cancelled() const {
            return impl_->is_cancelled();
        }

        std::exception_ptr JobSupport::get_cancellation_exception() {
            auto *s = impl_->state.load(std::memory_order_acquire);

            if (auto *finishing = dynamic_cast<Finishing *>(s)) {
                auto root = finishing->get_root_cause();
                if (root) return root;
                throw std::logic_error("Job is still new or active: " + to_debug_string());
            }
            if (dynamic_cast<Incomplete *>(s)) {
                throw std::logic_error("Job is still new or active: " + to_debug_string());
            }
            if (auto *ex = dynamic_cast<CompletedExceptionally *>(s)) {
                return ex->cause;
            }
            return std::make_exception_ptr(CancellationException(
                cancellation_exception_message() + " has completed normally"));
        }

        bool JobSupport::start() {
            while (true) {
                int result = impl_->start_internal(this);
                if (result == FALSE) return false;
                if (result == TRUE) return true;
                // RETRY - loop again
            }
        }

        void JobSupport::cancel(std::exception_ptr cause) {
            impl_->make_cancelling(this, cause ? cause : impl_->default_cancellation_exception(nullptr));
        }

        void *JobSupport::join(Continuation<void *> *continuation) {
            // Transliterated from: public final override suspend fun join()
            // Fast path: already complete
            if (!join_internal()) {
                // Job is complete - return immediately without suspending
                // TODO: Check coroutineContext.ensureActive() for cancellation
                return nullptr; // Unit
            }

            // Slow path: need to suspend and wait
            return join_suspend(continuation);
        }

        bool JobSupport::join_internal() {
            // Transliterated from: private fun joinInternal(): Boolean
            // Returns true if we need to wait (job is incomplete)
            // Returns false if job is already complete
            while (true) {
                auto *s = impl_->state.load(std::memory_order_acquire);

                // If not incomplete, job is done - no need to wait
                if (!dynamic_cast<Incomplete *>(s)) {
                    return false;
                }

                // Try to start if in New state
                int start_result = impl_->start_internal(this);
                if (start_result >= 0) {
                    return true; // Need to wait
                }
                // RETRY - loop again
            }
        }

        void *JobSupport::join_suspend(Continuation<void *> *continuation) {
            // Transliterated from: private suspend fun joinSuspend()
            // Register a completion handler that resumes the continuation
            auto *node = new ResumeOnCompletion(continuation);
            node->job = this;

            // Add the node to the completion handler list
            bool added = impl_->try_put_node_into_list(this, node,
                                                       [node](Incomplete *state, NodeList *list) -> bool {
                                                           return list->add_last(node);
                                                       });

            if (!added) {
                // Job completed while we were setting up - resume immediately
                delete node;
                return nullptr; // Unit - already complete
            }

            // TODO: cont.disposeOnCancellation(handle) for proper cancellation support
            return COROUTINE_SUSPENDED;
        }

        void JobSupport::join_blocking() {
            // Blocking version for non-coroutine contexts
            while (!is_completed()) {
                std::this_thread::yield();
            }
        }

        // ============================================================================
        // await implementation (for Deferred)
        // Transliterated from: protected suspend fun awaitInternal(): Any?
        // ============================================================================

        JobState *JobSupport::get_state_for_await() const {
            // Return the raw state - caller must check for CompletedExceptionally
            return impl_->state.load(std::memory_order_acquire);
        }

        void *JobSupport::await_internal(Continuation<void *> *continuation) {
            // Transliterated from: protected suspend fun awaitInternal(): Any?
            // Fast path: check state (avoid extra object creation)
            while (true) {
                auto *s = impl_->state.load(std::memory_order_acquire);

                // If not incomplete, job is done - return result immediately
                if (!dynamic_cast<Incomplete *>(s)) {
                    if (auto *ex = dynamic_cast<CompletedExceptionally *>(s)) {
                        // Throw the exception
                        std::rethrow_exception(ex->cause);
                    }
                    // Return the result (type-erased as void*)
                    return s;
                }

                // Try to start if in New state
                int start_result = impl_->start_internal(this);
                if (start_result >= 0) {
                    break; // Need to suspend and wait
                }
                // RETRY - loop again
            }

            // Slow path: suspend and wait
            return await_suspend(continuation);
        }

        void *JobSupport::await_suspend(Continuation<void *> *continuation) {
            // Transliterated from: private suspend fun awaitSuspend(): Any?
            // Register a completion handler that resumes with the result
            auto *node = new ResumeAwaitOnCompletion(continuation);
            node->job = this;

            // Add the node to the completion handler list
            bool added = impl_->try_put_node_into_list(this, node,
                                                       [node](Incomplete *state, NodeList *list) -> bool {
                                                           return list->add_last(node);
                                                       });

            if (!added) {
                // Job completed while we were setting up - return result immediately
                delete node;
                auto *s = impl_->state.load(std::memory_order_acquire);
                if (auto *ex = dynamic_cast<CompletedExceptionally *>(s)) {
                    std::rethrow_exception(ex->cause);
                }
                return s; // Return the result
            }

            // TODO: cont.disposeOnCancellation(handle) for proper cancellation support
            return COROUTINE_SUSPENDED;
        }

        JobState *JobSupport::await_internal_blocking() {
            // Blocking version for non-coroutine contexts
            while (!is_completed()) {
                std::this_thread::yield();
            }
            auto *s = impl_->state.load(std::memory_order_acquire);
            if (auto *ex = dynamic_cast<CompletedExceptionally *>(s)) {
                std::rethrow_exception(ex->cause);
            }
            return s;
        }

        std::vector<std::shared_ptr<Job> > JobSupport::get_children() const {
            std::vector<std::shared_ptr<Job> > result;
            auto *s = impl_->state.load(std::memory_order_acquire);
            if (auto *incomplete = dynamic_cast<Incomplete *>(s)) {
                if (auto *list = incomplete->get_list()) {
                    list->for_each([&](internal::LockFreeLinkedListNode *node) {
                        if (auto *child_node = dynamic_cast<ChildHandleNode *>(node)) {
                            if (child_node->child_job) {
                                // child_job is now stored as shared_ptr<ChildJob>, which inherits from Job
                                result.push_back(std::static_pointer_cast<Job>(child_node->child_job));
                            }
                        }
                    });
                }
            }
            return result;
        }

        std::shared_ptr<ChildHandle> JobSupport::attach_child(std::shared_ptr<ChildJob> child) {
            auto *node = new ChildHandleNode(std::move(child));
            node->job = this;

            bool added = impl_->try_put_node_into_list(this, node,
                                                       [node](Incomplete *state, NodeList *list) -> bool {
                                                           return list->add_last(node);
                                                       });

            if (added) {
                return std::shared_ptr<ChildHandle>(node, [](ChildHandle *) {
                    // Node lifetime managed by list
                });
            }

            // Already completed
            auto *s = impl_->state.load();
            auto *ex = dynamic_cast<CompletedExceptionally *>(s);
            node->invoke(ex ? ex->cause : nullptr);
            delete node;
            // Return a non-disposable ChildHandle (cast the NonDisposableHandle)
            return std::shared_ptr<ChildHandle>(nullptr);
        }

        std::shared_ptr<DisposableHandle> JobSupport::invoke_on_completion(
            std::function<void(std::exception_ptr)> handler) {
            return invoke_on_completion(false, true, std::move(handler));
        }

        std::shared_ptr<DisposableHandle> JobSupport::invoke_on_completion(
            bool on_cancelling,
            bool invoke_immediately,
            std::function<void(std::exception_ptr)> handler) {
            JobNode *node = on_cancelling
                                ? static_cast<JobNode *>(new InvokeOnCancelling(std::move(handler)))
                                : static_cast<JobNode *>(new InvokeOnCompletion(std::move(handler)));
            node->job = this;

            bool added = impl_->try_put_node_into_list(this, node,
                                                       [on_cancelling, invoke_immediately, node](
                                                   Incomplete *state, NodeList *list) -> bool {
                                                           if (on_cancelling) {
                                                               auto *finishing = dynamic_cast<Finishing *>(state);
                                                               auto root_cause = finishing
                                                                   ? finishing->get_root_cause()
                                                                   : nullptr;
                                                               if (root_cause) {
                                                                   if (invoke_immediately) node->invoke(root_cause);
                                                                   return false;
                                                               }
                                                           }
                                                           return list->add_last(node);
                                                       });

            if (added) {
                return std::shared_ptr<DisposableHandle>(node, [](DisposableHandle *) {
                });
            }

            if (invoke_immediately) {
                auto *s = impl_->state.load();
                auto *ex = dynamic_cast<CompletedExceptionally *>(s);
                node->invoke(ex ? ex->cause : nullptr);
            }
            delete node;
            return non_disposable_handle();
        }

        // disposeOnCompletion extension function equivalent
        std::shared_ptr<DisposableHandle> JobSupport::dispose_on_completion(std::shared_ptr<DisposableHandle> handle) {
            return invoke_on_completion([handle](std::exception_ptr) {
                handle->dispose();
            });
        }

        CoroutineContext::Key *JobSupport::key() const {
            return Job::type_key;
        }

        std::shared_ptr<CoroutineContext::Element> JobSupport::get(CoroutineContext::Key *k) const {
            if (k == Job::type_key) {
                return std::const_pointer_cast<CoroutineContext::Element>(
                    std::static_pointer_cast<const CoroutineContext::Element>(
                        const_cast<JobSupport *>(this)->shared_from_this()));
            }
            return nullptr;
        }

        std::exception_ptr JobSupport::get_child_job_cancellation_cause() {
            return get_cancellation_exception();
        }

        void JobSupport::parent_cancelled(ParentJob *parent) {
            cancel(impl_->default_cancellation_exception("Parent cancelled"));
        }

        void JobSupport::handle_on_completion_exception(std::exception_ptr exception) {
            std::rethrow_exception(exception);
        }

        std::string JobSupport::cancellation_exception_message() const {
            return "Job was cancelled";
        }

        bool JobSupport::child_cancelled(std::exception_ptr cause) {
            // Check if it's a CancellationException
            if (cause) {
                try {
                    std::rethrow_exception(cause);
                } catch (const CancellationException &) {
                    return true;
                } catch (...) {
                    // Not a CancellationException
                }
            }

            // Cancel this job and return whether we handle exceptions
            impl_->make_cancelling(this, cause);
            return get_handles_exception();
        }

        bool JobSupport::cancel_coroutine(std::exception_ptr cause) {
            // Transliterated from: public fun cancelCoroutine(cause: Throwable?): Boolean
            // This is essentially the same as cancel() but returns whether cancellation was initiated
            auto result = impl_->make_cancelling(this, cause ? cause : impl_->default_cancellation_exception(nullptr));
            return result != TOO_LATE_TO_CANCEL;
        }

        std::exception_ptr JobSupport::get_completion_cause() const {
            // Transliterated from: protected val completionCause: Throwable?
            // Returns the cause that signals the completion of this job.
            // Returns null if this job completed normally.
            // Throws if job has not completed nor is being cancelled yet.
            auto *s = impl_->state.load(std::memory_order_acquire);
            if (auto *finishing = dynamic_cast<Finishing *>(s)) {
                auto root = finishing->get_root_cause();
                if (!root) {
                    throw std::logic_error("Job is still new or active: " + to_debug_string());
                }
                return root;
            }
            if (dynamic_cast<Incomplete *>(s)) {
                throw std::logic_error("Job is still new or active: " + to_debug_string());
            }
            if (auto *ex = dynamic_cast<CompletedExceptionally *>(s)) {
                return ex->cause;
            }
            // Normal completion - no cause
            return nullptr;
        }

        bool JobSupport::get_completion_cause_handled() const {
            // Transliterated from: protected val completionCauseHandled: Boolean
            auto *s = impl_->state.load(std::memory_order_acquire);
            if (auto *ex = dynamic_cast<CompletedExceptionally *>(s)) {
                return ex->handled;
            }
            return false;
        }

        bool JobSupport::is_completed_exceptionally() const {
            // Transliterated from: val isCompletedExceptionally: Boolean
            auto *s = impl_->state.load(std::memory_order_acquire);
            return dynamic_cast<CompletedExceptionally *>(s) != nullptr;
        }

        std::exception_ptr JobSupport::get_completion_exception_or_null() const {
            // Transliterated from: fun getCompletionExceptionOrNull(): Throwable?
            auto *s = impl_->state.load(std::memory_order_acquire);
            if (dynamic_cast<Incomplete *>(s)) {
                throw std::logic_error("This job has not completed yet");
            }
            if (auto *ex = dynamic_cast<CompletedExceptionally *>(s)) {
                return ex->cause;
            }
            return nullptr;
        }

        void JobSupport::init_parent_job(std::shared_ptr<Job> parent) {
            assert(impl_->parent_handle.load() == nullptr);
            if (!parent) {
                impl_->parent_handle.store(&NonDisposableHandle::instance());
                return;
            }
            impl_->parent = parent;
            parent->start();
            auto handle = parent->attach_child(std::dynamic_pointer_cast<ChildJob>(shared_from_this()));
            impl_->parent_handle.store(handle.get());

            if (is_completed()) {
                handle->dispose();
                impl_->parent_handle.store(&NonDisposableHandle::instance());
            }
        }

        bool JobSupport::make_completing(JobState *proposed_state) {
            return impl_->make_completing(this, proposed_state);
        }

        JobSupport::CompletingResult JobSupport::make_completing_once(JobState *proposed_state) {
            while (true) {
                auto *s = impl_->state.load(std::memory_order_acquire);
                auto *final_state = impl_->try_make_completing(this, s, proposed_state);

                if (final_state == COMPLETING_ALREADY) return CompletingResult::ALREADY_COMPLETING;
                if (final_state == COMPLETING_WAITING_CHILDREN) return CompletingResult::COMPLETING;
                if (final_state == COMPLETING_RETRY) continue;
                return CompletingResult::COMPLETED;
            }
        }

        JobState *JobSupport::get_completed_internal() const {
            auto *s = impl_->state.load(std::memory_order_acquire);
            if (dynamic_cast<Incomplete *>(s)) {
                throw std::logic_error("This job has not completed yet");
            }
            if (auto *ex = dynamic_cast<CompletedExceptionally *>(s)) {
                std::rethrow_exception(ex->cause);
            }
            return s;
        }

        std::string JobSupport::to_debug_string() const {
            return name_string() + "{" + state_string() + "}";
        }

        std::string JobSupport::name_string() const {
            return "JobSupport";
        }

        std::string JobSupport::state_string() const {
            auto *s = impl_->state.load();
            if (auto *finishing = dynamic_cast<Finishing *>(s)) {
                if (finishing->is_cancelling()) return "Cancelling";
                if (finishing->is_completing.load()) return "Completing";
                return "Active";
            }
            if (auto *incomplete = dynamic_cast<Incomplete *>(s)) {
                return incomplete->is_active() ? "Active" : "New";
            }
            if (dynamic_cast<CompletedExceptionally *>(s)) {
                return "Cancelled";
            }
            return "Completed";
        }

        // ============================================================================
        // JobSupport::Impl Private Methods
        // ============================================================================

        int JobSupport::Impl::start_internal(JobSupport *job) {
            auto *s = state.load(std::memory_order_acquire);

            if (s == &EMPTY_NEW) {
                auto *expected = s;
                if (state.compare_exchange_strong(expected, static_cast<JobState *>(&EMPTY_ACTIVE))) {
                    job->on_start();
                    return TRUE;
                }
                return RETRY;
            }

            if (auto *inactive_list = dynamic_cast<InactiveNodeList *>(s)) {
                auto *expected = s;
                if (state.compare_exchange_strong(expected, inactive_list->get_list())) {
                    job->on_start();
                    return TRUE;
                }
                return RETRY;
            }

            return FALSE; // Already active or completed
        }

        JobState *JobSupport::Impl::make_cancelling(JobSupport *job, std::exception_ptr cause) {
            std::exception_ptr cause_cache;

            while (true) {
                auto *s = state.load(std::memory_order_acquire);

                if (auto *finishing = dynamic_cast<Finishing *>(s)) {
                    std::lock_guard<std::recursive_mutex> lock(finishing->mutex);
                    if (finishing->is_sealed()) return TOO_LATE_TO_CANCEL;

                    bool was_cancelling = finishing->is_cancelling();
                    if (cause || !was_cancelling) {
                        if (!cause_cache) cause_cache = create_cause_exception(cause);
                        finishing->add_exception_locked(cause_cache);
                    }

                    auto root = finishing->get_root_cause();
                    if (root && !was_cancelling) {
                        notify_cancelling(job, finishing->list, root);
                    }
                    return COMPLETING_ALREADY;
                }

                if (auto *incomplete = dynamic_cast<Incomplete *>(s)) {
                    if (!cause_cache) cause_cache = create_cause_exception(cause);
                    if (incomplete->is_active()) {
                        if (try_make_cancelling(job, incomplete, cause_cache)) {
                            return COMPLETING_ALREADY;
                        }
                    } else {
                        // Start completing from inactive state
                        if (job->make_completing(new CompletedExceptionally(cause_cache))) {
                            return COMPLETING_ALREADY;
                        }
                    }
                    continue; // Retry
                }

                // Already completed
                return TOO_LATE_TO_CANCEL;
            }
        }

        bool JobSupport::Impl::try_make_cancelling(JobSupport *job, Incomplete *s, std::exception_ptr root_cause) {
            auto *list = get_or_promote_cancelling_list(s);
            if (!list) return false;

            auto *finishing = new Finishing(list, false, root_cause);
            auto *expected = static_cast<JobState *>(s);
            if (!state.compare_exchange_strong(expected, finishing)) {
                delete finishing;
                return false;
            }

            notify_cancelling(job, list, root_cause);
            return true;
        }

        NodeList *JobSupport::Impl::get_or_promote_cancelling_list(Incomplete *s) {
            if (auto *list = s->get_list()) {
                return list;
            }

            if (dynamic_cast<Empty *>(s)) {
                return new NodeList();
            }

            if (auto *node = dynamic_cast<JobNode *>(s)) {
                promote_single_to_node_list(node);
                return nullptr; // Retry
            }

            return nullptr;
        }

        void JobSupport::Impl::promote_empty_to_node_list(Empty *empty) {
            auto *list = new NodeList();
            if (empty->is_active()) {
                auto *expected = static_cast<JobState *>(empty);
                if (!state.compare_exchange_strong(expected, list)) {
                    delete list;
                }
            } else {
                auto *inactive = new InactiveNodeList(list);
                auto *expected = static_cast<JobState *>(empty);
                if (!state.compare_exchange_strong(expected, inactive)) {
                    delete inactive;
                    delete list;
                }
            }
        }

        void JobSupport::Impl::promote_single_to_node_list(JobNode *node) {
            auto *list = new NodeList();
            if (node->add_one_if_empty(list)) {
                auto *expected = static_cast<JobState *>(node);
                if (state.compare_exchange_strong(expected, list)) {
                    return;
                }
            }
            delete list;
        }

        void JobSupport::Impl::notify_cancelling(JobSupport *job, NodeList *list, std::exception_ptr cause) {
            job->on_cancelling(cause);

            std::exception_ptr handler_exception;
            list->for_each([&](internal::LockFreeLinkedListNode *raw_node) {
                if (auto *node = dynamic_cast<JobNode *>(raw_node)) {
                    if (node->get_on_cancelling()) {
                        try {
                            node->invoke(cause);
                        } catch (...) {
                            if (!handler_exception) handler_exception = std::current_exception();
                        }
                    }
                }
            });

            cancel_parent(cause);

            if (handler_exception) {
                job->handle_on_completion_exception(handler_exception);
            }
        }

        bool JobSupport::Impl::cancel_parent(std::exception_ptr cause) {
            auto *handle = parent_handle.load(std::memory_order_relaxed);
            if (handle) {
                if (auto *child_handle = dynamic_cast<ChildHandle *>(handle)) {
                    return child_handle->child_cancelled(cause);
                }
            }
            return false;
        }

        bool JobSupport::Impl::try_put_node_into_list(JobSupport *job, JobNode *node,
                                                      std::function<bool(Incomplete *, NodeList *)> try_add) {
            while (true) {
                auto *s = state.load(std::memory_order_acquire);

                if (s == &EMPTY_ACTIVE || s == &EMPTY_NEW) {
                    promote_empty_to_node_list(static_cast<Empty *>(s));
                    continue;
                }

                if (auto *incomplete = dynamic_cast<Incomplete *>(s)) {
                    auto *list = incomplete->get_list();
                    if (!list) {
                        if (auto *job_node = dynamic_cast<JobNode *>(s)) {
                            promote_single_to_node_list(job_node);
                            continue;
                        }
                        if (auto *empty = dynamic_cast<Empty *>(s)) {
                            promote_empty_to_node_list(empty);
                            continue;
                        }
                    } else {
                        return try_add(incomplete, list);
                    }
                } else {
                    return false; // Completed
                }
            }
        }

        void JobSupport::Impl::remove_node(JobNode *node) {
            while (true) {
                auto *s = state.load(std::memory_order_acquire);

                if (auto *state_node = dynamic_cast<JobNode *>(s)) {
                    if (state_node != node) return;
                    auto *expected = s;
                    if (state.compare_exchange_strong(expected, static_cast<JobState *>(&EMPTY_ACTIVE))) {
                        return;
                    }
                    continue;
                }

                if (auto *incomplete = dynamic_cast<Incomplete *>(s)) {
                    if (incomplete->get_list()) {
                        node->remove();
                    }
                    return;
                }

                return; // Completed
            }
        }

        bool JobSupport::Impl::make_completing(JobSupport *job, JobState *proposed) {
            while (true) {
                auto *s = state.load(std::memory_order_acquire);
                auto *final_state = try_make_completing(job, s, proposed);

                if (final_state == COMPLETING_ALREADY) return false;
                if (final_state == COMPLETING_WAITING_CHILDREN) return true;
                if (final_state == COMPLETING_RETRY) continue;
                return true;
            }
        }

        JobState *JobSupport::Impl::try_make_completing(JobSupport *job, JobState *s, JobState *proposed) {
            auto *incomplete = dynamic_cast<Incomplete *>(s);
            if (!incomplete) return COMPLETING_ALREADY;

            bool is_proposed_exception = dynamic_cast<CompletedExceptionally *>(proposed) != nullptr;

            // Fast path for simple states
            if ((dynamic_cast<Empty *>(s) || (dynamic_cast<JobNode *>(s) && !dynamic_cast<ChildHandleNode *>(s)))
                && !is_proposed_exception) {
                if (try_finalize_simple_state(job, incomplete, proposed)) {
                    return s;
                }
                return COMPLETING_RETRY;
            }

            return try_make_completing_slow_path(job, incomplete, proposed);
        }

        JobState *JobSupport::Impl::try_make_completing_slow_path(JobSupport *job, Incomplete *s, JobState *proposed) {
            auto *list = get_or_promote_cancelling_list(s);
            if (!list) return COMPLETING_RETRY;

            auto *finishing = dynamic_cast<Finishing *>(s);
            if (!finishing) {
                finishing = new Finishing(list, false, nullptr);
                auto *expected = static_cast<JobState *>(s);
                if (!state.compare_exchange_strong(expected, finishing)) {
                    delete finishing;
                    return COMPLETING_RETRY;
                }
            }

            {
                std::lock_guard<std::recursive_mutex> lock(finishing->mutex);
                if (finishing->is_completing.load()) return COMPLETING_ALREADY;
                finishing->is_completing.store(true);

                if (auto *ex = dynamic_cast<CompletedExceptionally *>(proposed)) {
                    finishing->add_exception_locked(ex->cause);
                }
            }

            // Check for children
            auto *first_child = next_child(list);
            if (first_child && try_wait_for_child(job, finishing, first_child, proposed)) {
                return COMPLETING_WAITING_CHILDREN;
            }

            return finalize_finishing_state(job, finishing, proposed);
        }

        JobState *JobSupport::Impl::finalize_finishing_state(JobSupport *job, Finishing *finishing,
                                                             JobState *proposed) {
            auto *proposed_ex = dynamic_cast<CompletedExceptionally *>(proposed);
            std::exception_ptr proposed_exception = proposed_ex ? proposed_ex->cause : nullptr;

            bool was_cancelling;
            std::exception_ptr final_exception;
            {
                std::lock_guard<std::recursive_mutex> lock(finishing->mutex);
                was_cancelling = finishing->is_cancelling();
                auto exceptions = finishing->seal_locked(proposed_exception);
                final_exception = get_final_root_cause(finishing, exceptions);
            }

            JobState *final_state;
            if (!final_exception) {
                final_state = proposed;
            } else if (final_exception == proposed_exception) {
                final_state = proposed;
            } else {
                final_state = new CompletedExceptionally(final_exception);
            }

            if (final_exception) {
                bool handled = cancel_parent(final_exception) || job->handle_job_exception(final_exception);
                if (handled) {
                    if (auto *ex_state = dynamic_cast<CompletedExceptionally *>(final_state)) {
                        ex_state->make_handled();
                    }
                }
            }

            if (!was_cancelling) job->on_cancelling(final_exception);
            job->on_completion_internal(final_state);

            auto *expected = static_cast<JobState *>(finishing);
            bool cas_ok = state.compare_exchange_strong(expected, final_state);
            assert(cas_ok);

            complete_state_finalization(job, finishing, final_state);
            return final_state;
        }

        std::exception_ptr JobSupport::Impl::get_final_root_cause(Finishing *finishing,
                                                                  const std::vector<std::exception_ptr> &exceptions) {
            if (exceptions.empty()) {
                if (finishing->is_cancelling()) {
                    return default_cancellation_exception(nullptr);
                }
                return nullptr;
            }
            return exceptions[0];
        }

        bool JobSupport::Impl::try_finalize_simple_state(JobSupport *job, Incomplete *s, JobState *update) {
            auto *expected = static_cast<JobState *>(s);
            if (!state.compare_exchange_strong(expected, update)) {
                return false;
            }
            job->on_cancelling(nullptr);
            job->on_completion_internal(update);
            complete_state_finalization(job, s, update);
            return true;
        }

        void JobSupport::Impl::complete_state_finalization(JobSupport *job, Incomplete *s, JobState *update) {
            auto *handle = parent_handle.load();
            if (handle) {
                handle->dispose();
                parent_handle.store(&NonDisposableHandle::instance());
            }

            auto *ex = dynamic_cast<CompletedExceptionally *>(update);
            std::exception_ptr cause = ex ? ex->cause : nullptr;

            if (auto *node = dynamic_cast<JobNode *>(s)) {
                try {
                    node->invoke(cause);
                } catch (...) {
                    job->handle_on_completion_exception(std::current_exception());
                }
            } else if (auto *list = s->get_list()) {
                list->notify_completion(cause);
            }

            job->after_completion(update);
        }

        ChildHandleNode *JobSupport::Impl::next_child(internal::LockFreeLinkedListNode *node) {
            auto *cur = node;
            while (cur->is_removed()) {
                cur = cur->prev_node();
            }
            while (true) {
                cur = cur->next_node();
                if (!cur || dynamic_cast<NodeList *>(cur)) return nullptr;
                if (cur->is_removed()) continue;
                if (auto *child = dynamic_cast<ChildHandleNode *>(cur)) return child;
            }
        }

        bool JobSupport::Impl::try_wait_for_child(JobSupport *job, Finishing *finishing,
                                                  ChildHandleNode *child, JobState *proposed) {
            auto *completion = new ChildCompletion(job, finishing, child, proposed);
            auto handle = child->child_job->invoke_on_completion(false, false,
                                                                 [completion](std::exception_ptr cause) {
                                                                     completion->invoke(cause);
                                                                 });

            if (handle.get() != &NonDisposableHandle::instance()) {
                return true;
            }

            delete completion;
            auto *next = next_child(child);
            if (!next) return false;
            return try_wait_for_child(job, finishing, next, proposed);
        }

        void JobSupport::Impl::continue_completing(JobSupport *job, Finishing *finishing,
                                                   ChildHandleNode *last_child, JobState *proposed) {
            auto *wait_child = next_child(last_child);
            if (wait_child && try_wait_for_child(job, finishing, wait_child, proposed)) {
                return;
            }

            finishing->list->close(LIST_CHILD_PERMISSION);

            auto *wait_child_again = next_child(last_child);
            if (wait_child_again && try_wait_for_child(job, finishing, wait_child_again, proposed)) {
                return;
            }

            auto *final_state = finalize_finishing_state(job, finishing, proposed);
            job->after_completion(final_state);
        }

        std::exception_ptr JobSupport::Impl::create_cause_exception(std::exception_ptr cause) {
            if (!cause) return default_cancellation_exception(nullptr);
            return cause;
        }

        std::exception_ptr JobSupport::Impl::default_cancellation_exception(const char *message) {
            return std::make_exception_ptr(CancellationException(message ? message : "Job was cancelled"));
        }

        // ============================================================================
        // Finishing Methods
        // ============================================================================

        std::vector<std::exception_ptr> Finishing::seal_locked(std::exception_ptr proposed) {
            std::vector<std::exception_ptr> result;
            auto *eh = exceptions_holder.load();

            if (eh == nullptr) {
                // No exceptions yet
            } else if (eh == &SEALED_SYMBOL) {
                return result;
            } else {
                // eh is exception_ptr* or vector*
                // Simplified: assume single exception
                result.push_back(*static_cast<std::exception_ptr *>(eh));
            }

            auto root = get_root_cause();
            if (root) result.insert(result.begin(), root);
            if (proposed && proposed != root) result.push_back(proposed);

            exceptions_holder.store(&SEALED_SYMBOL);
            return result;
        }

        void Finishing::add_exception_locked(std::exception_ptr exception) {
            auto root = get_root_cause();
            if (!root) {
                set_root_cause(exception);
                return;
            }
            // Store additional exceptions (simplified)
            auto *eh = exceptions_holder.load();
            if (eh == nullptr) {
                exceptions_holder.store(new std::exception_ptr(exception));
            }
        }

        // ============================================================================
        // Other Internal Class Methods
        // ============================================================================

        void JobNode::dispose() {
            if (job) {
                job->impl_->remove_node(this);
            }
        }

        void NodeList::notify_completion(std::exception_ptr cause) {
            close(LIST_ON_COMPLETION_PERMISSION);

            std::exception_ptr handler_exception;
            for_each([&](internal::LockFreeLinkedListNode *raw_node) {
                if (auto *node = dynamic_cast<JobNode *>(raw_node)) {
                    try {
                        node->invoke(cause);
                    } catch (...) {
                        if (!handler_exception) {
                            handler_exception = std::current_exception();
                        }
                    }
                }
            });

            if (handler_exception) {
                std::rethrow_exception(handler_exception);
            }
        }

        void ChildCompletion::invoke(std::exception_ptr cause) {
            parent->impl_->continue_completing(parent, state, child, proposed_update);
        }
    } // namespace coroutines
} // namespace kotlinx
