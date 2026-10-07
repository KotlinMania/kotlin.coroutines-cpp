"""Execute actual Kotlin-generated continuations inside a generated C++ chain."""
import argparse
from pathlib import Path
import re
import shlex

from test_suspend_plugin import run


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--plugin', type=Path, required=True)
    parser.add_argument('--ir-plugin', type=Path, required=True)
    parser.add_argument('--compiler', required=True)
    parser.add_argument('--konanc', required=True)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--library-link-options', default='')
    parser.add_argument('--work-dir', type=Path, required=True)
    args = parser.parse_args()
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    fixtures = args.root / 'src/tests/ir/fixtures/kotlin_handoff'
    for filename in ('Handoff.kt', 'api.hpp', 'input.cpp', 'main.cpp'):
        (work / filename).write_text((fixtures / filename).read_text())
    ir_directory = work / 'native-ir'
    ir_directory.mkdir(exist_ok=True)
    run([args.konanc, '-version'], work, 'kotlin-version')
    run([args.konanc, str(work / 'Handoff.kt'), '-produce', 'dynamic',
         '-o', str(work / 'actual'), '-g', '-Xsave-llvm-ir-after=Codegen',
         '-Xsave-llvm-ir-directory=' + str(ir_directory)], work, 'kotlin-compile')
    native_ir = '\n'.join(path.read_text() for path in ir_directory.glob('*.ll'))
    assert native_ir, 'Kotlin compiler must retain its actual generated LLVM IR'
    functions = re.findall(r'^define\b.*?^}', native_ir, re.MULTILINE | re.DOTALL)
    retained = [body for body in functions if 'retained' in body.splitlines()[0]
                and 'invokeSuspend' in body.splitlines()[0]]
    assert retained, 'Missing the Kotlin-generated retained-local resume function'
    assert any('indirectbr' in body and 'blockaddress' in body for body in retained), \
        'Kotlin frame must contain native address dispatch and saved resume addresses'
    run([args.compiler, '-std=c++20', '-fsyntax-only', '-I' + str(args.root / 'src'),
         '-Xclang', '-load', '-Xclang', str(args.plugin),
         '-Xclang', '-add-plugin', '-Xclang', 'kotlinx-suspend',
         '-Xclang', '-plugin-arg-kotlinx-suspend', '-Xclang', 'out-dir=' + str(work),
         str(work / 'input.cpp')], work, 'cpp-extraction')
    native_libraries = list(work.glob('libactual.dylib')) + list(work.glob('libactual.so'))
    assert len(native_libraries) == 1, native_libraries
    run([args.compiler, '-std=c++20', '-O2', '-g', '-UNDEBUG',
         '-fsanitize=address', '-Wno-gnu-label-as-value',
         '-I' + str(args.root / 'src'), '-include', str(work / 'api.hpp'),
         '-fpass-plugin=' + str(args.ir_plugin),
         '-Xclang', '-load', '-Xclang', str(args.plugin),
         '-Xclang', '-add-plugin', '-Xclang', 'kotlinx-suspend', str(work / 'input.cpp'),
         str(work / 'main.cpp'), str(args.library), str(native_libraries[0]),
         '-pthread', '-Wl,-rpath,' + str(work),
         *shlex.split(args.library_link_options), '-o', str(work / 'handoff')],
        work, 'cpp-compile')
    trace = run([str(work / 'handoff')], work, 'handoff-run')
    expected = ('kotlin-cpp:0:0:399\nkotlin-cpp:1:0:399\n'
                'kotlin-cpp:2:1:0\nkotlin-cpp:3:2:0\n')
    assert trace == expected, trace
    print('Executed actual Kotlin/C++ frames: immediate and worker-thread results, failure, cancellation and ownership')


if __name__ == '__main__':
    main()
