// The macro mappings as constants: usable in constant expressions, with the
// values of the configuration compat.pocketpy builds; and at run time, the
// same configuration as the interpreter reports it (pkpy.configmacros).
import std;
import mcpplibs.mwrap.pocketpy;

namespace m = mcpplibs::mwrap::pocketpy::macros;

static_assert(std::string_view(m::PK_VERSION) == "2.2.0");
static_assert(m::PK_VERSION_MAJOR == 2 && m::PK_VERSION_MINOR == 2 && m::PK_VERSION_PATCH == 0);
static_assert(m::PK_ENABLE_OS == 1 && m::PK_ENABLE_THREADS == 0 && m::PK_ENABLE_DLL == 0);
static_assert(m::PK_ENABLE_DETERMINISM == 1 && m::PK_ENABLE_WATCHDOG == 1);
static_assert(m::PK_ENABLE_CUSTOM_SNAME == 0 && m::PK_ENABLE_MIMALLOC == 0);
static_assert(m::PK_GC_MIN_THRESHOLD == 20000 && m::PK_VM_STACK_SIZE == 16384);
static_assert(m::PK_MAX_CO_VARNAMES == 64 && m::PK_MAX_MODULE_PATH_LEN == 63);
static_assert(std::is_same_v<std::remove_cv_t<decltype(m::PK_INST_ATTR_LOAD_FACTOR)>, float>);
static_assert(m::PK_INST_ATTR_LOAD_FACTOR == 0.67f && m::PK_TYPE_ATTR_LOAD_FACTOR == 0.5f);
static_assert(std::is_same_v<std::remove_cv_t<decltype(m::PK_PLATFORM_SEP)>, char>);
static_assert(m::PK_IS_DESKTOP_PLATFORM == (m::PY_SYS_PLATFORM == 0 || m::PY_SYS_PLATFORM == 3 || m::PY_SYS_PLATFORM == 5));
static_assert(m::py_offset(nullptr, 0) == nullptr);
static_assert(std::is_same_v<decltype(m::py_r0()), py_GlobalRef>);
static_assert(std::is_same_v<decltype(m::TypeError("%d", 1)), bool>);

int main() {
    py_initialize();
    int failures = 0;
    for (auto [name, value] : {std::pair{"PK_ENABLE_OS", m::PK_ENABLE_OS}, {"PK_ENABLE_THREADS", m::PK_ENABLE_THREADS},
                               {"PK_ENABLE_DETERMINISM", m::PK_ENABLE_DETERMINISM},
                               {"PK_ENABLE_WATCHDOG", m::PK_ENABLE_WATCHDOG},
                               {"PK_GC_MIN_THRESHOLD", m::PK_GC_MIN_THRESHOLD},
                               {"PK_VM_STACK_SIZE", m::PK_VM_STACK_SIZE}}) {
        const std::string source = std::string("__import__('pkpy').configmacros['") + name + "']";
        const bool ok = py_eval(source.c_str(), nullptr) && py_toint(py_retval()) == value;
        std::println("{:<24} {:>6} {}", name, value, ok ? "ok" : "FAILED (the interpreter reports otherwise)");
        failures += !ok;
    }
    const bool platform = py_eval("__import__('sys').platform", nullptr) &&
                          std::string_view(py_tostr(py_retval())) == m::PY_SYS_PLATFORM_STRING;
    std::println("sys.platform == {:<12} {}", m::PY_SYS_PLATFORM_STRING, platform ? "ok" : "FAILED");
    failures += !platform;
    // PK_MALLOC / PK_FREE are the interpreter's allocator.
    void* p = m::PK_REALLOC(m::PK_MALLOC(8), 64);
    failures += p == nullptr;
    m::PK_FREE(p);
    py_finalize();
    return failures;
}
