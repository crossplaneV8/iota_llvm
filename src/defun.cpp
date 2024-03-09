
#include "iota.h"


llvm::FunctionType* iota_compiler::fn_type_from_proto(std::vector<Expr> &proto)
{
    llvm::Type *ret_type = anno_type(proto[0].anno, builder->getInt64Ty());
    std::vector<llvm::Type*> arg_type;
    for (int i=1; i<proto.size(); i++) {
        auto anno = proto[i].anno == "" ? proto[i].name : proto[i].anno;
        arg_type.push_back(anno_type(anno, builder->getInt64Ty()));
    }
    return llvm::FunctionType::get(ret_type, arg_type, false);
}


// import extern function: (import (<func> <arg0> <arg1> ...))
llvm::Value* iota_compiler::visit_import(std::vector<Expr> &list, iota_env *env)
{
    assert(list.size() == 2);
    assert(list[0].name == "import");
    assert(list[1].type == ExprType::LIST);

    std::vector<Expr> &proto = list[1].list;
    llvm::FunctionType *fn_type = fn_type_from_proto(proto);
    module->getOrInsertFunction(proto[0].name, fn_type);
    global_env->define(proto[0].name, module->getFunction(proto[0].name));
    return builder->getInt64(0);
}


// define function: (def (<func> <arg0> <arg1> ...) <exp0> <exp1> ...)
llvm::Value* iota_compiler::visit_define(std::vector<Expr> &list, iota_env *env)
{
    assert(list.size() >= 3);
    assert(list[0].name == "def");
    assert(list[1].type == ExprType::LIST);

    auto curr_fn = target_fn;
    auto curr_bb = builder->GetInsertBlock();

    std::vector<Expr> &proto = list[1].list;
    llvm::FunctionType *fn_type = fn_type_from_proto(proto);
    target_fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, proto[0].name, *module);
    llvm::verifyFunction(*target_fn);
    global_env->define(proto[0].name, target_fn);
    builder->SetInsertPoint(llvm::BasicBlock::Create(*context, "entry", target_fn));
    {
        iota_env *inner_env = new iota_env(global_env);
        for (int i=1; i<proto.size(); i++) {
            auto arg = target_fn->getArg(i - 1);
            auto var = create_var(proto[i].name, arg->getType(), inner_env);
            builder->CreateStore(arg, var);
        }
        llvm::Value *ret = NULL;
        for (int i=2; i<list.size(); i++) {
            ret = visit(list[i], inner_env);
        }
        builder->CreateRet(cast_to(ret, fn_type->getReturnType()));
        delete inner_env;
    }
    target_fn = curr_fn;
    builder->SetInsertPoint(curr_bb);
    return builder->getInt64(0);
}

