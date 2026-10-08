/**
 * @file Job.cpp
 * @brief Implementation of Job factory and related functions.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Job.kt
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `kotlinx/coroutines/Job.hpp`.
 */
// port-lint: source kotlinx-coroutines-core/common/src/Job.kt

#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
// kotlinx.coroutines.selects.* (from Kotlin)
#include "kotlinx/coroutines/selects/Select.hpp"
#include <stdexcept>

namespace kotlinx {
    namespace coroutines {
        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:346-346
        std::shared_ptr<Job> operator+(std::shared_ptr<Job>, std::shared_ptr<Job> other) {
            return other;
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:509-512
        // NOTE(port): The raw entry binds the caller to the compiler-lowered body.
        void* cancel_and_join(Job& job, Continuation<void*>* continuation) {
            return cancel_and_join(job, internal::retain_continuation(continuation));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:509-512
        [[clang::annotate("suspend")]]
        void* cancel_and_join(Job& job, std::shared_ptr<Continuation<void*>> continuation) {
            job.cancel();
            job.join();
            return nullptr;
        }

        // NOTE(port): This explicitly blocking C++ convenience is not a suspend translation.
        void cancel_and_join_blocking(Job& job) {
            job.cancel();
            job.join_blocking();
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:519-521
        void cancel_children(Job& job, std::exception_ptr cause) {
            for (auto& child : job.get_children()) child->cancel(cause);
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:584-586
        void ensure_active(Job& job) {
            if (!job.is_active()) std::rethrow_exception(job.get_cancellation_exception());
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:610-610
        void cancel(Job& job, const std::string& message, std::exception_ptr cause) {
            job.cancel(std::make_exception_ptr(CancellationException(message, cause)));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:555-556
        bool is_active(const CoroutineContext& context) {
            auto job = std::dynamic_pointer_cast<Job>(context.get(Job::type_key));
            return job ? job->is_active() : true;
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:562-564
        void cancel(const CoroutineContext& context, std::exception_ptr cause) {
            if (auto job = std::dynamic_pointer_cast<Job>(context.get(Job::type_key))) job->cancel(cause);
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:602-604
        void ensure_active(const CoroutineContext& context) {
            if (auto job = std::dynamic_pointer_cast<Job>(context.get(Job::type_key))) ensure_active(*job);
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:627-629
        void cancel_children(const CoroutineContext& context, std::exception_ptr cause) {
            if (auto job = std::dynamic_pointer_cast<Job>(context.get(Job::type_key))) {
                for (auto& child : job->get_children()) child->cancel(cause);
            }
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:644-644
        std::shared_ptr<Job> get_job(const CoroutineContext& context) {
            if (auto job = std::dynamic_pointer_cast<Job>(context.get(Job::type_key))) return job;
            throw IllegalStateException("Current context doesn't contain Job in it: " + context.to_string());
        }

        // NOTE(port): Existing C++ prefixed bindings forward to the source-named extensions.
        bool context_is_active(const CoroutineContext& context) { return is_active(context); }
        void context_cancel(const CoroutineContext& context, std::exception_ptr cause) { cancel(context, cause); }
        void context_ensure_active(const CoroutineContext& context) { ensure_active(context); }
        void context_cancel_children(const CoroutineContext& context, std::exception_ptr cause) { cancel_children(context, cause); }
        std::shared_ptr<Job> context_job(const CoroutineContext& context) { return get_job(context); }

        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:288
        // NOTE(port): The generated frame owns this continuation during the virtual call.
        void* Job::join(std::shared_ptr<Continuation<void*>> continuation) {
            return join(continuation.get());
        }
        // -------------------- Factory function implementation --------------------

        /**
 * Creates a job object in an active state.
 * See Job.hpp for full documentation.
 */
        // Transliterated from: kotlinx-coroutines-core/common/src/Job.kt:390-390
        std::shared_ptr<CompletableJob> make_job(std::shared_ptr<struct Job> parent) {
            return JobImpl::create(parent);
        }
    } // namespace coroutines
} // namespace kotlinx
