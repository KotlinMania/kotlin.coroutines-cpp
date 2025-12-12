// Transliterated from Kotlin to C++ - kotlinx.coroutines.test.internal.ExceptionCollector
// Original package: kotlinx.coroutines.test.internal
//
// TODO: Import statements removed; fully qualify types or add appropriate includes
// TODO: Kotlin class singleton pattern translated to namespace/class with static members
// TODO: Kotlin visibility needs C++ equivalent
// TODO: Annotations (@Suppress) preserved as comments

#include <functional>
#include <map>
#include <vector>
#include <exception>
#include <mutex>

namespace kotlinx {
    namespace coroutines {
        namespace test {
            namespace {
                // package kotlinx.coroutines.test.internal

                // import kotlinx.coroutines.*
                // import kotlinx.coroutines.internal.*
                // import kotlin.coroutines.*

                /**
 * If [addOnExceptionCallback] is called, the provided callback will be evaluated each time
 * [handleCoroutineException] is executed and can't find a [CoroutineExceptionHandler] to
 * process the exception.
 *
 * When a callback is registered once, even if it's later removed, the system starts to assume that
 * other callbacks will eventually be registered, and so collects the exceptions.
 * Once a new callback is registered, the collected exceptions are used with it.
 *
 * The callbacks in this class are the last resort before relying on platform-dependent
 * ways to report uncaught exceptions from coroutines.
 */
                // TODO: class singleton pattern; translate to class with static members or namespace
                class ExceptionCollector : AbstractCoroutineContextElement, CoroutineExceptionHandler {
                private:
                    static std::mutex lock_;
                    static bool enabled_;
                    static std::vector<std::exception_ptr> unprocessed_exceptions_;
                    static std::map<void *, std::function<void(std::exception_ptr)> > callbacks_;

                public:
                    ExceptionCollector() : AbstractCoroutineContextElement(CoroutineExceptionHandler::Key) {
                    }

                    /**
     * Registers [callback] to be executed when an uncaught exception happens.
     * [owner] is a key by which to distinguish different callbacks.
     */
                    static void add_on_exception_callback(void *owner,
                                                          std::function<void(std::exception_ptr)> callback) {
                        std::lock_guard<std::mutex> guard(lock_);
                        enabled_ = true; // never becomes `false` again
                        auto previous_value = callbacks_.find(owner);
                        if (previous_value != callbacks_.end()) {
                            throw std::logic_error("Callback already registered for this owner");
                        }
                        callbacks_[owner] = callback;
                        // try to process the exceptions using the newly-registered callback
                        for (auto &ex: unprocessed_exceptions_) {
                            report_exception(ex);
                        }
                        unprocessed_exceptions_.clear();
                    }

                    /**
     * Unregisters the callback associated with [owner].
     */
                    static void remove_on_exception_callback(void *owner) {
                        std::lock_guard<std::mutex> guard(lock_);
                        if (enabled_) {
                            auto existing_value = callbacks_.find(owner);
                            if (existing_value == callbacks_.end()) {
                                throw std::logic_error("No callback registered for this owner");
                            }
                            callbacks_.erase(owner);
                        }
                    }

                    /**
     * Tries to handle the exception by propagating it to an interested consumer.
     * Returns `true` if the exception does not need further processing.
     *
     * Doesn't throw.
     */
                    static bool handle_exception(std::exception_ptr exception) {
                        std::lock_guard<std::mutex> guard(lock_);
                        if (!enabled_) return false;
                        if (report_exception(exception)) return true;
                        /** we don't return the result of the `add` function because we don't have a guarantee
         * that a callback will eventually appear and collect the unprocessed exceptions, so
         * we can't consider [exception] to be properly handled. */
                        unprocessed_exceptions_.push_back(exception);
                        return false;
                    }

                private:
                    /**
     * Try to report [exception] to the existing callbacks.
     */
                    static bool report_exception(std::exception_ptr exception) {
                        bool executed_a_callback = false;
                        for (auto &[owner, callback]: callbacks_) {
                            callback(exception);
                            executed_a_callback = true;
                            /** We don't leave the function here because we want to fan-out the exceptions to every interested consumer,
             * it's not enough to have the exception processed by one of them.
             * The reason is, it's less big of a deal to observe multiple concurrent reports of bad behavior than not
             * to observe the report in the exact callback that is connected to that bad behavior. */
                        }
                        return executed_a_callback;
                    }

                public:
                    // @Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE") // do not remove the INVISIBLE_REFERENCE suppression: required in K2
                    void handle_exception(const CoroutineContext &context, std::exception_ptr exception) override {
                        if (handle_exception(exception)) {
                            throw ExceptionSuccessfullyProcessed();
                        }
                    }

                    bool equals(const CoroutineContextElement *other) const override {
                        // TODO: Kotlin type checking with 'is' operator; use dynamic_cast in C++
                        return dynamic_cast<const ExceptionCollector *>(other) != nullptr ||
                               dynamic_cast<const ExceptionCollectorAsService *>(other) != nullptr;
                    }

                    // Singleton instance
                    static ExceptionCollector instance;
                };

                // Static member definitions
                std::mutex ExceptionCollector::lock_;
                bool ExceptionCollector::enabled_ = false;
                std::vector<std::exception_ptr> ExceptionCollector::unprocessed_exceptions_;
                std::map<void *, std::function<void(std::exception_ptr)> > ExceptionCollector::callbacks_;
                ExceptionCollector ExceptionCollector::instance;

                /**
 * A workaround for being unable to treat an class as a `ServiceLoader` service.
 */
                class ExceptionCollectorAsService : CoroutineExceptionHandler {
                public:
                    void handle_exception(const CoroutineContext &context, std::exception_ptr exception) override {
                        ExceptionCollector::instance.handle_exception(context, exception);
                    }

                    bool equals(const CoroutineContextElement *other) const override {
                        return dynamic_cast<const ExceptionCollectorAsService *>(other) != nullptr ||
                               dynamic_cast<const ExceptionCollector *>(other) != nullptr;
                    }

                    int hash_code() const override {
                        return std::hash<void *>{}(&ExceptionCollector::instance);
                    }
                };
            } // namespace internal
        } // namespace test
    } // namespace coroutines
} // namespace kotlinx