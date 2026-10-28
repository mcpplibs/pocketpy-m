// The PocketPy embedding API through the module: the VM's life, exec and
// eval, values read back and built, dicts and lists walked, native functions
// (argc-based and signature-based), errors with their messages and the VM
// going on, exceptions alive across collections, exit() made the host's,
// code re-run and modules reloaded in one VM, imports served by the host,
// several VMs, the watchdog, the print callback, a native type with userdata,
// properties and a destructor, JSON and pickle, deterministic floats —
// spelled as pocketpy.h documents them.
#include <gtest/gtest.h>

import std;
import mcpplibs.mwrap.pocketpy;

using namespace mcpplibs::mwrap::pocketpy::macros;  // py_isint, py_r0, TypeError …

namespace {

// py_initialize once: py_finalize is irreversible. Each test starts from a
// reset VM 0.
struct Interpreter : ::testing::Environment {
    void SetUp() override { py_initialize(); }
    void TearDown() override { py_finalize(); }
};
const auto* const interpreter = ::testing::AddGlobalTestEnvironment(new Interpreter);

class Api : public ::testing::Test {
protected:
    void SetUp() override {
        py_switchvm(0);
        py_resetvm();
    }
    void TearDown() override { EXPECT_FALSE(py_checkexc()) << error(); }

