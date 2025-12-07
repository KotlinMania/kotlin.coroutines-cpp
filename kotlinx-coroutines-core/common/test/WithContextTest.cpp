// Original: kotlinx-coroutines-core/common/test/WithContextTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: @file:Suppress("NAMED_ARGUMENTS_NOT_ALLOWED") // KT-22237
// TODO: Handle withContext behavior and context switching
// TODO: Map test framework annotations to C++ test framework

namespace kotlinx {
namespace coroutines {

class WithContextTest : public TestBase {
public:
    // TODO: @Test
    void test_throw_exception() {
        // TODO: runTest { expect(1); try { with_context<void>(coroutineContext) { expect(2); throw std::runtime_error(); } } catch (...) { expect(3); } yield(); finish(4); }
    }

    // TODO: @Test
    void test_throw_exception_from_wrapped_context() {
        // TODO: runTest { expect(1); try { with_context<void>(wrapper_dispatcher(coroutineContext)) { expect(2); throw std::runtime_error(); } } catch (...) { expect(3); } yield(); finish(4); }
    }

    // TODO: @Test
    void test_same_context_no_suspend() {
        // TODO: runTest { expect(1); launch(coroutineContext) { finish(5); } expect(2); const auto result = with_context(coroutineContext) { expect(3); "OK".wrap(); }.unwrap(); assertEquals("OK", result); expect(4); }
    }

    // TODO: @Test
    void test_same_context_with_suspend() {
        // TODO: runTest { expect(1); launch(coroutineContext) { expect(4); } expect(2); const auto result = with_context(coroutineContext) { expect(3); yield(); expect(5); "OK".wrap(); }.unwrap(); assertEquals("OK", result); finish(6); }
    }

    // TODO: @Test
    void test_cancel_with_job_no_suspend() {
        // TODO: runTest { expect(1); launch(coroutineContext) { finish(6); } expect(2); const auto job = Job(); try { with_context(coroutineContext + job) { expect(3); job.cancel(); try { yield(); expectUnreached(); } catch (...) { expect(4); } "OK".wrap(); } expectUnreached(); } catch (...) { expect(5); } }
    }

    // TODO: @Test
    void test_cancel_with_job_with_suspend() {
        // TODO: runTest(expected = { it is CancellationException }) { expect(1); launch(coroutineContext) { expect(4); } expect(2); const auto job = Job(); with_context(coroutineContext + job) { expect(3); yield(); expect(5); job.cancel(); try { yield(); expectUnreached(); } catch (...) { finish(6); } "OK".wrap(); } expectUnreached(); }
    }

    // TODO: @Test
    void test_run_cancellable_default() {
        // TODO: runTest(expected = { it is CancellationException }) { const auto job = Job(); job.cancel(); with_context(job + wrapper_dispatcher(coroutineContext)) { expectUnreached(); } }
    }

    // TODO: @Test
    void test_run_cancellation_undispatched_vs_exception() {
        // TODO: runTest { expect(1); Job* job = nullptr; job = launch(start = CoroutineStart.UNDISPATCHED) { expect(2); try { with_context<void>(CoroutineName("test")) { expect(3); yield(); expect(5); job->cancel(); throw TestException(); } } catch (const TestException& e) { expect(6); } } expect(4); yield(); finish(7); }
    }

    // TODO: @Test
    void test_run_cancellation_dispatched_vs_exception() {
        // TODO: runTest { expect(1); Job* job = nullptr; job = launch(start = CoroutineStart.UNDISPATCHED) { expect(2); try { with_context<void>(wrapper_dispatcher(coroutineContext)) { expect(4); yield(); expect(6); job->cancel(); throw TestException(); } } catch (const TestException& e) { expect(8); } } expect(3); yield(); expect(5); yield(); expect(7); yield(); finish(9); }
    }

    // TODO: @Test
    void test_run_self_cancellation_with_exception() {
        // TODO: runTest { expect(1); Job* job = nullptr; job = launch(Job()) { try { expect(3); with_context<void>(wrapper_dispatcher(coroutineContext)) { require(is_active()); expect(5); job->cancel(); require(!is_active()); throw TestException(); } } catch (const std::exception& e) { expect(7); assertIs<TestException>(e); } } expect(2); yield(); expect(4); yield(); expect(6); yield(); finish(8); }
    }

    // TODO: @Test
    void test_run_self_cancellation() {
        // TODO: runTest { expect(1); Job* job = nullptr; job = launch(Job()) { try { expect(3); with_context(wrapper_dispatcher(coroutineContext)) { require(is_active()); expect(5); job->cancel(); require(!is_active()); "OK".wrap(); } expectUnreached(); } catch (const std::exception& e) { expect(7); assertIs<CancellationException>(e); } } expect(2); yield(); expect(4); yield(); expect(6); yield(); finish(8); }
    }

    // TODO: @Test
    void test_with_context_scope_failure() {
        // TODO: runTest { expect(1); try { with_context(wrapper_dispatcher(coroutineContext)) { expect(2); launch { expect(4); throw TestException(); } expect(3); "OK".wrap(); } expectUnreached(); } catch (const TestException& e) { expect(5); } finish(6); }
    }

    // TODO: @Test
    void test_with_context_child_wait_same_context() {
        // TODO: runTest { expect(1); with_context(coroutineContext) { expect(2); launch { expect(4); } expect(3); "OK".wrap(); }.unwrap(); finish(5); }
    }

    // TODO: @Test
    void test_with_context_child_wait_wrapped_context() {
        // TODO: runTest { expect(1); with_context(wrapper_dispatcher(coroutineContext)) { expect(2); launch { expect(4); } expect(3); "OK".wrap(); }.unwrap(); finish(5); }
    }

    // TODO: @Test
    void test_incomplete_with_context_state() {
        // TODO: runTest { Job* ctx_job = nullptr; with_context(wrapper_dispatcher(coroutineContext)) { ctx_job = &coroutineContext[Job]!!; ctx_job->invoke_on_completion([]{  }); } ctx_job->join(); assertTrue(ctx_job->is_completed()); assertFalse(ctx_job->is_active()); assertFalse(ctx_job->is_cancelled()); }
    }

    // TODO: @Test
    void test_with_context_cancelled_job() {
        // TODO: runTest { expect(1); const auto job = Job(); job.cancel(); try { with_context(job) { expectUnreached(); } } catch (...) { expect(2); } finish(3); }
    }

    // TODO: @Test
    void test_with_context_cancelled_this_job() {
        // TODO: runTest(expected = { it is CancellationException }) { coroutineContext.cancel(); with_context(wrapper_dispatcher(coroutineContext)) { expectUnreached(); } expectUnreached(); }
    }

    // TODO: @Test
    void test_sequential_cancellation() {
        // TODO: runTest { const auto job = launch { expect(1); with_context(wrapper_dispatcher()) { expect(2); } expectUnreached(); }; yield(); const auto job2 = launch { expect(3); job.cancel(); }; join_all(job, job2); finish(4); }
    }

private:
    struct Wrapper { // TODO: : public Incomplete {
        std::string value;
        // TODO: bool is_active() const override { error(""); }
        // TODO: NodeList* list() const override { error(""); }
    };

    Wrapper wrap(const std::string& s) { return Wrapper{s}; }
    std::string unwrap(const Wrapper& w) { return w.value; }
};

} // namespace coroutines
} // namespace kotlinx
