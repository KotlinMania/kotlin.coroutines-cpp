# Clang Suspend Keyword: Required Functions for Extraction

**Goal:** Extract minimal LLVM/Clang code to add native `suspend` keyword support.

## IR Generation (LLVM)

### IndirectBr Instruction
```cpp
llvm::IRBuilder::CreateIndirectBr(llvm::Value *Addr)
llvm::IndirectBrInst::addDestination(llvm::BasicBlock *Dest)
```

### Block Addresses (labels-as-values)
```cpp
llvm::BlockAddress::get(llvm::Function *F, llvm::BasicBlock *BB)
```

### Basic Block Creation
```cpp
llvm::BasicBlock::Create(llvm::LLVMContext &Context, const Twine &Name, llvm::Function *Parent)
```

### PHI Nodes
```cpp
llvm::IRBuilder::CreatePHI(llvm::Type *Ty, unsigned NumReservedValues, const Twine &Name)
llvm::PHINode::addIncoming(llvm::Value *V, llvm::BasicBlock *BB)
```

### Struct Type (coroutine frame)
```cpp
llvm::StructType::create(llvm::LLVMContext &Context, ArrayRef<llvm::Type*> Elements, StringRef Name)
```

### Field Access
```cpp
llvm::IRBuilder::CreateStructGEP(llvm::Type *Ty, llvm::Value *Ptr, unsigned Idx)
```

### Load/Store
```cpp
llvm::IRBuilder::CreateLoad(llvm::Type *Ty, llvm::Value *Ptr)
llvm::IRBuilder::CreateStore(llvm::Value *Val, llvm::Value *Ptr)
```

### Control Flow
```cpp
llvm::IRBuilder::CreateICmpEQ(llvm::Value *LHS, llvm::Value *RHS)
llvm::IRBuilder::CreateCondBr(llvm::Value *Cond, llvm::BasicBlock *True, llvm::BasicBlock *False)
```

---

## CFG/Analysis (Clang)

### CFG Construction
```cpp
clang::CFG::buildCFG(const Decl *D, Stmt *Statement, ASTContext *C, const CFG::BuildOptions &BO)
```

### CFG Iteration
```cpp
clang::CFG::begin() / end()
clang::CFGBlock::begin() / end()
clang::CFGElement::getAs<CFGStmt>()
```

### AST Traversal
```cpp
clang::RecursiveASTVisitor<Derived>::TraverseStmt(Stmt *S)
clang::RecursiveASTVisitor<Derived>::VisitVarDecl(VarDecl *VD)
clang::RecursiveASTVisitor<Derived>::VisitDeclRefExpr(DeclRefExpr *DRE)
```

---

## AST/Decl (Clang)

### Function Declaration
```cpp
clang::FunctionDecl::hasBody()
clang::FunctionDecl::getBody()
clang::FunctionDecl::parameters()
clang::FunctionDecl::getReturnType()
clang::FunctionDecl::getNameAsString()
```

### Variable Declarations
```cpp
clang::VarDecl::getType()
clang::VarDecl::getNameAsString()
clang::VarDecl::isLocalVarDecl()
clang::ParmVarDecl
```

### Type Info
```cpp
clang::QualType::getAsString(const PrintingPolicy &Policy)
```

---

## Lexer/Parser (for keyword)

### Token Handling
```cpp
clang::Token::is(tok::TokenKind K)
clang::Token::getKind()
clang::Token::getLocation()
```

### DeclSpec (function specifiers)
```cpp
clang::DeclSpec::setFunctionSpecInline(SourceLocation Loc)  // pattern for suspend
```

---

## Source Files to Examine

### LLVM (vendor/llvm-project/llvm/)
- `include/llvm/IR/IRBuilder.h` - CreateIndirectBr, CreatePHI, CreateStructGEP, etc.
- `include/llvm/IR/Instructions.h` - IndirectBrInst, PHINode
- `include/llvm/IR/Constants.h` - BlockAddress
- `include/llvm/IR/BasicBlock.h` - BasicBlock::Create
- `include/llvm/IR/DerivedTypes.h` - StructType

### Clang (vendor/llvm-project/clang/)
- `include/clang/Analysis/CFG.h` - CFG, CFGBlock, CFGElement
- `include/clang/AST/RecursiveASTVisitor.h` - traversal
- `include/clang/AST/Decl.h` - FunctionDecl, VarDecl, ParmVarDecl
- `include/clang/AST/Type.h` - QualType
- `include/clang/Basic/TokenKinds.def` - keyword definitions
- `include/clang/Sema/DeclSpec.h` - function specifiers
- `lib/CodeGen/CodeGenFunction.cpp` - GetIndirectGotoBlock pattern

---

## Generated IR Pattern (Target)

```llvm
; Entry dispatch
%label = load ptr, ptr %_label
%is_null = icmp eq ptr %label, null
br i1 %is_null, label %start, label %dispatch

dispatch:
  indirectbr ptr %label, [label %resume0, label %resume1]

start:
  ; ... code before suspend ...
  store ptr blockaddress(@invoke_suspend, %resume0), ptr %_label
  ; ... call suspend function ...
  ; if suspended, return COROUTINE_SUSPENDED

resume0:
  ; ... restore spilled variables ...
  ; ... continue execution ...
```
