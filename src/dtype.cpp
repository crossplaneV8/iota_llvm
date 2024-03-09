
#include "iota.h"


llvm::Type* iota_compiler::anno_type(std::string anno, llvm::Type *init)
{
    llvm::Type *ret = init;

    if (anno.size() > 0) {
        std::string buf;
        int ptr_rank = 0;
        for (char c : anno) {
            if (c == '*') {ptr_rank++;}
            else {buf.push_back(c);}
        }
        if (buf == "i8")    {ret = builder->getInt8Ty();}
        if (buf == "i16")   {ret = builder->getInt16Ty();}
        if (buf == "i32")   {ret = builder->getInt32Ty();}
        if (buf == "i64")   {ret = builder->getInt64Ty();}
        if (buf == "f32")   {ret = builder->getFloatTy();}
        if (buf == "f64")   {ret = builder->getDoubleTy();}
        if (ret != NULL) {
            for (int i=0; i<ptr_rank; i++) {
                ret = ret->getPointerTo();
            }
        }
    }
    return ret;
}


llvm::Value* iota_compiler::cast_to(llvm::Value *val, llvm::Type *type)
{
    llvm::Type *src_type = val->getType();
    if (src_type != type && type != NULL) {
        if (src_type->isPointerTy()) {
            if (type->isPointerTy()) {
                return builder->CreatePointerCast(val, type);
            } else if (type->isIntegerTy()) {
                return builder->CreatePtrToInt(val, type);
            } else if (type->isFloatingPointTy()) {
                assert(0);  // can not cast pointer to float
            }
        } else if (src_type->isIntegerTy()) {
            if (type->isPointerTy()) {
                return builder->CreateIntToPtr(val, type);
            } else if (type->isIntegerTy()) {
                return builder->CreateIntCast(val, type, true);
            } else if (type->isFloatingPointTy()) {
                return builder->CreateSIToFP(val, type);
            }
        } else if (src_type->isFloatingPointTy()) {
            if (type->isPointerTy()) {
                assert(0);  // can not cast float to pointer
            } else if (type->isIntegerTy()) {
                return builder->CreateFPToSI(val, type);
            } else if (type->isFloatingPointTy()) {
                return builder->CreateFPCast(val, type);
            }
        }
        assert(0);  // unexpected case
    }
    return val;
}

