// Kotlin source: kotlinx-coroutines-core/common/src/channels/Channel.kt
#include "kotlinx/coroutines/channels/Channel.hpp"
#include <string>

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // ClosedSendChannelException implementation
            ClosedSendChannelException::ClosedSendChannelException(const std::string &message)
                : std::runtime_error(message) {
            }

            // ClosedReceiveChannelException implementation
            ClosedReceiveChannelException::ClosedReceiveChannelException(const std::string &message)
                : std::runtime_error(message) {
            }

            namespace detail {
                int get_default_buffer_capacity_impl() {
                    // TODO: Read implementation from system property/environment variable if needed
                    // For now hardcoded 64
                    return 64;
                }
            }
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx