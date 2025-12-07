#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/native/src/SchedulerTask.kt
//
// TODO: actual keyword - platform-specific implementation

namespace kotlinx {
namespace coroutines {

// TODO: actual abstract class
class SchedulerTask : Runnable {
public:
    virtual ~SchedulerTask() = default;
};

} // namespace coroutines
} // namespace kotlinx
