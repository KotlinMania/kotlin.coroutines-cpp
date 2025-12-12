// Original: kotlinx-coroutines-core/common/test/flow/internal/FlowScopeTest.kt
// TODO: Translate imports to proper C++ includes
// TODO: Implement TestBase base class
// TODO: Implement @Test annotation equivalent
// TODO: Implement runTest coroutine runner
// TODO: Implement flowScope function
// TODO: Implement launch, cancel, cancelAndJoin
// TODO: Implement yield, hang
// TODO: Implement assertFailsWith
// TODO: Implement CancellationException, ChildCancelledException

// TODO: #include proper headers

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            namespace internal {
                class FlowScopeTest : public TestBase {
                public:
                    // @Test
                    void test_cancellation() {
                        run_test([]() -> /* suspend */ void {
                            assert_fails_with<CancellationException>([&]() -> /* suspend */ void {
                                flow_scope([](auto &scope) -> /* suspend */ void {
                                    expect(1);
                                    auto child = scope.launch([](auto &) -> /* suspend */ void {
                                        expect(3);
                                        hang([]() { expect(5); });
                                    });
                                    expect(2);
                                    yield();
                                    expect(4);
                                    child.cancel();
                                });
                            });
                            finish(6);
                        });
                    }

                    // @Test
                    void test_cancellation_with_child_cancelled() {
                        run_test([]() -> /* suspend */ void {
                            flow_scope([](auto &scope) -> /* suspend */ void {
                                expect(1);
                                auto child = scope.launch([](auto &) -> /* suspend */ void {
                                    expect(3);
                                    hang([]() { expect(5); });
                                });
                                expect(2);
                                yield();
                                expect(4);
                                child.cancel(ChildCancelledException());
                            });
                            finish(6);
                        });
                    }

                    // @Test
                    void test_cancellation_with_suspension_point() {
                        run_test([]() -> /* suspend */ void {
                            assert_fails_with<CancellationException>([&]() -> /* suspend */ void {
                                flow_scope([](auto &scope) -> /* suspend */ void {
                                    expect(1);
                                    auto child = scope.launch([](auto &) -> /* suspend */ void {
                                        expect(3);
                                        hang([]() { expect(6); });
                                    });
                                    expect(2);
                                    yield();
                                    expect(4);
                                    child.cancel();
                                    hang([]() { expect(5); });
                                });
                            });
                            finish(7);
                        });
                    }

                    // @Test
                    void test_nested_scopes() {
                        run_test([]() -> /* suspend */ void {
                            assert_fails_with<CancellationException>([&]() -> /* suspend */ void {
                                flow_scope([](auto &outer_scope) -> /* suspend */ void {
                                    flow_scope([&](auto &inner_scope) -> /* suspend */ void {
                                        inner_scope.launch([](auto &) -> /* suspend */ void {
                                            throw CancellationException("");
                                        });
                                    });
                                });
                            });
                        });
                    }
                };
            } // namespace internal
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx