// port-lint: source channels/Channel.kt
#include "kotlinx/coroutines/channels/Channel.hpp"
#include <string>

namespace kotlinx {
    namespace coroutines {
        namespace channels {
            // Exception constructors are defined inline in Channel.hpp

            namespace detail {
                int get_default_buffer_capacity_impl() {
                    // Upstream reads `kotlinx.coroutines.channels.defaultBuffer` from the
                    // JVM system properties with a default of 64. Native targets ignore
                    // the property and always return 64; the C++ port matches the K/N
                    // behavior exactly.
                    return 64;
                }
            }
        } // namespace channels
    } // namespace coroutines
} // namespace kotlinx