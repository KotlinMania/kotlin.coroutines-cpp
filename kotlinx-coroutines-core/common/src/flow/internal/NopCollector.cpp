#include "kotlinx/coroutines/core_fwd.hpp"
namespace kotlinx {namespace coroutines {namespace flow {namespace {
// import kotlinx.coroutines.flow.*
class NopCollector : FlowCollector<Any*> {
    virtual auto  emit(value: Any*) {
        // does nothing
    }
}

}}}} // namespace kotlinx::coroutines::flow::internal