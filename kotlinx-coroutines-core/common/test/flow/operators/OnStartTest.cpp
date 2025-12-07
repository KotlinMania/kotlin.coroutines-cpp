// Original file: kotlinx-coroutines-core/common/test/flow/operators/OnStartTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations

namespace kotlinx {
namespace coroutines {
namespace flow {

class OnStartTest : public TestBase {
public:
    // @Test
    void test_emit_example() /* TODO: = runTest */ {
        auto flow = flow_of("a", "b", "c")
            .on_start([]() /* TODO: suspend */ { emit("Begin"); });
        assert_equals(std::vector<std::string>{"Begin", "a", "b", "c"}, flow.to_list());
    }

    // @Test
    void test_transparency_violation() /* TODO: = runTest */ {
        auto flow = empty_flow<int>().on_start([]() /* TODO: suspend */ {
            expect(2);
            coroutine_scope([&]() /* TODO: suspend */ {
                launch([&]() /* TODO: suspend */ {
                    try {
                        emit(1);
                    } catch (const IllegalStateException& e) {
                        expect(3);
                    }
                });
            });
        });
        expect(1);
        assert_null(flow.single_or_null());
        finish(4);
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
