// Original file: kotlinx-coroutines-core/common/test/flow/operators/TransformTest.kt
// TODO: handle imports (kotlinx.coroutines.testing, kotlinx.coroutines, kotlin.test)
// TODO: translate @Test annotations to appropriate C++ test framework
// TODO: handle suspend functions and coroutines
// TODO: translate runTest {} blocks
// TODO: handle Flow types and operations

namespace kotlinx {
namespace coroutines {
namespace flow {

class TransformTest : public TestBase {
public:
    // @Test
    void test_double_emit() /* TODO: = runTest */ {
        auto flow = flow_of(1, 2, 3)
            .transform([](int it) /* TODO: suspend */ {
                emit(it);
                emit(it);
            });
        assert_equals(std::vector<int>{1, 1, 2, 2, 3, 3}, flow.to_list());
    }
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
