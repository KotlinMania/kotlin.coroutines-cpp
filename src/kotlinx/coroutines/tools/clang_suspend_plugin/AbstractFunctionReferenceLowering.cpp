// port-lint: source compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:135-295
#include "AbstractFunctionReferenceLowering.hpp"
#include "clang/AST/AST.h"
#include "clang/AST/QualTypeNames.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include <stdexcept>

namespace org::jetbrains::kotlin::backend::common::lower {
using namespace clang;
// NOTE(port): C++ array bound values keep their native field type. Explicit
// aggregate initialization copies each element, including nested arrays, and
// gives the compiler its normal partial-construction cleanup responsibility.
static std::string array_initializer(ASTContext& context, QualType type, const std::string& parameter) {
    const auto* array = context.getAsConstantArrayType(type);
    if (!array) {
        if (type->isArrayType())
            throw std::runtime_error("array capture requires a constant complete array type");
        return parameter;
    }
    std::string result = "{";
    for (uint64_t index = 0; index < array->getSize().getZExtValue(); ++index) {
        if (index) result += ", ";
        result += array_initializer(context, array->getElementType(), parameter + "[" + std::to_string(index) + "]");
    }
    return result + "}";
}
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:135-194
// NOTE(port): Clang has already resolved capture types and C++ ownership. Keep
// those real fields and construct them from bound values in their source order.
// C++ call syntax requires operator() for Kotlin's invoke method.
std::string AbstractFunctionReferenceLowering::build_class(
    ASTContext& context, const LambdaExpr* function_reference, const std::string& name) const {
    PrintingPolicy policy(context.getLangOpts());
    policy.PrintAsCanonical = true;
    const auto* closure = function_reference->getLambdaClass();
    llvm::DenseMap<const ValueDecl*, FieldDecl*> capture_fields;
    FieldDecl* captured_this = nullptr;
    closure->getCaptureFields(capture_fields, captured_this);
    std::string fields, parameters, initializers;
    unsigned index = 0;
    for (const auto& capture : closure->captures()) {
        const auto* field = capture.capturesThis() ? captured_this : capture_fields.lookup(capture.getCapturedVar());
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:171-182
        const auto field_name = "f_" + std::to_string(index) + "_";
        const auto parameter_name = "p" + std::to_string(index++);
        if (!parameters.empty()) parameters += ", ";
        if (!initializers.empty()) initializers += ", ";
        const auto type = TypeName::getFullyQualifiedType(field->getType(), context);
        llvm::raw_string_ostream field_stream(fields);
        type.print(field_stream, policy, field_name);
        fields += ";\n";
        llvm::raw_string_ostream parameter_stream(parameters);
        if (type->isArrayType()) {
            // NOTE(port): Native arrays cannot be by-value C++ parameters.
            // Borrow only during construction; the bound field owns its copy.
            context.getLValueReferenceType(type).print(parameter_stream, policy, parameter_name);
            initializers += field_name + array_initializer(context, type, parameter_name);
        } else {
            type.print(parameter_stream, policy, parameter_name);
            initializers += field_name + "(" + (type->isReferenceType() ? parameter_name : "std::move(" + parameter_name + ")") + ")";
        }
    }
    return "struct " + name + " {\nprivate:\n" + fields + "public:\nexplicit " + name + "(" + parameters + ")" +
        (initializers.empty() ? "" : " : " + initializers) + " {}\n" +
        build_invoke_method(context, function_reference) + "\n};\n";
}

// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:210-295
// NOTE(port): Original captured declarations map to indexed private fields.
// Print the current invoke declaration's body, never LambdaExpr's cached body.
// The parsed class establishes the new parameter, field and return-target symbols.
std::string AbstractFunctionReferenceLowering::build_invoke_method(
    ASTContext& context, const LambdaExpr* function_reference) const {
    const auto* invoke = function_reference->getCallOperator();
    if (invoke->getDescribedFunctionTemplate())
        throw std::runtime_error("generic callable class requires invoke template parameter lowering");
    PrintingPolicy policy(context.getLangOpts());
    policy.PrintAsCanonical = true;
    std::string method;
    llvm::raw_string_ostream stream(method);
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:228-249
    for (const auto* annotation : invoke->attrs()) {
        annotation->printPretty(stream, policy);
        stream << " ";
    }
    TypeName::getFullyQualifiedType(invoke->getReturnType(), context).print(stream, policy);
    stream << " operator()(";
    for (unsigned index = 0; index < invoke->getNumParams(); ++index) {
        if (index) stream << ", ";
        auto* parameter = invoke->getParamDecl(index);
        for (const auto* annotation : parameter->attrs()) {
            annotation->printPretty(stream, policy);
            stream << " ";
        }
        TypeName::getFullyQualifiedType(parameter->getType(), context).print(stream, policy, parameter->getNameAsString());
    }
    stream << ")";
    if (invoke->isConst()) stream << " const";
    if (invoke->isVolatile()) stream << " volatile";
    invoke->getType()->castAs<FunctionProtoType>()->printExceptionSpecification(stream, policy);
    llvm::DenseMap<const ValueDecl*, std::string> variables_mapping;
    llvm::DenseMap<const ValueDecl*, FieldDecl*> fields;
    FieldDecl* this_field = nullptr;
    function_reference->getLambdaClass()->getCaptureFields(fields, this_field);
    std::string receiver_field;
    QualType receiver_type;
    unsigned bound_index = 0;
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:261-274
    // NOTE(port): Clang's capture declarations replace Kotlin's leading bound
    // parameters. Lookup uses declaration identity, not the identifier spelling.
    for (const auto& bound : function_reference->getLambdaClass()->captures()) {
        const auto field = "this->f_" + std::to_string(bound_index++) + "_";
        if (bound.capturesThis()) {
            // NOTE(port): A copied receiver is an owned object field. The body
            // still contains pointer-typed receiver expressions, so bind them
            // to this field's address rather than the original enclosing object.
            const auto captured_type = this_field->getType();
            const bool copied_receiver = !captured_type->isPointerType();
            const auto object_type = invoke->isConst() ? context.getConstType(captured_type) : captured_type;
            receiver_type = copied_receiver ? context.getPointerType(object_type) : captured_type;
            receiver_field = copied_receiver ? "(&" + field + ")" : field;
        }
        else variables_mapping[bound.getCapturedVar()] = field;
    }
    // NOTE(port): Clang printing adapter for the variable remapping in
    // LowerUtils.kt:40-59. Reparsed member accesses bind to actual field symbols.
    class BoundValues : public PrinterHelper {
    public:
        BoundValues(QualType type, std::string receiver,
                    const llvm::DenseMap<const ValueDecl*, std::string>& mapping, PrintingPolicy policy)
            : type_(type), receiver_(std::move(receiver)), mapping_(mapping), policy_(policy) {}
        bool handledStmt(Stmt* statement, llvm::raw_ostream& output) override {
            // NOTE(port): Clang's declaration printer does not propagate the
            // expression helper. Print an affected initializer with this same
            // binding map; leave declaration identity and initialization style intact.
            if (auto* declarations = dyn_cast<DeclStmt>(statement)) {
                class BoundUses : public RecursiveASTVisitor<BoundUses> {
                public:
                    BoundUses(QualType type, const llvm::DenseMap<const ValueDecl*, std::string>& mapping)
                        : type_(type), mapping_(mapping) {}
                    bool VisitCXXThisExpr(CXXThisExpr* expression) {
                        found |= !type_.isNull() && expression->getType().getCanonicalType() == type_.getCanonicalType();
                        return true;
                    }
                    bool VisitDeclRefExpr(DeclRefExpr* expression) {
                        found |= mapping_.contains(expression->getDecl());
                        return true;
                    }
                    bool found = false;
                private:
                    QualType type_;
                    const llvm::DenseMap<const ValueDecl*, std::string>& mapping_;
                } uses(type_, mapping_);
                for (auto* declaration : declarations->decls()) {
                    auto* variable = dyn_cast<VarDecl>(declaration);
                    if (!variable) return false;
                    uses.TraverseStmt(variable->getInit());
                }
                if (!uses.found) return false;
                for (auto* declaration : declarations->decls()) {
                    auto* variable = cast<VarDecl>(declaration);
                    auto declaration_policy = policy_;
                    declaration_policy.SuppressInitializers = true;
                    variable->print(output, declaration_policy);
                    if (variable->hasInit()) {
                        const auto style = variable->getInitStyle();
                        const bool braces = style == VarDecl::ListInit && !isa<InitListExpr>(variable->getInit());
                        output << (style == VarDecl::CInit ? " = " : style == VarDecl::CallInit ? "(" : braces ? "{" : "");
                        variable->getInit()->printPretty(output, this, policy_);
                        if (style == VarDecl::CallInit) output << ")";
                        else if (braces) output << "}";
                    }
                    output << ";\n";
                }
                return true;
            }
            if (const auto* reference = dyn_cast<DeclRefExpr>(statement)) {
                auto replacement = mapping_.find(reference->getDecl());
                if (replacement != mapping_.end()) {
                    output << replacement->second;
                    return true;
                }
            }
            if (const auto* receiver = dyn_cast<CXXThisExpr>(statement);
                receiver && !type_.isNull() && receiver->getType().getCanonicalType() == type_.getCanonicalType()) {
                output << receiver_;
                return true;
            }
            return false;
        }
    private:
        QualType type_;
        std::string receiver_;
        const llvm::DenseMap<const ValueDecl*, std::string>& mapping_;
        PrintingPolicy policy_;
    };
    BoundValues bound_values(receiver_type, receiver_field, variables_mapping, policy);
    stream << " ";
    invoke->getBody()->printPretty(stream, &bound_values, policy);
    return method;
}
}
