# Using mcpplibs.mwrap.pocketpy

`import mcpplibs.mwrap.pocketpy;` declares PocketPy 2.2.0's embedding API as `<pocketpy.h>` does. Types, enumerators and functions are the upstream globals, the very same entities as the header's, so a translation unit may include the header too. The interpreter is `compat.pocketpy` 2.2.0, built from the same release.

## Modules

| Module | What it holds |
|---|---|
| `mcpplibs.mwrap.pocketpy` | every type (`py_Ref`, `py_TValue`, `py_Name`, `py_CFunction`, `py_Callbacks`, `c11_sv`, `c11_vec2` …), enumerator (`EXEC_MODE`, `EVAL_MODE`, `tp_int`, `tp_dict`, `tp_TypeError` …) and function (`py_initialize`, `py_exec`, `py_eval`, `py_bindfunc`, `py_formatexc` … 246 of them) of the public header; re-exports the one below |
| `mcpplibs.mwrap.pocketpy.macros` | the macros, in `mcpplibs::mwrap::pocketpy::macros` |

## Embedding, in brief

```cpp
import std;
import mcpplibs.mwrap.pocketpy;
using namespace mcpplibs::mwrap::pocketpy::macros;  // py_isint, py_r0, TypeError …

bool add(int argc, py_StackRef argv) {              // a native function
    if (argc != 2) return TypeError("add() takes 2 arguments, got %d", argc);
    if (!py_checkint(&argv[0]) || !py_checkint(&argv[1])) return false;
    py_newint(py_retval(), py_toint(&argv[0]) + py_toint(&argv[1]));
    return true;
}

int main() {
    py_initialize();                                   // VM 0
    py_bindfunc(py_getmodule("__main__"), "add", add);
    if (!py_exec("total = add(40, 2)", "<main>", EXEC_MODE, nullptr)) {
        char* message = py_formatexc();                 // the traceback, as text
        std::println(std::cerr, "{}", message);
        py_free(message);
        py_clearexc(nullptr);                           // the VM goes on
    }
    if (py_eval("total", nullptr) && py_isint(py_retval())) std::println("{}", py_toint(py_retval()));
    py_finalize();
}
```

- **The VM.** `py_initialize()` creates VM 0; `py_switchvm(i)` (0 ≤ i < 16) selects another, created on first use; `py_resetvm()` empties the current one; `py_finalize()` frees them all and cannot be undone. `py_setvmctx` / `py_getvmctx` hold a host pointer per VM.
- **Running code.** `py_exec(source, filename, EXEC_MODE, module)` runs statements; `py_eval(source, module)` evaluates an expression into `py_retval()`; `module` is `nullptr` for `__main__` or a module from `py_newmodule`. Running new code in the same VM or module replaces definitions and keeps state; `py_importlib_reload` re-runs an imported module.
- **Values.** `py_retval()` is overwritten by the next call and is not an argument: copy it first (`py_assign(py_r0(), py_retval())`). `py_toint`, `py_tofloat`, `py_tostr`, `py_tobool` read values whose type was checked (`py_isint` …); `py_castint` / `py_castfloat` convert or raise. Dicts: `py_dict_len`, `py_dict_getitem_by_str`, `py_dict_apply`; lists and tuples: `py_list_len` / `py_list_getitem`, `py_tuple_len` / `py_tuple_getitem`; any iterable: `py_iter` and `py_next`.
- **Native functions** return `true` with their result in `py_retval()`, or `false` with an exception raised (`TypeError(...)`, `ValueError(...)`, `py_exception(type, fmt, ...)`). `py_bindfunc` passes the arguments as given; `py_bind(module, "clamp(x, lo=0, hi=10)", f)` fills defaults and keywords from a signature.
- **Errors** never abort: a syntax error, an exception, runaway recursion (`RecursionError`) each make `py_exec` / `py_eval` return `false`; `py_matchexc(tp_X)` tests the type, `py_formatexc()` gives the message with its traceback, `py_clearexc(nullptr)` clears it.
- **Runaway scripts.** `py_watchdog_begin(ms)` … `py_watchdog_end()` around a call ends it with a `TimeoutError` once the CPU time passes `ms`.
- **The host decides.** `py_callbacks()` sets `print`, `importfile` (where `import` reads source; called with a null size by `py_importlib_reload`), `lazyimport` and others. `builtins.exit` calls the C library's `exit`: a host that must outlive its scripts binds its own (`py_bindfunc(py_getmodule("builtins"), "exit", f)`, raising `SystemExit`).

## Macros

A named module cannot export a macro. `mcpplibs::mwrap::pocketpy::macros` holds them under their original names:

