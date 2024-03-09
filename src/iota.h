
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"


#include "env.h"
#include "parser/expr.h"


class iota_compiler
{
    iota_env *global_env;

    llvm::LLVMContext *context;
    llvm::Module *module;
    llvm::IRBuilder<> *builder;

    llvm::Function *target_fn;

public:

    iota_compiler(void) {
        global_env = new iota_env(NULL);
        context = new llvm::LLVMContext();
        module = new llvm::Module("iota", *context);
        builder = new llvm::IRBuilder<>(*context);
        target_fn = NULL;
    }

    ~iota_compiler(void) {
        delete global_env;
        delete builder;
        delete module;
        delete context;
    }

    void compile(const char *src_file, const char *out_file);

private:

    void import_std_functions(void);

    llvm::Value* visit(Expr &exp, iota_env *env);

    llvm::Value* visit_list(std::vector<Expr> &list, iota_env *env);

    llvm::FunctionType* fn_type_from_proto(std::vector<Expr> &proto);

    llvm::Value* visit_import(std::vector<Expr> &list, iota_env *env);

    llvm::Value* visit_define(std::vector<Expr> &list, iota_env *env);

    llvm::Value* visit_if(std::vector<Expr> &list, iota_env *env);

    llvm::Value* visit_while(std::vector<Expr> &list, iota_env *env);

    llvm::Value* visit_math_expr(std::vector<Expr> &list, iota_env *env);

    llvm::AllocaInst* create_var(std::string name, llvm::Type *type, iota_env *env);

    llvm::Type* anno_type(std::string anno, llvm::Type *init=NULL);

    llvm::Value* cast_to(llvm::Value *val, llvm::Type *type);

    llvm::Type* auto_promote(llvm::Value *args[], int num);

    llvm::Value* is_non_zero(llvm::Value *val);

};


