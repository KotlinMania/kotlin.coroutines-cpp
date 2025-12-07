// Original file: kotlinx-coroutines-core/common/test/flow/operators/FlowContextOptimizationsTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle coroutine context operations
// TODO: translate 'import kotlin.coroutines.coroutineContext as currentContext'

namespace kotlinx {
namespace coroutines {
namespace flow {

class FlowContextOptimizationsTest : public TestBase {
public:
    // @Test
    void test_baseline() /* TODO: = runTest */ {
        auto flow_dispatcher = wrapper_dispatcher(current_context());
        auto collect_context = current_context();
        flow([]() /* TODO: suspend */ {
            assert_same(flow_dispatcher, current_context()[ContinuationInterceptor::key]);
            expect(1);
            emit(1);
            expect(2);
            emit(2);
            expect(3);
        })
            .flow_on(flow_dispatcher)
            .collect([&](int value) {
                assert_equals(collect_context.minus_key(Job::key), current_context().minus_key(Job::key));
                if (value == 1) expect(4);
                else expect(5);
            });

        finish(6);
    }

    // @Test
    void test_fused_same_context() /* TODO: = runTest */ {
        flow([]() /* TODO: suspend */ {
            expect(1);
            emit(1);
            expect(3);
            emit(2);
            expect(5);
        })
            .flow_on(current_context().minus_key(Job::key))
            .collect([](int value) {
                if (value == 1) expect(2);
                else expect(4);
            });
        finish(6);
    }

    // @Test
    void test_fused_same_context_with_intermediate_operators() /* TODO: = runTest */ {
        flow([]() /* TODO: suspend */ {
            expect(1);
            emit(1);
            expect(3);
            emit(2);
            expect(5);
        })
            .flow_on(current_context().minus_key(Job::key))
            .map([](int it) { return it; })
            .flow_on(current_context().minus_key(Job::key))
            .collect([](int value) {
                if (value == 1) expect(2);
                else expect(4);
            });
        finish(6);
    }

    // @Test
    void test_fused_same_dispatcher() /* TODO: = runTest */ {
        flow([]() /* TODO: suspend */ {
            assert_equals("Name", current_context()[CoroutineName::key]->name);
            expect(1);
            emit(1);
            expect(3);
            emit(2);
            expect(5);
        })
            .flow_on(CoroutineName("Name"))
            .collect([](int value) {
                assert_null(current_context()[CoroutineName::key]->name);
                if (value == 1) expect(2);
                else expect(4);
            });
        finish(6);
    }

    // @Test
    void test_fused_many_same_dispatcher() /* TODO: = runTest */ {
        flow([]() /* TODO: suspend */ {
            assert_equals("Name1", current_context()[CoroutineName::key]->name);
            assert_equals("OK", current_context()[CustomContextElement::key]->str);
            expect(1);
            emit(1);
            expect(3);
            emit(2);
            expect(5);
        })
            .flow_on(CoroutineName("Name1")) // the first one works
            .flow_on(CoroutineName("Name2"))
            .flow_on(CoroutineName("Name3") + CustomContextElement("OK")) // but this is not lost
            .collect([](int value) {
                assert_null(current_context()[CoroutineName::key]->name);
                assert_null(current_context()[CustomContextElement::key]->str);
                if (value == 1) expect(2);
                else expect(4);
            });
        finish(6);
    }

    // data class CustomContextElement
    struct CustomContextElement : public AbstractCoroutineContextElement {
        std::string str;

        CustomContextElement(const std::string& str_) :
            AbstractCoroutineContextElement(Key), str(str_) {}

        // companion object Key
        static const CoroutineContext::Key<CustomContextElement> Key;
    };
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
