// pkpy — run a Python file through import mcpplibs.mwrap.pocketpy, as
// upstream's own host (src2/main.c) runs it:
//
//   pkpy <file.py | file.pyc> [args…]   run it; exit 1 on an unhandled exception
//   pkpy --compile <file.py> <out.pyc>  compile it to bytecode
//
// tools/run_conformance.py runs PocketPy's tests/*.py on it, from source and
// compiled.
import std;
import mcpplibs.mwrap.pocketpy;

namespace {

std::optional<std::string> read_file(const char* path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return std::nullopt;
    return std::string(std::istreambuf_iterator<char>(in), {});
}

int usage() {
    std::println(std::cerr, "usage: pkpy <file.py|file.pyc> [args...]\n       pkpy --compile <file.py> <out.pyc>");
    return 2;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) return usage();
    const std::string_view first = argv[1];
    py_initialize();
    py_sys_setargv(argc, argv);  // sys.argv: argv[1] onwards

    bool ok = false;
    if (first == "--compile") {
        if (argc != 4) return usage();
        ok = py_compilefile(argv[2], argv[3]);
        if (!ok) py_printexc();
        py_finalize();
        return ok ? 0 : 1;
    }

    const std::optional<std::string> data = read_file(argv[1]);
    if (!data) {
        std::println(std::cerr, "cannot open file '{}'", first);
        py_finalize();
        return 1;
    }
    if (first.ends_with(".pyc")) {
        ok = py_execo(data->data(), static_cast<int>(data->size()), argv[1], nullptr);
    } else {
        ok = py_exec(data->c_str(), argv[1], EXEC_MODE, nullptr);
    }
    if (!ok) py_printexc();
    const int code = py_checkexc() ? 1 : 0;
    py_finalize();
    return code;
}
