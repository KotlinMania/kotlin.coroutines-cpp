// port-lint: source Debug.common.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Debug.common.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *
 * Upstream:
 *   internal expect val DEBUG: Boolean
 *   internal expect val Any.hexAddress: String
 *   internal expect val Any.classSimpleName: String
 *   internal expect inline fun assert(value: () -> Boolean)
 *   @ExperimentalCoroutinesApi
 *   public interface CopyableThrowable<T> where T : Throwable, T : CopyableThrowable<T> {
 *       fun createCopy(): T?
 *   }
 *
 * The actuals are provided per-platform in native/Debug.cpp. Here the common file
 * declares the forward signatures plus the CopyableThrowable interface; Kotlin's
 * `where T : Throwable, T : CopyableThrowable<T>` constraint has no exact C++ analogue,
 * so the template parameter is left unconstrained — concrete subclasses provide their
 * own derivation from std::exception (or a derived class) at the call site.
 */

#include <functional>
#include <string>

namespace kotlinx::coroutines {

extern const bool DEBUG;
extern std::string hex_address(const void *obj);
extern std::string class_simple_name(const void *obj);
extern void assert_predicate(std::function<bool()> value);

template <typename T>
class CopyableThrowable {
public:
    virtual ~CopyableThrowable() = default;

    /**
     * Creates a copy of the current instance.
     *
     * For better debuggability, use the original exception as `cause` of the resulting
     * one. An exception may opt out of copying by returning nullptr from this function.
     * Suppressed exceptions of the original exception should not be copied in order to
     * avoid circular exceptions.
     */
    virtual T *create_copy() = 0;
};

} // namespace kotlinx::coroutines
