// Compiler integration test for the source debug bridge used by frame allocation.
#include "DebugInfoC.hpp"
#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugProgramInstruction.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <cassert>

using namespace org::jetbrains::kotlin::backend::konan::llvm;

int main() {
    llvm::LLVMContext context;
    llvm::Module module("frame_debug", context);
    module.addModuleFlag(llvm::Module::Warning, "Debug Info Version",
                         llvm::DEBUG_METADATA_VERSION);
    llvm::DIBuilder debug(module);
    auto* file = debug.createFile("frame.cpp", "/source");
    debug.createCompileUnit(llvm::dwarf::DW_LANG_C_plus_plus, file,
                            "source contract", false, "", 0);
    auto* integer = debug.createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
    auto* debug_type = debug.createSubroutineType(debug.getOrCreateTypeArray({}));
    auto* scope = debug.createFunction(file, "frame", "frame", file, 4,
        debug_type, 4, llvm::DINode::FlagZero, llvm::DISubprogram::SPFlagDefinition);
    auto* function = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(context), false),
        llvm::GlobalValue::ExternalLinkage, "frame", module);
    function->setSubprogram(scope);
    auto* block = llvm::BasicBlock::Create(context, "entry", function);
    llvm::IRBuilder<> instructions(block);
    auto* storage = instructions.CreateAlloca(llvm::Type::getInt32Ty(context));
    auto* terminator = instructions.CreateRetVoid();
    const auto builder = llvm::wrap(&debug);
    auto* local = llvm::unwrap<llvm::DILocalVariable>(di_create_auto_variable(
        builder, llvm::wrap(scope), "samples", llvm::wrap(file), 7,
        llvm::wrap(integer)));
    auto* parameter = llvm::unwrap<llvm::DILocalVariable>(
        di_create_parameter_variable(builder, llvm::wrap(scope), "worker", 1,
            llvm::wrap(file), 4, llvm::wrap(integer)));
    assert(local->getScope() == scope && local->getFile() == file);
    assert(local->getType() == integer && local->getLine() == 7);
    assert(local->getName() == "samples" && local->getArg() == 0);
    assert(local->getFlags() == llvm::DINode::FlagZero && local->getAlignInBits() == 0);
    assert(parameter->getScope() == scope && parameter->getFile() == file);
    assert(parameter->getType() == integer && parameter->getLine() == 4);
    assert(parameter->getName() == "worker" && parameter->getArg() == 1);
    assert(scope->getRetainedNodes().empty());
    assert(llvm::unwrap<llvm::DIExpression>(di_create_empty_expression(builder))
               ->getElements().empty());
    auto* location = llvm::DILocation::get(context, 7, 2, scope);
    const std::int64_t operations[] = {llvm::dwarf::DW_OP_plus_uconst, 24,
                                     llvm::dwarf::DW_OP_deref};
    di_insert_declaration(builder, llvm::wrap(storage), llvm::wrap(local),
        llvm::wrap(location), llvm::wrap(block), operations, 3);
    di_insert_declaration(builder, llvm::wrap(storage), llvm::wrap(parameter),
        llvm::wrap(location), llvm::wrap(block), nullptr, 0);
    // LLVM23 represents declarations as debug records attached before ret.
    // Verify storage/metadata identity, expression order and insertion position.
    unsigned count = 0;
    for (auto& record : terminator->getDbgRecordRange()) {
        auto* declaration = llvm::dyn_cast<llvm::DbgVariableRecord>(&record);
        assert(declaration && declaration->isDbgDeclare());
        assert(declaration->getAddress() == storage);
        assert(declaration->getDebugLoc().get() == location);
        if (count == 0) {
            assert(declaration->getVariable() == local);
            assert(declaration->getExpression()->getElements() ==
                llvm::ArrayRef<std::uint64_t>({llvm::dwarf::DW_OP_plus_uconst,
                                             24, llvm::dwarf::DW_OP_deref}));
        } else {
            assert(count == 1 && declaration->getVariable() == parameter);
            assert(declaration->getExpression()->getElements().empty());
        }
        ++count;
    }
    assert(count == 2);
    debug.finalize();
    assert(!llvm::verifyModule(module, &llvm::errs()));
    llvm::outs() << "frame debug: local/parameter identity; location expressions; verified module\n";
}
