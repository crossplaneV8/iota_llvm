
#include "iota.h"
#include "parser/parser.h"


static std::string _read_src(const char *file_path)
{
    std::ostringstream oss;
    std::ifstream file(file_path);
    if (file.is_open()) {
        std::string line;
        oss << "(def (main:i32)\n";
        while (std::getline(file, line)) {
            oss << line << "\n";
        }
        oss << ")\n";
        file.close();
    } else {
        printf("failed to open file: %s\n", file_path);
        assert(0);
    }
    return oss.str();
}


void iota_compiler::import_std_functions(void)
{
    std::vector<std::string> fn_proto_list[] = {
        {"printf", "i32", "*i8", "..."},
        {"scanf", "i32", "*i8", "..."},
        {"malloc", "*i8", "i64"},
        {"free", "i32", "*i8"},
    };
    for (auto &proto : fn_proto_list) {
        bool is_var_arg = false;
        if (proto[proto.size() - 1] == "...") {
            proto.pop_back();
            is_var_arg = true;
        }
        llvm::Type *ret_type = anno_type(proto[1]);
        std::vector<llvm::Type*> arg_type;
        for (int i=2; i<proto.size(); i++) {
            arg_type.push_back(anno_type(proto[i]));
        }
        auto fn_type = llvm::FunctionType::get(ret_type, arg_type, is_var_arg);
        module->getOrInsertFunction(proto[0], fn_type);
        global_env->define(proto[0], module->getFunction(proto[0]));
    }
}


void iota_compiler::compile(const char *src_file, const char *out_file)
{
    // read source code from file
    std::string src_code = _read_src(src_file);

    // parse source code to AST
    auto parser = syntax::parser();
    auto ast = parser.parse(src_code);

    // convert AST to IR
    import_std_functions();
    visit(ast, global_env);

    // save IR to file
    std::error_code err;
    llvm::raw_fd_ostream outll(out_file, err);
    module->print(outll, NULL);
}

