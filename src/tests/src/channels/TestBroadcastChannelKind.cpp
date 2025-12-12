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
                kArray1,
                kArray10,
                kConflated
            };

            // TODO: Template specialization or visitor pattern for create() and toString()
            // TODO: class TestBroadcastChannelKindHelper {
            // public:
            //     template<typename T>
            //     static BroadcastChannel<T>* create(TestBroadcastChannelKind kind) {
            //         switch (kind) {
            //             case TestBroadcastChannelKind::kArray1:
            //                 return new BroadcastChannel<T>(1);
            //             case TestBroadcastChannelKind::kArray10:
            //                 return new BroadcastChannel<T>(10);
            //             case TestBroadcastChannelKind::kConflated:
            //                 return new ConflatedBroadcastChannel<T>();
            //         }
            //     }
            //
            //     static const char* toString(TestBroadcastChannelKind kind) {
            //         switch (kind) {
            //             case TestBroadcastChannelKind::kArray1:
            //                 return "BufferedBroadcastChannel(1)";
            //             case TestBroadcastChannelKind::kArray10:
            //                 return "BufferedBroadcastChannel(10)";
            //             case TestBroadcastChannelKind::kConflated:
            //                 return "ConflatedBroadcastChannel";
            //         }
            //     }
            //
            //     static bool isConflated(TestBroadcastChannelKind kind) {
            //         return kind == TestBroadcastChannelKind::kConflated;
            //     }
            // };
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx