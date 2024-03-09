
import glob
import os


_compile_flags = [
    "-O2",
    "-lstdc++",
]

_include_dirs = [
    "src",
]

_source_files = [
    "src/*.cpp",
    "main.cpp",
]


def _get_llvm_flags(tool="llvm-config"):
    test = os.popen(tool + " --cxxflags --ldflags --system-libs --libs core")
    flags = ""
    for line in test.readlines():
        flags += line.replace("\n", " ")
    return flags.replace("-fno-exceptions", "")


if __name__ == "__main__":

    command = "clang " + _get_llvm_flags()
    out_file = "iota_compiler"

    for flag in _compile_flags:
        command += " " + flag

    for dir in _include_dirs:
        command += " -I" + dir

    for src in _source_files:
        for file in glob.glob(src):
            command += " " + file

    command += " -o " + out_file

    print("command:")
    print(command)

    if os.system(command) == 0:
        print("build complete")
    else:
        print("build failed")

