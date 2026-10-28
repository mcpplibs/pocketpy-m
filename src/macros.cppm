// mcpplibs.mwrap.pocketpy.macros — the macros of <pocketpy.h> (a named module
// cannot export a macro), under their original names in
// mcpplibs::mwrap::pocketpy::macros:
//
//   - the version, the configuration and the limits (PK_VERSION*,
//     PK_ENABLE_*, PK_GC_MIN_THRESHOLD, PK_VM_STACK_SIZE …) and the platform
//     (PY_SYS_PLATFORM, PY_SYS_PLATFORM_STRING, PK_PLATFORM_SEP …) as
//     `inline constexpr`, each of its macro's type;
//   - the type tests (py_isint … py_isnone, py_checkint … py_checkstr), the
//     registers (py_r0 … py_r7, py_tmpr0 … py_tmpr3, py_sysr0, py_sysr1),
//     py_offset, py_assign, PK_MALLOC / PK_REALLOC / PK_FREE and py_callcfunc
//     as functions;
//   - the exception helpers (TypeError, ValueError, RuntimeError …) as
//     functions; those taking a format and its arguments are variadic
//     templates passing them on to py_exception unchanged.
//
// PY_CHECK_ARGC and PY_CHECK_ARG_TYPE return from the native function they
// are written in, and py_arg names that function's `argv`: those stay
// macros, in <mwrap/pocketpy/macros.hpp>, included before the import.
// tools/check_exports.py accounts for every macro.
//
// The PK_ENABLE_* constants are the library's configuration, the values
// compat.pocketpy builds the interpreter with. config.h's defaults, which a
// translation unit including <pocketpy.h> itself reads, differ from them for
// DETERMINISM and WATCHDOG (0 there); nothing else in the header depends on
// either.
module;
#define PK_ENABLE_OS 1
#define PK_ENABLE_THREADS 0
#define PK_ENABLE_DLL 0
#define PK_ENABLE_DETERMINISM 1
#define PK_ENABLE_WATCHDOG 1
#include <pocketpy.h>

namespace mwrap_pocketpy_capture {
inline constexpr char value_PK_VERSION[] = PK_VERSION;
inline constexpr auto value_PK_VERSION_MAJOR = PK_VERSION_MAJOR;
inline constexpr auto value_PK_VERSION_MINOR = PK_VERSION_MINOR;
inline constexpr auto value_PK_VERSION_PATCH = PK_VERSION_PATCH;
inline constexpr auto value_PK_ENABLE_OS = PK_ENABLE_OS;
inline constexpr auto value_PK_ENABLE_THREADS = PK_ENABLE_THREADS;
inline constexpr auto value_PK_ENABLE_DLL = PK_ENABLE_DLL;
inline constexpr auto value_PK_ENABLE_DETERMINISM = PK_ENABLE_DETERMINISM;
inline constexpr auto value_PK_ENABLE_WATCHDOG = PK_ENABLE_WATCHDOG;
inline constexpr auto value_PK_ENABLE_CUSTOM_SNAME = PK_ENABLE_CUSTOM_SNAME;
inline constexpr auto value_PK_ENABLE_MIMALLOC = PK_ENABLE_MIMALLOC;
inline constexpr auto value_PK_GC_MIN_THRESHOLD = PK_GC_MIN_THRESHOLD;
inline constexpr auto value_PK_VM_STACK_SIZE = PK_VM_STACK_SIZE;
inline constexpr auto value_PK_MAX_CO_VARNAMES = PK_MAX_CO_VARNAMES;
inline constexpr auto value_PK_MAX_MODULE_PATH_LEN = PK_MAX_MODULE_PATH_LEN;
inline constexpr auto value_PK_INST_ATTR_LOAD_FACTOR = PK_INST_ATTR_LOAD_FACTOR;
inline constexpr auto value_PK_TYPE_ATTR_LOAD_FACTOR = PK_TYPE_ATTR_LOAD_FACTOR;
inline constexpr auto value_PK_PLATFORM_SEP = PK_PLATFORM_SEP;
inline constexpr auto value_PY_SYS_PLATFORM = PY_SYS_PLATFORM;
inline constexpr char value_PY_SYS_PLATFORM_STRING[] = PY_SYS_PLATFORM_STRING;
inline constexpr auto value_PK_IS_DESKTOP_PLATFORM = PK_IS_DESKTOP_PLATFORM;
}  // namespace mwrap_pocketpy_capture

