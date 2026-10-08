// port-lint: source kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractSuspendFunctionsLowering.kt:55-234
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:119-335
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/CoroutinesVarSpillingLowering.kt:49-105
#include "NativeSuspendLowering.hpp"
#include "SuspendFunctionAnalyzer.hpp"
#include "RestrictSuspensionUtils.hpp"
#include "AbstractFunctionReferenceLowering.hpp"
#include "clang/AST/AST.h"
#include "clang/AST/Mangle.h"
#include "clang/AST/QualTypeNames.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/SaveAndRestore.h"
#include "llvm/Support/MD5.h"
#include "llvm/ADT/SmallString.h"
#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace clang;
namespace org::jetbrains::kotlin::backend::konan::lower {
using kotlinx::suspend::SuspendFunctionAnalyzer;
namespace {
struct Replacement { unsigned begin; unsigned end; std::string text; };
struct Slot { std::string name; std::string access; std::string type; bool reference; bool array = false; bool object = false; bool handler_exception = false; bool dynamic = false; };
struct Loop { std::string next; size_t scope; bool iteration = true; bool retain_condition = false; bool continued = false; };

// NOTE(port): Concrete fields retain C++ construction/destruction state;
// arrays use aligned delayed storage and references borrow typed pointers.
// Kotlin GC fields do not need C++ object lifetime bookkeeping.
// Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractSuspendFunctionsLowering.kt:115-203;
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:119-335
// Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/CoroutinesVarSpillingLowering.kt:49-105
class NativeSuspendFunctionsLowering {
public:
    NativeSuspendFunctionsLowering(ASTContext& context, FunctionDecl* function)
        : context_(context), function_(function), policy_(context.getLangOpts()),
          manager_(context.getSourceManager()) {}

    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractSuspendFunctionsLowering.kt:59-87,115-234
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:112-176
    std::string build_coroutine() {
        const auto* method = dyn_cast<CXXMethodDecl>(function_);
        const bool lambda = method && method->getParent()->isLambda();
        const bool instantiated = function_->getTemplateSpecializationInfo() ||
            (method && !method->isDependentContext() && method->getInstantiatedFromMemberFunction());
        if (instantiated) policy_.PrintAsCanonical = true;
        const bool local_frame = method || function_->getDescribedFunctionTemplate() || function_->getTemplateSpecializationInfo() ||
                                 !manager_.isWrittenInMainFile(function_->getLocation());
        if ((!method && !function_->getDeclContext()->isTranslationUnit() &&
             !isa<NamespaceDecl>(function_->getDeclContext())) ||
            function_->getReturnType() != context_.VoidPtrTy)
            throw std::runtime_error("suspend lowering requires a void* continuation entry");
        std::string parameters, arguments, constructor, initializers, capture_types;
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractSuspendFunctionsLowering.kt:91-118,169-196
        // NOTE(port): C++ receivers retain a borrowed typed pointer, preserving
        // the caller's ownership and cv-qualification across suspension.
        if (method && !method->isStatic() && !lambda) {
            receiver_ = "_kxs_receiver";
            auto type = method->getThisType();
            receiver_type_ = type.getAsString(policy_);
            fields_.push_back(declaration(type, receiver_) + ";");
            constructor += ", " + declaration(type, "_kxs_receiver_parameter");
            initializers += ", " + receiver_ + "(_kxs_receiver_parameter)";
            arguments += ", this";
        }
        if (lambda) {
            // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractFunctionReferenceLowering.kt:254-288
            // NOTE(port): Clang owns the closure's bound fields. Borrow their
            // actual referents, preserving C++ capture identity and constness;
            // the caller retains the closure across asynchronous invocation.
            for (const auto& capture : method->getParent()->captures()) {
                if (capture.capturesVariable()) {
                    const auto* variable = capture.getCapturedVar();
                    const std::string name = variable->getNameAsString();
                    const std::string field = "_kxs_capture_" + std::to_string(variables_.size());
                    const std::string type = field + "_type";
                    capture_types += "using " + type + " = decltype((" + name + "));\n";
                    fields_.push_back(type + " " + field + ";");
                    variables_[variable] = {field, field, type, true};
                    constructor += ", " + type + " " + field + "_parameter";
                    initializers += ", " + field + "(" + field + "_parameter)";
                    arguments += ", " + name;
                } else if (capture.capturesThis()) {
                    receiver_ = "_kxs_receiver";
                    receiver_type_ = "_kxs_enclosing_this_type";
                    capture_types += "using " + receiver_type_ + " = decltype(this);\n";
                    fields_.push_back(receiver_type_ + " " + receiver_ + ";");
                    constructor += ", " + receiver_type_ + " _kxs_receiver_parameter";
                    initializers += ", " + receiver_ + "(_kxs_receiver_parameter)";
                    arguments += ", this";
                }
            }
        }
        // Save all arguments to fields.
        completion_ = SuspendFunctionAnalyzer::continuation_parameter(function_);
        if (!completion_) throw std::runtime_error("suspend entry needs a trailing shared Continuation<void*> parameter");
        for (auto* parameter : function_->parameters()) {
            std::string name = parameter->getNameAsString();
            auto type = parameter->getType();
            // NOTE(port): Transfer by-value C++ ownership into the frame.
            // The stored field preserves source constness; references borrow.
            auto transfer_type = type->isReferenceType() ? type : type.getUnqualifiedType();
            if (!parameters.empty()) parameters += ", ";
            parameters += declaration(transfer_type, name);
            if (parameter == completion_) {
                variables_[parameter] = {"", "static_cast<kotlin::coroutines::native::internal::BaseContinuationImpl*>(this)->shared_from_this()", "", false};
                continue;
            }
            std::string field = "_kxs_argument_" + std::to_string(variables_.size());
            fields_.push_back(declaration(parameter->getType(), field) + ";");
            variables_[parameter] = {field, field, "", false};
            constructor += ", " + declaration(transfer_type, name);
            std::string transferred = type->isLValueReferenceType() ? name : "std::move(" + name + ")";
            arguments += ", " + transferred;
            initializers += ", " + field + "(" + transferred + ")";
        }
        const std::string state_machine = build_state_machine();
        const std::string frame = name_for_coroutine_class(instantiated);
        std::ostringstream output;
        std::vector<const NamespaceDecl*> namespaces;
        for (auto* parent = function_->getDeclContext(); !local_frame && !parent->isTranslationUnit(); parent = parent->getParent()) {
            const auto* scope = dyn_cast<NamespaceDecl>(parent);
            if (!scope) throw std::runtime_error("suspend lowering requires a namespace or translation-unit context");
            namespaces.push_back(scope);
        }
        for (auto scope = namespaces.rbegin(); scope != namespaces.rend(); ++scope)
            output << ((*scope)->isInline() ? "inline namespace " : "namespace ")
                   << (*scope)->getNameAsString() << " {\n";
        const auto base_class = get_coroutine_base_class(function_);
        if (lambda) output << "{\n" << capture_types << "struct ";
        else output << (local_frame ? "{ struct " : "namespace { struct ");
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/AbstractSuspendFunctionsLowering.kt:35-38,94-109
        // NOTE(port): Preserve the generated coroutine declaration origin in Clang.
        output << "[[clang::annotate(\"kotlin.ir.origin.COROUTINE_IMPL\")]] " << frame
               << " final : kotlin::coroutines::native::internal::" << base_class << " {\n"
               << "void* _label = nullptr;\n";
        for (const auto& reference : reference_classes_) output << reference << '\n';
        if (exception_region_) output << "std::exception_ptr _kxs_active_exception, _kxs_pending_failure, _kxs_retired_exception; bool _kxs_reenter = false;\n";
        for (const auto& field : fields_) output << field << '\n';
        output << "explicit " << frame
               << "(std::shared_ptr<kotlin::coroutines::Continuation<void*>> " << completion_->getNameAsString()
               << constructor << ") : " << base_class << "(std::move(" << completion_->getNameAsString() << "))" << initializers << " {}\n"
               << frame << "(const " << frame << "&) = delete;\n"
               << frame << "& operator=(const " << frame << "&) = delete;\n"
               << frame << "(" << frame << "&&) = delete;\n"
               << frame << "& operator=(" << frame << "&&) = delete;\n"
               << "void clear_locals() {\n";
        for (auto it = slots_.rbegin(); it != slots_.rend(); ++it) output << it->name << ".reset();\n";
        if (exception_region_) output << "_kxs_active_exception = {}; _kxs_reenter = false;\n";
        output << "}\n";
        output << state_machine;
        output << "};\n";
        if (!local_frame) output << "}\nvoid* " << function_->getNameAsString() << "(" << parameters << ") {\n";
        output << "auto frame = std::make_shared<" << frame << ">(" << completion_->getNameAsString() << arguments << ");\n"
               << "return frame->start(kotlinx::coroutines::Result<void*>::success(nullptr));\n}\n";
        for (size_t i = 0; i < namespaces.size(); ++i) output << "}\n";
        return output.str();
    }

private:
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:93-98
    // NOTE(port): The compiler's copied invoke-method IR flag is represented by
    // declaration metadata, separately from extension-receiver parameter roles.
    std::string get_coroutine_base_class(const FunctionDecl* function) {
        const bool restricted_invoke = std::any_of(
            function->specific_attr_begin<AnnotateAttr>(), function->specific_attr_end<AnnotateAttr>(),
            [](const auto* annotation) {
                return annotation->getAnnotation() == "kotlin.ir.isRestrictedSuspensionInvokeMethod";
            });
        if (org::jetbrains::kotlin::backend::common::is_restricted_suspension_function(function) || restricted_invoke)
            return "RestrictedContinuationImpl";
        return "ContinuationImpl";
    }

    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:100-101
    // NOTE(port): Kotlin's fileLowerState allocates a unique synthesized name.
    // Clang uses the declaration's source identity and concrete specialization's
    // mangled identity, keeping distinct instantiations in distinct frame types.
    std::string name_for_coroutine_class(bool instantiated) {
        const auto* method = dyn_cast<CXXMethodDecl>(function_);
        const std::string function_name = method && method->getParent()->isLambda() ? "invoke" : function_->getNameAsString();
        std::string frame = "__kxs_frame_" + function_name + "_" +
                            std::to_string(offset(function_->getLocation()));
        if (instantiated) {
            std::unique_ptr<MangleContext> mangler(context_.createMangleContext());
            std::string identity;
            llvm::raw_string_ostream stream(identity);
            mangler->mangleName(GlobalDecl(function_), stream);
            llvm::MD5 digest;
            digest.update(identity);
            auto result = digest.final();
            llvm::SmallString<32> name;
            llvm::MD5::stringifyResult(result, name);
            frame += "_inst_" + name.str().str();
        }
        return frame;
    }

    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:112-176
    // NOTE(port): Clang statements and retained typed fields supply the native
    // body/argument map. C++ catch-context and destructor handling surround the
    // same normal/resume regions; these have no direct Kotlin GC counterpart.
    std::string build_state_machine() {
        // Extract all suspend calls to temporaries in order to make correct jumps to them.
        emit_statement(function_->getBody());
        body_ << "clear_locals(); return nullptr;\n";
        std::string execute = body_.str();
        std::string unwind;
        unsigned unwind_id = 0;
        if (exception_region_) {
            unwind_id = suspension_++;
            body_.str("");
            body_.clear();
            body_ << "_kxs_unwind_" << unwind_id << ":;\n";
            emit_slot_cleanup(0, slots_.size());
            body_ << "_kxs_active_exception = {}; _kxs_reenter = false;\n"
                  << "std::rethrow_exception(_kxs_pending_failure);\n";
            unwind = body_.str();
        }
        std::ostringstream output;
        if (exception_region_) {
            output << "void* invoke_suspend(kotlinx::coroutines::Result<void*> _kxs_result) override {\n"
                   << "for (;;) { void* outcome;\nif (_kxs_active_exception) {\n"
                   << "try { std::rethrow_exception(_kxs_active_exception); }\n"
                   << "catch (...) { outcome = invoke_body(std::move(_kxs_result)); }\n"
                   << "} else { outcome = invoke_body(std::move(_kxs_result)); }\n"
                   << "if (!_kxs_reenter) return outcome;\n_kxs_reenter = false;\n"
                   << "_kxs_result = kotlinx::coroutines::Result<void*>::success(nullptr);\n}\n}\n"
                   << "void* invoke_body(kotlinx::coroutines::Result<void*> _kxs_result) {\n";
        } else output << "void* invoke_suspend(kotlinx::coroutines::Result<void*> _kxs_result) override {\n";
        if (exception_region_) output << "_kxs_retired_exception = {};\n";
        output
               << "__kxs_coroutine_begin(&_label);\ntry {\n(void)_kxs_result.get_or_throw();\n"
               << execute;
        if (exception_region_) {
            output << "} catch (...) {\n_kxs_pending_failure = std::current_exception();\n"
                   << suspension_site(unwind_id, "_kxs_unwind_")
                   << "_kxs_reenter = true; return nullptr;\n}\n" << unwind;
        } else output << "} catch (...) { clear_locals(); throw; }\n";
        output << "}\n";
        return output.str();
    }

