// Transliterated from Kotlin to C++ (first-pass, mechanical syntax mapping)
// Original: kotlinx-coroutines-core/common/src/SchedulerTask.common.kt
//
// TODO:
// - expect abstract class needs platform-specific implementation
// - internal constructor needs access control

namespace kotlinx {
namespace coroutines {

class Runnable; // Forward declaration

/**
 * A [Runnable] that's especially optimized for running in [Dispatchers.Default] on the JVM.
 *
 * Replacing a [SchedulerTask] with a [Runnable] should not lead to any change in observable behavior.
 *
 * An arbitrary [Runnable], once it is dispatched by [Dispatchers.Default], gets wrapped into a class that
 * stores the submission time, the execution context, etc.
 * For [Runnable] instances that we know are only going to be executed in dispatch procedures, we can avoid the
 * overhead of separately allocating a wrapper, and instead have the [Runnable] contain the required fields
 * on construction.
 *
 * When running outside the standard dispatchers, these new fields are just dead weight.
 */
// TODO: expect abstract class - needs platform-specific implementation
class SchedulerTask : public Runnable {
protected:
    // internal constructor
    SchedulerTask() = default;

public:
    virtual ~SchedulerTask() = default;
};

} // namespace coroutines
} // namespace kotlinx
