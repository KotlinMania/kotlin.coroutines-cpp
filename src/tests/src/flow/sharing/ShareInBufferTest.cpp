// Transliterated from: kotlinx-coroutines-core/common/test/flow/sharing/ShareInBufferTest.kt

// TODO: #include equivalent for kotlinx.coroutines.testing.*
// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for kotlin.math.*
// TODO: #include equivalent for kotlin.test.*

namespace kotlinx {
    namespace coroutines {
        namespace flow {
            /**
 * Similar to [BufferTest], but tests [shareIn] buffering and its fusion with [buffer] operators.
 */
            class ShareInBufferTest : public TestBase {
            private:
                static constexpr int kN = 200; // number of elements to emit for test
                static constexpr int kDefaultBufferSize = 64; // expected default buffer size (per docs)

                // Use capacity == -1 to check case of "no buffer"
                template<typename Op>
                void check_buffer(int capacity, Op op) {
                    // TODO: implement coroutine suspension
                    run_test([&]() {
                        expect(1);
                        /*
               Shared flows do not perform full rendezvous. On buffer overflow emitter always suspends until all
               subscribers get the value and then resumes. Thus, perceived batch size is +1 from buffer capacity.
             */
                        int batch_size = capacity + 1;
                        auto upstream = flow([&]() {
                            for (int i = 0; i < kN; ++i) {
                                int batch_no = i / batch_size;
                                int batch_idx = i % batch_size;
                                expect(batch_no * batch_size * 2 + batch_idx + 2);
                                emit(i);
                            }
                            emit(-1); // done
                        });
                        coroutine_scope([&]() {
                            op(upstream, this)
                                    .take_while([](int i) { return i >= 0; }) // until done
                                    .collect([&](int i) {
                                        int batch_no = i / batch_size;
                                        int batch_idx = i % batch_size;
                                        // last batch might have smaller size
                                        int k = std::min((batch_no + 1) * batch_size, kN) - batch_no * batch_size;
                                        expect(batch_no * batch_size * 2 + k + batch_idx + 2);
                                    });
                            coroutine_context().cancel_children(); // cancels sharing
                        });
                        finish(2 * kN + 2);
                    });
                }

            public:
                // @Test
                void test_replay0_default_buffer() {
                    check_buffer(kDefaultBufferSize, [](auto &flow, auto *scope) {
                        return flow.share_in(*scope, SharingStarted::kEagerly);
                    });
                }

                // @Test
                void test_replay1_default_buffer() {
                    check_buffer(kDefaultBufferSize, [](auto &flow, auto *scope) {
                        return flow.share_in(*scope, SharingStarted::kEagerly, 1);
                    });
                }

                // @Test // buffer is padded to default size as needed
                void test_replay10_default_buffer() {
                    check_buffer(std::max(10, kDefaultBufferSize), [](auto &flow, auto *scope) {
                        return flow.share_in(*scope, SharingStarted::kEagerly, 10);
                    });
                }

                // @Test // buffer is padded to default size as needed
                void test_replay100_default_buffer() {
                    check_buffer(std::max(100, kDefaultBufferSize), [](auto &flow, auto *scope) {
                        return flow.share_in(*scope, SharingStarted::kEagerly, 100);
                    });
                }

                // @Test
                void test_default_buffer_keeps_default() {
                    check_buffer(kDefaultBufferSize, [](auto &flow, auto *scope) {
                        return flow.buffer().share_in(*scope, SharingStarted::kEagerly);
                    });
                }

                // @Test
                void test_override_default_buffer0() {
                    check_buffer(0, [](auto &flow, auto *scope) {
                        return flow.buffer(0).share_in(*scope, SharingStarted::kEagerly);
                    });
                }

                // @Test
                void test_override_default_buffer10() {
                    check_buffer(10, [](auto &flow, auto *scope) {
                        return flow.buffer(10).share_in(*scope, SharingStarted::kEagerly);
                    });
                }

                // @Test // buffer and replay sizes add up
                void test_buffer_replay_sum() {
                    check_buffer(41, [](auto &flow, auto *scope) {
                        return flow.buffer(10).buffer(20).share_in(*scope, SharingStarted::kEagerly, 11);
                    });
                }
            };
        } // namespace flow
    } // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement coroutine suspension for all suspend functions
// 2. Implement share_in method
// 3. Implement SharingStarted enumeration with kEagerly
// 4. Implement flow, emit, buffer, take_while, collect
// 5. Implement coroutine_scope, coroutine_context
// 6. Implement cancel_children method
// 7. Implement template lambda support
// 8. Add proper includes for all dependencies