#undef PK_VERSION
#undef PK_VERSION_MAJOR
#undef PK_VERSION_MINOR
#undef PK_VERSION_PATCH
#undef PK_ENABLE_OS
#undef PK_ENABLE_THREADS
#undef PK_ENABLE_DLL
#undef PK_ENABLE_DETERMINISM
#undef PK_ENABLE_WATCHDOG
#undef PK_ENABLE_CUSTOM_SNAME
#undef PK_ENABLE_MIMALLOC
#undef PK_GC_MIN_THRESHOLD
#undef PK_VM_STACK_SIZE
#undef PK_MAX_CO_VARNAMES
#undef PK_MAX_MODULE_PATH_LEN
#undef PK_INST_ATTR_LOAD_FACTOR
#undef PK_TYPE_ATTR_LOAD_FACTOR
#undef PK_PLATFORM_SEP
#undef PY_SYS_PLATFORM
#undef PY_SYS_PLATFORM_STRING
#undef PK_IS_DESKTOP_PLATFORM
#undef PK_MALLOC
#undef PK_REALLOC
#undef PK_FREE
#undef py_isint
#undef py_isfloat
#undef py_isbool
#undef py_isstr
#undef py_islist
#undef py_istuple
#undef py_isdict
#undef py_isnil
#undef py_isnone
#undef py_checkint
#undef py_checkfloat
#undef py_checkbool
#undef py_checkstr
#undef py_r0
#undef py_r1
#undef py_r2
#undef py_r3
#undef py_r4
#undef py_r5
#undef py_r6
#undef py_r7
#undef py_tmpr0
#undef py_tmpr1
#undef py_tmpr2
#undef py_tmpr3
#undef py_sysr0
#undef py_sysr1
#undef py_callcfunc
#undef py_offset
#undef py_assign
#undef NameError
#undef TypeError
#undef RuntimeError
#undef TimeoutError
#undef OSError
#undef ValueError
#undef IndexError
#undef ImportError
#undef ZeroDivisionError
#undef AttributeError
#undef UnboundLocalError

export module mcpplibs.mwrap.pocketpy.macros;

namespace c = mwrap_pocketpy_capture;

