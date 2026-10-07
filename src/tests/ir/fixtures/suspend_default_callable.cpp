// NOTE(port): A default can construct a suspend callable without executing its body.
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include <functional>
using namespace kotlinx::coroutines;
using Completion = std::shared_ptr<Continuation<void*>>;
using Callable = std::function<void*(Completion)>;
[[suspend]] void* source(Completion caller);
void accepts(Callable callable = [] [[suspend]] (Completion caller) -> void* {
    return source(caller);
});
