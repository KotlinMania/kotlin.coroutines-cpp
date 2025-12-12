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
                static constexpr int kN = 100;

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
                                    for (int i = 0; i < kN; ++i) {
                                        expect(i + 2);
                                        emit(i);
                                    }
                                    done->join(); // wait until done collection
                                    emit(-1); // signal flow completion
                                })
                                .apply(op, this)
                                .take_while([](int i) { return i >= 0; })
                                .collect([&](int i) {
                                    int first = (on_buffer_overflow == BufferOverflow::kDropLatest)
                                                    ? 0
                                                    : kN - buffer_capacity;
                                    int last = first + buffer_capacity - 1;
                                    if (i >= first && i <= last) {
                                        expect(kN + i - first + 2);
                                        if (i == last) done->complete(); // received the last one
                                    } else {
                                        throw std::runtime_error("Unexpected " + std::to_string(i));
                                    }
                                });
                        finish(kN + buffer_capacity + 2);
                    });
                }

            public:
                // @Test
                void test_conflate_replay1() {
                    check_conflation(1, BufferOverflow::kDropOldest, [](auto &flow, auto *scope) {
                        return flow.conflate().share_in(*scope, SharingStarted::kEagerly, 1);
                    });
                }

                // @Test // still looks like conflating the last value for the first subscriber (will not replay to others though)
                void test_conflate_replay0() {
                    check_conflation(1, BufferOverflow::kDropOldest, [](auto &flow, auto *scope) {
                        return flow.conflate().share_in(*scope, SharingStarted::kEagerly, 0);
                    });
                }

                // @Test
                void test_conflate_replay5() {
                    check_conflation(5, BufferOverflow::kDropOldest, [](auto &flow, auto *scope) {
                        return flow.conflate().share_in(*scope, SharingStarted::kEagerly, 5);
                    });
                }

                // @Test
                void test_buffer_drop_oldest_replay1() {
                    check_conflation(1, BufferOverflow::kDropOldest, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::kDropOldest)
                                .share_in(*scope, SharingStarted::kEagerly, 1);
                    });
                }

                // @Test
                void test_buffer_drop_oldest_replay0() {
                    check_conflation(1, BufferOverflow::kDropOldest, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::kDropOldest)
                                .share_in(*scope, SharingStarted::kEagerly, 0);
                    });
                }

                // @Test
                void test_buffer_drop_oldest_replay10() {
                    check_conflation(10, BufferOverflow::kDropOldest, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::kDropOldest)
                                .share_in(*scope, SharingStarted::kEagerly, 10);
                    });
                }

                // @Test
                void test_buffer20_drop_oldest_replay0() {
                    check_conflation(20, BufferOverflow::kDropOldest, [](auto &flow, auto *scope) {
                        return flow.buffer(20, /*onBufferOverflow=*/BufferOverflow::kDropOldest)
                                .share_in(*scope, SharingStarted::kEagerly, 0);
                    });
                }

                // @Test
                void test_buffer7_drop_oldest_replay11() {
                    check_conflation(18, BufferOverflow::kDropOldest, [](auto &flow, auto *scope) {
                        return flow.buffer(7, /*onBufferOverflow=*/BufferOverflow::kDropOldest)
                                .share_in(*scope, SharingStarted::kEagerly, 11);
                    });
                }

                // @Test // a preceding buffer() gets overridden by conflate()
                void test_buffer_conflate_override() {
                    check_conflation(1, BufferOverflow::kDropOldest, [](auto &flow, auto *scope) {
                        return flow.buffer(23).conflate().share_in(*scope, SharingStarted::kEagerly, 1);
                    });
                }

                // @Test // a preceding buffer() gets overridden by buffer(onBufferOverflow = BufferOverflow.DROP_OLDEST)
                void test_buffer_drop_oldest_override() {
                    check_conflation(1, BufferOverflow::kDropOldest, [](auto &flow, auto *scope) {
                        return flow.buffer(23)
                                .buffer(/*onBufferOverflow=*/BufferOverflow::kDropOldest)
                                .share_in(*scope, SharingStarted::kEagerly, 1);
                    });
                }

                // @Test
                void test_buffer_drop_latest_replay0() {
                    check_conflation(1, BufferOverflow::kDropLatest, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::kDropLatest)
                                .share_in(*scope, SharingStarted::kEagerly, 0);
                    });
                }

                // @Test
                void test_buffer_drop_latest_replay1() {
                    check_conflation(1, BufferOverflow::kDropLatest, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::kDropLatest)
                                .share_in(*scope, SharingStarted::kEagerly, 1);
                    });
                }

                // @Test
                void test_buffer_drop_latest_replay10() {
                    check_conflation(10, BufferOverflow::kDropLatest, [](auto &flow, auto *scope) {
                        return flow.buffer(/*onBufferOverflow=*/BufferOverflow::kDropLatest)
                                .share_in(*scope, SharingStarted::kEagerly, 10);
                    });
                }

                // @Test
                void test_buffer0_drop_latest_replay0() {
                    check_conflation(1, BufferOverflow::kDropLatest, [](auto &flow, auto *scope) {
                        return flow.buffer(0, /*onBufferOverflow=*/BufferOverflow::kDropLatest)
                                .share_in(*scope, SharingStarted::kEagerly, 0);
                    });
                }

                // @Test
                void test_buffer0_drop_latest_replay1() {
                    check_conflation(1, BufferOverflow::kDropLatest, [](auto &flow, auto *scope) {
                        return flow.buffer(0, /*onBufferOverflow=*/BufferOverflow::kDropLatest)
                                .share_in(*scope, SharingStarted::kEagerly, 1);
                    });
                }

                // @Test
                void test_buffer0_drop_latest_replay10() {
                    check_conflation(10, BufferOverflow::kDropLatest, [](auto &flow, auto *scope) {
                        return flow.buffer(0, /*onBufferOverflow=*/BufferOverflow::kDropLatest)
                                .share_in(*scope, SharingStarted::kEagerly, 10);
                    });
                }

                // @Test
                void test_buffer5_drop_latest_replay0() {
                    check_conflation(5, BufferOverflow::kDropLatest, [](auto &flow, auto *scope) {
                        return flow.buffer(5, /*onBufferOverflow=*/BufferOverflow::kDropLatest)
                                .share_in(*scope, SharingStarted::kEagerly, 0);
                    });
                }

                // @Test
                void test_buffer5_drop_latest_replay10() {
                    check_conflation(15, BufferOverflow::kDropLatest, [](auto &flow, auto *scope) {
                        return flow.buffer(5, /*onBufferOverflow=*/BufferOverflow::kDropLatest)
                                .share_in(*scope, SharingStarted::kEagerly, 10);
                    });
                }

                // @Test // a preceding buffer() gets overridden by buffer(onBufferOverflow = BufferOverflow.DROP_LATEST)
                void test_buffer_drop_latest_override() {
                    check_conflation(1, BufferOverflow::kDropLatest, [](auto &flow, auto *scope) {
                        return flow.buffer(23)
                                .buffer(/*onBufferOverflow=*/BufferOverflow::kDropLatest)
                                .share_in(*scope, SharingStarted::kEagerly, 0);
                    });
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement coroutine suspension
// 2. Implement BufferOverflow enumeration (kDropLatest, kDropOldest)
// 3. Implement conflate, buffer, share_in, take_while, collect
// 4. Implement Job class with join, complete methods
// 5. Implement template lambda support for operators
// 6. Add proper includes for all dependencies