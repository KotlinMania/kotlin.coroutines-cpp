// Original file: kotlinx-coroutines-core/common/test/flow/operators/FlatMapMergeBaseTest.kt
//
// TODO: Mechanical C++ transliteration - Requires comprehensive updates:
// - Import test framework headers
// - Extend FlatMapBaseTest
// - Map abstract testFlatMapConcurrency as pure virtual
// - Handle TestResult return type

namespace kotlinx {
namespace coroutines {
namespace flow {

// TODO: import kotlinx.coroutines.channels.*
// TODO: import kotlinx.coroutines.testing.*
// TODO: import kotlin.test.*
// TODO: import kotlin.test.assertFailsWith

class FlatMapMergeBaseTest : public FlatMapBaseTest {
public:
    // TODO: @Test
    void testFailureCancellation() {
        // TODO: runTest {
        auto flow_var = flow([](auto& emit) {
            expect(2);
            emit(1);
            expect(3);
            emit(2);
            expect(4);
        }).flat_map([](auto it) {
            if (it == 1) {
                return flow([](auto& emit) {
                    hang([&]() { expect(6); });
                });
            } else {
                return flow<int>([](auto& emit) {
                    expect(5);
                    throw TestException();
                });
            }
        });

        expect(1);
        assertFailsWith<TestException>([&]() { flow_var.single_or_null(); });
        finish(7);
        // TODO: }
    }

    // TODO: @Test
    void testConcurrentFailure() {
        // TODO: runTest {
        auto latch = Channel<Unit>();
        auto flow_var = flow([](auto& emit) {
            expect(2);
            emit(1);
            expect(3);
            emit(2);
        }).flat_map([&](auto it) {
            if (it == 1) {
                return flow<int>([&](auto& emit) {
                    expect(5);
                    latch.send(Unit{});
                    hang([&]() {
                        expect(7);
                        throw TestException2();

                    });
                });
            } else {
                expect(4);
                latch.receive();
                expect(6);
                throw TestException();
            }
        });

        expect(1);
        assertFailsWith<TestException>(flow_var);
        finish(8);
        // TODO: }
    }

    // TODO: @Test
    void testFailureInMapOperationCancellation() {
        // TODO: runTest {
        auto latch = Channel<Unit>();
        auto flow_var = flow([](auto& emit) {
            expect(2);
            emit(1);
            expect(3);
            emit(2);
            expectUnreached();
        }).flat_map([&](auto it) {
            if (it == 1) {
                return flow([&](auto& emit) {
                    expect(5);
                    latch.send(Unit{});
                    hang([&]() { expect(7); });
                });
            } else {
                expect(4);
                latch.receive();
                expect(6);
                throw TestException();
            }
        });

        expect(1);
        assertFailsWith<TestException>([&]() { flow_var.count(); });
        finish(8);
        // TODO: }
    }

    // TODO: @Test
    virtual void testFlatMapConcurrency() = 0; // TODO: TestResult
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
