// Original: kotlinx-coroutines-core/common/test/JobTest.kt
// TODO: Transliterated from Kotlin - needs C++ implementation
// TODO: @file:Suppress("DEPRECATION")
// TODO: Handle Job handlers, disposable handles, and completion callbacks
// TODO: Map test framework annotations to C++ test framework

#include <vector>
#include <array>

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlin.test.*

        class JobTest : public TestBase {
        public:
            // TODO: @Test
            void test_state() {
                // TODO: const auto job = Job();
                // TODO: assertNull(job->parent());
                // TODO: assertTrue(job->is_active());
                // TODO: job->cancel();
                // TODO: assertTrue(!job->is_active());
            }

            // TODO: @Test
            void test_handler() {
                // TODO: const auto job = Job();
                int fire_count = 0;
                // TODO: job->invoke_on_completion([&] { fire_count++; });
                // TODO: assertTrue(job->is_active());
                // TODO: assertEquals(0, fire_count);
                // cancel once
                // TODO: job->cancel();
                // TODO: assertTrue(!job->is_active());
                // TODO: assertEquals(1, fire_count);
                // cancel again
                // TODO: job->cancel();
                // TODO: assertTrue(!job->is_active());
                // TODO: assertEquals(1, fire_count);
            }

            // TODO: @Test
            void test_many_handlers() {
                // TODO: const auto job = Job();
                const int n = 100; // TODO: * stressTestMultiplier;
                std::vector<int> fire_count(n, 0);
                for (int i = 0; i < n; i++) {
                    // TODO: job->invoke_on_completion([&, i] { fire_count[i]++; });
                }
                // TODO: assertTrue(job->is_active());
                for (int i = 0; i < n; i++) {
                    // TODO: assertEquals(0, fire_count[i]);
                }
                // cancel once
                // TODO: job->cancel();
                // TODO: assertTrue(!job->is_active());
                for (int i = 0; i < n; i++) {
                    // TODO: assertEquals(1, fire_count[i]);
                }
                // cancel again
                // TODO: job->cancel();
                // TODO: assertTrue(!job->is_active());
                for (int i = 0; i < n; i++) {
                    // TODO: assertEquals(1, fire_count[i]);
                }
            }

            // TODO: @Test
            void test_unregister_in_handler() {
                // TODO: const auto job = Job();
                const int n = 100; // TODO: * stressTestMultiplier;
                std::vector<int> fire_count(n, 0);
                for (int i = 0; i < n; i++) {
                    // TODO: DisposableHandle* registration = nullptr;
                    // TODO: registration = job->invoke_on_completion([&, i] {
                    //     fire_count[i]++;
                    //     registration->dispose();
                    // });
                }
                // TODO: assertTrue(job->is_active());
                for (int i = 0; i < n; i++) {
                    // TODO: assertEquals(0, fire_count[i]);
                }
                // cancel once
                // TODO: job->cancel();
                // TODO: assertTrue(!job->is_active());
                for (int i = 0; i < n; i++) {
                    // TODO: assertEquals(1, fire_count[i]);
                }
                // cancel again
                // TODO: job->cancel();
                // TODO: assertTrue(!job->is_active());
                for (int i = 0; i < n; i++) {
                    // TODO: assertEquals(1, fire_count[i]);
                }
            }

            // TODO: @Test
            void test_many_handlers_with_unregister() {
                // TODO: const auto job = Job();
                const int n = 100; // TODO: * stressTestMultiplier;
                std::vector<int> fire_count(n, 0);
                // TODO: std::vector<DisposableHandle*> registrations(n);
                // TODO: for (int i = 0; i < n; i++) {
                //     registrations[i] = job->invoke_on_completion([&, i] { fire_count[i]++; });
                // }
                // TODO: assertTrue(job->is_active());
                auto unreg = [](int i) { return i % 4 <= 1; };
                for (int i = 0; i < n; i++) {
                    // TODO: if (unreg(i)) registrations[i]->dispose();
                }
                for (int i = 0; i < n; i++) {
                    // TODO: assertEquals(0, fire_count[i]);
                }
                // TODO: job->cancel();
                // TODO: assertTrue(!job->is_active());
                for (int i = 0; i < n; i++) {
                    // TODO: assertEquals(unreg(i) ? 0 : 1, fire_count[i]);
                }
            }

            // TODO: @Test
            void test_exceptions_in_handler() {
                // TODO: const auto job = Job();
                const int n = 100; // TODO: * stressTestMultiplier;
                std::vector<int> fire_count(n, 0);
                for (int i = 0; i < n; i++) {
                    // TODO: job->invoke_on_completion([&, i] {
                    //     fire_count[i]++;
                    //     throw TestException();
                    // });
                }
                // TODO: assertTrue(job->is_active());
                for (int i = 0; i < n; i++) {
                    // TODO: assertEquals(0, fire_count[i]);
                }
                // TODO: const auto cancel_result = run_catching([&] { job->cancel(); });
                // TODO: assertTrue(!job->is_active());
                for (int i = 0; i < n; i++) {
                    // TODO: assertEquals(1, fire_count[i]);
                }
                // TODO: assertIs<CompletionHandlerException>(cancel_result.exception_or_null());
                // TODO: assertIs<TestException>(cancel_result.exception_or_null()->cause);
            }

            // TODO: @Test
            void test_cancelled_parent() {
                // TODO: const auto parent = Job();
                // TODO: parent->cancel();
                // TODO: assertTrue(!parent->is_active());
                // TODO: const auto child = Job(parent);
                // TODO: assertTrue(!child->is_active());
            }

            // TODO: @Test
            void test_dispose_single_handler() {
                // TODO: const auto job = Job();
                int fire_count = 0;
                // TODO: const auto handler = job->invoke_on_completion([&] { fire_count++; });
                // TODO: handler->dispose();
                // TODO: job->cancel();
                // TODO: assertEquals(0, fire_count);
            }

            // TODO: @Test
            void test_dispose_multiple_handler() {
                // TODO: const auto job = Job();
                const int handler_count = 10;
                int fire_count = 0;
                // TODO: std::array<DisposableHandle*, handler_count> handlers;
                // TODO: for (int i = 0; i < handler_count; i++) {
                //     handlers[i] = job->invoke_on_completion([&] { fire_count++; });
                // }
                // TODO: for (auto h : handlers) h->dispose();
                // TODO: job->cancel();
                // TODO: assertEquals(0, fire_count);
            }

            // TODO: @Test
            void test_cancel_and_join_parent_wait_children() {
                // TODO: runTest {
                expect(1);
                // TODO: const auto parent = Job();
                // TODO: launch(parent, start = CoroutineStart.UNDISPATCHED) {
                expect(2);
                // TODO: try {
                //     yield(); // will get cancelled
                // } finally {
                expect(5);
                // }
                // }
                expect(3);
                // TODO: parent->cancel();
                expect(4);
                // TODO: parent->join();
                finish(6);
                // TODO: }
            }

            // TODO: @Test
            void test_on_cancelling_handler() {
                // TODO: runTest {
                // TODO: const auto job = launch {
                expect(2);
                // TODO: delay(Long.MAX_VALUE);
                // }

                // TODO: job->invoke_on_completion(on_cancelling = true) {
                //     assertNotNull(it);
                expect(3);
                // }

                expect(1);
                // TODO: yield();
                // TODO: job->cancel_and_join();
                finish(4);
                // TODO: }
            }

            // TODO: @Test
            void test_invoke_on_cancelling_firing_on_normal_exit() {
                // TODO: runTest {
                // TODO: const auto job = launch {
                expect(2);
                // }
                // TODO: job->invoke_on_completion(on_cancelling = true) {
                //     assertNull(it);
                expect(3);
                // }
                expect(1);
                // TODO: job->join();
                finish(4);
                // TODO: }
            }

            // TODO: @Test
            void test_overridden_parent() {
                // TODO: runTest {
                // TODO: const auto parent = Job();
                // TODO: const auto deferred = launch(parent, CoroutineStart.ATOMIC) {
                expect(2);
                // TODO: delay(Long.MAX_VALUE);
                // }

                // TODO: parent->cancel();
                expect(1);
                // TODO: deferred.join();
                finish(3);
                // TODO: }
            }

            // TODO: @Test
            void test_job_with_parent_cancel_normally() {
                // TODO: const auto parent = Job();
                // TODO: const auto job = Job(parent);
                // TODO: job->cancel();
                // TODO: assertTrue(job->is_cancelled());
                // TODO: assertFalse(parent->is_cancelled());
            }

            // TODO: @Test
            void test_job_with_parent_cancel_exception() {
                // TODO: const auto parent = Job();
                // TODO: const auto job = Job(parent);
                // TODO: job->complete_exceptionally(TestException());
                // TODO: assertTrue(job->is_cancelled());
                // TODO: assertTrue(parent->is_cancelled());
            }

            // TODO: @Test
            void test_incomplete_job_state() {
                // TODO: runTest {
                // TODO: const auto parent = coroutineContext.job;
                // TODO: const auto job = launch {
                //     coroutineContext[Job]!!.invoke_on_completion([]{});
                // }
                // TODO: assertSame(parent, job->parent());
                // TODO: job->join();
                // TODO: assertNull(job->parent());
                // TODO: assertTrue(job->is_completed());
                // TODO: assertFalse(job->is_active());
                // TODO: assertFalse(job->is_cancelled());
                // TODO: }
            }

            // TODO: @Test
            void test_children_with_incomplete_state() {
                // TODO: runTest {
                // TODO: const auto job = async { Wrapper() };
                // TODO: job->join();
                // TODO: assertTrue(job->children().to_list().empty());
                // TODO: }
            }

        private:
            class Wrapper {
                // TODO: : public Incomplete {
            public:
                // TODO: bool is_active() const override { error(""); }
                // TODO: NodeList* list() const override { error(""); }
            };
        };
    } // namespace coroutines
} // namespace kotlinx