// Original: ui/kotlinx-coroutines-android/android-unit-tests/src/EmptyCoroutineScopeImpl.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement CoroutineScope interface
// TODO: Map CoroutineContext to C++ equivalent
// TODO: Implement EmptyCoroutineContext

namespace kotlinx {
namespace coroutines {
namespace android {

// TODO: import kotlinx.coroutines.*
// TODO: import kotlin.coroutines.*

// Classes for testing service loader
class EmptyCoroutineScopeImpl1 : public CoroutineScope {
public:
    CoroutineContext coroutine_context() const override {
        return EmptyCoroutineContext;
    }
};

class EmptyCoroutineScopeImpl2 : public CoroutineScope {
public:
    CoroutineContext coroutine_context() const override {
        return EmptyCoroutineContext;
    }
};

class EmptyCoroutineScopeImpl3 : public CoroutineScope {
public:
    CoroutineContext coroutine_context() const override {
        return EmptyCoroutineContext;
    }
};

} // namespace android
} // namespace coroutines
} // namespace kotlinx
