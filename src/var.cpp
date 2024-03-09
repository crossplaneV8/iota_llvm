
#include "iota.h"


llvm::AllocaInst* iota_compiler::create_var(std::string name, llvm::Type *type, iota_env *env)
{
    auto curr_block = builder->GetInsertBlock();

    llvm::BasicBlock *entry = &(target_fn->getEntryBlock());
    if (entry->getInstList().size() > 0) {
        builder->SetInsertPoint(entry->getFirstNonPHI());
    } else {
        builder->SetInsertPoint(entry);
    }
    llvm::AllocaInst *var = builder->CreateAlloca(type, NULL, name.c_str());
    builder->SetInsertPoint(curr_block);
    env->define(name, var);
    return var;
}