    std::string declaration(QualType type, const std::string& name) const {
        std::string text;
        llvm::raw_string_ostream output(text);
        type.print(output, policy_, name);
        return text;
    }
    unsigned offset(SourceLocation location) const {
        return manager_.getFileOffset(manager_.getSpellingLoc(location));
    }
    unsigned end_offset(SourceLocation location) const {
        return offset(Lexer::getLocForEndOfToken(manager_.getSpellingLoc(location), 0,
                                               manager_, context_.getLangOpts()));
    }
    CharSourceRange file_range(const Stmt* statement) const {
        auto range = statement->getSourceRange();
        auto characters = range.getBegin().isMacroID() || range.getEnd().isMacroID()
            ? manager_.getExpansionRange(range)
            : CharSourceRange::getTokenRange(range);
        return Lexer::makeFileCharRange(characters, manager_, context_.getLangOpts());
    }
    std::string source(const Stmt* statement) const {
        return Lexer::getSourceText(file_range(statement),
                                    manager_, context_.getLangOpts()).str();
    }
    void collect_references(const Stmt* statement, std::vector<Replacement>& replacements, bool type_only = false) const {
        if (!statement) return;
        // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/LocalDeclarationPopupLowering.kt:55-59
        // NOTE(port): Capture initializers belong to this construction scope.
        // The invoke body has its own frame and binding map; rewriting it with
        // the enclosing frame's fields changes captured-variable identity.
        if (const auto* lambda = dyn_cast<LambdaExpr>(statement)) {
            for (const auto* initializer : lambda->capture_inits())
                collect_references(initializer, replacements, type_only);
            return;
        }
        if (!receiver_.empty()) {
            if (const auto* member = dyn_cast<MemberExpr>(statement); member && member->isImplicitAccess()) {
                replacements.push_back({offset(member->getBeginLoc()), end_offset(member->getEndLoc()),
                                        (type_only ? "std::declval<" + receiver_type_ + ">()" : receiver_) + "->" + source(member)});
                return;
            }
            if (const auto* receiver = dyn_cast<CXXThisExpr>(statement); receiver && !receiver->isImplicit()) {
                replacements.push_back({offset(receiver->getBeginLoc()), end_offset(receiver->getEndLoc()),
                    type_only ? "std::declval<" + receiver_type_ + ">()" : receiver_});
                return;
            }
        }
        if (const auto* reference = dyn_cast<DeclRefExpr>(statement)) {
            if (const auto* variable = dyn_cast<ValueDecl>(reference->getDecl())) {
                auto found = variables_.find(variable);
                if (found != variables_.end()) {
                    std::string value = found->second.access;
                    if (type_only) {
                        auto type = found->second.type.empty() ? variable->getType().getAsString(policy_) : found->second.type;
                        value = "std::declval<std::add_lvalue_reference_t<" + type + ">>()";
                    }
                    replacements.push_back({offset(reference->getBeginLoc()), end_offset(reference->getEndLoc()), value});
                }
            }
        }
        for (const auto* child : statement->children()) collect_references(child, replacements, type_only);
    }
    std::string rewrite(const Stmt* statement, std::vector<Replacement> replacements = {}, bool type_only = false) const {
        auto range = file_range(statement);
        if (range.isInvalid()) throw std::runtime_error("cannot recover the source range of a lowered expression");
        unsigned begin = offset(range.getBegin());
        std::string text = source(statement);
        std::vector<Replacement> references;
        collect_references(statement, references, type_only);
        // Use the compiler's actual substitution, including occurrences inside
        // type expressions. A synthesized constexpr variable would change
        // decltype(N) and could introduce a new runtime object.
        class TemplateValues : public RecursiveASTVisitor<TemplateValues> {
        public:
            TemplateValues(const NativeSuspendFunctionsLowering& lowering, std::vector<Replacement>& references)
                : lowering_(lowering), references_(references) {}
            // NOTE(port): Substitutions in a nested invoke are emitted when
            // lowering that invoke, using its own template specialization.
            bool TraverseLambdaExpr(LambdaExpr* expression) {
                for (auto* initializer : expression->capture_inits())
                    if (!TraverseStmt(initializer)) return false;
                return true;
            }
            // NOTE(port): C++ aliases introduce no object or nominal type.
            // Preserve the compiler's declaration binding, including aliases
            // shadowed by a different declaration in a nested source scope.
            bool VisitTypedefTypeLoc(TypedefTypeLoc location) {
                auto alias = lowering_.local_aliases_.find(location.getTypePtr()->getDecl());
                if (alias != lowering_.local_aliases_.end()) {
                    auto name = location.getNameLoc();
                    references_.push_back({lowering_.offset(name), lowering_.end_offset(name), alias->second});
                }
                return true;
            }
            bool VisitSubstNonTypeTemplateParmExpr(SubstNonTypeTemplateParmExpr* expression) {
                class Symbols : public PrinterHelper {
                public:
                    explicit Symbols(const NativeSuspendFunctionsLowering& lowering) : lowering_(lowering) {}
                    bool handledStmt(Stmt* statement, llvm::raw_ostream& stream) override {
                        auto* reference = dyn_cast<DeclRefExpr>(statement);
                        if (!reference) return false;
                        auto* declaration = reference->getDecl();
                        if (auto* record = dyn_cast<CXXRecordDecl>(declaration->getDeclContext())) {
                            stream << "::" << QualType(lowering_.context_.getCanonicalTypeDeclType(record)).getAsString(lowering_.policy_)
                                   << "::" << declaration->getNameAsString();
                            return true;
                        }
                        std::vector<const NamespaceDecl*> scopes;
                        for (auto* parent = declaration->getDeclContext(); !parent->isTranslationUnit(); parent = parent->getParent()) {
                            auto* scope = dyn_cast<NamespaceDecl>(parent);
                            if (!scope) return false;
                            if (!scope->isAnonymousNamespace()) scopes.push_back(scope);
                        }
                        stream << "::";
                        for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope)
                            stream << (*scope)->getNameAsString() << "::";
                        stream << declaration->getNameAsString();
                        return true;
                    }
                private:
                    const NativeSuspendFunctionsLowering& lowering_;
                } symbols(lowering_);
                std::string value;
                llvm::raw_string_ostream stream(value);
                expression->getReplacement()->printPretty(stream, &symbols, lowering_.policy_);
                auto parameter_type = expression->getParameterType();
                if (parameter_type->isUndeducedType()) {
                    // NOTE(port): C++ auto non-type arguments retain the type
                    // deduced by their owning function or class specialization.
                    // Match the actual parameter declaration as well as its index;
                    // nested templates can reuse the same index.
                    const auto index = expression->getIndex();
                    auto recover_type = [&](const TemplateParameterList* parameters,
                                            const TemplateArgumentList* arguments) {
                        if (!parameters || !arguments || index >= arguments->size() ||
                            index >= parameters->size() ||
                            parameters->getParam(index) != expression->getParameter()) return;
                        const auto& argument = arguments->get(index);
                        if (argument.getKind() == TemplateArgument::Declaration) parameter_type = argument.getParamTypeForDecl();
                        else if (argument.getKind() == TemplateArgument::Integral) parameter_type = argument.getIntegralType();
                        else if (argument.getKind() == TemplateArgument::NullPtr) parameter_type = argument.getNullPtrType();
                    };
                    auto* primary = lowering_.function_->getPrimaryTemplate();
                    recover_type(primary ? primary->getTemplateParameters() : nullptr,
                                 lowering_.function_->getTemplateSpecializationArgs());
                    for (auto* owner = lowering_.function_->getDeclContext();
                         parameter_type->isUndeducedType() && !owner->isTranslationUnit();
                         owner = owner->getParent()) {
                        if (auto* specialization = dyn_cast<ClassTemplateSpecializationDecl>(owner))
                            recover_type(specialization->getSpecializedTemplate()->getTemplateParameters(),
                                         &specialization->getTemplateArgs());
                    }
                    if (parameter_type->isUndeducedType())
                        throw std::runtime_error("non-type template substitution lost its deduced parameter type");
                }
                auto type = parameter_type.getAsString(lowering_.policy_);
                references_.push_back({lowering_.offset(expression->getBeginLoc()),
                    lowering_.end_offset(expression->getEndLoc()), "static_cast<" + type + ">(" + value + ")"});
                return true;
            }
        private:
            const NativeSuspendFunctionsLowering& lowering_;
            std::vector<Replacement>& references_;
        } template_values(*this, references);
        template_values.TraverseStmt(const_cast<Stmt*>(statement));
        for (const auto& reference : references) {
            bool covered = false;
            for (const auto& replacement : replacements)
                if (reference.begin >= replacement.begin && reference.end <= replacement.end) covered = true;
            if (!covered) replacements.push_back(reference);
        }
        std::sort(replacements.begin(), replacements.end(), [](const auto& a, const auto& b) {
            return a.begin > b.begin;
        });
        replacements.erase(std::unique(replacements.begin(), replacements.end(), [](const auto& a, const auto& b) {
            return a.begin == b.begin && a.end == b.end && a.text == b.text;
        }), replacements.end());
        unsigned last = begin + text.size();
        for (const auto& replacement : replacements) {
            if (replacement.begin < begin || replacement.end > last || replacement.end < replacement.begin)
                throw std::runtime_error("cannot lower overlapping or macro-expanded suspension regions");
            text.replace(replacement.begin - begin, replacement.end - replacement.begin, replacement.text);
            last = replacement.begin;
        }
        return text;
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:368-387
    // NOTE(port): Kotlin local declarations have already been moved out before
    // this phase. Clang retains lambda bodies here; only capture initializers
    // execute while constructing the lambda in the enclosing function.
    bool has_suspend_calls(const Stmt* statement) const {
        if (!statement) return false;
        if (const auto* lambda = dyn_cast<LambdaExpr>(statement)) {
            for (const auto* initializer : lambda->capture_inits())
                if (has_suspend_calls(initializer)) return true;
            return false;
        }
        if (SuspendFunctionAnalyzer::is_suspend_call(statement)) return true;
        for (const auto* child : statement->children()) if (has_suspend_calls(child)) return true;
        return false;
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/CoroutinesVarSpillingLowering.kt:53-66
    // NOTE(port): C++ storage also records construction and borrowed references;
    // locals live directly in frame fields rather than being copied at each edge.
    Slot new_slot(QualType type, bool reference = false, bool retained_object = false,
                  const std::string& deduced_type = {}) {
        type = type.getNonReferenceType();
        // NOTE(port): A resolved auto still carries placeholder sugar; remove
        // it before qualifying the type in the synthesized frame's scope.
        if (type->getContainedAutoType() && !type->isDependentType()) type = type.getCanonicalType();
        // NOTE(port): Local alias spellings are unavailable outside their source
        // scope. Canonicalization retains the actual type and cv-qualification,
        // including alias arguments nested inside a template specialization.
        if (!local_aliases_.empty()) type = type.getCanonicalType();
        std::string reference_type = deduced_type;
        if (const auto* record = type->getAsCXXRecordDecl()) {
            auto found = reference_types_.find(record->getCanonicalDecl());
            if (found != reference_types_.end()) reference_type = found->second;
        }
        std::string name = "_kxs_slot_" + std::to_string(slots_.size());
        if (reference) {
            type = type.getCanonicalType();
            std::string storage = name + "_reference";
            fields_.push_back("struct " + storage + " { using type = " + (reference_type.empty() ? type.getAsString(policy_) : reference_type) +
                "; type* value = nullptr; type& get() { return *value; } "
                "void bind(type& source) { value = std::addressof(source); } "
                "void bind(type&& source) { value = std::addressof(source); } "
                "void reset() { value = nullptr; } }; " + storage + " " + name + ";");
            Slot slot{name, name + ".get()", reference_type.empty() ? type.getAsString(policy_) : reference_type, true};
            slots_.push_back(slot);
            return slot;
        }
        if (type->isArrayType() || retained_object) {
            if (type->isArrayType() && !context_.getAsConstantArrayType(type))
                throw std::runtime_error("retained arrays require a constant complete array type");
            std::string storage = name + "_storage";
            fields_.push_back("struct " + storage + " { using type = " + (reference_type.empty() ? type.getAsString(policy_) : reference_type) +
                "; alignas(type) unsigned char data[sizeof(type)]; bool engaged = false; "
                "type& get() { return *std::launder(reinterpret_cast<type*>(data)); } "
                "void reset() { if (engaged) { engaged = false; std::destroy_at(&get()); } } "
                "~" + storage + "() { reset(); } }; " + storage + " " + name + ";");
            Slot slot{name, name + ".get()", storage + "::type", false, type->isArrayType(), retained_object};
            slots_.push_back(slot);
            return slot;
        }
        // NOTE(port): A synthesized frame has a different declaration context;
        // preserve namespaces on concrete value types and their template arguments.
        // NOTE(port): The spill keeps the declared C++ cv-qualification. Removing
        // it changes overload resolution and decltype when the body is reparsed.
        std::string value_type = reference_type.empty() ? TypeName::getFullyQualifiedName(type, context_, policy_) : reference_type;
        if (!reference_type.empty() && type.isConstQualified())
            value_type = "std::add_const_t<" + value_type + ">";
        if (!reference_type.empty() && type.isVolatileQualified())
            value_type = "std::add_volatile_t<" + value_type + ">";
        std::string stored_type = value_type;
        Slot slot{name, "(*" + name + ")", stored_type, false};
        fields_.push_back("std::optional<" + stored_type + "> " + name + ";");
        slots_.push_back(slot);
        return slot;
    }
    // NOTE(port): A dependent receiver's value category is resolved only at
    // instantiation. Reference results borrow; value results retain ownership.
    Slot new_dependent_receiver(const Expr* expression) {
        std::string name = "_kxs_slot_" + std::to_string(slots_.size());
        std::string storage = name + "_receiver";
        std::string category = "decltype(" + rewrite(expression, {}, true) + ")";
        fields_.push_back("struct " + storage + " { using expression_type = " + category +
            "; using type = std::remove_reference_t<expression_type>; "
            "type* borrowed = nullptr; alignas(type) unsigned char data[sizeof(type)]; bool engaged = false; "
            "type& get() { if constexpr (std::is_reference_v<expression_type>) return *borrowed; "
            "else return *std::launder(reinterpret_cast<type*>(data)); } "
            "void bind(type& source) { borrowed = std::addressof(source); } "
            "void bind(type&& source) { borrowed = std::addressof(source); } "
            "void reset() { if constexpr (!std::is_reference_v<expression_type>) { "
            "if (engaged) { engaged = false; std::destroy_at(&get()); } } borrowed = nullptr; } "
            "~" + storage + "() { reset(); } }; " + storage + " " + name + ";");
        Slot slot{name, name + ".get()", "typename " + storage + "::type", false};
        slot.dynamic = true;
        slots_.push_back(slot);
        return slot;
    }
    void construct(const Slot& slot, const std::string& value) {
        if (slot.dynamic) {
            body_ << "if constexpr (std::is_reference_v<typename " << slot.name << "_receiver::expression_type>) {\n"
                  << slot.name << ".bind(" << value << ");\n} else {\n"
                  << "::new (static_cast<void*>(" << slot.name << ".data)) " << slot.type << "(" << value << ");\n"
                  << slot.name << ".engaged = true;\n}\n";
            return;
        }
        if (slot.reference) {
            body_ << slot.name << ".bind(" << value << ");\n";
            return;
        }
        if (slot.array || slot.object) {
            // NOTE(port): Raw aligned storage preserves delayed construction,
            // native array reference types and partial-initialization cleanup.
            body_ << "::new (static_cast<void*>(" << slot.name << ".data)) " << slot.type
                  << (slot.object ? "(" + value + ")" : value) << ";\n"
                  << slot.name << ".engaged = true;\n";
            return;
        }
        body_ << slot.name << ".emplace(" << value << ");\n";
    }
    std::string capture(const Expr* expression, const std::string& value, bool reference = false) {
        auto type = expression->getType();
        std::string deduced_type;
        if (type->isDependentType())
            deduced_type = "std::remove_reference_t<decltype(" + rewrite(expression, {}, true) + ")>";
        auto slot = new_slot(type, reference, false, deduced_type);
        construct(slot, value);
        return slot.access;
    }
    const Expr* spelled(const Expr* expression) const {
        return expression->IgnoreUnlessSpelledInSource();
    }
    // NOTE(port): Clang's implicit iterator and decomposition operations have no independent
    // source tokens. Print their AST with retained variable references.
    // NOTE(port): C++ AST printing support; no direct Kotlin counterpart.
    std::string generated_expression(const Expr* expression) const {
        class References : public PrinterHelper {
        public:
            References(const std::map<const ValueDecl*, Slot>& variables, const PrintingPolicy& policy)
                : variables_(variables), policy_(policy) {}
            bool handledStmt(Stmt* statement, llvm::raw_ostream& output) override {
                // NOTE(port): Implicit decomposition casts select get() && or
                // the rvalue tuple overload. The pretty-printer otherwise
                // omits this cast and changes overload resolution to lvalue.
                if (const auto* cast = dyn_cast<ImplicitCastExpr>(statement);
                    cast && cast->isXValue() && cast->getSubExpr()->isLValue()) {
                    output << "std::move(";
                    cast->getSubExpr()->printPretty(output, this, policy_);
                    output << ")";
                    return true;
                }
                auto* reference = dyn_cast<DeclRefExpr>(statement);
                if (!reference) return false;
                auto* variable = dyn_cast<ValueDecl>(reference->getDecl());
                auto found = variables_.find(variable);
                if (found == variables_.end()) return false;
                output << found->second.access;
                return true;
            }
        private:
            const std::map<const ValueDecl*, Slot>& variables_;
            const PrintingPolicy& policy_;
        } references(variables_, policy_);
        if (has_suspend_calls(expression))
            throw std::runtime_error("implicit compiler expression suspension requires expression lowering");
        std::string text;
        llvm::raw_string_ostream output(text);
        expression->printPretty(output, &references, policy_);
        return text;
    }
    // NOTE(port): Resolve the compiler-produced callable class before allocating
    // any retained closure storage; a diagnostic anonymous type is not C++ syntax.
    std::string build_reference_class(const LambdaExpr* reference) {
        const auto* record = reference->getLambdaClass()->getCanonicalDecl();
        auto found = reference_types_.find(record);
        if (found != reference_types_.end()) return found->second;
        const std::string name = "_kxs_reference_" + std::to_string(reference_types_.size());
        org::jetbrains::kotlin::backend::common::lower::AbstractFunctionReferenceLowering lowering;
        reference_classes_.push_back(lowering.build_class(context_, reference, name));
        reference_types_[record] = name;
        return name;
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:197-335
    std::string emit_expression(const Expr* original) {
        if (!original) return "";
        // NOTE(port): An implicit overloaded arrow has no complete standalone
        // source expression. Preserve the resolved operator call and its result
        // rather than extracting the syntactic receiver plus the arrow token.
        if (const auto* arrow = dyn_cast<CXXOperatorCallExpr>(original->IgnoreParenImpCasts());
            arrow && arrow->getOperator() == OO_Arrow)
            return "(" + emit_expression(arrow->getArg(0)) + ").operator->()";
        const Expr* expression = spelled(original);
        if (const auto* reference = dyn_cast<LambdaExpr>(expression)) {
            const auto type = build_reference_class(reference);
            std::string arguments;
            auto bound = reference->getLambdaClass()->captures_begin();
            for (const auto* initializer : reference->capture_inits()) {
                if (!arguments.empty()) arguments += ", ";
                const bool borrowed = bound->getCaptureKind() == LCK_ByRef;
                const bool array_value = initializer->getType()->isArrayType();
                // NOTE(port): Clang's copied-this initializer has an implicit
                // dereference whose source range spells only `this`. Preserve
                // the actual object value when constructing the bound field.
                auto value = bound->getCaptureKind() == LCK_StarThis
                    ? "*(" + (receiver_.empty() ? std::string("this") : receiver_) + ")"
                    : emit_expression(initializer);
                if (array_value && bound->capturesVariable()) {
                    // NOTE(port): Array capture initializers are compiler-built
                    // element lists whose source range only names the array.
                    // Resolve the original captured declaration to its retained
                    // storage rather than reprinting that implicit list.
                    value = variables_.at(bound->getCapturedVar()).access;
                }
                value = capture(initializer, value, borrowed || array_value);
                if (!borrowed && !array_value) value = "std::move(" + value + ")";
                arguments += value;
                ++bound;
            }
            return type + "(" + arguments + ")";
        }
        if (isa<CXXThisExpr>(expression) && !receiver_.empty()) return receiver_;
        if (const auto* binary = dyn_cast<BinaryOperator>(expression);
            binary && binary->getOpcode() == BO_Comma && (full_expression_ || has_suspend_calls(expression))) {
            auto left = emit_expression(binary->getLHS());
            if (binary->getLHS()->getType()->isRecordType() && binary->getLHS()->isPRValue()) {
                if (!full_expression_)
                    throw std::runtime_error("comma temporary lifetime lowering requires retained full-expression storage");
                auto retained = new_slot(binary->getLHS()->getType(), false, true);
                construct(retained, left);
                comma_temporaries_.push_back(retained);
                left = retained.access;
            }
            body_ << "static_cast<void>(" << left << ");\n";
            return emit_expression(binary->getRHS());
        }
        if (!has_suspend_calls(expression)) return rewrite(original);
        if (const auto* construction = dyn_cast<CXXConstructExpr>(expression)) {
            auto values = slice_constructor_arguments(construction);
            std::vector<Replacement> replacements;
            std::string defaults;
            const Expr* last_explicit_argument = nullptr;
            for (unsigned index = 0; index < construction->getNumArgs(); ++index) {
                if (values[index].empty()) continue;
                const auto* argument = construction->getArg(index);
                if (isa<CXXDefaultArgExpr>(argument)) {
                    if (!defaults.empty()) defaults += ", ";
                    defaults += values[index];
                    continue;
                }
                last_explicit_argument = argument;
                replacements.push_back({offset(argument->getBeginLoc()),
                    end_offset(argument->getEndLoc()), values[index]});
            }
            if (!defaults.empty()) {
                const auto insertion = last_explicit_argument ? end_offset(last_explicit_argument->getEndLoc()) :
                    offset(construction->getEndLoc());
                replacements.push_back({insertion, insertion, (last_explicit_argument ? ", " : "") + defaults});
            }
            return rewrite(expression, replacements);
        }
        if (const auto* call = dyn_cast<CallExpr>(expression)) {
            if (SuspendFunctionAnalyzer::is_suspend_wrapper(call) &&
                call->getNumArgs() == 1) return emit_suspend_call(spelled(call->getArg(0)));
            if (SuspendFunctionAnalyzer::is_suspend_call(call)) return emit_suspend_call(call);
            return emit_call(call);
        }
        if (const auto* binary = dyn_cast<BinaryOperator>(expression)) {
            if (binary->isLogicalOp()) {
                auto result = new_slot(context_.BoolTy);
                auto left = emit_expression(binary->getLHS());
                body_ << "if (" << (binary->getOpcode() == BO_LOr ? "!(" : "(") << left << ")) {\n";
                construct(result, emit_expression(binary->getRHS()));
                body_ << "} else {\n";
                construct(result, binary->getOpcode() == BO_LOr ? "true" : "false");
                body_ << "}\n";
                return result.access;
            }
            if (binary->isAssignmentOp()) {
                // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:212-250,300-304
                // Save to a temporary to preserve receiver-before-value order.
                // NOTE(port): Bind the C++ destination by reference; Kotlin's
                // IrSetField retains the receiver rather than a field address.
                const auto* destination = binary->getLHS();
                std::string left;
                if (const auto* member = dyn_cast<MemberExpr>(spelled(destination));
                    member && !member->isArrow() && spelled(member->getBase())->isPRValue() &&
                    member->getBase()->getType()->isRecordType()) {
                    // NOTE(port): A temporary C++ receiver needs owning storage
                    // until this full expression completes, including suspension.
                    const auto* receiver = member->getBase();
                    auto retained = new_slot(receiver->getType(), false, true);
                    construct(retained, emit_expression(receiver));
                    comma_temporaries_.push_back(retained);
                    left = rewrite(destination, {{offset(receiver->getBeginLoc()),
                        end_offset(receiver->getEndLoc()), retained.access}});
                } else {
                    left = emit_expression(destination);
                }
                left = capture(destination, left, true);
                auto right = emit_expression(binary->getRHS());
                return "(" + left + ") " + binary->getOpcodeStr().str() + " (" + right + ")";
            }
        }
        if (const auto* subscript = dyn_cast<ArraySubscriptExpr>(expression)) {
            auto left = capture(subscript->getLHS(), emit_expression(subscript->getLHS()));
            auto right = capture(subscript->getRHS(), emit_expression(subscript->getRHS()));
            return "(" + left + ")[" + right + "]";
        }
        if (const auto* conditional = dyn_cast<ConditionalOperator>(expression)) {
            bool no_value = conditional->getType()->isVoidType();
            Slot result;
            if (!no_value) result = new_slot(conditional->getType(), conditional->isGLValue());
            auto condition = emit_expression(conditional->getCond());
            body_ << "if (" << condition << ") {\n";
            auto selected = emit_expression(conditional->getTrueExpr());
            if (no_value) body_ << selected << ";\n";
            else construct(result, selected);
            body_ << "} else {\n";
            selected = emit_expression(conditional->getFalseExpr());
            if (no_value) body_ << selected << ";\n";
            else construct(result, selected);
            body_ << "}\n";
            if (no_value) return "static_cast<void>(0)";
            return conditional->isXValue() ? "std::move(" + result.access + ")" : result.access;
        }
        std::vector<Replacement> replacements;
        for (const auto* child : expression->children()) {
            if (const auto* operand = dyn_cast_or_null<Expr>(child)) {
                std::string lowered = emit_expression(operand);
                if (!operand->getType()->isVoidType()) lowered = capture(operand, lowered);
                replacements.push_back({offset(operand->getBeginLoc()), end_offset(operand->getEndLoc()), lowered});
            }
        }
        return rewrite(expression, replacements);
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:400-408
    bool is_pure(const Expr* expression) const {
        expression = expression->IgnoreParens();
        if (isa<IntegerLiteral, FloatingLiteral, CharacterLiteral, StringLiteral,
                CXXBoolLiteralExpr, CXXNullPtrLiteralExpr, GNUNullExpr>(expression)) return true;
        if (isa<CallExpr>(expression)) return false;
        // NOTE(port): Implicit Clang conversions represent non-checking type
        // operations. Explicit/checked casts remain impure; volatile loads
        // remain observable even when their declared value is const.
        if (const auto* operation = dyn_cast<ImplicitCastExpr>(expression))
            return !operation->getSubExpr()->getType().isVolatileQualified() && is_pure(operation->getSubExpr());
        if (const auto* value = dyn_cast<DeclRefExpr>(expression)) {
            const auto* variable = dyn_cast<VarDecl>(value->getDecl());
            // NOTE(port): Kotlin value parameters are immutable. C++ ordinary
            // mutable parameters retain conservative temporary storage unless
            // const qualifies the actual declaration.
            return variable && variable->getType().isConstQualified() && !variable->getType().isVolatileQualified();
        }
        return false;
    }

    // NOTE(port): Clang constructor operands represent Kotlin constructor-call children.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:215-250
    std::vector<std::string> slice_constructor_arguments(const CXXConstructExpr* construction) {
        // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:215-250
        // No constructor argument is first: Kotlin inserts the allocated
        // instance before these arguments when lowering the constructor.
        const auto number_of_children = construction->getNumArgs();
        std::vector<bool> has_suspend_call_in_tail(number_of_children + 1, false);
        std::vector<bool> has_impure_child_in_tail(number_of_children + 1, false);
        for (unsigned i = number_of_children; i > 0; --i) {
            const auto* child = construction->getArg(i - 1);
            has_suspend_call_in_tail[i - 1] = has_suspend_call_in_tail[i] || has_suspend_calls(child);
            // NOTE(port): Preserve Kotlin evaluation order despite C++
            // constructor arguments having no corresponding ordering.
            has_impure_child_in_tail[i - 1] = has_impure_child_in_tail[i] || !is_pure(child);
        }
        std::vector<std::string> values(number_of_children);
        for (unsigned index = 0; index < number_of_children; ++index) {
            const Expr* argument = construction->getArg(index);
            if (const auto* omitted = dyn_cast<CXXDefaultArgExpr>(argument)) {
                // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/DefaultArgumentStubGenerator.kt:91-117
                // Resolve omitted parameter values in order before invoking the constructor.
                // NOTE(port): Clang supplies the selected default expression rather than Kotlin's mask.
                const auto* expression = omitted->getExpr();
                const auto parameter_type = construction->getConstructor()->getParamDecl(index)->getType();
                const bool reference = expression->isGLValue() && spelled(expression)->isGLValue() &&
                    parameter_type->isReferenceType();
                auto value = capture(expression, SuspendFunctionAnalyzer::default_argument(omitted, policy_), reference);
                if (!expression->isLValue()) value = "std::move(" + value + ")";
                values[index] = value;
                continue;
            }
            auto value = emit_expression(argument);
            if (!is_pure(argument) && (has_suspend_call_in_tail[index] ||
                has_impure_child_in_tail[index + 1] || argument->getType()->isRecordType())) {
                const auto* constructor = construction->getConstructor();
                const auto parameter_type = index < constructor->getNumParams() ?
                    constructor->getParamDecl(index)->getType() : QualType();
                const bool reference = argument->isGLValue() && spelled(argument)->isGLValue() &&
                    !parameter_type.isNull() && parameter_type->isReferenceType();
                value = capture(argument, value, reference);
                if (!argument->isLValue()) value = "std::move(" + value + ")";
            }
            values[index] = value;
        }
        return values;
    }

    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:201-252
    // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/coroutines/AddContinuationToFunctionCallsLowering.kt:74-99
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeAddContinuationToFunctionCallsLowering.kt:15-22
    // Save to temporary in order to save execution order.
    // NOTE(port): Retained C++ arguments additionally preserve value category.
    std::string emit_call(const CallExpr* call) {
        std::vector<Replacement> replacements;
        const Expr* receiver = nullptr;
        const auto* callee_expression = call->getCallee()->IgnoreParenImpCasts();
        if (const auto* member = dyn_cast<MemberExpr>(callee_expression); member && !member->isImplicitAccess())
            receiver = member->getBase();
        else if (const auto* member = dyn_cast<CXXDependentScopeMemberExpr>(callee_expression);
                 member && !member->isImplicitAccess()) receiver = member->getBase();
        else if (const auto* member = dyn_cast<UnresolvedMemberExpr>(callee_expression);
                 member && !member->isImplicitAccess()) receiver = member->getBase();
        // NOTE(port): Clang stores a non-static operator's dispatch receiver as
        // argument zero. It precedes regular parameters but is not parameter zero.
        // Preserve Kotlin's receiver-before-arguments order and borrow an lvalue.
        const auto* method = dyn_cast_or_null<CXXMethodDecl>(call->getDirectCallee());
        const bool operator_receiver = isa<CXXOperatorCallExpr>(call) && method && !method->isStatic();
        if (operator_receiver) receiver = call->getArg(0);
        if (receiver) {
            auto value = emit_expression(receiver);
            if (receiver->getType()->isDependentType()) {
                if (!full_expression_) throw std::runtime_error("dependent receiver requires retained full-expression storage");
                auto retained = new_dependent_receiver(receiver);
                construct(retained, value);
                comma_temporaries_.push_back(retained);
                value = "std::forward<typename " + retained.name + "_receiver::expression_type>(" + retained.access + ")";
            } else if (receiver->getType()->isRecordType() && spelled(receiver)->isPRValue()) {
                if (!full_expression_) throw std::runtime_error("temporary receiver requires retained full-expression storage");
                auto retained = new_slot(receiver->getType(), false, true);
                construct(retained, value);
                comma_temporaries_.push_back(retained);
                value = "std::move(" + retained.access + ")";
            } else {
                value = capture(receiver, value, receiver->getType()->isRecordType());
                if (receiver->isXValue()) value = "std::move(" + value + ")";
            }
            unsigned receiver_end = end_offset(receiver->getEndLoc());
            if (const auto* arrow = dyn_cast<CXXOperatorCallExpr>(receiver->IgnoreParenImpCasts());
                arrow && arrow->getOperator() == OO_Arrow) {
                const auto* member = cast<MemberExpr>(callee_expression);
                receiver_end = offset(member->getOperatorLoc());
            }
            replacements.push_back({offset(receiver->getBeginLoc()), receiver_end, value});
        }
        // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:212-252
        // NOTE(port): An indirect C++ callee is evaluated before its arguments.
        // Retain its value before nested argument suspension, just as Kotlin's
        // member-access receiver precedes the remaining expression children.
        const auto callee_type = call->getCallee()->getType();
        if (!receiver && !call->getDirectCallee() && callee_type->isPointerType() &&
            callee_type->getPointeeType()->isFunctionType()) {
            auto value = capture(call->getCallee(), emit_expression(call->getCallee()));
            replacements.push_back({offset(callee_expression->getBeginLoc()),
                end_offset(callee_expression->getEndLoc()), value});
        }
        unsigned index = 0;
        std::vector<std::string> defaults;
        // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:215-250
        const auto number_of_children = call->getNumArgs();
        std::vector<bool> has_suspend_call_in_tail(number_of_children + 1, false);
        std::vector<bool> has_impure_child_in_tail(number_of_children + 1, false);
        for (unsigned i = number_of_children; i > 0; --i) {
            const auto* child = call->getArg(i - 1);
            has_suspend_call_in_tail[i - 1] = has_suspend_call_in_tail[i] || has_suspend_calls(child);
            // NOTE(port): C++ does not specify Kotlin's argument evaluation
            // order. Earlier impure children retain order before later impure
            // children even when that suffix contains no suspension.
            has_impure_child_in_tail[i - 1] = has_impure_child_in_tail[i] || !is_pure(child);
        }
        bool first = receiver == nullptr;
        for (unsigned child_index = 0; child_index < number_of_children; ++child_index) {
            const Expr* argument = call->getArg(child_index);
            if (operator_receiver && argument == receiver) continue;
            const bool first_only_suspend = first && !has_suspend_call_in_tail[child_index + 1];
            first = false;
            if (const auto* omitted = dyn_cast<CXXDefaultArgExpr>(argument)) {
                if (const auto* callee = call->getDirectCallee()) {
                    for (const auto* attribute : callee->attrs()) {
                        const auto* annotation = dyn_cast<AnnotateAttr>(attribute);
                        if (annotation && annotation->getAnnotation() == "kxs_implicit_continuation") {
                            // Transliterated from: compiler/ir/backend.common/src/org/jetbrains/kotlin/backend/common/lower/DefaultArgumentStubGenerator.kt:91-130
                            // Evaluate omitted values before saving the suspend address.
                            const auto* expression = omitted->getExpr();
                            const bool reference = expression->isGLValue() && spelled(expression)->isGLValue() &&
                                callee->getParamDecl(index)->getType()->isReferenceType();
                            auto value = capture(expression, SuspendFunctionAnalyzer::default_argument(omitted, policy_), reference);
                            if (!expression->isLValue()) value = "std::move(" + value + ")";
                            defaults.push_back(value);
                            break;
                        }
                    }
                }
                ++index;
                continue;
            }
            const auto* callee = call->getDirectCallee();
            QualType parameter_type;
            if (callee && index < callee->getNumParams())
                parameter_type = callee->getParamDecl(index)->getType();
            else if (!callee && callee_type->isPointerType()) {
                if (const auto* prototype = callee_type->getPointeeType()->getAs<FunctionProtoType>();
                    prototype && index < prototype->getNumParams())
                    parameter_type = prototype->getParamType(index);
            }
            // NOTE(port): Both lvalues and xvalues can bind reference parameters.
            // Preserve the referent identity instead of moving it into a value slot.
            // A materialized prvalue still needs owning storage; only an
            // explicitly spelled glvalue denotes existing borrowed storage.
            bool reference = argument->isGLValue() && spelled(argument)->isGLValue() &&
                (!parameter_type.isNull() ? parameter_type->isReferenceType() : !callee);
            auto value = emit_expression(argument);
            const auto* variable = dyn_cast<DeclRefExpr>(spelled(argument));
            bool current_frame = variable && variable->getDecl() == completion_;
            // NOTE(port): Suspend-call operands precede the saved address.
            // C++ constructed/dependent values retain their existing lifetime
            // storage; scalar trailing operands can remain in the final call.
            const bool retain = SuspendFunctionAnalyzer::is_suspend_call(call) ||
                (!first_only_suspend && has_suspend_call_in_tail[child_index]) ||
                has_impure_child_in_tail[child_index + 1] ||
                argument->getType()->isRecordType() || argument->getType()->isDependentType();
            if (!argument->getType()->isVoidType() && !current_frame && !is_pure(argument) && retain) {
                value = capture(argument, value, reference);
                if (!argument->isLValue()) value = "std::move(" + value + ")";
            }
            replacements.push_back({offset(argument->getBeginLoc()), end_offset(argument->getEndLoc()), value});
            ++index;
        }
        // NOTE(port): Kotlin supplies the current continuation implicitly.
        // An annotated authoring intrinsic selects the matching ABI overload
        // with this frame, using the same ownership as an explicit completion.
        if (const auto* callee = call->getDirectCallee()) {
            for (const auto* attribute : callee->attrs()) {
                const auto* annotation = dyn_cast<AnnotateAttr>(attribute);
                if (annotation && annotation->getAnnotation() == "kxs_implicit_continuation") {
                    const unsigned insertion = offset(call->getRParenLoc());
                    replacements.push_back({insertion, insertion,
                        SuspendFunctionAnalyzer::continuation_arguments(call, variables_.at(completion_).access, policy_, defaults)});
                    break;
                }
            }
        }
        return rewrite(call, replacements);
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:254-335
    // NOTE(port): Locals already occupy persistent frame fields; the injected
    // address stores the resume location before the call. Resume reads those
    // fields before extracting the resumed result, as Kotlin restores state.
    std::string emit_suspend_call(const Expr* expression) {
        const auto* call = dyn_cast<CallExpr>(expression);
        if (!call) throw std::runtime_error("suspend expression must call a continuation entry");
        size_t first_argument = slots_.size();
        std::string invoked = emit_call(call);
        size_t argument_end = slots_.size();
        unsigned id = suspension_++;
        auto result = new_slot(context_.VoidPtrTy);
        // Save state as late as possible.
        body_ << suspension_site(id, "_kxs_resume_")
              << result.name << ".emplace(" << invoked << ");\n";
        // NOTE(port): The callee has returned, so C++ argument temporaries end
        // their lifetime even when its result is COROUTINE_SUSPENDED.
        for (size_t i = argument_end; i > first_argument; --i) {
            const auto& slot = slots_[i - 1];
            if (std::any_of(comma_temporaries_.begin(), comma_temporaries_.end(),
                            [&](const Slot& retained) { return retained.name == slot.name; })) continue;
            body_ << slot.name << ".reset();\n";
        }
        body_ << "if (kotlin::coroutines::intrinsics::is_coroutine_suspended(" << result.access
              << ")) return " << result.access << ";\n"
              << "goto _kxs_continue_" << id << ";\n_kxs_resume_" << id << ":\n"
              << result.name << ".emplace(_kxs_result.get_or_throw());\n_kxs_continue_" << id << ":;\n";
        return result.access;
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/CoroutinesVarSpillingLowering.kt:49-105
    // NOTE(port): C++ declarations additionally preserve static initialization,
    // alias bindings and object construction in their original storage category.
    void emit_declaration(const DeclStmt* statement, bool generated = false) {
        for (const Decl* declaration : statement->decls()) {
            if (const auto* alias = dyn_cast<TypedefNameDecl>(declaration)) {
                // NOTE(port): Bind the original alias declaration to a unique
                // frame spelling; an alias has no initialization or cleanup.
                std::string name = "_kxs_type_" + std::to_string(local_aliases_.size());
                local_aliases_.emplace(alias, name);
                fields_.push_back("using " + name + " = " + TypeName::getFullyQualifiedName(
                    alias->getUnderlyingType().getCanonicalType(), context_, policy_) + ";");
                continue;
            }
            const auto* variable = dyn_cast<VarDecl>(declaration);
            if (!variable) throw std::runtime_error("suspend local declaration is not a variable");
            if (variable->isStaticLocal()) {
                // NOTE(port): Storage duration belongs to the whole C++
                // declaration statement. Emit a grouped static declaration once.
                // Its initializer still reads the source binding map: parameters,
                // automatic locals and the receiver now reside in this frame.
                // Keep the actual static declaration and its initialization guard.
                body_ << rewrite(statement) << ";\n";
                return;
            }
            emit_variable(variable, generated);
        }
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/CoroutinesVarSpillingLowering.kt:49-105
    // NOTE(port): Retain one actual C++ variable, including implicit tuple
    // holding variables, using the same construction and destruction rules.
    void emit_variable(const VarDecl* variable, bool generated) {
        llvm::SaveAndRestore<bool> expression_scope(full_expression_, true);
        size_t first_comma = comma_temporaries_.size();
        const Expr* initializer = variable->getInit();
        const Expr* unwrapped = initializer;
        if (unwrapped) {
            if (const auto* cleanup = dyn_cast<ExprWithCleanups>(unwrapped)) unwrapped = cleanup->getSubExpr();
            unwrapped = unwrapped->IgnoreParens();
            while (const auto* cast = dyn_cast<ImplicitCastExpr>(unwrapped))
                unwrapped = cast->getSubExpr()->IgnoreParens();
        }
        const auto* temporary = dyn_cast_or_null<MaterializeTemporaryExpr>(unwrapped);
        if (const auto* reference = initializer ? dyn_cast<LambdaExpr>(spelled(initializer)) : nullptr)
            build_reference_class(reference);
        Slot owner;
        if (variable->getType()->isReferenceType() && temporary && temporary->getExtendingDecl() == variable) {
            owner = new_slot(temporary->getSubExpr()->getType(), false, true);
            if (!scopes_.empty()) scopes_.back().push_back(owner);
            construct(owner, generated ? generated_expression(temporary->getSubExpr()) :
                                         emit_expression(temporary->getSubExpr()));
        }
        std::string deduced_type;
        if (auto* placeholder = variable->getType()->getContainedAutoType();
            placeholder && variable->getType()->isDependentType()) {
            if (!initializer) throw std::runtime_error("deduced suspend local requires an initializer");
            // NOTE(port): Preserve C++ placeholder deduction in an unevaluated
            // generic lambda. Clang resolves the concrete field type when the
            // surrounding coroutine entry is instantiated.
            std::string expression_type;
            if (placeholder->isDecltypeAuto()) {
                expression_type = "decltype(" + rewrite(initializer, {}, true) + ")";
                // An unparenthesized id uses its declared type, rather than
                // the reference category of the unevaluated stand-in.
                if (const auto* id = dyn_cast<DeclRefExpr>(initializer->IgnoreImpCasts())) {
                    if (const auto* original = dyn_cast<VarDecl>(id->getDecl())) {
                        auto found = variables_.find(original);
                        if (found != variables_.end() && !found->second.type.empty())
                            expression_type = found->second.type +
                                (found->second.reference ? (original->getType()->isRValueReferenceType() ? "&&" : "&") : "");
                        else expression_type = original->getType().getAsString(policy_);
                    }
                }
            }
            else expression_type = "decltype([](" + this->declaration(variable->getType(), "_kxs_deduced") +
                ") -> decltype(_kxs_deduced) { return std::forward<decltype(_kxs_deduced)>(_kxs_deduced); }(" +
                rewrite(initializer, {}, true) + "))";
            deduced_type = "std::remove_reference_t<" + expression_type + ">";
        }
        bool reference = variable->getType()->isReferenceType();
        if (auto* placeholder = variable->getType()->getContainedAutoType();
            placeholder && placeholder->isDecltypeAuto() && variable->getType()->isDependentType())
            {
                reference = initializer->isGLValue();
                if (const auto* id = dyn_cast<DeclRefExpr>(initializer->IgnoreImpCasts()))
                    reference = id->getDecl()->getType()->isReferenceType();
            }
        auto slot = new_slot(variable->getType(), reference, false, deduced_type);
        variables_[variable] = slot;
        if (!scopes_.empty()) scopes_.back().push_back(slot);
        if (!owner.name.empty()) construct(slot, owner.access);
        else if (generated) construct(slot, generated_expression(initializer));
        else if (slot.array) {
            auto value = initializer ? emit_expression(initializer) : "";
            if (initializer && isa<StringLiteral>(spelled(initializer))) value = "{" + value + "}";
            construct(slot, value);
        }
        else if (!initializer) body_ << slot.name << ".emplace();\n";
        else if (const auto* construction = dyn_cast<CXXConstructExpr>(spelled(initializer))) {
            std::string values;
            for (const auto& value : slice_constructor_arguments(construction)) {
                if (value.empty()) continue;
                if (!values.empty()) values += ", ";
                values += value;
            }
            body_ << slot.name << ".emplace(" << values << ");\n";
        } else construct(slot, emit_expression(initializer));
        clear_comma_temporaries(first_comma);
        if (const auto* decomposition = dyn_cast<DecompositionDecl>(variable)) {
            // NOTE(port): Clang has already selected array, member or tuple
            // decomposition. Follow its bound declarations, not a new
            // std::get protocol or copies of component objects.
            for (const auto* binding : decomposition->flat_bindings()) {
                const auto* expression = binding->getBinding();
                if (!expression)
                    throw std::runtime_error("structured binding requires its resolved compiler binding");
                if (const auto* holding = binding->getHoldingVar()) emit_variable(holding, true);
                // Member/array bindings are native lvalues, including bit
                // fields; they must not be replaced by pointer storage.
                Slot component{"", "(" + generated_expression(expression) + ")",
                    binding->getType().getCanonicalType().getAsString(policy_), false};
                variables_[binding] = component;
            }
        }
    }
    // NOTE(port): Destroy C++ discarded comma operands at their enclosing
    // full-expression boundary, in reverse construction order.
    void clear_comma_temporaries(size_t first) {
        for (size_t i = comma_temporaries_.size(); i > first; --i)
            body_ << comma_temporaries_[i - 1].name << ".reset();\n";
        comma_temporaries_.resize(first);
    }
    // NOTE(port): Evaluate the control value before ending C++ comma temporary
    // lifetimes. The retained value remains available after native resumption.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:197-335
    std::string emit_condition(const Expr* expression, bool truth = true) {
        llvm::SaveAndRestore<bool> expression_scope(full_expression_, true);
        size_t first_comma = comma_temporaries_.size();
        auto result = new_slot(truth ? context_.BoolTy : expression->getType());
        auto value = emit_expression(expression);
        construct(result, truth ? "static_cast<bool>(" + value + ")" : value);
        clear_comma_temporaries(first_comma);
        return result.access;
    }
    // NOTE(port): Restore C++ handler context between nested scope exits.
    void clear_scope(size_t first) {
        for (size_t i = scopes_.size(); i > first; --i) {
            bool handler = false;
            for (auto it = scopes_[i - 1].rbegin(); it != scopes_[i - 1].rend(); ++it) {
                body_ << it->name << ".reset();\n";
                handler = handler || it->handler_exception;
            }
            if (handler) emit_context_transition();
        }
    }
    // NOTE(port): Destruct each handler's objects in its restored native
    // context before proceeding to surrounding handler/function storage.
    void emit_slot_cleanup(size_t first, size_t end) {
        for (size_t i = end; i > first; --i) {
            body_ << slots_[i - 1].name << ".reset();\n";
            if (slots_[i - 1].handler_exception) emit_context_transition();
        }
    }
    // NOTE(port): Return to the native catch-context wrapper at handler
    // boundaries. Resume still uses the same injected field/blockaddress pair.
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt:2309-2337
    // NOTE(port): A marker branch retains the actual labelled region in Clang's
    // CFG. Mandatory LLVM injection forms its blockaddress and erases the branch
    // condition; source parsing needs no GNU labels-as-values extension.
    std::string suspension_site(unsigned id, const std::string& label_prefix) const {
        const auto name = std::to_string(id);
        return "__kxs_suspend_site(" + name + ", &_label);\nif (__kxs_resume_point(" + name +
            ")) goto " + label_prefix + name + ";\n";
    }
    void emit_context_transition() {
        unsigned id = suspension_++;
        body_ << "if (_kxs_reenter) {\n" << suspension_site(id, "_kxs_context_")
              << "return nullptr;\n}\n"
              << "_kxs_context_" << id << ":;\n";
    }
    // NOTE(port): Leave native C++ catches before executing retained handlers.
    // An exception_ptr owns reference-caught exceptions across suspension.
    void emit_try(const CXXTryStmt* statement) {
        unsigned id = exception_region_++;
        std::string done = "_kxs_try_done_" + std::to_string(id);
        size_t first_protected = slots_.size();
        body_ << "try ";
        emit_statement(statement->getTryBlock());
        size_t protected_end = slots_.size();
        struct Handler { const CXXCatchStmt* source; std::string label; Slot exception; Slot variable; Slot context; Slot source_value; };
        std::vector<Handler> handlers;
        for (unsigned index = 0; index < statement->getNumHandlers(); ++index) {
            auto* handler = statement->getHandler(index);
            std::string name = "_kxs_exception_" + std::to_string(id) + "_" + std::to_string(index);
            fields_.push_back("struct " + name + "_storage { std::exception_ptr value; "
                              "void reset() { value = {}; } }; " + name + "_storage " + name + ";");
            Slot exception{name, name + ".value", "std::exception_ptr", false, false, false, true};
            slots_.push_back(exception);
            Slot variable;
            Slot source_value;
            auto* caught = handler->getExceptionDecl();
            if (caught) {
                auto type = caught->getType();
                variable = new_slot(type, type->isReferenceType(), !type->isReferenceType());
                if (!type->isReferenceType() && !type->isPointerType())
                    source_value = new_slot(type.getUnqualifiedType(), true);
                variables_[caught] = variable;
            }
            std::string context_name = name + "_context";
            fields_.push_back("struct " + context_name + "_storage { std::exception_ptr* active; std::exception_ptr* retired; bool* reenter; "
                "std::exception_ptr previous{}; bool engaged = false; "
                "void enter(std::exception_ptr exception) { previous = *active; *active = std::move(exception); "
                "engaged = true; *reenter = true; } "
                "void reset() { if (engaged) { engaged = false; *retired = std::move(*active); "
                "*active = std::move(previous); *reenter = true; } } "
                "}; " + context_name + "_storage " + context_name + "{&_kxs_active_exception, &_kxs_retired_exception, &_kxs_reenter};");
            Slot handler_context{context_name, context_name, "", false};
            slots_.push_back(handler_context);
            std::string label = "_kxs_handler_" + std::to_string(id) + "_" + std::to_string(index);
            auto native_type = caught ? caught->getType() : QualType();
            if (caught && !native_type->isReferenceType())
                native_type = context_.getLValueReferenceType(native_type.getUnqualifiedType());
            body_ << "catch (" << (caught ? declaration(native_type, caught->getNameAsString()) : "...") << ") {\n"
                  << exception.access << " = std::current_exception();\n";
            if (caught) {
                construct(source_value.name.empty() ? variable : source_value, caught->getNameAsString());
            }
            body_ << "goto " << label << ";\n}\n";
            handlers.push_back({handler, label, exception, variable, handler_context, source_value});
        }
        body_ << "goto " << done << ";\n";
        for (const auto& handler : handlers) {
            body_ << handler.label << ": {\n";
            scopes_.emplace_back();
            scopes_.back().push_back(handler.exception);
            if (!handler.variable.name.empty()) scopes_.back().push_back(handler.variable);
            scopes_.back().push_back(handler.context);
            emit_slot_cleanup(first_protected, protected_end);
            body_ << handler.context.name << ".enter(" << handler.exception.access << ");\n";
            emit_context_transition();
            if (!handler.source_value.name.empty()) {
                // NOTE(port): A throwing catch-variable copy terminates.
                body_ << "try {\n";
                construct(handler.variable, handler.source_value.access);
                body_ << "} catch (...) { std::terminate(); }\n" << handler.source_value.name << ".reset();\n";
            }
            emit_statement(handler.source->getHandlerBlock());
            clear_scope(scopes_.size() - 1);
            scopes_.pop_back();
            emit_context_transition();
            body_ << "goto " << done << ";\n}\n";
        }
        body_ << done << ":;\n";
    }
    // Transliterated from: kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/lower/NativeSuspendFunctionLowering.kt:119-170,197-335
    void emit_statement(const Stmt* statement) {
        if (!statement) return;
        if (const auto* compound = dyn_cast<CompoundStmt>(statement)) {
            body_ << "{\n";
            scopes_.emplace_back();
            for (const Stmt* child : compound->body()) emit_statement(child);
            clear_scope(scopes_.size() - 1);
            scopes_.pop_back();
            body_ << "}\n";
        } else if (const auto* protected_region = dyn_cast<CXXTryStmt>(statement)) emit_try(protected_region);
        else if (const auto* declaration = dyn_cast<DeclStmt>(statement)) emit_declaration(declaration);
        else if (const auto* returned = dyn_cast<ReturnStmt>(statement)) {
            llvm::SaveAndRestore<bool> expression_scope(full_expression_, true);
            size_t first_comma = comma_temporaries_.size();
            auto value = emit_expression(returned->getRetValue());
            if (exception_region_) {
                auto outcome = new_slot(context_.VoidPtrTy);
                construct(outcome, value.empty() ? "nullptr" : value);
                clear_comma_temporaries(first_comma);
                clear_scope(0);
                body_ << "{ void* outcome = " << outcome.access << "; clear_locals(); return outcome; }\n";
            } else {
                body_ << "{ void* outcome = " << (value.empty() ? "nullptr" : value) << ";\n";
                clear_comma_temporaries(first_comma);
                body_ << "clear_locals(); return outcome; }\n";
            }
        } else if (const auto* branch = dyn_cast<IfStmt>(statement)) {
            body_ << "{\n";
            scopes_.emplace_back();
            emit_statement(branch->getInit());
            emit_statement(branch->getConditionVariableDeclStmt());
            auto condition = emit_condition(branch->getCond());
            body_ << "if (" << condition << ") {\n";
            emit_statement(branch->getThen());
            body_ << "}\n";
            if (branch->getElse()) {
                body_ << "else {\n";
                emit_statement(branch->getElse());
                body_ << "}\n";
            }
            clear_scope(scopes_.size() - 1);
            scopes_.pop_back();
            body_ << "}\n";
        } else if (const auto* loop = dyn_cast<CXXForRangeStmt>(statement)) {
            body_ << "{\n";
            scopes_.emplace_back();
            emit_statement(loop->getInit());
            emit_declaration(loop->getRangeStmt());
            emit_declaration(loop->getBeginStmt(), true);
            emit_declaration(loop->getEndStmt(), true);
            std::string next = "_kxs_next_" + std::to_string(loop_++);
            loops_.push_back({next, scopes_.size()});
            body_ << "while (" << generated_expression(loop->getCond()) << ") {\n";
            scopes_.emplace_back();
            emit_declaration(loop->getLoopVarStmt(), true);
            emit_statement(loop->getBody());
            clear_scope(scopes_.size() - 1);
            scopes_.pop_back();
            if (loops_.back().continued) body_ << next << ":;\n";
            body_ << generated_expression(loop->getInc()) << ";\n}\n";
            loops_.pop_back();
            clear_scope(scopes_.size() - 1);
            scopes_.pop_back();
            body_ << "}\n";
        } else if (const auto* loop = dyn_cast<ForStmt>(statement)) {
            body_ << "{\n";
            scopes_.emplace_back();
            emit_statement(loop->getInit());
            std::string next = "_kxs_next_" + std::to_string(loop_++);
            loops_.push_back({next, scopes_.size(), true, true});
            body_ << "while (true) {\n";
            scopes_.emplace_back();
            emit_statement(loop->getConditionVariableDeclStmt());
            if (loop->getCond()) {
                auto condition = emit_condition(loop->getCond());
                body_ << "if (!(" << condition << ")) {\n";
                clear_scope(scopes_.size() - 1);
                body_ << "break;\n}\n";
            }
            scopes_.emplace_back();
            emit_statement(loop->getBody());
            clear_scope(scopes_.size() - 1);
            scopes_.pop_back();
            // NOTE(port): This C++ increment target is needed only by an
            // authored continue. Ordinary iteration falls through to the increment.
            if (loops_.back().continued) body_ << next << ":;\n";
            emit_statement(loop->getInc());
            clear_scope(scopes_.size() - 1);
            scopes_.pop_back();
            body_ << "}\n";
            loops_.pop_back();
            clear_scope(scopes_.size() - 1);
            scopes_.pop_back();
            body_ << "}\n";
        } else if (const auto* loop = dyn_cast<WhileStmt>(statement)) {
            loops_.push_back({"", scopes_.size()});
            body_ << "while (true) {\n";
            scopes_.emplace_back();
            emit_statement(loop->getConditionVariableDeclStmt());
            auto condition = emit_condition(loop->getCond());
            body_ << "if (!(" << condition << ")) {\n";
            clear_scope(scopes_.size() - 1);
            body_ << "break;\n}\n";
            emit_statement(loop->getBody());
            clear_scope(scopes_.size() - 1);
            scopes_.pop_back();
            body_ << "}\n";
            loops_.pop_back();
        } else if (const auto* loop = dyn_cast<DoStmt>(statement)) {
            std::string next = "_kxs_next_" + std::to_string(loop_++);
            loops_.push_back({next, scopes_.size()});
            body_ << "while (true) {\n";
            emit_statement(loop->getBody());
            if (loops_.back().continued) body_ << next << ":;\n";
            auto condition = emit_condition(loop->getCond());
            body_ << "if (!(" << condition << ")) break;\n}\n";
            loops_.pop_back();
        } else if (const auto* branch = dyn_cast<SwitchStmt>(statement)) {
            body_ << "{\n";
            scopes_.emplace_back();
            emit_statement(branch->getInit());
            emit_statement(branch->getConditionVariableDeclStmt());
            auto condition = emit_condition(branch->getCond(), false);
            loops_.push_back({"", scopes_.size(), false});
            body_ << "switch (" << condition << ")\n";
            emit_statement(branch->getBody());
            loops_.pop_back();
            clear_scope(scopes_.size() - 1);
            scopes_.pop_back();
            body_ << "}\n";
        } else if (const auto* branch = dyn_cast<CaseStmt>(statement)) {
            body_ << "case " << rewrite(branch->getLHS());
            if (branch->getRHS()) body_ << " ... " << rewrite(branch->getRHS());
            body_ << ":;\n";
            emit_statement(branch->getSubStmt());
        } else if (const auto* branch = dyn_cast<DefaultStmt>(statement)) {
            body_ << "default:;\n";
            emit_statement(branch->getSubStmt());
        } else if (isa<BreakStmt>(statement)) {
            if (loops_.empty()) throw std::runtime_error("break has no lowered loop or switch");
            clear_scope(loops_.back().scope);
            if (exception_region_) emit_context_transition();
            body_ << "break;\n";
        } else if (isa<ContinueStmt>(statement)) {
            auto loop = std::find_if(loops_.rbegin(), loops_.rend(),
                                     [](const Loop& target) { return target.iteration; });
            if (loop == loops_.rend()) throw std::runtime_error("continue has no lowered loop");
            loop->continued = true;
            clear_scope(loop->scope + (loop->retain_condition ? 1 : 0));
            if (exception_region_) emit_context_transition();
            if (loop->next.empty()) body_ << "continue;\n";
            else body_ << "goto " << loop->next << ";\n";
        } else if (const auto* expression = dyn_cast<Expr>(statement)) {
            llvm::SaveAndRestore<bool> expression_scope(full_expression_, true);
            size_t first_comma = comma_temporaries_.size();
            auto value = emit_expression(expression);
            body_ << "static_cast<void>(" << value << ");\n";
            clear_comma_temporaries(first_comma);
        }
        else if (isa<NullStmt>(statement)) body_ << ";\n";
        else if (!has_suspend_calls(statement)) body_ << rewrite(statement) << "\n";
        else throw std::runtime_error(std::string("missing suspend lowering for ") + statement->getStmtClassName());
    }
    ASTContext& context_;
    FunctionDecl* function_;
    const ParmVarDecl* completion_ = nullptr;
    std::string receiver_;
    std::string receiver_type_;
    PrintingPolicy policy_;
    SourceManager& manager_;
    std::map<const ValueDecl*, Slot> variables_;
    std::map<const TypedefNameDecl*, std::string> local_aliases_;
    std::map<const CXXRecordDecl*, std::string> reference_types_;
    std::vector<std::string> reference_classes_;
    std::vector<std::string> fields_;
    std::vector<Slot> slots_;
    std::vector<std::vector<Slot>> scopes_;
    std::vector<Loop> loops_;
    std::vector<Slot> comma_temporaries_;
    bool full_expression_ = false;
    std::ostringstream body_;
    unsigned suspension_ = 0;
    unsigned loop_ = 0;
    unsigned exception_region_ = 0;
};
}

// NOTE(port): Typed Clang entry adapter for the native lowering implementation.
std::string build_coroutine(ASTContext& context, FunctionDecl* function) {
    try {
        return NativeSuspendFunctionsLowering(context, function).build_coroutine();
    } catch (const std::runtime_error& error) {
        unsigned id = context.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error,
            "Kotlin native suspend lowering: %0");
        context.getDiagnostics().Report(function->getLocation(), id) << error.what();
        return {};
    }
}
}
