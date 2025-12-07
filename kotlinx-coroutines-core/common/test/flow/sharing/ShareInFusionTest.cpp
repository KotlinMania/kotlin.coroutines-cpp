// Transliterated from: kotlinx-coroutines-core/common/test/flow/sharing/ShareInFusionTest.kt

// TODO: #include equivalent for kotlinx.coroutines.testing.*
// TODO: #include equivalent for kotlinx.coroutines.*
// TODO: #include equivalent for kotlinx.coroutines.channels.*
// TODO: #include equivalent for kotlin.test.*

namespace kotlinx {
namespace coroutines {
namespace flow {

class ShareInFusionTest : public TestBase {
public:
    /**
     * Test perfect fusion for operators **after** [shareIn].
     */
    // @Test
    void test_operator_fusion() {
        // TODO: implement coroutine suspension
        run_test([this]() {
            auto sh = empty_flow<int>().share_in(*this, SharingStarted::kEagerly);
            assert_true(dynamic_cast<MutableSharedFlow<int>*>(&sh) == nullptr); // cannot be cast to mutable shared flow!!!
            assert_same(&sh, &(static_cast<Flow<int>&>(sh).cancellable()));
            assert_same(&sh, &(static_cast<Flow<int>&>(sh).flow_on(Dispatchers::kDefault)));
            assert_same(&sh, &sh.buffer(Channel::kRendezvous));
            coroutine_context().cancel_children();
        });
    }

    // @Test
    void test_flow_on_context_fusion() {
        // TODO: implement coroutine suspension
        run_test([this]() {
            auto flow_obj = flow([this]() {
                assert_equals("FlowCtx", current_coroutine_context()[CoroutineName]->name);
                emit("OK");
            }).flow_on(CoroutineName("FlowCtx"));
            assert_equals("OK", flow_obj.share_in(*this, SharingStarted::kEagerly, 1).first());
            coroutine_context().cancel_children();
        });
    }

    /**
     * Tests that `channelFlow { ... }.buffer(x)` works according to the [channelFlow] docs, and subsequent
     * application of [shareIn] does not leak upstream.
     */
    // @Test
    void test_channel_flow_buffer_share_in() {
        // TODO: implement coroutine suspension
        run_test([this]() {
            expect(1);
            auto flow_obj = channel_flow([this]() {
                // send a batch of 10 elements using [offer]
                for (int i = 1; i <= 10; ++i) {
                    assert_true(try_send(i).is_success); // offer must succeed, because buffer
                }
                send(0); // done
            }).buffer(10); // request a buffer of 10
            // ^^^^^^^^^ buffer stays here
            auto shared = flow_obj.share_in(*this, SharingStarted::kEagerly);
            shared
                .take_while([](int it) { return it > 0; })
                .collect([this](int i) { expect(i + 1); });
            finish(12);
        });
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement coroutine suspension
// 2. Implement share_in, empty_flow, channel_flow methods
// 3. Implement SharingStarted enumeration
// 4. Implement Flow operators (cancellable, flow_on, buffer, take_while, collect)
// 5. Implement Dispatchers and Channel constants
// 6. Implement current_coroutine_context, CoroutineName
// 7. Implement try_send with is_success property
// 8. Add proper includes for all dependencies
