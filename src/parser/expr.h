
#pragma once

#include <iostream>
#include <string>
#include <vector>


enum ExprType
{
    I_VALUE,
    F_VALUE,
    STRING,
    SYMBOL,
    LIST,
};


struct Expr
{
    ExprType type;

    union {
        int64_t i_val;
        double f_val;
    };

    std::string name;
    std::string anno;   // type annotation
    std::vector<Expr> list;

    // int value
    Expr(int64_t val) {
        type = ExprType::I_VALUE;
        i_val = val;
    }

    // float value
    Expr(double val) {
        type = ExprType::F_VALUE;
        f_val = val;
    }

    // string | symbol
    Expr(std::string val) {
        if (val[0] == '"') {
            type = ExprType::STRING;
            name = val.substr(1, val.size() - 2);
        } else {
            type = ExprType::SYMBOL;
            auto idx = val.find(":");

            if (idx != std::string::npos) {
                name = val.substr(0, idx);
                anno = val.substr(idx + 1);
            } else {
                name = val;
                anno = "";
            }
        }
    }

    // lists
    Expr(std::vector<Expr> val) {
        type = ExprType::LIST;
        list = val;
    }

    // print expr
    void print(void) {
        switch (type) {
        case I_VALUE:   std::cout << i_val; break;
        case F_VALUE:   std::cout << f_val; break;
        case STRING:    std::cout << "\"" << name << "\""; break;
        case SYMBOL:    std::cout << "<" << name << ">"; break;
        case LIST:      std::cout << "(";
                        for (int i=0; i<list.size(); i++) {
                            if (i > 0) {std::cout << " ";}
                            list[i].print();
                        }
                        std::cout << ")";
        }
    }
};

