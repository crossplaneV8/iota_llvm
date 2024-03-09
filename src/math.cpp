
#include "iota.h"


llvm::Type* iota_compiler::auto_promote(llvm::Value *args[], int num)
{
    llvm::Type *ftype = NULL;
    llvm::Type *itype = NULL;
    int fbits = 0, ibits = 0;

    for (int i=0; i<num; i++) {
        llvm::Type *type = args[i]->getType();
        int bits = type->getScalarSizeInBits();

        if (type->isFloatingPointTy() && bits > fbits) {
            ftype = type; fbits = bits;
        }
        if (type->isIntegerTy() && bits > ibits) {
            itype = type; ibits = bits;
        }
    }
    llvm::Type *type = ftype ? ftype : itype;
    for (int i=0; i<num; i++) {
        args[i] = cast_to(args[i], type);
        assert(args[i]->getType() == type);
    }
    return type;
}


llvm::Value* iota_compiler::visit_math_expr(std::vector<Expr> &list, iota_env *env)
{
    std::string &head = list[0].name;

    // math ops: (<math_op> <arg0> <arg1> ...)
    if (list.size() >= 3) {
        std::map<std::string, std::vector<llvm::Instruction::BinaryOps>> op_map = {
            {"+", {llvm::Instruction::BinaryOps::Add, llvm::Instruction::BinaryOps::FAdd}},
            {"-", {llvm::Instruction::BinaryOps::Sub, llvm::Instruction::BinaryOps::FSub}},
            {"*", {llvm::Instruction::BinaryOps::Mul, llvm::Instruction::BinaryOps::FMul}},
            {"/", {llvm::Instruction::BinaryOps::SDiv, llvm::Instruction::BinaryOps::FDiv}},
            {"%", {llvm::Instruction::BinaryOps::SRem, llvm::Instruction::BinaryOps::FRem}},
            {"<<", {llvm::Instruction::BinaryOps::Shl}},
            {">>", {llvm::Instruction::BinaryOps::LShr}},
            {"&", {llvm::Instruction::BinaryOps::And}},
            {"and", {llvm::Instruction::BinaryOps::And}},
            {"|", {llvm::Instruction::BinaryOps::Or}},
            {"or", {llvm::Instruction::BinaryOps::Or}},
            {"^", {llvm::Instruction::BinaryOps::Xor}},
            {"xor", {llvm::Instruction::BinaryOps::Xor}},
        };
        if (op_map.count(head) > 0) {
            const int num = list.size() - 1;
            llvm::Value *args[num];
            for (int i=0; i<num; i++) {
                args[i] = visit(list[i+1], env);
            }
            auto type = auto_promote(args, num);
            int idx = type->isFloatingPointTy();
            auto op = op_map[head].at(idx);
            auto out = args[0];
            for (int i=1; i<num; i++) {
                out = builder->CreateBinOp(op, out, args[i]);
            }
            return out;
        }
    }

    // bit inverse
    if ((head == "~" || head == "not") && list.size() == 2) {
        auto x = visit(list[1], env);
        return builder->CreateNot(x);
    }

    // cmp ops
    if (list.size() == 3) {
        std::map<std::string, std::vector<llvm::CmpInst::Predicate>> op_map = {
            {">", {llvm::CmpInst::Predicate::ICMP_SGT, llvm::CmpInst::Predicate::FCMP_OGT}},
            {"<", {llvm::CmpInst::Predicate::ICMP_SLT, llvm::CmpInst::Predicate::FCMP_OLT}},
            {">=", {llvm::CmpInst::Predicate::ICMP_SGE, llvm::CmpInst::Predicate::FCMP_OGE}},
            {"<=", {llvm::CmpInst::Predicate::ICMP_SLE, llvm::CmpInst::Predicate::FCMP_OLE}},
            {"==", {llvm::CmpInst::Predicate::ICMP_EQ, llvm::CmpInst::Predicate::FCMP_OEQ}},
            {"!=", {llvm::CmpInst::Predicate::ICMP_NE, llvm::CmpInst::Predicate::FCMP_UNE}},
        };
        if (op_map.count(head) > 0) {
            llvm::Value *args[] = {visit(list[1], env), visit(list[2], env)};
            auto type = auto_promote(args, sizeof(args)/sizeof(args[0]));
            auto op = op_map[head].at(type->isFloatingPointTy());
            return builder->CreateCmp(op, args[0], args[1]);
        }
    }

    return NULL;
}

