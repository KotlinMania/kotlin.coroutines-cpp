// Transliterated from: kotlinx-coroutines-core/common/test/flow/sharing/ShareInConflationTest.kt

// TODO: #include equivalent for kotlinx.coroutines.testing.*
// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for kotlinx.coroutines.channels.*
// TODO: #include equivalent for kotlin.test.*

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            /**
 * Similar to [ShareInBufferTest] and [BufferConflationTest],
 * but tests [shareIn] and its fusion with [conflate] operator.
 */
            class ShareInConflationTest : public TestBase {
            private:
                static constexpr int N = 100;

                template<typename Op>
                void check_conflation(
                    int buffer_capacity,
                    BufferOverflow on_buffer_overflow,
                    Op op
                ) {
                    // TODO: implement coroutine suspension
                    run_test([&]() {
                        expect(1);
                        // emit all and conflate, then should collect buffer_capacity the latest ones
                        Job *done = new Job();
                        flow([&]() {
                                    for (int i = 0; i < N; ++i) {
                                        expect(i + 2);
                                        emit(i);
                                    }
                                    done->join(); // wait until done collection
                                    emit(-1); // signal flow completion
                                })
                                .apply(op, this)
                                .take_while([](int i) { return i >= 0; })
                                .collect([&](int i) {
                                    int first = (on_buffer_overflow == BufferOverflow::DROP_LATEST)
                                                    ? 0
                                                    : N - buffer_capacity;
                                    int last = first + buffer_capacity - 1;
                                    if (i >= first && i <= last) {
                                        expect(N + i - first + 2);
                                        if (i == last) done->complete(); // received the last one
                                    } else {
                                        throw std::runtime_error("Unexpected " + std::to_string(i));
                                    }
                                });
                        finish(N + buffer_capacity + 2);
                    });
                }

            public:
                // @Test
                void test_conflate_replay1() {
                    check_conflation(1, BufferOverflow::DROP_OLDEST, [](auto &flow, auto *scope) {
                        return flow.conflate().share_in(*scope, SharingStarted::eagerly(), 1);
                    });
                }

                // @Test // still looks like conflating the last value for the first subscriber (will not replay to others though)
                void test_conflate_replay0() {
                    check_conflation(1, BufferOverflow::DROP_OLDEST, [](auto &flow, auto *scope) {
                        return flow.conflate().share_in(*scope, SharingStarted::eagerly(), 0);
                    });
                }

                // @Test
                void test_conflate_replay5() {
                    check_conflation(5, BufferOverflow::DROP_OLDEST, [](auto &flow, auto *scope) {
                        return flow.conflate().share_in(*scope, SharingStarted::eagerly(), 5);
                    });
                }

                // @Test
                void test_buffer_drop_oldest_replay1() {
                    check_conflation(1, BufferOverflow::DROP_OLDEST, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::DROP_OLDEST)
                                .share_in(*scope, SharingStarted::eagerly(), 1);
                    });
                }

                // @Test
                void test_buffer_drop_oldest_replay0() {
                    check_conflation(1, BufferOverflow::DROP_OLDEST, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::DROP_OLDEST)
                                .share_in(*scope, SharingStarted::eagerly(), 0);
                    });
                }

                // @Test
                void test_buffer_drop_oldest_replay10() {
                    check_conflation(10, BufferOverflow::DROP_OLDEST, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::DROP_OLDEST)
                                .share_in(*scope, SharingStarted::eagerly(), 10);
                    });
                }

                // @Test
                void test_buffer20_drop_oldest_replay0() {
                    check_conflation(20, BufferOverflow::DROP_OLDEST, [](auto &flow, auto *scope) {
                        return flow.buffer(20, /*onBufferOverflow=*/BufferOverflow::DROP_OLDEST)
                                .share_in(*scope, SharingStarted::eagerly(), 0);
                    });
                }

                // @Test
                void test_buffer7_drop_oldest_replay11() {
                    check_conflation(18, BufferOverflow::DROP_OLDEST, [](auto &flow, auto *scope) {
                        return flow.buffer(7, /*onBufferOverflow=*/BufferOverflow::DROP_OLDEST)
                                .share_in(*scope, SharingStarted::eagerly(), 11);
                    });
                }

                // @Test // a preceding buffer() gets overridden by conflate()
                void test_buffer_conflate_override() {
                    check_conflation(1, BufferOverflow::DROP_OLDEST, [](auto &flow, auto *scope) {
                        return flow.buffer(23).conflate().share_in(*scope, SharingStarted::eagerly(), 1);
                    });
                }

                // @Test // a preceding buffer() gets overridden by buffer(onBufferOverflow = BufferOverflow.DROP_OLDEST)
                void test_buffer_drop_oldest_override() {
                    check_conflation(1, BufferOverflow::DROP_OLDEST, [](auto &flow, auto *scope) {
                        return flow.buffer(23)
                                .buffer(/*onBufferOverflow=*/BufferOverflow::DROP_OLDEST)
                                .share_in(*scope, SharingStarted::eagerly(), 1);
                    });
                }

                // @Test
                void test_buffer_drop_latest_replay0() {
                    check_conflation(1, BufferOverflow::DROP_LATEST, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::DROP_LATEST)
                                .share_in(*scope, SharingStarted::eagerly(), 0);
                    });
                }

                // @Test
                void test_buffer_drop_latest_replay1() {
                    check_conflation(1, BufferOverflow::DROP_LATEST, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::DROP_LATEST)
                                .share_in(*scope, SharingStarted::eagerly(), 1);
                    });
                }

                // @Test
                void test_buffer_drop_latest_replay10() {
                    check_conflation(10, BufferOverflow::DROP_LATEST, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::DROP_LATEST)
                                .share_in(*scope, SharingStarted::eagerly(), 10);
                    });
                }

                // @Test
                void test_buffer0_drop_latest_replay0() {
                    check_conflation(1, BufferOverflow::DROP_LATEST, [](auto &flow, auto *scope) {
                        return flow.buffer(0, /*onBufferOverflow=*/BufferOverflow::DROP_LATEST)
                                .share_in(*scope, SharingStarted::eagerly(), 0);
                    });
                }

                // @Test
                void test_buffer0_drop_latest_replay1() {
                    check_conflation(1, BufferOverflow::DROP_LATEST, [](auto &flow, auto *scope) {
                        return flow.buffer(0, /*onBufferOverflow=*/BufferOverflow::DROP_LATEST)
                                .share_in(*scope, SharingStarted::eagerly(), 1);
                    });
                }

                // @Test
                void test_buffer0_drop_latest_replay10() {
                    check_conflation(10, BufferOverflow::DROP_LATEST, [](auto &flow, auto *scope) {
                        return flow.buffer(0, /*onBufferOverflow=*/BufferOverflow::DROP_LATEST)
                                .share_in(*scope, SharingStarted::eagerly(), 10);
                    });
                }

                // @Test
                void test_buffer5_drop_latest_replay0() {
                    check_conflation(5, BufferOverflow::DROP_LATEST, [](auto &flow, auto *scope) {
                        return flow.buffer(5, /*onBufferOverflow=*/BufferOverflow::DROP_LATEST)
                                .share_in(*scope, SharingStarted::eagerly(), 0);
                    });
                }

                // @Test
                void test_buffer5_drop_latest_replay10() {
                    check_conflation(15, BufferOverflow::DROP_LATEST, [](auto &flow, auto *scope) {
                        return flow.buffer(5, /*onBufferOverflow=*/BufferOverflow::DROP_LATEST)
                                .share_in(*scope, SharingStarted::eagerly(), 10);
                    });
                }

                // @Test // a preceding buffer() gets overridden by buffer(onBufferOverflow = BufferOverflow.DROP_LATEST)
                void test_buffer_drop_latest_override() {
                    check_conflation(1, BufferOverflow::DROP_LATEST, [](auto &flow, auto *scope) {
                        return flow.buffer(23)
                                .buffer(/*onBufferOverflow=*/BufferOverflow::DROP_LATEST)
                                .share_in(*scope, SharingStarted::eagerly(), 0);
                    });
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement coroutine suspension
// 2. Implement BufferOverflow enumeration (DROP_LATEST, DROP_OLDEST)
// 3. Implement conflate, buffer, share_in, take_while, collect
// 4. Implement Job class with join, complete methods
// 5. Implement template lambda support for operators
// 6. Add proper includes for all dependencies
