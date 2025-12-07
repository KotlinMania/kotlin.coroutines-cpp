// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/internal/ThreadLocal.kt
//
// TODO: actual keyword - platform-specific implementation
// TODO: kotlin.native.concurrent.ThreadLocal annotation
// TODO: @Suppress annotation
// TODO: Symbol type (likely a unique identifier)

namespace kotlinx {
namespace coroutines {
namespace internal {

// TODO: Remove imports, fully qualify or add includes:
// import kotlin.native.concurrent.ThreadLocal

// TODO: internal actual class
template<typename T>
class CommonThreadLocal {
private:
    void* name; // TODO: Symbol type

public:
    CommonThreadLocal(void* name) : name(name) {}

    // TODO: @Suppress("UNCHECKED_CAST")
    T get() {
        return static_cast<T>(Storage::instance()[name]);
    }

    void set(T value) {
        Storage::instance()[name] = static_cast<void*>(value);
    }
};

// TODO: internal actual function
template<typename T>
CommonThreadLocal<T>* common_thread_local(void* name) {
    return new CommonThreadLocal<T>(name);
}

// TODO: @ThreadLocal private object implementing MutableMap
class Storage {
private:
    std::unordered_map<void*, void*> storage;

    Storage() = default;

public:
    static Storage& instance() {
        // TODO: thread_local for proper ThreadLocal behavior
        static Storage instance;
        return instance;
    }

    void*& operator[](void* key) {
        return storage[key];
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
