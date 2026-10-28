#!/usr/bin/env python3
"""The public surface of PocketPy's pocketpy.h, from the pinned release.

Shared by tools/gen_exports.py (which writes the export list) and
tools/check_exports.py (which audits the module against it). The headers come
from the upstream tarball, downloaded once into target/upstream and checked
against upstream/documents.json: the same bytes compat.pocketpy builds.

The public header <pocketpy.h> defines PK_IS_PUBLIC_INCLUDE and includes
pocketpy/pocketpy.h, which includes pocketpy/config.h, pocketpy/export.h and
pocketpy/vmath.h: those four files are the surface. Everything else under
include/pocketpy is the library's private headers.
"""
from dataclasses import dataclass, field
import hashlib
import json
from pathlib import Path
import re
import sys
import tarfile
import urllib.request

PUBLIC = ['pocketpy.h', 'pocketpy/pocketpy.h', 'pocketpy/config.h', 'pocketpy/export.h', 'pocketpy/vmath.h']

# Declared only without NDEBUG (pocketpy.h: `#ifndef NDEBUG … #else #define
# py_callcfunc …`): its export follows the build profile (tools/gen_exports.py).
DEBUG_ONLY = {'py_callcfunc'}


def fetch(root: Path) -> Path:
    pins = json.loads((root / 'upstream/documents.json').read_text(encoding='utf-8'))
    cache = root / 'target/upstream'
    cache.mkdir(parents=True, exist_ok=True)
    archive = cache / 'pocketpy-release.tar.gz'
    if not archive.exists() or hashlib.sha256(archive.read_bytes()).hexdigest() != pins['upstream_sha256']:
        print('+ fetch', pins['upstream'], flush=True)
        with urllib.request.urlopen(pins['upstream'], timeout=120) as response:
            archive.write_bytes(response.read())
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    if digest != pins['upstream_sha256']:
        sys.exit(f'upstream archive checksum {digest} != {pins["upstream_sha256"]}')
    with tarfile.open(archive) as tar:
        top = tar.getmembers()[0].name.split('/')[0]
        if not (cache / top).exists():
            tar.extractall(cache, filter='data')
    return cache / top


@dataclass
class Surface:
    types: list[str] = field(default_factory=list)
    enumerators: list[str] = field(default_factory=list)
    functions: list[str] = field(default_factory=list)      # every declared function but DEBUG_ONLY
    debug_only: list[str] = field(default_factory=list)
    macros: list[str] = field(default_factory=list)


def strip(text: str) -> str:
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    return re.sub(r'//[^\n]*', '', text)


def surface(upstream: Path) -> Surface:
    include = upstream / 'include'
    raw = {name: (include / name).read_text(encoding='utf-8') for name in PUBLIC}
    # Declarations only: the preprocessor's lines (with their continuations) are not.
    decls = '\n'.join(re.sub(r'^\s*#[^\n]*', '', strip(text).replace('\\\n', ' '), flags=re.M) for text in raw.values())
    s = Surface()
    names = set()
    names |= set(re.findall(r'typedef\s+(?:struct|union)\s+(\w+)\s+\1\s*;', decls))
    names |= set(re.findall(r'typedef\s+(?:struct|union)\s+(\w+)\s*\{', decls))
    names |= set(re.findall(r'typedef\s+[\w\s\*]+?\b(\w+)\s*;', decls))
    names |= set(re.findall(r'typedef\s+[\w\s\*]+?\(\s*\*\s*(\w+)\s*\)\s*\(', decls))
    for m in re.finditer(r'(?<!typedef\s)\benum\s+(\w+)\s*\{(.*?)\}\s*;', decls, flags=re.S):
        names.add(m.group(1))
        s.enumerators += re.findall(r'\b([A-Za-z_]\w*)\b\s*(?:=[^,]*)?(?:,|$)', m.group(2).strip())
    s.types = sorted(names)
    # Functions: every declaration at file scope that is not a typedef, with
    # or without PK_API (time_ns, cpy312__* have none).
    body = re.sub(r'extern\s+"C"\s*\{', '', decls)
    while True:  # struct, union and enum bodies
        collapsed = re.sub(r'\{[^{}]*\}', ' ', body)
        if collapsed == body:
            break
        body = collapsed
    functions = set()
    for statement in body.split(';'):
        statement = ' '.join(statement.replace('}', ' ').split())
        if statement.startswith('typedef'):
            continue
        m = re.fullmatch(r'(?:PK_API )?(?:PK_DEPRECATED )?[A-Za-z_][\w \*]*?\b(\w+) ?\((.*)\)(?: PY_RAISE| PY_RETURN)*', statement)
        if m:
            functions.add(m.group(1))
    s.debug_only = sorted(functions & DEBUG_ONLY)
    s.functions = sorted(functions - DEBUG_ONLY)
    s.macros = sorted({m for text in raw.values() for m in re.findall(r'^\s*#\s*define\s+(\w+)', text, flags=re.M)})
    return s


if __name__ == '__main__':
    root = Path(__file__).resolve().parents[1]
    sf = surface(fetch(root))
    print(f'{len(sf.types)} types, {len(sf.enumerators)} enumerators, {len(sf.functions)} functions '
          f'(+ {len(sf.debug_only)} without NDEBUG), {len(sf.macros)} macros')
    print('types:', ' '.join(sf.types))
    print('functions:', ' '.join(sf.functions))
    print('macros:', ' '.join(sf.macros))
