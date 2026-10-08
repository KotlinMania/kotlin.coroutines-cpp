// NOTE(port): Clang AST integration for the Kotlin-derived frame lowering.
// This file is C++ compiler infrastructure, not a Kotlin function transliteration.
#include "CompilerFrameLowering.hpp"
#include "NativeSuspendLowering.hpp"
#include "clang/AST/ASTImporter.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Sema/Sema.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/ADT/ScopeExit.h"
#include <map>
#include <set>
#include <vector>

namespace kotlinx::suspend {
namespace {
thread_local bool parser_active = false;
// Reuse declarations already parsed from the same headers. Importing another
// copy of the standard library would lose the owning compiler's template state.
class HeaderDeclarations : public clang::RecursiveASTVisitor<HeaderDeclarations> {
public:
    explicit HeaderDeclarations(clang::ASTContext& context, std::string edited_file = {},
                                unsigned edited_begin = 0, unsigned edited_end = 0,
                                long replacement_size = 0)
        : context_(context), edited_file_(std::move(edited_file)), edited_begin_(edited_begin),
          edited_end_(edited_end), replacement_size_(replacement_size) {}
    bool shouldVisitTemplateInstantiations() const { return true; }
    // Lambda invoke declarations are implicit but their written bodies are
    // compiler entry contexts. Include their records in declaration reuse/import.
    bool TraverseLambdaExpr(clang::LambdaExpr* expression) {
        for (auto* initializer : expression->capture_inits())
            if (!TraverseStmt(initializer)) return false;
        if (!WalkUpFromCXXRecordDecl(expression->getLambdaClass())) return false;
        return TraverseCXXMethodDecl(expression->getCallOperator());
    }
    bool VisitFunctionDecl(clang::FunctionDecl* function) {
        functions.push_back(function);
        return true;
    }
    bool VisitNamedDecl(clang::NamedDecl* declaration) {
        auto& manager = context_.getSourceManager();
        auto location = manager.getExpansionLoc(declaration->getLocation());
        if (location.isInvalid() || manager.isWrittenInMainFile(location)) return true;
        auto filename = manager.getFilename(location).str();
        long source_offset = manager.getFileOffset(location);
        if (filename == edited_file_ && source_offset >= edited_begin_) {
            if (source_offset < edited_begin_ + replacement_size_) return true;
            source_offset -= replacement_size_ - (edited_end_ - edited_begin_);
        }
        std::string key = filename + ":" +
            std::to_string(source_offset) + ":" + declaration->getDeclKindName() +
            ":" + declaration->getQualifiedNameAsString();
        if (auto* value = llvm::dyn_cast<clang::ValueDecl>(declaration)) key += ":" + value->getType().getAsString();
        else if (auto* type = llvm::dyn_cast<clang::TypeDecl>(declaration))
            key += ":" + clang::QualType(context_.getCanonicalTypeDeclType(type)).getAsString();
        if (auto* record = llvm::dyn_cast<clang::RecordDecl>(declaration->getDeclContext()))
            key += ":parent=" + clang::QualType(context_.getCanonicalTypeDeclType(record)).getAsString();
        if (auto* function = llvm::dyn_cast<clang::FunctionDecl>(declaration->getDeclContext())) {
            key += ":function=" + function->getType().getCanonicalType().getAsString();
            if (auto* record = llvm::dyn_cast<clang::RecordDecl>(function->getDeclContext()))
                key += ":owner=" + clang::QualType(context_.getCanonicalTypeDeclType(record)).getAsString();
        }
        // Template-local declarations can share a source location and printed
        // type while belonging to different instantiations. Never bind an
        // ambiguous textual identity to an arbitrary declaration from that set.
        if (ambiguous.contains(key)) return true;
        auto [existing, inserted] = declarations.emplace(key, declaration);
        if (!inserted && existing->second->getCanonicalDecl() != declaration->getCanonicalDecl()) {
            declarations.erase(existing);
            ambiguous.insert(std::move(key));
        }
        return true;
    }
    std::map<std::string, clang::Decl*> declarations;
    std::set<std::string> ambiguous;
    std::vector<clang::FunctionDecl*> functions;
private:
    clang::ASTContext& context_;
    std::string edited_file_;
    unsigned edited_begin_;
    unsigned edited_end_;
    long replacement_size_;
};
class ReferencedFunctions : public clang::RecursiveASTVisitor<ReferencedFunctions> {
public:
    // Default member initializers and implicit constructor calls are real
    // dependencies of the retained frame even without spelled source tokens.
    bool shouldVisitImplicitCode() const { return true; }
    bool VisitCXXConstructExpr(clang::CXXConstructExpr* construction) {
        functions.insert(construction->getConstructor()->getCanonicalDecl());
        if (auto* record = construction->getType()->getAsCXXRecordDecl())
            if (auto* destructor = record->getDestructor()) functions.insert(destructor->getCanonicalDecl());
        return true;
    }
    bool VisitCallExpr(clang::CallExpr* call) {
        if (auto* function = call->getDirectCallee()) functions.insert(function->getCanonicalDecl());
        return true;
    }
    std::set<clang::FunctionDecl*> functions;
};
}
// NOTE(port): Preserve the Clang plugin entry API while the translated lowering
// retains its Kotlin compiler package identity.
std::string lower_native_suspend(clang::ASTContext& context, clang::FunctionDecl* function) {
    return org::jetbrains::kotlin::backend::konan::lower::build_coroutine(context, function);
}
bool is_lowering_parser_active() { return parser_active; }
// NOTE(port): Clang parses the synthesized frame in memory with the original
// invocation, then imports real AST nodes. No files, compiler subprocesses or
// serialized LLVM modules take part in this frontend operation.
bool install_native_frame(clang::CompilerInstance& compiler, clang::FunctionDecl* function,
                          std::string lowered) {
    using namespace clang;
    auto& context = compiler.getASTContext();
    auto& manager = context.getSourceManager();
    unsigned diagnostic = context.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error,
        "Kotlin native frame integration: %0");
    auto fail = [&](const std::string& message) {
        context.getDiagnostics().Report(function->getLocation(), diagnostic) << message;
        return false;
    };
    if (lowered.empty()) return false;
    auto* body = function->getBody();
    // Replacing the body does not erase Sema's ODR uses of its local lambdas.
    // Complete their referenced template definitions while that body still
    // belongs to the original function and its declaration context is intact.
    ReferencedFunctions original_references;
    original_references.TraverseStmt(body);
    std::set<FunctionDecl*> completed_original_references;
    while (completed_original_references.size() < original_references.functions.size()) {
        auto pending = original_references.functions;
        for (auto* candidate : pending) {
            if (!completed_original_references.insert(candidate).second) continue;
            if (!candidate->hasBody() && candidate->getTemplateSpecializationInfo())
                compiler.getSema().InstantiateFunctionDefinition(function->getLocation(), candidate, true);
            if (candidate->hasBody()) original_references.TraverseDecl(candidate);
        }
    }
    auto body_location = manager.getSpellingLoc(body->getBeginLoc());
    auto body_file_id = manager.getFileID(body_location);
    auto file = manager.getFileEntryRefForID(body_file_id);
    auto main_file = manager.getFileEntryRefForID(manager.getMainFileID());
    if (!file || !main_file || body->getBeginLoc().isMacroID())
        return fail("suspend definition requires a concrete source-file body");
    bool header = body_file_id != manager.getMainFileID();
    std::string source = manager.getBufferData(body_file_id).str();
    unsigned begin = manager.getFileOffset(body->getBeginLoc());
    unsigned end = manager.getFileOffset(Lexer::getLocForEndOfToken(body->getEndLoc(), 0,
        manager, context.getLangOpts()));
    const bool member = llvm::isa<CXXMethodDecl>(function);
    const bool member_instantiation = member && !function->isDependentContext() &&
        cast<CXXMethodDecl>(function)->getInstantiatedFromMemberFunction();
    const bool instantiated = function->getTemplateSpecializationInfo() || member_instantiation;
    const bool local_frame = member || header || function->getDescribedFunctionTemplate() || instantiated;
    long replacement_size = instantiated ? end - begin : local_frame ? lowered.size() : 1;
    if (!instantiated) source.replace(begin, end - begin, local_frame ? lowered : ";");
    std::string name = "__kxs_entry_" + function->getNameAsString() + "_" + std::to_string(begin);
    std::string instantiated_entry;
    if (instantiated) {
        // NOTE(port): Overload selection can require a concrete instantiation.
        // Parse its frame as a separate typed entry without replacing the
        // primary template or changing other specializations.
        PrintingPolicy policy(context.getLangOpts());
        policy.PrintAsCanonical = true;
        auto* parameters = function->getPrimaryTemplate() ? function->getPrimaryTemplate()->getTemplateParameters() : nullptr;
        auto* arguments = function->getTemplateSpecializationArgs();
        std::string aliases;
        for (unsigned i = 0; parameters && i < parameters->size(); ++i) {
            if (i >= arguments->size()) return fail("instantiated entry lost a template argument");
            auto parameter_name = parameters->getParam(i)->getNameAsString();
            if (parameter_name.empty()) continue;
            const auto& argument = arguments->get(i);
            if (argument.getKind() == TemplateArgument::Type)
                aliases += "using " + parameter_name + " = " + argument.getAsType().getAsString(policy) + ";\n";
            else if (argument.getKind() != TemplateArgument::Integral &&
                     argument.getKind() != TemplateArgument::Declaration &&
                     argument.getKind() != TemplateArgument::NullPtr)
                return fail("instantiated entry requires a type, integral, declaration or null template argument");
        }
        std::vector<const NamespaceDecl*> namespaces;
        auto* enclosing_context = function->getDeclContext();
        if (member) enclosing_context = enclosing_context->getParent();
        for (auto* parent = enclosing_context; !parent->isTranslationUnit(); parent = parent->getParent()) {
            if (member && isa<CXXRecordDecl>(parent)) continue;
            auto* scope = llvm::dyn_cast<NamespaceDecl>(parent);
            if (!scope) return fail("instantiated entry requires a namespace context");
            namespaces.push_back(scope);
        }
        for (auto scope = namespaces.rbegin(); scope != namespaces.rend(); ++scope)
            instantiated_entry += ((*scope)->isInline() ? "inline namespace " : "namespace ") +
                                  (*scope)->getNameAsString() + " {\n";
        if (member) {
            auto* method = cast<CXXMethodDecl>(function);
            for (auto* parent = method->getDeclContext(); !parent->isTranslationUnit(); parent = parent->getParent()) {
                if (auto* specialization = dyn_cast<ClassTemplateSpecializationDecl>(parent)) {
                    if (specialization->getSpecializationKind() == TSK_ExplicitSpecialization) break;
                    instantiated_entry += "template<> ";
                }
            }
            if (arguments) instantiated_entry += "template<> ";
            instantiated_entry += "void* " +
                QualType(context.getCanonicalTypeDeclType(method->getParent())).getAsString(policy) + "::" +
                function->getNameAsString();
            if (arguments) instantiated_entry += "<";
            for (unsigned i = 0; arguments && i < arguments->size(); ++i) {
                if (i) instantiated_entry += ", ";
                llvm::raw_string_ostream stream(instantiated_entry);
                arguments->get(i).print(policy, stream, true);
            }
            if (arguments) instantiated_entry += ">";
            instantiated_entry += "(";
        } else {
            instantiated_entry += "namespace __kxs_instantiation_" +
                std::to_string(static_cast<uint64_t>(function->getID())) + " {\n" + aliases;
            instantiated_entry += "void* " + name + "(";
        }
        for (unsigned i = 0; i < function->getNumParams(); ++i) {
            if (i) instantiated_entry += ", ";
            auto* parameter = function->getParamDecl(i);
            llvm::raw_string_ostream stream(instantiated_entry);
            parameter->getType().print(stream, policy, parameter->getNameAsString());
        }
        instantiated_entry += ")";
        if (member) {
            auto* method = cast<CXXMethodDecl>(function);
            if (method->isConst()) instantiated_entry += " const";
            if (method->isVolatile()) instantiated_entry += " volatile";
            if (method->getRefQualifier() == RQ_LValue) instantiated_entry += " &";
            if (method->getRefQualifier() == RQ_RValue) instantiated_entry += " &&";
            lowered.insert(1, "\n" + aliases);
        }
        {
            llvm::raw_string_ostream stream(instantiated_entry);
            function->getType()->castAs<FunctionProtoType>()->printExceptionSpecification(stream, policy);
        }
        instantiated_entry += " " + lowered;
        if (!member) instantiated_entry += "\n}";
        for (size_t i = 0; i < namespaces.size(); ++i) instantiated_entry += "\n}";
    }
    std::string declaration = "void* " + function->getNameAsString() + "(";
    if (!local_frame) {
        size_t entry = lowered.rfind(declaration);
        if (entry == std::string::npos) return fail("generated continuation entry is missing");
        lowered.replace(entry, declaration.size(), "void* " + name + "(");
        source += "\n" + lowered;
    }
    std::string header_source;
    if (header) {
        header_source = std::move(source);
        source = manager.getBufferData(manager.getMainFileID()).str();
        // Parse the including context only through this include. Later main-file
        // definitions belong to the owning parser and must not be imported early.
        auto include = manager.getIncludeLoc(body_file_id);
        auto is_preinclude = [&](SourceLocation location) {
            return manager.isWrittenInBuiltinFile(location) || manager.isWrittenInCommandLineFile(location);
        };
        while (include.isValid() && !is_preinclude(include) &&
               manager.getFileID(include) != manager.getMainFileID())
            include = manager.getIncludeLoc(manager.getFileID(include));
        if (include.isValid() && is_preinclude(include)) {
            // Command-line forced includes run before the main file. The cloned
            // invocation retains those includes and their preprocessor state.
            source.clear();
        } else {
            if (include.isInvalid()) return fail("header definition has no compiler include context");
            auto newline = source.find('\n', manager.getFileOffset(include));
            if (newline != std::string::npos) source.resize(newline + 1);
        }
    }
    source.insert(0, "#include <optional>\n#include <functional>\n");
    if (instantiated) source += "\n" + instantiated_entry;
    auto invocation = std::make_shared<CompilerInvocation>(compiler.getInvocation());
    auto& frontend = invocation->getFrontendOpts();
    frontend.Plugins.clear();
    frontend.PluginArgs.clear();
    frontend.AddPluginActions.clear();
    frontend.ProgramAction = frontend::ParseSyntaxOnly;
    std::string helper_name = main_file->getName().str() + ".kxs.frontend.cpp";
    auto input_kind = frontend.Inputs.front().getKind();
    frontend.Inputs.clear();
    frontend.Inputs.emplace_back(helper_name, input_kind);
    invocation->getPreprocessorOpts().addRemappedFile(helper_name,
        llvm::MemoryBuffer::getMemBufferCopy(source, helper_name).release());
    if (header) invocation->getPreprocessorOpts().addRemappedFile(file->getName(),
        llvm::MemoryBuffer::getMemBufferCopy(header_source, file->getName()).release());
    auto diagnostics = std::make_shared<DiagnosticOptions>(compiler.getDiagnosticOpts());
    auto files = llvm::IntrusiveRefCntPtr<FileManager>(new FileManager(invocation->getFileSystemOpts(),
        compiler.getFileManager().getVirtualFileSystemPtr()));
    auto helper_diagnostics = CompilerInstance::createDiagnostics(files->getVirtualFileSystem(), *diagnostics);
    parser_active = true;
    auto parser_scope = llvm::scope_exit([&] { parser_active = false; });
    auto owned_unit = ASTUnit::LoadFromCompilerInvocation(invocation, compiler.getPCHContainerOperations(),
        diagnostics, helper_diagnostics, files);
    if (!owned_unit || helper_diagnostics->hasErrorOccurred()) return fail("generated frame could not be parsed");
    auto* unit = owned_unit.release();
    // Imported attributed types can retain attribute storage from the source
    // AST. Keep that AST alive for the complete owning compilation.
    context.AddDeallocation([](void* storage) { delete static_cast<ASTUnit*>(storage); }, unit);
    FunctionDecl* wrapper = nullptr;
    HeaderDeclarations generated(unit->getASTContext(), header ? file->getName().str() : "",
                                 begin, end, replacement_size);
    generated.TraverseDecl(unit->getASTContext().getTranslationUnitDecl());
    for (auto* candidate : generated.functions)
        if (((!local_frame || (instantiated && !member)) && candidate->getNameAsString() == name) ||
            (local_frame && (!instantiated || member) && candidate->getQualifiedNameAsString() == function->getQualifiedNameAsString() &&
             candidate->getType().getCanonicalType().getAsString() == function->getType().getCanonicalType().getAsString() &&
             candidate->doesThisDeclarationHaveABody())) wrapper = candidate;
    if (!wrapper) return fail("generated continuation entry was not parsed");
    ASTImporter importer(context, compiler.getFileManager(), unit->getASTContext(),
                         unit->getFileManager(), false);
    helper_diagnostics->getClient()->BeginSourceFile(unit->getASTContext().getLangOpts(), nullptr);
    auto diagnostic_scope = llvm::scope_exit([&] { helper_diagnostics->getClient()->EndSourceFile(); });
    HeaderDeclarations existing(context);
    existing.TraverseDecl(context.getTranslationUnitDecl());
    for (const auto& [key, declaration] : generated.declarations) {
        auto found = existing.declarations.find(key);
        if (found != existing.declarations.end()) importer.RegisterImportedDecl(declaration, found->second);
    }
    // A member definition has a separate declaration from its header prototype.
    // Reuse that prototype before importing its enclosing record, so parsing
    // this frame cannot install unrelated definitions ahead of the main parser.
    for (auto* candidate : generated.functions) {
        if (candidate == wrapper) continue;
        if (auto* installed = importer.GetAlreadyImportedOrNull(candidate->getCanonicalDecl()))
            importer.RegisterImportedDecl(candidate, installed);
    }
    FunctionDecl* imported_wrapper = nullptr;
    if (local_frame) {
        importer.RegisterImportedDecl(wrapper, function);
        if (auto* target = dyn_cast<CXXMethodDecl>(function); target && target->getParent()->isLambda()) {
            auto* source = cast<CXXMethodDecl>(wrapper);
            importer.RegisterImportedDecl(source->getParent(), target->getParent());
            // Captured references must retain the original closure's value and
            // field identities. CodeGen indexes that closure by captured decl.
            llvm::DenseMap<const ValueDecl*, FieldDecl*> source_fields, target_fields;
            FieldDecl* source_this = nullptr;
            FieldDecl* target_this = nullptr;
            source->getParent()->getCaptureFields(source_fields, source_this);
            target->getParent()->getCaptureFields(target_fields, target_this);
            auto original = target->getParent()->captures_begin();
            for (const auto& capture : source->getParent()->captures()) {
                if (capture.capturesVariable()) {
                    auto* source_variable = capture.getCapturedVar();
                    auto* target_variable = original->getCapturedVar();
                    importer.RegisterImportedDecl(source_variable, target_variable);
                    importer.RegisterImportedDecl(source_fields.lookup(source_variable), target_fields.lookup(target_variable));
                }
                ++original;
            }
            if (source_this) importer.RegisterImportedDecl(source_this, target_this);
        }
        if (auto* target_template = function->getDescribedFunctionTemplate()) {
            auto* source_template = wrapper->getDescribedFunctionTemplate();
            if (!source_template || source_template->getTemplateParameters()->size() !=
                                    target_template->getTemplateParameters()->size())
                return fail("generated entry lost its template parameter context");
            importer.RegisterImportedDecl(source_template, target_template);
            for (unsigned i = 0; i < source_template->getTemplateParameters()->size(); ++i)
                importer.RegisterImportedDecl(source_template->getTemplateParameters()->getParam(i),
                                              target_template->getTemplateParameters()->getParam(i));
        }
        for (unsigned i = 0; i < wrapper->getNumParams(); ++i)
            importer.RegisterImportedDecl(wrapper->getParamDecl(i), function->getParamDecl(i));
        auto imported_body = importer.Import(wrapper->getBody());
        if (!imported_body) return fail(llvm::toString(imported_body.takeError()));
        function->setBody(*imported_body);
        imported_wrapper = function;
    } else {
        auto imported = importer.Import(wrapper);
        if (!imported) return fail(llvm::toString(imported.takeError()));
        imported_wrapper = llvm::cast<FunctionDecl>(*imported);
    }
    llvm::SmallVector<ParmVarDecl*, 8> parameters;
    for (auto* parameter : imported_wrapper->parameters()) {
        parameter->setDeclContext(function);
        parameters.push_back(parameter);
    }
    function->setParams(parameters);
    function->setBody(imported_wrapper->getBody());
    std::set<FunctionDecl*> original_functions(existing.functions.begin(), existing.functions.end());
    ReferencedFunctions referenced;
    ReferencedFunctions generated_references;
    generated_references.TraverseDecl(wrapper);
    for (auto* candidate : generated.functions) {
        auto* installed = llvm::dyn_cast_or_null<FunctionDecl>(importer.GetAlreadyImportedOrNull(candidate));
        if (installed && !original_functions.contains(installed) && candidate->hasBody())
            generated_references.TraverseDecl(candidate);
    }
    std::set<FunctionDecl*> expanded;
    while (expanded.size() < generated_references.functions.size()) {
        auto pending = generated_references.functions;
        for (auto* candidate : pending) {
            if (!expanded.insert(candidate).second) continue;
            if (candidate->hasBody()) generated_references.TraverseDecl(candidate);
        }
    }
    for (auto* candidate : generated_references.functions)
        if (auto* installed = llvm::dyn_cast_or_null<FunctionDecl>(importer.GetAlreadyImportedOrNull(candidate)))
            referenced.functions.insert(installed->getCanonicalDecl());
    for (auto* generated_function : generated.functions) {
        auto* installed = llvm::dyn_cast_or_null<FunctionDecl>(importer.GetAlreadyImportedOrNull(generated_function));
        if (!installed || installed == imported_wrapper) continue;
        if (original_functions.contains(installed)) continue;
        if (generated_function->hasBody()) {
            // ASTImporter does not preserve implicit inline on these local
            // class definitions. Restore it before CodeGen chooses linkage.
            installed->setImplicitlyInline(generated_function->isInlined());
            if (!installed->hasBody()) {
                if (auto error = importer.ImportDefinition(generated_function)) return fail(llvm::toString(std::move(error)));
            }
            if (auto* constructor = llvm::dyn_cast<CXXConstructorDecl>(generated_function)) {
                auto* target = llvm::cast<CXXConstructorDecl>(installed);
                {
                    auto** initializers = context.Allocate<CXXCtorInitializer*>(constructor->getNumCtorInitializers());
                    unsigned index = 0;
                    for (auto* initializer : constructor->inits()) {
                        auto imported_initializer = importer.Import(initializer);
                        if (!imported_initializer) return fail(llvm::toString(imported_initializer.takeError()));
                        // Template instantiation rebuilds only explicitly written
                        // initializers. ASTImporter loses that source-order flag.
                        if (initializer->isWritten() && !(*imported_initializer)->isWritten())
                            (*imported_initializer)->setSourceOrder(initializer->getSourceOrder());
                        initializers[index++] = *imported_initializer;
                    }
                    target->setCtorInitializers(initializers);
                    target->setNumCtorInitializers(index);
                }
            }
            if (!installed->isDependentContext()) compiler.getASTConsumer().HandleTopLevelDecl(DeclGroupRef(installed));
            referenced.TraverseStmt(installed->getBody());
        }
    }
    for (auto* generated_function : generated.functions) {
        auto* installed = llvm::dyn_cast_or_null<FunctionDecl>(importer.GetAlreadyImportedOrNull(generated_function));
        if (!installed || !original_functions.contains(installed) ||
            !referenced.functions.contains(installed->getCanonicalDecl()) || !generated_function->hasBody()) continue;
        if (!installed->hasBody()) {
            for (unsigned i = 0; i < generated_function->getNumParams(); ++i)
                importer.RegisterImportedDecl(generated_function->getParamDecl(i), installed->getParamDecl(i));
            if (auto* constructor = llvm::dyn_cast<CXXConstructorDecl>(generated_function)) {
                auto* target = llvm::cast<CXXConstructorDecl>(installed);
                auto** initializers = context.Allocate<CXXCtorInitializer*>(constructor->getNumCtorInitializers());
                unsigned index = 0;
                for (auto* initializer : constructor->inits()) {
                    auto imported_initializer = importer.Import(initializer);
                    if (!imported_initializer) return fail(llvm::toString(imported_initializer.takeError()));
                    // Template instantiation rebuilds only explicitly written
                        // initializers. ASTImporter loses that source-order flag.
                        if (initializer->isWritten() && !(*imported_initializer)->isWritten())
                            (*imported_initializer)->setSourceOrder(initializer->getSourceOrder());
                        initializers[index++] = *imported_initializer;
                }
                target->setCtorInitializers(initializers);
                target->setNumCtorInitializers(index);
            }
            auto imported_body = importer.Import(generated_function->getBody());
            if (!imported_body) return fail(llvm::toString(imported_body.takeError()));
            installed->setBody(*imported_body);
        }
        compiler.getASTConsumer().HandleTopLevelDecl(DeclGroupRef(installed));
    }
    referenced.TraverseDecl(function);
    std::set<FunctionDecl*> expanded_references;
    while (expanded_references.size() < referenced.functions.size()) {
        auto pending = referenced.functions;
        for (auto* candidate : pending) {
            if (!expanded_references.insert(candidate).second) continue;
            if (candidate->hasBody()) referenced.TraverseDecl(candidate);
        }
    }
    for (auto* referenced_function : referenced.functions) {
        if (!referenced_function->getTemplateSpecializationInfo()) continue;
        if (!referenced_function->hasBody())
            compiler.getSema().InstantiateFunctionDefinition(function->getLocation(),
                referenced_function, true);
        if (referenced_function->hasBody() && !referenced_function->isDependentContext())
            compiler.getASTConsumer().HandleTopLevelDecl(DeclGroupRef(referenced_function));
    }
    return true;
}
}
