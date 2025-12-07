// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/CompletableJobTest.kt
// TODO: Review imports and dependencies
// TODO: Adapt test framework annotations to C++ testing framework
// TODO: Handle suspend functions and coroutine context
// TODO: Handle nullable types appropriately

namespace kotlinx {
namespace coroutines {

// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*

class CompletableJobTest : public TestBase {
public:
    // @Test
    void test_complete() {
        auto job = Job();
        assert_true(job.is_active());
        assert_false(job.is_completed());
        assert_true(job.complete());
        assert_true(job.is_completed());
        assert_false(job.is_active());
        assert_false(job.is_cancelled());
        assert_false(job.complete());
    }

    // @Test
    void test_complete_with_exception() {
        auto job = Job();
        assert_true(job.is_active());
        assert_false(job.is_completed());
        assert_true(job.complete_exceptionally(TestException()));
        assert_true(job.is_completed());
        assert_false(job.is_active());
        assert_true(job.is_cancelled());
        assert_false(job.complete_exceptionally(TestException()));
        assert_false(job.complete());
    }

    // @Test
    void test_complete_with_children() {
        auto parent = Job();
        auto child = Job(parent);
        assert_true(parent.complete());
        assert_false(parent.complete());
        assert_true(parent.is_active());
        assert_false(parent.is_completed());
        assert_true(child.complete());
        assert_true(child.is_completed());
        assert_true(parent.is_completed());
        assert_false(child.is_active());
        assert_false(parent.is_active());
    }

    // @Test
    void test_exception_is_not_reported_to_children() {
        parametrized([this](auto job) {
            expect(1);
            auto child = launch(job, [this]() {
                expect(2);
                try {
                    // KT-33840
                    hang([]() {});
                } catch (const std::exception& e) {
                    assert_is<CancellationException>(e);
                    auto cause = RECOVER_STACK_TRACES ? e.cause->cause : e.cause;
                    assert_is<TestException>(cause);
                    expect(4);
                    throw;
                }
            });
            yield();
            expect(3);
            job.complete_exceptionally(TestException());
            child.join();
            finish(5);
        });
    }

    // @Test
    void test_complete_exceptionally_doesnt_affect_deferred() {
        parametrized([this](auto job) {
            expect(1);
            auto child = async(job, [this]() {
                expect(2);
                try {
                    // KT-33840
                    hang([]() {});
                } catch (const std::exception& e) {
                    assert_is<CancellationException>(e);
                    auto cause = RECOVER_STACK_TRACES ? e.cause->cause : e.cause;
                    assert_is<TestException>(cause);
                    expect(4);
                    throw;
                }
            });
            yield();
            expect(3);
            job.complete_exceptionally(TestException());
            child.join();
            assert_true(dynamic_cast<CancellationException*>(child.get_completion_exception_or_null()) != nullptr);
            finish(5);
        });
    }

private:
    template<typename Block>
    void parametrized(Block block) {
        run_test([&block]() {
            block(Job());
            reset();
            block(SupervisorJob());
        });
    }
};

} // namespace coroutines
} // namespace kotlinx
