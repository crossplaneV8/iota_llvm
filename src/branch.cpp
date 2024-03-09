
#include "iota.h"


llvm::Value* iota_compiler::is_non_zero(llvm::Value *val)
{
    llvm::Type *type = val->getType();

    if (type != builder->getInt1Ty()) {
        if (type->isIntegerTy() || type->isFloatingPointTy()) {
            auto zero = cast_to(builder->getInt64(0), type);
            return builder->CreateICmpNE(val, zero);
        }
        if (type->isPointerTy()) {
            return builder->CreateIsNotNull(val);
        }
        assert(0);  // unexpected case
    }
    return val;
}


llvm::Value* iota_compiler::visit_if(std::vector<Expr> &list, iota_env *env)
{
    assert(list.size() >= 3 && list.size() <= 4);
    assert(list[0].name == "if");

    // (if <cond> <then> <else>)
    if (list.size() == 4) {
        auto block_a = llvm::BasicBlock::Create(*context, "if_then", target_fn);
        auto block_b = llvm::BasicBlock::Create(*context, "if_else", target_fn);
        auto block_c = llvm::BasicBlock::Create(*context, "if_end", target_fn);

        auto cond = is_non_zero(visit(list[1], env));
        builder->CreateCondBr(cond, block_a, block_b);

        builder->SetInsertPoint(block_a);
        auto value_a = visit(list[2], env);
        block_a = builder->GetInsertBlock();
        builder->CreateBr(block_c);

        builder->SetInsertPoint(block_b);
        auto value_b = visit(list[3], env);
        block_b = builder->GetInsertBlock();
        builder->CreateBr(block_c);

        builder->SetInsertPoint(block_c);
        auto phi = builder->CreatePHI(value_a->getType(), 2);
        phi->addIncoming(value_a, block_a);
        phi->addIncoming(value_b, block_b);
        return phi;
    }

    // (if <cond> <then>)
    if (list.size() == 3) {
        auto block_a = llvm::BasicBlock::Create(*context, "if_then", target_fn);
        auto block_b = llvm::BasicBlock::Create(*context, "if_end", target_fn);

        auto cond = is_non_zero(visit(list[1], env));
        builder->CreateCondBr(cond, block_a, block_b);

        builder->SetInsertPoint(block_a);
        visit(list[2], env);
        builder->CreateBr(block_b);

        builder->SetInsertPoint(block_b);
    }

    return builder->getInt64(0);
}


// (while <cond> <exp0> <exp1> ...)
llvm::Value* iota_compiler::visit_while(std::vector<Expr> &list, iota_env *env)
{
    assert(list.size() >= 2);
    assert(list[0].name == "while");

    auto block_a = llvm::BasicBlock::Create(*context, "while_cond", target_fn);
    auto block_b = llvm::BasicBlock::Create(*context, "while_loop", target_fn);
    auto block_c = llvm::BasicBlock::Create(*context, "while_end", target_fn);

    builder->CreateBr(block_a);
    builder->SetInsertPoint(block_a);
    auto cond = is_non_zero(visit(list[1], env));
    builder->CreateCondBr(cond, block_b, block_c);

    builder->SetInsertPoint(block_b);
    for (int i=2; i<list.size(); i++) {
        visit(list[i], env);
    }
    builder->CreateBr(block_a);
    builder->SetInsertPoint(block_c);
    return builder->getInt64(0);
}

