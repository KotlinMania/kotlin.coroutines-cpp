// Original Kotlin package: kotlinx.coroutines.debug
#include "kotlinx/coroutines/debug/internal/DebugCoroutineInfo.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/internal/CoroutineStackFrame.hpp"

#include <vector>
#include <string>
#include <memory>
#include <sstream>

namespace kotlinx::coroutines::debug {

    enum class State {
        CREATED,
        RUNNING,
        SUSPENDED
    };

    class CoroutineInfo {
    public:
        explicit CoroutineInfo(const internal::DebugCoroutineInfo& delegate)
            : context_(delegate.context),
              state_(state_value_of(delegate.state)),
              creation_stack_bottom_(delegate.creation_stack_bottom),
              last_observed_frame_(delegate.last_observed_frame) {}

        const std::shared_ptr<CoroutineContext>& context() const { return context_; }

        State state() const { return state_; }

        std::shared_ptr<Job> job() const {
             if (!context_) return nullptr;
             return context_job(*context_);
        }

        std::vector<kotlinx::coroutines::internal::StackTraceElement> creation_stack_trace() const {
            return creation_stack_trace_impl();
        }

        std::vector<kotlinx::coroutines::internal::StackTraceElement> last_observed_stack_trace() const {
            kotlinx::coroutines::internal::CoroutineStackFrame* frame = last_observed_frame_;
            if (!frame) return {};
            
            std::vector<kotlinx::coroutines::internal::StackTraceElement> result;
            while (frame) {
                // TODO: In Kotlin Native this would resolve file/line info
                // For now we rely on the stub impl
                auto element = frame->get_stack_trace_element();
                if (element) {
                   result.push_back(*element);
                }
                frame = frame->get_caller_frame();
            }
            return result;
        }

        std::string to_string() const {
            std::stringstream ss;
            ss << "CoroutineInfo(state=" << state_to_string(state_) << ",context=" << context_.get() << ")";
            return ss.str();
        }

    private:
        std::shared_ptr<CoroutineContext> context_;
        State state_;
        kotlinx::coroutines::internal::CoroutineStackFrame* creation_stack_bottom_;
        kotlinx::coroutines::internal::CoroutineStackFrame* last_observed_frame_;

        static State state_value_of(const std::string& state_str) {
            if (state_str == "CREATED") return State::CREATED;
            if (state_str == "RUNNING") return State::RUNNING;
            if (state_str == "SUSPENDED") return State::SUSPENDED;
            return State::CREATED;
        }

        static std::string state_to_string(State state) {
            switch (state) {
                case State::CREATED: return "CREATED";
                case State::RUNNING: return "RUNNING";
                case State::SUSPENDED: return "SUSPENDED";
            }
            return "UNKNOWN";
        }

        std::vector<kotlinx::coroutines::internal::StackTraceElement> creation_stack_trace_impl() const {
            const kotlinx::coroutines::internal::CoroutineStackFrame* bottom = creation_stack_bottom_;
            if (!bottom) return {};
            
            // Walk up from bottom->get_caller_frame()
            // Similar to last_observed_stack_trace but starting point differs
            return yield_frames_to_list(bottom->get_caller_frame());
        }

        std::vector<kotlinx::coroutines::internal::StackTraceElement> yield_frames_to_list(
            kotlinx::coroutines::internal::CoroutineStackFrame* frame
        ) const {
             std::vector<kotlinx::coroutines::internal::StackTraceElement> result;
            while (frame) {
                auto element = frame->get_stack_trace_element();
                if (element) {
                    result.push_back(*element);
                }
                frame = frame->get_caller_frame();
            }
            return result;
        }
    };
}
