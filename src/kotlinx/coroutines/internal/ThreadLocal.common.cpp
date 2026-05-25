// port-lint: source internal/ThreadLocal.common.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/ThreadLocal.common.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Upstream:
 *   internal expect class CommonThreadLocal<T> { fun get(): T; fun set(value: T) }
 *   internal expect fun <T> commonThreadLocal(name: Symbol): CommonThreadLocal<T>
 *
 * The C++ port resolves `CommonThreadLocal` to an interface so each platform target can
 * supply its own storage. Native uses `kotlin.native.concurrent.ThreadLocal`-equivalent
 * (`thread_local`); JVM uses `java.lang.ThreadLocal`; JS uses a process-wide map. See the
 * matching per-platform files for the concrete actuals.
 */

namespace kotlinx::coroutines::internal {

class Symbol;

/**
 * Upstream:
 *   internal expect class CommonThreadLocal<T> { fun get(): T; fun set(value: T) }
 */
template <typename T>
class CommonThreadLocal {
public:
    virtual ~CommonThreadLocal() = default;
    virtual T get() = 0;
    virtual void set(T value) = 0;
};

/**
 * Upstream:
 *   internal expect fun <T> commonThreadLocal(name: Symbol): CommonThreadLocal<T>
 *
 * Two callers with the same name are *not* guaranteed to observe the same value, but may.
 * Use a unique name per thread-local instance. Per-platform actual lives alongside the
 * target source set.
 */
template <typename T>
CommonThreadLocal<T>* common_thread_local(Symbol* name);

} // namespace kotlinx::coroutines::internal
