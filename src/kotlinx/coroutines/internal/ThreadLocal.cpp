/**
 * Transliterated from: kotlinx-coroutines-core/native/src/internal/ThreadLocal.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Native-side actual for `CommonThreadLocal<T>`. The C++ port uses C++11
 * `thread_local` storage keyed by Symbol, so each thread sees its own slot per
 * (Symbol, T) pair. The forward-declared abstract `CommonThreadLocal<T>` in the
 * matching common file (internal/ThreadLocal.common.cpp) is realised here.
 */

#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/ThreadLocal.common.cpp"

#include <unordered_map>

namespace kotlinx::coroutines::internal {

template <typename T>
class NativeThreadLocal : public CommonThreadLocal<T> {
public:
    explicit NativeThreadLocal(Symbol* name) : name_(name) {}

    T get() override { return storage()[name_]; }
    void set(T value) override { storage()[name_] = std::move(value); }

private:
    Symbol* name_;
    static std::unordered_map<Symbol*, T>& storage() {
        thread_local std::unordered_map<Symbol*, T> slot;
        return slot;
    }
};

template <typename T>
CommonThreadLocal<T>* common_thread_local(Symbol* name) {
    return new NativeThreadLocal<T>(name);
}

} // namespace kotlinx::coroutines::internal
