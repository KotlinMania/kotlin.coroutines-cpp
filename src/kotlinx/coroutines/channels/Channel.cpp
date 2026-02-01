// port-lint: source channels/Channel.kt
#include "kotlinx/coroutines/channels/Channel.hpp"
#include <string>

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // Exception constructors are defined inline in Channel.hpp

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