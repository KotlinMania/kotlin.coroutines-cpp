// Original file: kotlinx-coroutines-core/common/test/flow/operators/TransformWhileTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations
// TODO: handle data classes

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            class TransformWhileTest : public TestBase {
            public:
                // @Test
                void test_simple() /* TODO: = runTest */ {
                    auto flow = range(0, 10).as_flow();
                    auto expected = std::vector<std::string>{"A", "B", "C", "D"};
                    auto actual = flow.transform_while([](int value) /* TODO: suspend */ -> bool {
                        switch (value) {
                            case 0: emit("A");
                                return true;
                            case 1: return true;
                            case 2: emit("B");
                                emit("C");
                                return true;
                            case 3: emit("D");
                                return false;
                            default: expect_unreached();
                                return false;
                        }
                    }).to_list();
                    assert_equals(expected, actual);
                }

                // @Test
                void test_cancel_upstream() /* TODO: = runTest */ {
                    bool cancelled = false;
                    auto flow = flow([&cancelled]() /* TODO: suspend */ {
                        coroutine_scope([&cancelled]() /* TODO: suspend */ {
                            launch(CoroutineStart::ATOMIC, [&cancelled]() /* TODO: suspend */ {
                                hang([&cancelled]() { cancelled = true; });
                            });
                            emit(1);
                            emit(2);
                            emit(3);
                        });
                    });
                    auto transformed = flow.transform_while([](int it) /* TODO: suspend */ -> bool {
                        emit(it);
                        return it < 2;
                    });
                    assert_equals(std::vector<int>{1, 2}, transformed.to_list());
                    assert_true(cancelled);
                }

                // @Test
                void test_example() /* TODO: = runTest */ {
                    std::vector<DownloadProgress> source{
                        DownloadProgress{0},
                        DownloadProgress{50},
                        DownloadProgress{100},
                        DownloadProgress{147}
                    };
                    auto expected = std::vector<DownloadProgress>(source.begin(), source.begin() + 3);
                    auto actual = source.as_flow().complete_when_done().to_list();
                    assert_equals(expected, actual);
                }

            private:
                // data class DownloadProgress
                struct DownloadProgress {
                    int percent;

                    bool is_done() const { return percent >= 100; }

                    bool operator==(const DownloadProgress &other) const {
                        return percent == other.percent;
                    }
                };

                template<typename Flow>
                auto complete_when_done(Flow flow) {
                    return flow.transform_while([](const DownloadProgress &progress) /* TODO: suspend */ -> bool {
                        emit(progress); // always emit progress
                        return !progress.is_done(); // continue while download is not done
                    });
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx