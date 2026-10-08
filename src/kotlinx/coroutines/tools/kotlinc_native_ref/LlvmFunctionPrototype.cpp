// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:21-92
#include "LlvmFunctionPrototype.hpp"
#include <mutex>
#include <vector>
#include <utility>

namespace org::jetbrains::kotlin::backend::konan::llvm {
namespace {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:38-42
class DummyLlvmFunctionAttributeProvider final : public LlvmFunctionAttributeProvider {
public:
    // NOTE(port): These empty operations are the upstream empty provider.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:39-39
    void add_call_site_attributes(LLVMValueRef /*call_site*/) override {}
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:41-41
    void add_function_attributes(LLVMValueRef /*function*/) override {}
};

// Copies attributes from a function declared in another LLVM module.
// NOTE(port): The external LLVM function and its attributes are borrowed from
// their owning LLVM context. The returned provider retains its own lazy lists.
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:44-92
class LlvmFunctionAttributesCopier final : public LlvmFunctionAttributeProvider {
public:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:49-49
    explicit LlvmFunctionAttributesCopier(LLVMValueRef external_function) : external_function_(external_function) {}
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:77-83
    void add_call_site_attributes(LLVMValueRef call_site) override {
        const auto& lists = attributes_for_call_site();
        for (std::size_t list_index = 0; list_index < lists.size(); ++list_index) {
            for (const auto attribute : lists[list_index])
                LLVMAddCallSiteAttribute(call_site, LLVMAttributeFunctionIndex + list_index, attribute);
        }
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:85-91
    void add_function_attributes(LLVMValueRef function) override {
        const auto& lists = attributes_for_function_declaration();
        for (std::size_t list_index = 0; list_index < lists.size(); ++list_index) {
            for (const auto attribute : lists[list_index])
                LLVMAddAttributeAtIndex(function, LLVMAttributeFunctionIndex + list_index, attribute);
        }
    }
private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:51-51
    int params_count() {
        std::call_once(params_once_, [&] { params_count_ = LLVMCountParams(external_function_); });
        return params_count_;
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:53-62
    const std::vector<std::vector<LLVMAttributeRef>>& attributes_for_call_site() {
        std::call_once(call_site_once_, [&] {
            std::vector<std::vector<LLVMAttributeRef>> result;
            for (const auto& declaration_attributes : attributes_for_function_declaration()) {
                std::vector<LLVMAttributeRef> attributes;
                for (const auto attribute : declaration_attributes) {
                    // LLVMIsEnumAttribute covers enum and integer attributes.
                    if (LLVMIsEnumAttribute(attribute) != 0) attributes.push_back(attribute);
                }
                result.push_back(std::move(attributes));
            }
            call_site_attributes_ = std::move(result);
        });
        return call_site_attributes_;
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:64-75
    const std::vector<std::vector<LLVMAttributeRef>>& attributes_for_function_declaration() {
        std::call_once(declaration_once_, [&] {
            std::vector<std::vector<LLVMAttributeRef>> result;
            const auto count = params_count();
            // NOTE(port): Walk Kotlin's signed -1..paramsCount range, then cast
            // each index to the actual LLVM-C attribute-index representation.
            for (int index = -1; index <= count; ++index) {
                const auto attribute_index = static_cast<LLVMAttributeIndex>(index);
                const auto attribute_count = LLVMGetAttributeCountAtIndex(external_function_, attribute_index);
                std::vector<LLVMAttributeRef> attributes(attribute_count);
                LLVMGetAttributesAtIndex(external_function_, attribute_index, attributes.data());
                result.push_back(std::move(attributes));
            }
            declaration_attributes_ = std::move(result);
        });
        return declaration_attributes_;
    }
    LLVMValueRef external_function_;
    // NOTE(port): call_once preserves Kotlin's synchronized lazy snapshots.
    std::once_flag params_once_;
    std::once_flag call_site_once_;
    std::once_flag declaration_once_;
    int params_count_ = 0;
    std::vector<std::vector<LLVMAttributeRef>> call_site_attributes_;
    std::vector<std::vector<LLVMAttributeRef>> declaration_attributes_;
};
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:30-31
std::shared_ptr<LlvmFunctionAttributeProvider> LlvmFunctionAttributeProvider::make_empty() {
    static const auto provider = std::make_shared<DummyLlvmFunctionAttributeProvider>();
    return provider;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmFunctionPrototype.kt:33-34
std::shared_ptr<LlvmFunctionAttributeProvider> LlvmFunctionAttributeProvider::copy_from_external(LLVMValueRef external_function) {
    return std::make_shared<LlvmFunctionAttributesCopier>(external_function);
}
}
