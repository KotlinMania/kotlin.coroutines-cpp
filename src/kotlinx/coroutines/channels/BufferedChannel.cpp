// port-lint: source channels/BufferedChannel.kt
/**
 * @file BufferedChannel.cpp
 * @brief Implementation of BufferedChannel.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt
 *
 * Generic algorithms remain in the header. Concrete continuation adaptation and
 * explicit template instantiations for common types are implemented here.
 */

#include "kotlinx/coroutines/channels/BufferedChannel.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            namespace {
            // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:141-164
            // NOTE(port): Adapt Kotlin's Unit completion to the repo's erased suspend ABI.
            class SendContinuationAdapter final : public Continuation<void> {
            public:
                explicit SendContinuationAdapter(Continuation<void*>* completion) : completion_(completion) {}
                std::shared_ptr<CoroutineContext> get_context() const override {
                    return completion_ ? completion_->get_context() : nullptr;
                }
                void resume_with(Result<void> result) override {
                    if (!completion_) return;
                    if (result.is_success()) completion_->resume_with(Result<void*>::success(nullptr));
                    else completion_->resume_with(Result<void*>::failure(result.exception_or_null()));
                }
            private:
                Continuation<void*>* completion_;
            };
            }
            namespace detail {
            // Transliterated from: kotlinx-coroutines-core/common/src/channels/BufferedChannel.kt:141-164
            std::shared_ptr<Continuation<void>> adapt_send_completion(Continuation<void*>* completion) {
                return std::make_shared<SendContinuationAdapter>(completion);
            }
            }
            // Explicit instantiations for common types
            template class BufferedChannel<int>;
            template class BufferedChannel<std::string>;
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx
