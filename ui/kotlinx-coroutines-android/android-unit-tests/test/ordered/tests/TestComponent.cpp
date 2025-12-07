// Original: ui/kotlinx-coroutines-android/android-unit-tests/test/ordered/tests/TestComponent.kt
// Transliterated from Kotlin to C++ - First pass syntax conversion
// TODO: Implement CoroutineScope interface
// TODO: Convert lateinit to C++ equivalent (pointer or optional)
// TODO: Implement SupervisorJob, Dispatchers.Main, CoroutineExceptionHandler
// TODO: Convert suspend functions to C++ coroutine equivalents
// TODO: Convert Long.MAX_VALUE to C++ equivalent

namespace ordered {
namespace tests {

// TODO: import kotlinx.coroutines.*

class TestComponent {
public:
    // TODO: Convert lateinit var
    std::exception_ptr caught_exception;

private:
    CoroutineScope scope_ =
        CoroutineScope(SupervisorJob() + Dispatchers.Main + CoroutineExceptionHandler([this](auto, auto e) {
            caught_exception = std::make_exception_ptr(e);
        }));

public:
    bool launch_completed = false;
    bool delayed_launch_completed = false;

    void launch_something() {
        scope_.launch([this] {
            launch_completed = true;
        });
    }

    void launch_delayed() {
        scope_.launch([this] {
            delay(LONG_MAX / 2);
            delayed_launch_completed = true;
        });
    }
};

} // namespace tests
} // namespace ordered