export namespace mcpplibs::mwrap::pocketpy::macros {

// ── version, configuration and limits (config.h) ─────────────────────────
inline constexpr auto& PK_VERSION = c::value_PK_VERSION;
inline constexpr auto PK_VERSION_MAJOR = c::value_PK_VERSION_MAJOR;
inline constexpr auto PK_VERSION_MINOR = c::value_PK_VERSION_MINOR;
inline constexpr auto PK_VERSION_PATCH = c::value_PK_VERSION_PATCH;
inline constexpr auto PK_ENABLE_OS = c::value_PK_ENABLE_OS;
inline constexpr auto PK_ENABLE_THREADS = c::value_PK_ENABLE_THREADS;
inline constexpr auto PK_ENABLE_DLL = c::value_PK_ENABLE_DLL;
inline constexpr auto PK_ENABLE_DETERMINISM = c::value_PK_ENABLE_DETERMINISM;
inline constexpr auto PK_ENABLE_WATCHDOG = c::value_PK_ENABLE_WATCHDOG;
inline constexpr auto PK_ENABLE_CUSTOM_SNAME = c::value_PK_ENABLE_CUSTOM_SNAME;
inline constexpr auto PK_ENABLE_MIMALLOC = c::value_PK_ENABLE_MIMALLOC;
inline constexpr auto PK_GC_MIN_THRESHOLD = c::value_PK_GC_MIN_THRESHOLD;
inline constexpr auto PK_VM_STACK_SIZE = c::value_PK_VM_STACK_SIZE;
inline constexpr auto PK_MAX_CO_VARNAMES = c::value_PK_MAX_CO_VARNAMES;
inline constexpr auto PK_MAX_MODULE_PATH_LEN = c::value_PK_MAX_MODULE_PATH_LEN;
inline constexpr auto PK_INST_ATTR_LOAD_FACTOR = c::value_PK_INST_ATTR_LOAD_FACTOR;
inline constexpr auto PK_TYPE_ATTR_LOAD_FACTOR = c::value_PK_TYPE_ATTR_LOAD_FACTOR;

// ── the platform (export.h, config.h) ────────────────────────────────────
inline constexpr auto PK_PLATFORM_SEP = c::value_PK_PLATFORM_SEP;
inline constexpr auto PY_SYS_PLATFORM = c::value_PY_SYS_PLATFORM;
inline constexpr auto& PY_SYS_PLATFORM_STRING = c::value_PY_SYS_PLATFORM_STRING;
inline constexpr auto PK_IS_DESKTOP_PLATFORM = c::value_PK_IS_DESKTOP_PLATFORM;

// ── memory (config.h: the C library's, PK_ENABLE_MIMALLOC being 0) ───────
inline void* PK_MALLOC(size_t size) { return py_malloc(size); }
inline void* PK_REALLOC(void* ptr, size_t size) { return py_realloc(ptr, size); }
inline void PK_FREE(void* ptr) { py_free(ptr); }

// ── type tests ───────────────────────────────────────────────────────────
inline bool py_isint(py_Ref self) { return py_istype(self, tp_int); }
inline bool py_isfloat(py_Ref self) { return py_istype(self, tp_float); }
inline bool py_isbool(py_Ref self) { return py_istype(self, tp_bool); }
inline bool py_isstr(py_Ref self) { return py_istype(self, tp_str); }
inline bool py_islist(py_Ref self) { return py_istype(self, tp_list); }
inline bool py_istuple(py_Ref self) { return py_istype(self, tp_tuple); }
inline bool py_isdict(py_Ref self) { return py_istype(self, tp_dict); }
inline bool py_isnil(py_Ref self) { return py_istype(self, 0); }
inline bool py_isnone(py_Ref self) { return py_istype(self, tp_NoneType); }
inline bool py_checkint(py_Ref self) { return py_checktype(self, tp_int); }
inline bool py_checkfloat(py_Ref self) { return py_checktype(self, tp_float); }
inline bool py_checkbool(py_Ref self) { return py_checktype(self, tp_bool); }
inline bool py_checkstr(py_Ref self) { return py_checktype(self, tp_str); }

// ── registers ────────────────────────────────────────────────────────────
inline py_GlobalRef py_r0() { return py_getreg(0); }
inline py_GlobalRef py_r1() { return py_getreg(1); }
inline py_GlobalRef py_r2() { return py_getreg(2); }
inline py_GlobalRef py_r3() { return py_getreg(3); }
inline py_GlobalRef py_r4() { return py_getreg(4); }
inline py_GlobalRef py_r5() { return py_getreg(5); }
inline py_GlobalRef py_r6() { return py_getreg(6); }
inline py_GlobalRef py_r7() { return py_getreg(7); }
inline py_GlobalRef py_tmpr0() { return py_getreg(8); }
inline py_GlobalRef py_tmpr1() { return py_getreg(9); }
inline py_GlobalRef py_tmpr2() { return py_getreg(10); }
inline py_GlobalRef py_tmpr3() { return py_getreg(11); }
inline py_GlobalRef py_sysr0() { return py_getreg(12); }  // the debugger's
inline py_GlobalRef py_sysr1() { return py_getreg(13); }  // pybind11's

// ── stack values ─────────────────────────────────────────────────────────
constexpr py_Ref py_offset(py_Ref p, int i) noexcept { return p + i; }
inline void py_assign(py_Ref dst, py_Ref src) { *dst = *src; }

// pocketpy.h: a function with extra checks without NDEBUG (it aborts if an
// exception is pending), a direct call with NDEBUG. Both, as the build
// profile has it, which compiles the library and this module alike.
inline bool py_callcfunc(py_CFunction f, int argc, py_Ref argv) {
#ifdef NDEBUG
    return f(argc, argv);
#else
    return ::py_callcfunc(f, argc, argv);
#endif
}

// ── raising exceptions (each returns false, as py_exception does) ────────
inline bool NameError(py_Name n) { return py_exception(tp_NameError, "name '%n' is not defined", n); }
template <class... Args>
bool TypeError(const char* fmt, Args... args) { return py_exception(tp_TypeError, fmt, args...); }
template <class... Args>
bool RuntimeError(const char* fmt, Args... args) { return py_exception(tp_RuntimeError, fmt, args...); }
template <class... Args>
bool TimeoutError(const char* fmt, Args... args) { return py_exception(tp_TimeoutError, fmt, args...); }
template <class... Args>
bool OSError(const char* fmt, Args... args) { return py_exception(tp_OSError, fmt, args...); }
template <class... Args>
bool ValueError(const char* fmt, Args... args) { return py_exception(tp_ValueError, fmt, args...); }
template <class... Args>
bool IndexError(const char* fmt, Args... args) { return py_exception(tp_IndexError, fmt, args...); }
template <class... Args>
bool ImportError(const char* fmt, Args... args) { return py_exception(tp_ImportError, fmt, args...); }
template <class... Args>
bool ZeroDivisionError(const char* fmt, Args... args) { return py_exception(tp_ZeroDivisionError, fmt, args...); }
inline bool AttributeError(py_Ref self, py_Name n) {
    return py_exception(tp_AttributeError, "'%t' object has no attribute '%n'", self->type, n);
}
inline bool UnboundLocalError(py_Name n) {
    return py_exception(tp_UnboundLocalError, "cannot access local variable '%n' where it is not associated with a value", n);
}

}  // namespace mcpplibs::mwrap::pocketpy::macros
