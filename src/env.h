
#pragma once

#include <map>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"


class iota_env
{
    std::map<std::string, llvm::Value*> record;
    iota_env *parent;

public:
    iota_env(iota_env *_parent)
    {
        record = std::map<std::string, llvm::Value*>{};
        parent = _parent;
    }

    llvm::Value *define(std::string name, llvm::Value *value)
    {
        record[name] = value;
        return value;
    }

    llvm::Value *lookup(std::string name)
    {
        iota_env *env = this;
        while (env != NULL) {
            if (env->record.count(name) > 0) {
                return env->record[name];
            } else {
                env = env->parent;
            }
        }
        return NULL;
    }
};


