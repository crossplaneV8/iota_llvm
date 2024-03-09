
#include "iota.h"


static size_t _size_in_bytes(llvm::Type *type)
{
    size_t bits = type->getScalarSizeInBits();
    if (bits > 0) {
        return (bits + 7) / 8;
    } else {
        assert(0);  // unexpected case
    }
    return 0;
}


llvm::Value* iota_compiler::visit(Expr &exp, iota_env *env)
{
    switch (exp.type) {
        case ExprType::I_VALUE: {
            return builder->getInt64(exp.i_val);
        }
        case ExprType::F_VALUE: {
            llvm::ConstantInt *data = builder->getInt64(exp.i_val);
            return builder->CreateBitCast(data, builder->getDoubleTy());
        }
        case ExprType::STRING: {
            auto re = std::regex("\\\\n");
            auto str = std::regex_replace(exp.name, re, "\n");
            return builder->CreateGlobalStringPtr(str);
        }
        case ExprType::SYMBOL: {
            // search symbol table
            if (auto item = env->lookup(exp.name)) {
                if (auto var = llvm::dyn_cast<llvm::AllocaInst>(item)) {
                    return builder->CreateLoad(var->getAllocatedType(), var, exp.name.c_str());
                }
                if (auto fn = llvm::dyn_cast<llvm::Function>(item)) {
                    return fn;
                }
            }
            printf("undefined symbol <%s>\n", exp.name.c_str());
            assert(0);
        }
        case ExprType::LIST: {
            return visit_list(exp.list, env);
        }
    }
    return builder->getInt64(0);
}


llvm::Value* iota_compiler::visit_list(std::vector<Expr> &list, iota_env *env)
{
    assert(list.size() > 0);

    if (list[0].type == ExprType::SYMBOL) {
        std::string head = list[0].name;

        // begin expression: (begin <exp0> <exp1> ...)
        if (head == "begin") {
            iota_env *inner_env = new iota_env(env);
            llvm::Value *ret = NULL;
            for (int i=1; i<list.size(); i++) {
                ret = visit(list[i], inner_env);
            }
            delete inner_env;
            return ret;
        }

        // define local variable: (var <name> <init>)
        if (head == "var" && list.size() == 3) {
            assert(list[1].type == ExprType::SYMBOL);
            auto init = visit(list[2], env);
            auto type = anno_type(list[1].anno, init->getType());
            auto var = create_var(list[1].name, type, env);
            builder->CreateStore(cast_to(init, type), var);
            return builder->getInt64(0);
        }

        // allocate array: (array <len>)
        if (head == "array" && list.size() == 2) {
            llvm::Type *type = anno_type(list[0].anno, builder->getInt64Ty());
            llvm::Value *len = visit(list[1], env);
            llvm::AllocaInst *arr = builder->CreateAlloca(type, len);
            llvm::Value *elem_size = builder->getInt64(_size_in_bytes(type));
            llvm::Value *size = builder->CreateMul(len, elem_size);
            builder->CreateMemSet(arr, builder->getInt8(0), size, llvm::MaybeAlign(8));
            return arr;
        }

        // write variable
        if (head == "set" && list.size() == 3) {
            // (set <var> <val>)
            if (list[1].type == ExprType::SYMBOL) {
                auto var = env->lookup(list[1].name);
                auto type = var->getType()->getPointerElementType();
                auto val = cast_to(visit(list[2], env), type);
                builder->CreateStore(val, var);
            }

            // (set (<arr> <idx>) <val>)
            if (list[1].type == ExprType::LIST) {
                assert(list[1].list.size() == 2);
                llvm::Value *arr = visit(list[1].list[0], env);
                std::vector<llvm::Value*> idx{visit(list[1].list[1], env)};
                auto elem_type = arr->getType()->getPointerElementType();
                auto ptr = builder->CreateGEP(elem_type, arr, idx, "ptr");
                auto val = cast_to(visit(list[2], env), elem_type);
                builder->CreateStore(val, ptr);
            }
            return builder->getInt64(0);
        }

        // (++ i) ==> (set i (+ i 1))
        // (-- i) ==> (set i (- i 1))
        if ((head == "++" || head == "--") && list.size() == 2) {
            auto step = builder->getInt64(head == "++" ? 1 : -1);
            auto val0 = visit(list[1], env);
            auto beta = cast_to(step, val0->getType());
            auto val1 = builder->CreateAdd(val0, beta);
            auto var = env->lookup(list[1].name);
            builder->CreateStore(val1, var);
            return builder->getInt64(0);
        }

        // (cast:<type> <val>)
        if (head == "cast" && list.size() == 2) {
            auto type = anno_type(list[0].anno);
            return cast_to(visit(list[1], env), type);
        }

        // (bitcast:<type> <val>)
        if (head == "bitcast" && list.size() == 2) {
            auto type = anno_type(list[0].anno);
            return builder->CreateBitCast(visit(list[1], env), type);
        }

        // get variable address: (& <var>)
        if (head == "&" && list.size() == 2) {
            return env->lookup(list[1].name);
        }

        // (if <cond> <then> <else>)
        if (head == "if" && list.size() >= 3 && list.size() <= 4) {
            return visit_if(list, env);
        }

        // (while <cond> <exp0> <exp1> ...)
        if (head == "while" && list.size() >= 2) {
            return visit_while(list, env);
        }

        // import extern function: (import (<func> <arg0> <arg1> ...))
        if (head == "import" && list.size() == 2) {
            return visit_import(list, env);
        }

        // define function: (def (<func> <arg0> <arg1> ...) <exp0> <exp1> ...)
        if (head == "def" && list.size() >= 3) {
            return visit_define(list, env);
        }

        // math expr
        if (auto result = visit_math_expr(list, env)) {
            return result;
        }
    }

    if (1) {
        llvm::Value *obj = visit(list[0], env);

        // call function: (<fn> <arg0> <arg1> ...)
        if (auto fn = llvm::dyn_cast<llvm::Function>(obj)) {
            std::vector<llvm::Value*> args;
            for (int i=1; i<list.size(); i++) {
                if (fn->isVarArg()) {
                    args.push_back(visit(list[i], env));
                } else {
                    llvm::Type *type = fn->getArg(i - 1)->getType();
                    args.push_back(cast_to(visit(list[i], env), type));
                }
            }
            return builder->CreateCall(fn, args);
        }

        // array indexing: (<arr> <idx>)
        if (obj->getType()->isPointerTy()) {
            assert(list.size() == 2);
            std::vector<llvm::Value*> idx{visit(list[1], env)};
            auto ptr = builder->CreateGEP(obj->getType()->getPointerElementType(), obj, idx, "ptr");
            return builder->CreateLoad(ptr->getType()->getPointerElementType(), ptr);
        }
        printf("unexpected case\n");
        assert(0);
    }

    return builder->getInt64(0);
}

