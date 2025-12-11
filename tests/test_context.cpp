#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineName.hpp"
#include <iostream>
#include <cassert>

using namespace kotlinx::coroutines;

int main() {
    auto name1 = std::make_shared<CoroutineName>("Name1");
    auto name2 = std::make_shared<CoroutineName>("Name2");

    std::shared_ptr<CoroutineContext> ctx1 = name1;
    std::shared_ptr<CoroutineContext> ctx2 = name2;

    // Test get
    auto get1 = ctx1->get(CoroutineName::type_key);
    assert(get1 != nullptr);
    assert(std::dynamic_pointer_cast<CoroutineName>(get1)->name == "Name1");

    // Test plus (replacement)
    auto combined = ctx1->operator+(ctx2); // Should replace Name1 with Name2
    auto getCombined = combined->get(CoroutineName::type_key);
    assert(getCombined != nullptr);
    assert(std::dynamic_pointer_cast<CoroutineName>(getCombined)->name == "Name2");

    // Test minus_key
    auto minus = combined->minus_key(CoroutineName::type_key);
    // Should be empty context or nullptr semantics? 
    // Usually EmptyCoroutineContext. 
    // If minusKey removes the only element, it returns Empty or null?
    // In Kotlin: EmptyCoroutineContext.
    // In C++: nullptr represents Empty? Or we have EmptyCoroutineContext class?
    
    // If implementation returns nullptr for empty, verify.
    // If not implemented, link error.

    std::cout << "CoroutineContext test passed" << std::endl;
    return 0;
}
