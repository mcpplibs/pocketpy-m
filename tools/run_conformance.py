#!/usr/bin/env python3
"""Run PocketPy's own tests (tests/*.py of the release) on examples/pkpy, a
host written against import mcpplibs.mwrap.pocketpy, as upstream's
scripts/run_tests.py runs them on its src2/main.c: every test from source,
then every test compiled to bytecode (compileall.py's tmp/tests) and run from
the .pyc, each from the release's root directory, each passing by exiting 0.

The release tarball is downloaded once into target/upstream and checked
against upstream/documents.json; its test files are used unchanged. The
tests of features compat.pocketpy leaves out (the lz4 and msgpack modules,
pkpy.ComputeThread with PK_ENABLE_THREADS) skip themselves, as they do
upstream: they are counted, by the message they print.
"""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from inventory import fetch  # noqa: E402


def host(example: Path, mcpp: str, toolchain: str | None, profile: str) -> Path:
    command = [mcpp, 'build', '--profile', profile] + (['--toolchain', toolchain] if toolchain else [])
    print('+', ' '.join(command), flush=True)
    subprocess.run(command, cwd=example, check=True)
    name = 'pkpy.exe' if os.name == 'nt' else 'pkpy'
    found = sorted((p for p in (example / 'target').rglob(name) if p.parent.name == 'bin'), key=lambda p: p.stat().st_mtime)
    if not found:
        sys.exit('examples/pkpy was not built')
    return found[-1]


def run(pkpy: Path, test: str, cwd: Path) -> tuple[bool, str]:
    result = subprocess.run([str(pkpy), test], cwd=cwd, capture_output=True, text=True, encoding='utf-8',
                            errors='replace', timeout=600)
    return result.returncode == 0, (result.stdout or '') + (result.stderr or '')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mcpp', default='mcpp')
    parser.add_argument('--toolchain')
    parser.add_argument('--profile', default='release')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    upstream = fetch(root)
    built = host(root / 'examples/pkpy', args.mcpp, args.toolchain, args.profile)
    # Under upstream's name in the release's root, where its tests call it
    # (922_py_compile.py runs `./main --compile …`), as run_tests.sh puts it.
    pkpy = upstream / ('main.exe' if os.name == 'nt' else 'main')
    shutil.copy2(built, pkpy)

    # compileall.py: every .py under tests/ (the tests and the packages they
    # import) to tmp/tests/…/*.pyc.
    compiled = upstream / 'tmp/tests'
    shutil.rmtree(compiled, ignore_errors=True)
    for source in sorted((upstream / 'tests').rglob('*.py')):
        target = compiled / source.relative_to(upstream / 'tests')
        target = target.with_name(target.name + 'c')
        target.parent.mkdir(parents=True, exist_ok=True)
        result = subprocess.run([str(pkpy), '--compile', str(source), str(target)], capture_output=True, text=True)
        if result.returncode != 0:
            sys.exit(f'cannot compile {source}:\n{result.stdout}{result.stderr}')

    passed, failed, skipped = [], [], []
    tests = sorted(p.name for p in (upstream / 'tests').glob('*.py'))
    for kind, directory, suffix in (('source', 'tests', '.py'), ('compiled', 'tmp/tests', '.pyc')):
        for name in tests:
            test = f'{directory}/{name[:-3]}{suffix}'
            ok, output = run(pkpy, test, upstream)
            if ok and 'skipping test' in output:
                skipped.append(test)
            (passed if ok else failed).append(test)
            if not ok:
                print(f'FAIL {test}\n{output[-1500:]}', flush=True)
    print(f'{len(passed)} passed ({len(skipped)} of them skip a feature left out: '
          f'{", ".join(sorted({Path(t).stem for t in skipped}))}), {len(failed)} failed, '
          f'from source and compiled')
    if failed or len(passed) < 2 * 90:  # 92 tests in 2.2.0: a guard against finding none
        sys.exit(1)


if __name__ == '__main__':
    main()
