# pocketpy

> 通过 C++23 模块使用 PocketPy（小巧的可嵌入 Python 3 解释器）。

[English](README.md) · **简体中文** · [上游](https://github.com/pocketpy/pocketpy) · [发布](https://github.com/mcpplibs/pocketpy-m/releases) · [mcpp](https://github.com/mcpp-community/mcpp) · [索引](https://github.com/mcpplibs/mcpp-index)

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)
[![PocketPy](https://img.shields.io/badge/PocketPy-MIT-blue.svg)](upstream/pocketpy/LICENSE)
[![mcpp-community](https://img.shields.io/badge/mcpp--community-mwrap-blue)](https://github.com/mcpp-community)
[![CI](https://github.com/mcpplibs/pocketpy-m/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/mcpplibs/pocketpy-m/actions/workflows/ci.yml)

PocketPy 2.2.0 是一个无依赖的 C11 库，用来运行 Python 3 代码。上游的 C API
保持不变：`py_initialize`、`py_exec`、`py_eval`、`py_bindfunc`、`py_formatexc`
就是 `<pocketpy.h>` 里的上游全局名字。解释器本身是 `compat.pocketpy` 2.2.0。
本包把头文件里的宏映射成常量和函数（`py_isint`、`py_r0`、`TypeError` …）。
每次 CI 都会在一个只用本模块写成的宿主上跑 PocketPy 自带的测试（源码和编译成
字节码各一遍），并且全部通过。

## 模块

- `import mcpplibs.mwrap.pocketpy;`：`<pocketpy.h>` 的嵌入 API（VM、exec 与 eval、值、dict 与 list、原生函数与类型、异常、模块与导入），并重新导出下面这个模块
- `import mcpplibs.mwrap.pocketpy.macros;`：`PK_VERSION`、`PK_ENABLE_*`、`py_isint`、`py_r0`、`TypeError` 等，位于 `mcpplibs::mwrap::pocketpy::macros`

> 更多模块 API -> [用法和 API](docs/usage.md)

## 快速开始

```sh
mcpp new scripted --template mcpplibs.mwrap.pocketpy@2.2.0
cd scripted
mcpp run
```

默认模板注册一个 C++ 函数，执行一段调用它的脚本，再打印脚本的结果，出错时打印
带回溯的错误信息。

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

## 解释器

`compat.pocketpy` 按上游 build.sh 的方式构建发布版的源文件：带 `os` 模块、确定性
浮点（`-ffp-contract=off` 加上游自己的数学库）和 watchdog（`py_watchdog_begin`
以 `TimeoutError` 结束失控的脚本）；不带线程（`pkpy.ComputeThread`）和从动态库
加载的原生模块。Linux、macOS、Windows 与 Web（wasm32-emscripten）完全相同。它
带有上游在 2.2.0 之后的修复（de8351ed），避免垃圾回收在异常对象存活时崩溃。脚本
错误不会中止进程：`py_exec` 返回 `false`，带回溯的错误信息由 `py_formatexc()`
给出。`int` 是 64 位整数。详见 [docs/usage.md](docs/usage.md)。

## 示例

- [examples/pkpy](examples/pkpy)：只用本模块写成，运行一个 Python 文件（源码或编译好的 `.pyc`），或编译一个文件；
  [tools/run_conformance.py](tools/run_conformance.py) 在它上面跑 PocketPy 的测试。

## 构建与测试

Linux（GCC、LLVM）、macOS、Windows 与 wasm32-emscripten（在 node 下）的 CI 会构建
模块，并在 dev 与 release 两种配置下运行测试；原生平台还会做导出审计
（[tools/check_exports.py](tools/check_exports.py)：生成的导出是最新的，每个声明都已导出，
每个宏都已映射或列出原因），并运行 PocketPy 自带的 `tests/*.py`。另有一个任务把默认
模板当作已安装的包来生成项目并运行。

## 许可证

封装部分为 [Apache-2.0](LICENSE)，由 mcpp-community 维护；PocketPy 仍为 MIT
（[upstream/pocketpy/LICENSE](upstream/pocketpy/LICENSE)）。
