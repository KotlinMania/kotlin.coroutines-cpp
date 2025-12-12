/**
 * @file SchedulerTask.common.cpp
 * @brief Scheduler task implementation
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/SchedulerTask.common.kt
 */

#include "kotlinx/coroutines/Runnable.hpp"
#include <stdexcept>

namespace kotlinx {
    namespace coroutines {
        /**
 * A Runnable that's especially optimized for running in Dispatchers::Default on the JVM.
 *
 * Replacing a SchedulerTask with a Runnable should not lead to any change in observable behavior.
 *
 * An arbitrary Runnable, once it is dispatched by Dispatchers::Default, gets wrapped into a class that
 * stores the submission time, the execution context, etc.
 * For Runnable instances that we know are only going to be executed in dispatch procedures, we can avoid the
 * overhead of separately allocating a wrapper, and instead have the Runnable contain the required fields
 * on construction.
 *
 * When running outside the standard dispatchers, these new fields are just dead weight.
 */
        class SchedulerTask : public Runnable {
        protected:
            SchedulerTask() = default;

        public:
            virtual ~SchedulerTask() = default;

            // Required by Runnable interface
            void run() override {
                // TODO: Abstract method - subclasses must override this
                throw std::logic_error("SchedulerTask::run() is abstract and must be overridden");
            }
        };
    } // namespace coroutines
} // namespace kotlinx