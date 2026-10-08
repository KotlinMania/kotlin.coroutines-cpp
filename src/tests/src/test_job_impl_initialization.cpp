/** Transliterated from: kotlinx-coroutines-core/common/test/CompletableJobTest.kt:8-45 */
#include "kotlinx/coroutines/JobImpl.hpp"
#include <cassert>
#include <stdexcept>

// NOTE(port): Plain executable harness for source assertions; the complete
// Kotlin TestBase/class API is outside this standalone regression executable.
namespace kotlinx::coroutines {

// Transliterated from: kotlinx-coroutines-core/common/test/CompletableJobTest.kt:8-17
void test_complete() {
    auto job = make_job(nullptr);
    assert(job->is_active() && !job->is_completed());
    assert(job->complete());
    assert(job->is_completed() && !job->is_active() && !job->is_cancelled());
    assert(!job->complete());
}

// Transliterated from: kotlinx-coroutines-core/common/test/CompletableJobTest.kt:20-30
void test_complete_with_exception() {
    auto job = make_job(nullptr);
    assert(job->is_active() && !job->is_completed());
    auto failure = std::make_exception_ptr(std::runtime_error("test failure"));
    assert(job->complete_exceptionally(failure));
    assert(job->is_completed() && !job->is_active() && job->is_cancelled());
    assert(!job->complete_exceptionally(failure) && !job->complete());
}

// Transliterated from: kotlinx-coroutines-core/common/test/CompletableJobTest.kt:33-45
void test_complete_with_children() {
    auto parent = make_job(nullptr);
    auto child = make_job(parent);
    assert(parent->complete() && !parent->complete());
    assert(parent->is_active() && !parent->is_completed());
    assert(child->complete());
    assert(child->is_completed() && parent->is_completed());
    assert(!child->is_active() && !parent->is_active());
}

// NOTE(port): Expose the existing protected completion operation on an actual
// JobSupport parent; its state and exception policy are the library's own.
struct CompletionParent final : JobSupport {
    CompletionParent() : JobSupport(true) {}
    using JobSupport::make_completing;
};

// NOTE(port): Regression for JobSupport.kt:1438,1444-1450. Read the source
// initialized property for the first time after the parent link is detached.
// A lazy calculation would lose the exception-handling parent at this point.
void test_exception_property_survives_detachment() {
    auto handling_parent = std::make_shared<CompletionParent>();
    auto middle = JobImpl::create(handling_parent);
    auto child = JobImpl::create(middle);
    assert(middle->complete() && !middle->is_completed());
    assert(child->complete() && middle->is_completed());
    assert(!child->get_parent() && !middle->get_parent());
    assert(static_cast<JobSupport&>(*child).get_handles_exception());
    assert(static_cast<JobSupport&>(*middle).get_handles_exception());
    assert(handling_parent->make_completing(nullptr));
    assert(handling_parent->is_completed());

    auto unhandled_parent = JobImpl::create(nullptr);
    auto unhandled_child = JobImpl::create(unhandled_parent);
    assert(unhandled_child->complete());
    assert(!unhandled_child->get_parent());
    assert(!static_cast<JobSupport&>(*unhandled_child).get_handles_exception());
    assert(unhandled_parent->complete());
}

// NOTE(port): Entry used by the separate ordinary C++ executable harness.
void run_job_impl_initialization_tests() {
    test_complete();
    test_complete_with_exception();
    test_complete_with_children();
    test_exception_property_survives_detachment();
}

} // namespace kotlinx::coroutines
