// Original Kotlin package: kotlinx.coroutines.debug
// Line-by-line C++ transliteration from Kotlin
//
// TODO: @file:Suppress - Kotlin compiler directives, not applicable in C++
// TODO: @ExperimentalCoroutinesApi - Kotlin annotation, translate to comment or attribute
// TODO: CoroutineContext - Kotlin coroutine type, needs C++ equivalent
// TODO: Job - Kotlin coroutine Job type, needs C++ equivalent
// TODO: CoroutineStackFrame - Kotlin type, needs C++ equivalent
// TODO: suspend function - coroutine semantics not implemented
// TODO: tailrec - Kotlin tail recursion optimization, use manual loop or comment
// TODO: sequence builder - Kotlin sequence generation, needs C++ equivalent (generator/coroutine)
// TODO: DebugCoroutineInfo - delegate type, needs definition

#include <vector>
#include <string>
#include <optional>

// Forward declarations - TODO: implement these types
// namespace kotlin { namespace coroutines { class CoroutineContext; class CoroutineStackFrame; }}
// namespace kotlinx { namespace coroutines { class Job; }}
// namespace kotlinx { namespace coroutines { namespace debug { namespace { class DebugCoroutineInfo; }}}}

namespace kotlinx::coroutines::debug {

    // Forward declarations
    class Job;
    class CoroutineContext;
    class CoroutineStackFrame;
    class StackTraceElement;

    namespace {
        class DebugCoroutineInfo;
    }

    // Current state of the coroutine.
    enum class State {
        // Created, but not yet started.
        kCreated,
        // Started and running.
        kRunning,
        // Suspended.
        kSuspended
    };

    // Class describing coroutine info such as its context, state and stacktrace.
    // TODO: @ExperimentalCoroutinesApi annotation
    class CoroutineInfo {
    public:
        // TODO: constructor - make with friend classes
        CoroutineInfo(internal::DebugCoroutineInfo* delegate)
            : context_(delegate->context)
              , state_(state_value_of(delegate->state))
              , creation_stack_bottom_(delegate->creation_stack_bottom)
              , last_observed_frame_(delegate->last_observed_frame)
        {
        }

        // Coroutine context of the coroutine
        const CoroutineContext& context() const { return context_; }

        // Last observed state of the coroutine
        State state() const { return state_; }

        // Job associated with a current coroutine or nullptr.
        // May be later used in DebugProbes.printJob.
        Job* job() const {
            // TODO: context[Job] - Kotlin coroutine context element lookup
            return nullptr; // TODO: implement context[Job]
        }

        // Creation stacktrace of the coroutine.
        // Can be empty if DebugProbes.enableCreationStackTraces is not set.
        std::vector<StackTraceElement> creation_stack_trace() const {
            return creation_stack_trace_impl();
        }

        // Last observed stacktrace of the coroutine captured on its suspension or resumption point.
        // It means that for running coroutines resulting stacktrace is inaccurate and
        // reflects stacktrace of the resumption point, not the actual current stacktrace.
        std::vector<StackTraceElement> last_observed_stack_trace() const {
            CoroutineStackFrame* frame = last_observed_frame_;
            if (frame == nullptr) {
                return {};
            }
            std::vector<StackTraceElement> result;
            while (frame != nullptr) {
                // TODO: frame.getStackTraceElement() - optional return
                auto elem = frame->get_stack_trace_element();
                if (elem.has_value()) {
                    result.push_back(elem.value());
                }
                frame = frame->caller_frame;
            }
            return result;
        }

        std::string to_string() const {
            // TODO: string interpolation and proper state/context to_string
            return "CoroutineInfo(state=" + state_to_string(state_) + ",context=...)";
        }

    private:
        CoroutineContext context_;
        State state_;
        CoroutineStackFrame* creation_stack_bottom_;
        CoroutineStackFrame* last_observed_frame_;

        static State state_value_of(const std::string& state_str) {
            // TODO: implement proper State.valueOf mapping
            if (state_str == "CREATED") return State::kCreated;
            if (state_str == "RUNNING") return State::kRunning;
            if (state_str == "SUSPENDED") return State::kSuspended;
            return State::kCreated;
        }

        static std::string state_to_string(State state) {
            switch (state) {
                case State::kCreated: return "CREATED";
                case State::kRunning: return "RUNNING";
                case State::kSuspended: return "SUSPENDED";
                default: return "UNKNOWN";
            }
        }

        std::vector<StackTraceElement> creation_stack_trace_impl() const {
            const CoroutineStackFrame* bottom = creation_stack_bottom_;
            if (bottom == nullptr) {
                return {};
            }
            // Skip "Coroutine creation stacktrace" frame
            // TODO: sequence<StackTraceElement> { yieldFrames(bottom.callerFrame) }.toList()
            // TODO: implement with manual iteration or C++20 generator
            return yield_frames_to_list(bottom->caller_frame);
        }

        // TODO: tailrec suspend function - coroutine semantics not implemented
        // Convert to iterative version
        std::vector<StackTraceElement> yield_frames_to_list(CoroutineStackFrame* frame) const {
            std::vector<StackTraceElement> result;
            while (frame != nullptr) {
                auto elem = frame->get_stack_trace_element();
                if (elem.has_value()) {
                    result.push_back(elem.value());
                }
                frame = frame->caller_frame;
            }
            return result;
        }
    };

}
