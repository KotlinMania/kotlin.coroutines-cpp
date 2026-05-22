/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Runnable.common.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines
 *
 * Upstream:
 *   public expect fun interface Runnable { fun run() }
 *
 * On the JVM target this maps to `java.lang.Runnable` (a typealias); on K/N and the C++
 * port it is a plain virtual interface. See native/Runnable.cpp for the matching actual
 * declaration and the templated `make_runnable` factory.
 */

namespace kotlinx::coroutines {

/**
 * A runnable task for [CoroutineDispatcher.dispatch].
 *
 * It is equivalent to the type `() -> Unit`. On the JVM it would map to `java.lang.Runnable`
 * so it interoperates with existing JVM dispatcher integrations; in the C++ port the
 * matching `make_runnable<F>` factory adapts any callable to this interface.
 */
class Runnable {
public:
    virtual ~Runnable() = default;
    virtual void run() = 0;
};

} // namespace kotlinx::coroutines
