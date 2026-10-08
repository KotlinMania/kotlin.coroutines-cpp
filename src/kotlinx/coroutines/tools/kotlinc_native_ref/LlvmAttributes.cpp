// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:67-105
#include "LlvmAttributes.hpp"
#include <map>
#include <mutex>
#include <utility>

namespace org::jetbrains::kotlin::backend::konan::llvm {
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:73-73
LlvmParameterAttribute::LlvmParameterAttribute(std::string llvm_attribute_name) : llvm_attribute_name_(std::move(llvm_attribute_name)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:75-81
LLVMAttributeKindId LlvmParameterAttribute::as_attribute_kind_id() const {
    // NOTE(port): Preserve the companion cache keyed by actual singleton
    // identity; serialize getOrPut when compiler threads share its attributes.
    static std::mutex mutex;
    static std::map<const LlvmParameterAttribute*, LLVMAttributeKindId> cache;
    std::lock_guard<std::mutex> lock(mutex);
    const auto found = cache.find(this);
    if (found != cache.end()) return found->second;
    const auto result = get_llvm_attribute_kind_id(llvm_attribute_name_);
    cache.emplace(this, result);
    return result;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:83-83
const LlvmParameterAttribute LlvmParameterAttribute::SIGN_EXT("signext");
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:84-84
const LlvmParameterAttribute LlvmParameterAttribute::ZERO_EXT("zeroext");
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:87-87
LlvmFunctionAttribute::LlvmFunctionAttribute(std::string llvm_attribute_name) : llvm_attribute_name_(std::move(llvm_attribute_name)) {}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:89-95
LLVMAttributeKindId LlvmFunctionAttribute::as_attribute_kind_id() const {
    static std::mutex mutex;
    static std::map<const LlvmFunctionAttribute*, LLVMAttributeKindId> cache;
    std::lock_guard<std::mutex> lock(mutex);
    const auto found = cache.find(this);
    if (found != cache.end()) return found->second;
    const auto result = get_llvm_attribute_kind_id(llvm_attribute_name_);
    cache.emplace(this, result);
    return result;
}
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:97-97
const LlvmFunctionAttribute LlvmFunctionAttribute::NO_UNWIND("nounwind");
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:98-98
const LlvmFunctionAttribute LlvmFunctionAttribute::NO_RETURN("noreturn");
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:99-99
const LlvmFunctionAttribute LlvmFunctionAttribute::NO_INLINE("noinline");
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:100-100
const LlvmFunctionAttribute LlvmFunctionAttribute::ALWAYS_INLINE("alwaysinline");
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:101-101
const LlvmFunctionAttribute LlvmFunctionAttribute::SANITIZE_THREAD("sanitize_thread");
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:102-102
const LlvmFunctionAttribute LlvmFunctionAttribute::SSP("ssp");
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:103-103
const LlvmFunctionAttribute LlvmFunctionAttribute::SSP_STRONG("sspstrong");
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/LlvmAttributes.kt:104-104
const LlvmFunctionAttribute LlvmFunctionAttribute::SSP_REQ("sspreq");
}