    // The pending exception, formatted with its traceback, then cleared.
    static std::string error() {
        char* text = py_formatexc();
        std::string out = text ? text : "";
        py_free(text);
        py_clearexc(nullptr);
        return out;
    }
    static void exec(const char* source, const char* file = "<test>") {
        ASSERT_TRUE(py_exec(source, file, EXEC_MODE, nullptr)) << error();
    }
    // An expression's value, kept in register 7 (py_retval() is overwritten
    // by the next call, and is not an argument).
    static py_GlobalRef eval(const char* source) {
        if (!py_eval(source, nullptr)) {
            ADD_FAILURE() << source << ": " << error();
            py_newnone(py_r7());
            return py_r7();
        }
        py_assign(py_r7(), py_retval());
        return py_r7();
    }
    static std::string repr(const char* source) {
        py_Ref value = eval(source);
        if (!py_repr(value)) return "<repr failed: " + error() + ">";
        return py_tostr(py_retval());
    }
};

TEST_F(Api, ExecThenEvalReadsValuesBack) {
    exec("n = 6 * 7\nratio = 1 / 8\nname = 'po' + 'cketpy'\nok = n == 42\nnothing = None");
    py_Ref n = eval("n");
    ASSERT_TRUE(py_isint(n));
    EXPECT_EQ(py_toint(n), 42);
    py_Ref ratio = eval("ratio");
    ASSERT_TRUE(py_isfloat(ratio));
    EXPECT_EQ(py_tofloat(ratio), 0.125);
    py_Ref name = eval("name");
    ASSERT_TRUE(py_isstr(name));
    EXPECT_EQ(std::string_view(py_tostr(name)), "pocketpy");
    c11_sv view = py_tosv(name);
    EXPECT_EQ(std::string_view(view.data, static_cast<std::size_t>(view.size)), "pocketpy");
    py_Ref ok = eval("ok");
    ASSERT_TRUE(py_isbool(ok));
    EXPECT_TRUE(py_tobool(ok));
    EXPECT_TRUE(py_isnone(eval("nothing")));
    py_f64 widened = 0;
    EXPECT_TRUE(py_castfloat(eval("3"), &widened));
    EXPECT_EQ(widened, 3.0);
    // A global is reachable from C++ by name, too.
    py_ItemRef global = py_getglobal(py_name("n"));
    ASSERT_NE(global, nullptr);
    EXPECT_EQ(py_toint(global), 42);
}

int dict_total = 0;
std::vector<std::string> dict_keys;
bool collect(py_Ref key, py_Ref value, void* ctx) {
    dict_keys.emplace_back(py_tostr(key));
    *static_cast<int*>(ctx) += static_cast<int>(py_toint(value));
    return true;
}

TEST_F(Api, DictsAndListsAreWalked) {
    py_Ref scores = eval("{'ada': 36, 'bob': 4, 'cy': 2}");
    ASSERT_TRUE(py_isdict(scores));
    EXPECT_EQ(py_dict_len(scores), 3);
    dict_keys.clear();
    dict_total = 0;
    ASSERT_TRUE(py_dict_apply(scores, collect, &dict_total));
    EXPECT_EQ(dict_total, 42);
    EXPECT_EQ(dict_keys, (std::vector<std::string>{"ada", "bob", "cy"}));  // insertion order
    ASSERT_EQ(py_dict_getitem_by_str(scores, "bob"), 1);
    EXPECT_EQ(py_toint(py_retval()), 4);
    EXPECT_EQ(py_dict_getitem_by_str(scores, "dan"), 0);

    py_Ref rows = eval("[[i, i * i] for i in range(4)]");
    ASSERT_TRUE(py_islist(rows));
    ASSERT_EQ(py_list_len(rows), 4);
    std::vector<std::pair<long long, long long>> got;
    for (int i = 0; i < py_list_len(rows); ++i) {
        py_ItemRef row = py_list_getitem(rows, i);
        ASSERT_TRUE(py_islist(row));
        got.emplace_back(py_toint(py_list_getitem(row, 0)), py_toint(py_list_getitem(row, 1)));
    }
    EXPECT_EQ(got.back(), (std::pair<long long, long long>{3, 9}));

    py_Ref pair = eval("('x', 2.5)");
    ASSERT_TRUE(py_istuple(pair));
    ASSERT_EQ(py_tuple_len(pair), 2);
    EXPECT_EQ(std::string_view(py_tostr(py_tuple_getitem(pair, 0))), "x");
    EXPECT_EQ(py_tofloat(py_tuple_getitem(pair, 1)), 2.5);

    // Any iterable, through the iterator protocol.
    py_Ref letters = eval("'abc'");
    ASSERT_TRUE(py_iter(letters));
    py_assign(py_r6(), py_retval());
    std::string joined;
    for (int more; (more = py_next(py_r6())) == 1;) joined += py_tostr(py_retval());
    EXPECT_EQ(joined, "abc");
}

TEST_F(Api, ValuesBuiltInCppReachPython) {
    py_newdict(py_r0());
    py_newint(py_r1(), 3);
    ASSERT_TRUE(py_dict_setitem_by_str(py_r0(), "width", py_r1()));
    py_newlist(py_r2());
    for (const char* tag : {"a", "b"}) {
        py_newstr(py_r3(), tag);
        py_list_append(py_r2(), py_r3());
    }
    ASSERT_TRUE(py_dict_setitem_by_str(py_r0(), "tags", py_r2()));
    py_setglobal(py_name("config"), py_r0());
    EXPECT_EQ(repr("config['width'] * len(config['tags'])"), "6");
    EXPECT_EQ(repr("'-'.join(config['tags'])"), "'a-b'");
}

bool add(int argc, py_StackRef argv) {
    if (argc != 2) return TypeError("add() takes 2 arguments, got %d", argc);
    if (!py_checkint(&argv[0]) || !py_checkint(&argv[1])) return false;
    py_newint(py_retval(), py_toint(&argv[0]) + py_toint(&argv[1]));
    return true;
}

// Signature-based: Python fills the defaults and keyword arguments.
bool clamp(int argc, py_StackRef argv) {
    (void)argc;  // 3: x, lo, hi
    py_i64 x = 0, lo = 0, hi = 0;
    if (!py_castint(&argv[0], &x) || !py_castint(&argv[1], &lo) || !py_castint(&argv[2], &hi)) return false;
    if (lo > hi) return ValueError("empty range %i..%i", lo, hi);
    py_newint(py_retval(), std::clamp(x, lo, hi));
    return true;
}

TEST_F(Api, NativeFunctionsAreCalledFromPython) {
    py_GlobalRef main = py_getmodule("__main__");
    py_bindfunc(main, "add", add);
    py_bind(main, "clamp(x, lo=0, hi=10)", clamp);
    EXPECT_EQ(repr("add(40, 2)"), "42");
    EXPECT_EQ(repr("[clamp(v) for v in (-5, 5, 50)]"), "[0, 5, 10]");
    EXPECT_EQ(repr("clamp(50, hi=99)"), "50");
    EXPECT_FALSE(py_eval("add(1)", nullptr));
    EXPECT_NE(error().find("TypeError: add() takes 2 arguments, got 1"), std::string::npos);
    EXPECT_FALSE(py_eval("add(1, 'x')", nullptr));
    EXPECT_NE(error().find("TypeError"), std::string::npos);
    EXPECT_FALSE(py_eval("clamp(1, 5, 2)", nullptr));
    EXPECT_NE(error().find("ValueError: empty range 5..2"), std::string::npos);
    // A native error caught by Python code.
    exec("try:\n    add('a')\nexcept TypeError as e:\n    caught = str(e)\n");
    EXPECT_EQ(repr("caught"), "'add() takes 2 arguments, got 1'");
    // C++ calls a Python function.
    exec("def area(w, h): return w * h");
    py_newint(py_r0(), 6);
    py_newint(py_r1(), 7);
    ASSERT_TRUE(py_call(py_getglobal(py_name("area")), 2, py_r0())) << error();
    EXPECT_EQ(py_toint(py_retval()), 42);
}

TEST_F(Api, ErrorsCarryMessagesAndTheVmGoesOn) {
    exec("kept = 'still here'");
    EXPECT_FALSE(py_exec("def f(:\n    pass", "broken.py", EXEC_MODE, nullptr));
    std::string syntax = error();
    EXPECT_NE(syntax.find("SyntaxError"), std::string::npos) << syntax;
    EXPECT_NE(syntax.find("broken.py"), std::string::npos) << syntax;

    EXPECT_FALSE(py_exec("def g(d):\n    return d['missing']\ng({})", "lookup.py", EXEC_MODE, nullptr));
    std::string key = error();
    EXPECT_NE(key.find("KeyError"), std::string::npos) << key;
    EXPECT_NE(key.find("lookup.py\", line 2"), std::string::npos) << key;

    EXPECT_FALSE(py_exec("raise RuntimeError('custom 7')", "<raise>", EXEC_MODE, nullptr));
    ASSERT_TRUE(py_checkexc());
    EXPECT_TRUE(py_matchexc(tp_RuntimeError));
    EXPECT_NE(error().find("custom 7"), std::string::npos);

    EXPECT_FALSE(py_exec("def r(n): return r(n + 1)\nr(0)", "<recursion>", EXEC_MODE, nullptr));
    EXPECT_NE(error().find("RecursionError"), std::string::npos);

    EXPECT_FALSE(py_eval("undefined_name", nullptr));
    EXPECT_NE(error().find("NameError"), std::string::npos);
    EXPECT_FALSE(py_eval("1 / 0", nullptr));
    EXPECT_NE(error().find("ZeroDivisionError"), std::string::npos);

    EXPECT_EQ(repr("kept"), "'still here'");
}

TEST_F(Api, ExceptionsSurviveCollections) {
    // compat.pocketpy applies upstream's fix (de8351ed): without it the
    // collector reads the unset traceback values of a live exception.
    exec("import gc\n"
         "def deep(n):\n"
         "    if n == 0: raise ValueError('deep')\n"
         "    return deep(n - 1)\n"
         "kept = []\n"
         "for r in range(20):\n"
         "    junk = [str(i) * 50 for i in range(2000)]\n"
         "    junk = None\n"
         "    gc.collect()\n"
         "    for i in range(20):\n"
         "        try:\n"
         "            deep(10)\n"
         "        except ValueError as e:\n"
         "            kept.append(e)\n"
         "    gc.collect()\n");
    EXPECT_EQ(repr("len(kept), str(kept[-1])"), "(400, 'deep')");
}

// builtins.exit ends the process (the C library's exit); a host that must
// outlive its scripts binds its own, raising SystemExit instead.
bool exit_raises(int argc, py_StackRef argv) {
    py_i64 code = 0;
    if (argc == 1 && !py_castint(&argv[0], &code)) return false;
    return py_exception(tp_SystemExit, "%i", code);
}

TEST_F(Api, TheHostDecidesWhatExitDoes) {
    py_bindfunc(py_getmodule("builtins"), "exit", exit_raises);
    EXPECT_FALSE(py_exec("before = 1\nexit(3)\nunreached = 1", "<exit>", EXEC_MODE, nullptr));
    EXPECT_TRUE(py_matchexc(tp_SystemExit));
    EXPECT_NE(error().find("SystemExit: 3"), std::string::npos);
    EXPECT_EQ(repr("before"), "1");
    EXPECT_FALSE(py_eval("unreached", nullptr));
    EXPECT_NE(error().find("NameError"), std::string::npos);
}

TEST_F(Api, CodeIsReRunInOneVm) {
    exec("count = 0\ndef step():\n    global count\n    count += 1\n    return 'v1'");
    for (int i = 0; i < 3; ++i) EXPECT_EQ(repr("step()"), "'v1'");
    // The new definition replaces the old; the state stays.
    exec("def step():\n    global count\n    count += 10\n    return 'v2'");
    EXPECT_EQ(repr("step()"), "'v2'");
    EXPECT_EQ(repr("count"), "13");
    // A module of the host's, re-run with new code.
    py_GlobalRef plugin = py_newmodule("plugin");
    ASSERT_TRUE(py_exec("def greet(): return 'hello'", "plugin.py", EXEC_MODE, plugin)) << error();
    EXPECT_EQ(repr("__import__('plugin').greet()"), "'hello'");
    ASSERT_TRUE(py_exec("def greet(): return 'hi again'", "plugin.py", EXEC_MODE, plugin)) << error();
    EXPECT_EQ(repr("__import__('plugin').greet()"), "'hi again'");
}

// Imports served from the host's memory, not the file system.
std::string library_source = "VERSION = 1\ndef twice(x): return 2 * x\n";
char* import_from_memory(const char* path, int* size) {
    if (std::string_view(path) != "helpers.py") return nullptr;
    auto* data = static_cast<char*>(py_malloc(library_source.size() + 1));
    std::memcpy(data, library_source.c_str(), library_source.size() + 1);
    if (size) *size = static_cast<int>(library_source.size());  // null from py_importlib_reload
    return data;
}

TEST_F(Api, ImportsAndReloadsFromTheHost) {
    auto* const previous = py_callbacks()->importfile;
    py_callbacks()->importfile = import_from_memory;
    exec("import helpers");
    EXPECT_EQ(repr("helpers.twice(21), helpers.VERSION"), "(42, 1)");
    library_source = "VERSION = 2\ndef twice(x): return x + x + 0.5\n";
    ASSERT_EQ(py_import("helpers"), 1) << error();
    py_assign(py_r0(), py_retval());
    ASSERT_TRUE(py_importlib_reload(py_r0())) << error();
    EXPECT_EQ(repr("helpers.twice(21), helpers.VERSION"), "(42.5, 2)");
    EXPECT_FALSE(py_exec("import missing_module", "<import>", EXEC_MODE, nullptr));
    EXPECT_NE(error().find("ImportError"), std::string::npos);
    py_callbacks()->importfile = previous;
    library_source = "VERSION = 1\ndef twice(x): return 2 * x\n";
}

TEST_F(Api, VmsAreSeparateAndResettable) {
    exec("who = 'vm0'");
    int context = 7;
    py_setvmctx(&context);
    py_switchvm(1);
    EXPECT_EQ(py_currentvm(), 1);
    EXPECT_EQ(py_getvmctx(), nullptr);
    EXPECT_FALSE(py_eval("who", nullptr));
    EXPECT_NE(error().find("NameError"), std::string::npos);
    exec("who = 'vm1'");
    py_switchvm(0);
    EXPECT_EQ(py_getvmctx(), &context);
    EXPECT_EQ(repr("who"), "'vm0'");
    py_resetvm();
    EXPECT_FALSE(py_eval("who", nullptr));
    EXPECT_NE(error().find("NameError"), std::string::npos);
    py_switchvm(1);
    EXPECT_EQ(repr("who"), "'vm1'");
    py_resetvm();
    py_switchvm(0);
}

TEST_F(Api, TheWatchdogEndsARunawayScript) {
    ASSERT_EQ(PK_ENABLE_WATCHDOG, 1);
    py_watchdog_begin(50);
    EXPECT_FALSE(py_exec("while True: pass", "<loop>", EXEC_MODE, nullptr));
    py_watchdog_end();
    EXPECT_TRUE(py_matchexc(tp_TimeoutError));
    EXPECT_NE(error().find("TimeoutError"), std::string::npos);
    EXPECT_EQ(repr("1 + 1"), "2");
}

std::string printed;
void capture(const char* text) { printed += text; }

TEST_F(Api, PrintGoesThroughTheCallback) {
    auto* const previous = py_callbacks()->print;
    py_callbacks()->print = capture;
    printed.clear();
    exec("print('a', 1, sep='=')\nprint([1, 2], end='!')");
    py_callbacks()->print = previous;
    EXPECT_EQ(printed, "a=1\n[1, 2]!");
}

struct Counter {
    py_i64 value;
};
int counters_destroyed = 0;
py_Type counter_type = 0;

bool counter_new(int argc, py_StackRef argv) {
    (void)argv;
    if (argc != 1) return TypeError("Counter() takes no arguments");
    auto* c = static_cast<Counter*>(py_newobject(py_retval(), counter_type, 0, sizeof(Counter)));
    c->value = 0;
    return true;
}
bool counter_bump(int argc, py_StackRef argv) {
    if (argc != 2) return TypeError("bump(n)");
    py_i64 n = 0;
    if (!py_castint(&argv[1], &n)) return false;
    auto* c = static_cast<Counter*>(py_touserdata(&argv[0]));
    c->value += n;
    py_newint(py_retval(), c->value);
    return true;
}
bool counter_value(int, py_StackRef argv) {
    py_newint(py_retval(), static_cast<Counter*>(py_touserdata(&argv[0]))->value);
    return true;
}

TEST_F(Api, ANativeTypeWithUserdata) {
    counters_destroyed = 0;
    counter_type = py_newtype("Counter", tp_object, py_getmodule("__main__"), [](void*) { ++counters_destroyed; });
    py_bindmagic(counter_type, py_name("__new__"), counter_new);
    py_bindmethod(counter_type, "bump", counter_bump);
    py_bindproperty(counter_type, "value", counter_value, nullptr);
    exec("c = Counter()\nc.bump(40)\nc.bump(2)");
    EXPECT_EQ(repr("c.value"), "42");
    EXPECT_EQ(repr("type(c).__name__"), "'Counter'");
    exec("c = None\nfor _ in range(3): Counter()");
    py_gc_collect();
    EXPECT_EQ(counters_destroyed, 4);
}

TEST_F(Api, JsonAndPickle) {
    ASSERT_TRUE(py_json_loads("{\"name\": \"py\", \"list\": [1, 2.5, null, true]}")) << error();
    py_assign(py_r0(), py_retval());
    ASSERT_TRUE(py_isdict(py_r0()));
    ASSERT_EQ(py_dict_getitem_by_str(py_r0(), "list"), 1);
    EXPECT_EQ(py_list_len(py_retval()), 4);
    ASSERT_TRUE(py_json_dumps(py_r0(), 0)) << error();
    EXPECT_EQ(std::string_view(py_tostr(py_retval())), "{\"name\": \"py\", \"list\": [1, 2.5, null, true]}");

    ASSERT_TRUE(py_pickle_dumps(py_r0())) << error();
    int size = 0;
    unsigned char* bytes = py_tobytes(py_retval(), &size);
    std::vector<unsigned char> copy(bytes, bytes + size);
    ASSERT_TRUE(py_pickle_loads(copy.data(), size)) << error();
    py_assign(py_r1(), py_retval());
    EXPECT_EQ(py_equal(py_r0(), py_r1()), 1);
}

TEST_F(Api, FloatsAreDeterministic) {
    // PK_ENABLE_DETERMINISM with upstream's own dmath: the same bits on every
    // platform, which are not always the C library's (log(10) is one ulp
    // below glibc's). Values pinned by upstream's 930_deterministic_float.py.
    EXPECT_EQ(PK_ENABLE_DETERMINISM, 1);
    EXPECT_EQ(py_tofloat(eval("__import__('math').log(10)")), 2.30258509299404545700440394284);
    EXPECT_EQ(py_tofloat(eval("__import__('math').log(28.897124)")), 3.363742074595449);
    EXPECT_EQ(py_tofloat(eval("__import__('math').exp(4.729036)")), 113.186398052200445363268954679);
    EXPECT_TRUE(py_tobool(eval("__import__('math').exp(1) == __import__('math').e")));
    // Left-to-right addition, the same everywhere (CPython 3.12+ compensates
    // the rounding in sum() and gets 4.5).
    EXPECT_EQ(py_tofloat(eval("sum([0.1 * i for i in range(10)])")), 4.500000000000001);
}

TEST_F(Api, Version) {
    EXPECT_EQ(std::string_view(PK_VERSION), "2.2.0");
    EXPECT_EQ(PK_VERSION_MAJOR, 2);
    EXPECT_EQ(PK_VERSION_MINOR, 2);
    EXPECT_EQ(repr("__import__('sys').version"), "'2.2.0'");
}

}  // namespace
