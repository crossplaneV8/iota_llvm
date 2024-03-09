
#include "iota.h"


int main(int argc, const char **argv)
{
    const char *src_file = "test.iota";
    const char *out_file = "out.ll";

    if (argc > 1) {
        src_file = argv[1];
    }
    if (argc > 2) {
        out_file = argv[2];
    }
    iota_compiler compiler;
    compiler.compile(src_file, out_file);
    return 0;
}

