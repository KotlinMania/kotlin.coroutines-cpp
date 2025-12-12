// Original: kotlinx-coroutines-core/concurrent/test/CommonThreadLocalTest.kt
// TODO: Remove or convert import statements
// TODO: Convert test annotations to C++ test framework
// TODO: Implement suspend functions and coroutines
// TODO: Handle TestBase inheritance
// TODO: Implement commonThreadLocal, Symbol
// TODO: Implement newSingleThreadContext, launch
// TODO: Implement nullable types with std::optional or pointers
// TODO: Implement test utilities (assertEquals, assertNull)

namespace kotlinx {
    namespace coroutines {
        // TODO: import kotlinx.coroutines.testing.*
        // TODO: import kotlinx.coroutines.exceptions.*
        // TODO: import kotlinx.coroutines.internal.*
        // TODO: import kotlin.test.*

        class CommonThreadLocalTest : public TestBase {
        public:
            /**
     * Tests the basic functionality of [commonThreadLocal]: storing a separate value for each thread.
     */
            // @Test
            // TODO: Convert test annotation
            void test_thread_local_being_thread_local() {
                runTest([&]() {
                    // TODO: suspend function
                    auto thread_local_var = common_thread_local<int>(Symbol("Test1"));
                    auto context = new_single_thread_context("");
                    context.use([&]() {
                        thread_local_var.set(10);
                        assertEquals(10, thread_local_var.get());
                        auto job1 = launch(context, [&]() {
                            // TODO: suspend function
                            thread_local_var.set(20);
                            assertEquals(20, thread_local_var.get());
                        });
                        assertEquals(10, thread_local_var.get());
                        job1.join();
                        auto job2 = launch(context, [&]() {
                            // TODO: suspend function
                            assertEquals(20, thread_local_var.get());
                        });
                        job2.join();
                    });
                });
            }

            /**
     * Tests using [commonThreadLocal] with a nullable type.
     */
            // @Test
            // TODO: Convert test annotation
            void test_thread_local_with_nullable_type() {
                runTest([&]() {
                    // TODO: suspend function
                    auto thread_local_var = common_thread_local<std::optional<int> >(Symbol("Test2"));
                    auto context = new_single_thread_context("");
                    context.use([&]() {
                        assertNull(thread_local_var.get());
                        thread_local_var.set(10);
                        assertEquals(10, thread_local_var.get());
                        auto job1 = launch(context, [&]() {
                            // TODO: suspend function
                            assertNull(thread_local_var.get());
                            thread_local_var.set(20);
                            assertEquals(20, thread_local_var.get());
                        });
                        assertEquals(10, thread_local_var.get());
                        job1.join();
                        thread_local_var.set(std::nullopt);
                        assertNull(thread_local_var.get());
                        auto job2 = launch(context, [&]() {
                            // TODO: suspend function
                            assertEquals(20, thread_local_var.get());
                            thread_local_var.set(std::nullopt);
                            assertNull(thread_local_var.get());
                        });
                        job2.join();
                    });
                });
            }

            /**
     * Tests that several instances of [commonThreadLocal] with different names don't affect each other.
     */
            // @Test
            // TODO: Convert test annotation
            void test_thread_locals_with_different_names_not_interfering() {
                auto value1 = common_thread_local<int>(Symbol("Test3a"));
                auto value2 = common_thread_local<int>(Symbol("Test3b"));
                value1.set(5);
                value2.set(6);
                assertEquals(5, value1.get());
                assertEquals(6, value2.get());
            }
        };
    } // namespace coroutines
} // namespace kotlinx