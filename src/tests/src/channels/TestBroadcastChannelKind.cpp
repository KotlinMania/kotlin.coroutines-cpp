// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/test/channels/TestBroadcastChannelKind.kt
//
// TODO: Translate enums to C++ enum class
// TODO: Translate @Suppress annotations

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // TODO: @Suppress("DEPRECATION_ERROR")
            enum class TestBroadcastChannelKind {
                ARRAY1,
                ARRAY10,
                CONFLATED
            };

            // TODO: Template specialization or visitor pattern for create() and toString()
            // TODO: class TestBroadcastChannelKindHelper {
            // public:
            //     template<typename T>
            //     static BroadcastChannel<T>* create(TestBroadcastChannelKind kind) {
            //         switch (kind) {
            //             case TestBroadcastChannelKind::ARRAY1:
            //                 return new BroadcastChannel<T>(1);
            //             case TestBroadcastChannelKind::ARRAY10:
            //                 return new BroadcastChannel<T>(10);
            //             case TestBroadcastChannelKind::CONFLATED:
            //                 return new ConflatedBroadcastChannel<T>();
            //         }
            //     }
            //
            //     static const char* toString(TestBroadcastChannelKind kind) {
            //         switch (kind) {
            //             case TestBroadcastChannelKind::ARRAY1:
            //                 return "BufferedBroadcastChannel(1)";
            //             case TestBroadcastChannelKind::ARRAY10:
            //                 return "BufferedBroadcastChannel(10)";
            //             case TestBroadcastChannelKind::CONFLATED:
            //                 return "ConflatedBroadcastChannel";
            //         }
            //     }
            //
            //     static bool isConflated(TestBroadcastChannelKind kind) {
            //         return kind == TestBroadcastChannelKind::CONFLATED;
            //     }
            // };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx
