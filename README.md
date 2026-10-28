# pocketpy

> PocketPy, the small embeddable Python 3 interpreter, through C++23 modules.

**English** · [简体中文](README.zh.md) · [Upstream](https://github.com/pocketpy/pocketpy) · [Releases](https://github.com/mcpplibs/pocketpy-m/releases) · [mcpp](https://github.com/mcpp-community/mcpp) · [Index](https://github.com/mcpplibs/mcpp-index)

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)
[![PocketPy](https://img.shields.io/badge/PocketPy-MIT-blue.svg)](upstream/pocketpy/LICENSE)
[![mcpp-community](https://img.shields.io/badge/mcpp--community-mwrap-blue)](https://github.com/mcpp-community)
[![CI](https://github.com/mcpplibs/pocketpy-m/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/mcpplibs/pocketpy-m/actions/workflows/ci.yml)

PocketPy 2.2.0 runs Python 3 code from a C11 library with no dependencies.
The upstream C API is unchanged: `py_initialize`, `py_exec`, `py_eval`,
`py_bindfunc` and `py_formatexc` are the upstream globals of `<pocketpy.h>`.
The interpreter is `compat.pocketpy` 2.2.0. This package maps the header's
macros to constants and functions (`py_isint`, `py_r0`, `TypeError` …).
PocketPy's own tests pass on every CI run, from source and compiled to
bytecode, on a host written against the module.

## Modules

- `import mcpplibs.mwrap.pocketpy;`: the embedding API of `<pocketpy.h>` (VMs, exec and eval, values, dicts and lists, native functions and types, exceptions, modules and imports), re-exporting the module below
- `import mcpplibs.mwrap.pocketpy.macros;`: `PK_VERSION`, `PK_ENABLE_*`, `py_isint`, `py_r0`, `TypeError` … in `mcpplibs::mwrap::pocketpy::macros`

> More Modules API -> [Usage and API](docs/usage.md)

## Getting started

```sh
mcpp new scripted --template mcpplibs.mwrap.pocketpy@2.2.0
cd scripted
mcpp run
```

The default template registers a C++ function, runs a script that calls it,
and prints the script's result, or its error with the traceback.

```toml
[dependencies]
"mcpplibs.mwrap.pocketpy" = "2.2.0"
```

```cpp
import std;
import mcpplibs.mwrap.pocketpy;
using namespace mcpplibs::mwrap::pocketpy::macros;  // py_isint, TypeError …

int main() {
    py_initialize();
    if (!py_exec("squares = {n: n * n for n in range(5)}", "<main>", EXEC_MODE, nullptr)) {
        char* message = py_formatexc();
        std::println(std::cerr, "{}", message);
        py_free(message);
        return 1;
    }
    if (py_eval("sum(squares.values())", nullptr) && py_isint(py_retval()))
        std::println("{}", py_toint(py_retval()));  // 30
    py_finalize();
}
```

## The interpreter

`compat.pocketpy` builds the release's sources as upstream's build.sh does,
with the `os` module, deterministic floats (`-ffp-contract=off`, upstream's
own math) and the watchdog (`py_watchdog_begin` ends a runaway script with a
`TimeoutError`); without threads (`pkpy.ComputeThread`) and native modules
from shared libraries, the same on Linux, macOS, Windows and the Web
(wasm32-emscripten). It carries upstream's fix for exceptions the garbage
collector could crash on (de8351ed, after 2.2.0). Script errors never abort:
`py_exec` returns `false`, and the message with its traceback is
`py_formatexc()`. `int` is 64-bit. Details: [docs/usage.md](docs/usage.md).

## Examples

- [examples/pkpy](examples/pkpy): runs a Python file, from source or compiled `.pyc`, or compiles one, written against
  the module; [tools/run_conformance.py](tools/run_conformance.py) runs PocketPy's tests on it.

## Build and testing

Linux (GCC, LLVM), macOS, Windows and wasm32-emscripten (under node) CI build
the modules and run the tests in the dev and release profiles; the native rows
also run the export audit ([tools/check_exports.py](tools/check_exports.py): the
generated surface is fresh, every declaration exported, every macro mapped or
listed with its reason) and PocketPy's own `tests/*.py`. A separate job
scaffolds the default template as an installed package and runs it.

## License

[Apache-2.0](LICENSE) for the wrapper, by mcpp-community; PocketPy remains MIT
([upstream/pocketpy/LICENSE](upstream/pocketpy/LICENSE)).
