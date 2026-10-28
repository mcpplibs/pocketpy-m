#!/usr/bin/env python3
"""Audit the module surface against pocketpy.h (the pinned release's).

- src/pocketpy.cppm is what tools/gen_exports.py writes: every type,
  enumerator and function of the public header exported.
- py_callcfunc, a function only without NDEBUG, is the macros module's.
- Every macro of the public header (pocketpy.h, config.h, export.h) is mapped
  in src/macros.cppm, kept as a macro in <mwrap/pocketpy/macros.hpp> (listed
  below with the reason a function cannot be it), or excluded below with a
  reason.
"""
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from inventory import fetch, surface  # noqa: E402

SIDE_HEADER = {
    'PY_CHECK_ARGC': 'returns from the native function it is written in, and reads its argc',
    'PY_CHECK_ARG_TYPE': 'returns from the native function it is written in, and reads its argv',
    'py_arg': "names the native function's own argv",
}

EXCLUDED = {
    'PK_IS_PUBLIC_INCLUDE': "<pocketpy.h>'s switch for the py_TValue definition, undefined again at its end",
    'PK_API': 'a linkage annotation of declarations',
    'PK_EXPORT': 'a linkage annotation of declarations',
    'PK_DEPRECATED': 'an attribute of declarations',
    'PK_INLINE': "the library's own inlining annotation",
    'PK_THREAD_LOCAL': 'a storage-class annotation (empty: PK_ENABLE_THREADS is 0)',
    'PY_RAISE': 'an empty annotation of declarations (may raise)',
    'PY_RETURN': 'an empty annotation of declarations (sets py_retval())',
    'PY_MAYBENULL': 'an empty annotation of declarations (may return NULL)',
    'restrict': "config.h's C keyword spelled away for C++",
    'py_debugger_waitforattach': 'the PK_ENABLE_OS=0 no-op form; with the OS on it is the exported function',
    'py_debugger_status': 'the PK_ENABLE_OS=0 no-op form; with the OS on it is the exported function',
    'py_debugger_exceptionbreakpoint': 'the PK_ENABLE_OS=0 no-op form; with the OS on it is the exported function',
    'py_debugger_exit': 'the PK_ENABLE_OS=0 no-op form; with the OS on it is the exported function',
}


def mapped_names(text: str) -> set[str]:
    text = text[text.index('export namespace'):]
    names = set(re.findall(r'inline constexpr (?:[\w:&]+\s+)+?(\w+)\s*=', text))
    names |= set(re.findall(r'^(?:inline |constexpr )[\w:\*\s]+?\b(\w+)\s*\(', text, flags=re.M))
    names |= set(re.findall(r'^bool\s+(\w+)\s*\(', text, flags=re.M))  # the variadic templates
    return names


def main():
    root = Path(__file__).resolve().parents[1]
    problems = []
    generated = subprocess.run([sys.executable, str(root / 'tools/gen_exports.py'), '--check'], capture_output=True, text=True)
    if generated.returncode != 0:
        problems.append(generated.stderr.strip() or generated.stdout.strip())
    sf = surface(fetch(root))
    exported = set(re.findall(r'export using ::(\w+);', (root / 'src/pocketpy.cppm').read_text(encoding='utf-8')))
    missing = (set(sf.types) | set(sf.enumerators) | set(sf.functions)) - exported
    if missing:
        problems.append('src/pocketpy.cppm lacks: ' + ', '.join(sorted(missing)))
    mapped = mapped_names((root / 'src/macros.cppm').read_text(encoding='utf-8'))
    missing = set(sf.debug_only) - mapped
    if missing:
        problems.append('without NDEBUG only, not in the macros module: ' + ', '.join(sorted(missing)))
    macros = set(sf.macros)
    unaccounted = macros - mapped - SIDE_HEADER.keys() - EXCLUDED.keys()
    if unaccounted:
        problems.append('macros neither mapped, side-header nor excluded: ' + ', '.join(sorted(unaccounted)))
    both = (SIDE_HEADER.keys() | EXCLUDED.keys()) & mapped
    if both:
        problems.append('listed as unmapped yet mapped: ' + ', '.join(sorted(both)))
    stale = (SIDE_HEADER.keys() | EXCLUDED.keys()) - macros
    if stale:
        problems.append('listed but not in the header: ' + ', '.join(sorted(stale)))
    extra = mapped - macros - set(sf.debug_only)
    if extra:
        problems.append('mapped but not a macro of the header: ' + ', '.join(sorted(extra)))
    if problems:
        sys.exit('\n'.join(problems))
    print(f'{len(sf.types)} types, {len(sf.enumerators)} enumerators and {len(sf.functions)} functions exported; '
          f'{len(sf.debug_only)} debug-only function in the macros module; {len(macros)} macros: '
          f'{len(macros & mapped)} mapped, {len(SIDE_HEADER)} kept as macros, {len(EXCLUDED)} excluded')


if __name__ == '__main__':
    main()
