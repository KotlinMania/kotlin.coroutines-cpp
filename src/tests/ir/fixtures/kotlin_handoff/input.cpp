#include "api.hpp"
[[suspend]] void* cpp_value(int value, std::shared_ptr<Continuation<void*>> completion) {
    std::string prefix("cpp");
    void* raw = suspend(kotlin_value(value, completion));
    std::unique_ptr<int> boxed(static_cast<int*>(raw));
    return new int(*boxed + prefix.size());
}