- **Constants** as `inline constexpr`, each with its macro's type: `PK_VERSION` (a `char` array), `PK_VERSION_MAJOR` / `_MINOR` / `_PATCH`, `PK_ENABLE_*`, `PK_GC_MIN_THRESHOLD`, `PK_VM_STACK_SIZE`, `PK_MAX_CO_VARNAMES`, `PK_MAX_MODULE_PATH_LEN`, the load factors, `PK_PLATFORM_SEP`, `PY_SYS_PLATFORM`, `PY_SYS_PLATFORM_STRING`, `PK_IS_DESKTOP_PLATFORM`.
- **Functions**: the type tests `py_isint` … `py_isnone` and `py_checkint` … `py_checkstr`; the registers `py_r0()` … `py_r7()`, `py_tmpr0()` … `py_tmpr3()`, `py_sysr0()`, `py_sysr1()`; `py_offset` (constexpr), `py_assign`; `PK_MALLOC`, `PK_REALLOC`, `PK_FREE` (the interpreter's allocator); `py_callcfunc` (pocketpy.h's checking function without `NDEBUG`, a direct call with it, as the build profile has it).
- **Raising**: `NameError(name)`, `AttributeError(self, name)`, `UnboundLocalError(name)`, and `TypeError`, `ValueError`, `RuntimeError`, `IndexError`, `ImportError`, `OSError`, `TimeoutError`, `ZeroDivisionError` as variadic templates passing a format and its arguments on to `py_exception` unchanged (`%d` int, `%i` py_i64, `%f` py_f64, `%s` C string, `%t` py_Type, `%n` py_Name …).

The `PK_ENABLE_*` constants are the library's configuration: `PK_ENABLE_OS` 1, `PK_ENABLE_THREADS` 0, `PK_ENABLE_DLL` 0, `PK_ENABLE_DETERMINISM` 1, `PK_ENABLE_WATCHDOG` 1 (see compat.pocketpy). A translation unit including `<pocketpy.h>` itself reads config.h's defaults, which have `DETERMINISM` and `WATCHDOG` at 0; nothing else in the header depends on either.

### What stays a macro

These stay macros in `<mwrap/pocketpy/macros.hpp>`, which is `<pocketpy.h>`:

| Macro | Why |
|---|---|
| `PY_CHECK_ARGC(n)` | returns from the native function it is written in, and reads its `argc` |
| `PY_CHECK_ARG_TYPE(i, type)` | returns from the native function it is written in, and reads its `argv` |
| `py_arg(i)` | names the native function's own `argv` (`&argv[i]` without the header) |

Include it before the import. While a macro is defined it expands even inside a qualified name, so `macros::PK_VERSION_MAJOR` is not reachable there; use the plain macro (see [tests/header_mix_test.cpp](../tests/header_mix_test.cpp)). [tools/check_exports.py](../tools/check_exports.py) accounts for all 82 macros: 65 mapped, 3 kept, 14 excluded with their reasons (annotations such as `PK_API` and `PY_RAISE`, and the `PK_ENABLE_OS=0` forms of the debugger functions).

## The interpreter as built

compat.pocketpy builds every `src/**/*.c` of the release as upstream's build.sh does, with the configuration above, `-ffp-contract=off`, and upstream's fix for traceback values the garbage collector read uninitialised (de8351ed, after 2.2.0). The same on Linux, macOS, Windows and wasm32-emscripten. Floats give the same bits everywhere (upstream's own `dmath`, which is not always the C library's: `math.log(10)` is one ulp below glibc's). PocketPy is a Python 3 subset: `int` is 64-bit (`2 ** 64` wraps to 0), and a generator expression cannot be a bare call argument (`sum(x for x in y)`; write `sum([x for x in y])`). Not built: `pkpy.ComputeThread` (threads), native modules from shared libraries (DLL), and the optional lz4, msgpack, cute_png and periphery modules.

## Checks

- `mcpp test`: the embedding API through the module (the VM, exec and eval, values, dicts and lists, native functions, errors, exceptions across collections, `exit`, re-running code, imports and reloads from the host, several VMs, the watchdog, print, a native type, JSON and pickle, deterministic floats), the constants, and the header mixed with the import. CI runs it on Linux (GCC, LLVM), macOS, Windows and under node (wasm32-emscripten), in the dev and release profiles.
- `python3 tools/check_exports.py`: `src/pocketpy.cppm` is what [tools/gen_exports.py](../tools/gen_exports.py) writes from the pinned header; every declaration is exported, every macro mapped or listed with its reason.
- `python3 tools/run_conformance.py`: PocketPy's own `tests/*.py` on [examples/pkpy](../examples/pkpy), a host written against the module, as upstream's `scripts/run_tests.py` runs them: all 92 from source and all 92 compiled to `.pyc` pass (the lz4, msgpack and thread tests skip themselves, as upstream's do without those features).
