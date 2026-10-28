// <mwrap/pocketpy/macros.hpp> — PocketPy's macros as macros, for the ones a
// function cannot be: PY_CHECK_ARGC and PY_CHECK_ARG_TYPE (they return from
// the native function they are written in) and py_arg (it names that
// function's `argv`). Include it before `import mcpplibs.mwrap.pocketpy;`: it
// is <pocketpy.h>, so every other macro is defined too, and while a macro is
// defined it expands even inside `mcpplibs::mwrap::pocketpy::macros::NAME`.
#pragma once
#include <pocketpy.h>
