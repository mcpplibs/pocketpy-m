// The raw header and the import in one translation unit (the side header
// first, the canonical order): the module's names are the very same entities;
// the macros a function cannot be (PY_CHECK_ARGC, PY_CHECK_ARG_TYPE, py_arg)
// work beside them; the header's own values equal the mapped constants. While
// a header's macro is defined it expands even inside a qualified name, so
// `macros::PK_VERSION_MAJOR` is only reachable with the macro set aside.
#include <mwrap/pocketpy/macros.hpp>
#include <gtest/gtest.h>

import std;
import mcpplibs.mwrap.pocketpy;

namespace {
constexpr int raw_major = PK_VERSION_MAJOR, raw_stack = PK_VM_STACK_SIZE;
constexpr const char* raw_platform = PY_SYS_PLATFORM_STRING;

// A native function written with the header's macros.
bool scale(int argc, py_StackRef argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(0, tp_int);
    PY_CHECK_ARG_TYPE(1, tp_float);
    py_newfloat(py_retval(), static_cast<double>(py_toint(py_arg(0))) * py_tofloat(py_arg(1)));
    return true;
}
}  // namespace

#pragma push_macro("PK_VERSION_MAJOR")
#pragma push_macro("PK_VM_STACK_SIZE")
#pragma push_macro("PY_SYS_PLATFORM_STRING")
#undef PK_VERSION_MAJOR
#undef PK_VM_STACK_SIZE
#undef PY_SYS_PLATFORM_STRING
static_assert(mcpplibs::mwrap::pocketpy::macros::PK_VERSION_MAJOR == raw_major);
static_assert(mcpplibs::mwrap::pocketpy::macros::PK_VM_STACK_SIZE == raw_stack);
constexpr std::string_view mapped_platform = mcpplibs::mwrap::pocketpy::macros::PY_SYS_PLATFORM_STRING;
#pragma pop_macro("PY_SYS_PLATFORM_STRING")
#pragma pop_macro("PK_VM_STACK_SIZE")
#pragma pop_macro("PK_VERSION_MAJOR")

TEST(HeaderMix, SameEntitiesAndValues) {
    static_assert(std::is_same_v<decltype(&::py_exec), bool (*)(const char*, const char*, py_CompileMode, py_Ref)>);
    static_assert(std::is_same_v<py_TValue, ::py_TValue> && sizeof(py_TValue) == 24);
    EXPECT_EQ(std::string_view(raw_platform), mapped_platform);
    py_initialize();
    py_bindfunc(py_getmodule("__main__"), "scale", scale);
    ASSERT_TRUE(py_eval("scale(4, 0.5)", nullptr));
    EXPECT_EQ(py_tofloat(py_retval()), 2.0);
    EXPECT_FALSE(py_eval("scale(4)", nullptr));
    EXPECT_TRUE(py_matchexc(tp_TypeError));
    py_clearexc(nullptr);
    EXPECT_FALSE(py_eval("scale(4, 2)", nullptr));  // an int where a float is checked
    EXPECT_TRUE(py_matchexc(tp_TypeError));
    py_clearexc(nullptr);
    // The header's register macro and the module's function: the same slot.
    py_newint(py_r3(), 9);
    EXPECT_EQ((mcpplibs::mwrap::pocketpy::macros::py_r3)(), py_getreg(3));
    EXPECT_EQ(py_toint((mcpplibs::mwrap::pocketpy::macros::py_r3)()), 9);
    py_finalize();
}
