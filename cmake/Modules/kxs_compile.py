"""CMake CXX_COMPILER_LAUNCHER for Clang's existing coroutine state machines."""

import argparse
from pathlib import Path
import subprocess
import sys

sys.dont_write_bytecode = True
import kxs_transform_ir
from kxs_transform_ir import transform_file


def run(command):
    return subprocess.run(command, check=False).returncode


def compile_object(command, compiler):
    compiler_index = command.index(compiler)
    prefix = command[:compiler_index + 1]
    args = command[compiler_index + 1:]

    # Header/PCH invocations have no coroutine functions to clean up.
    if '-x' in args and 'header' in args[args.index('-x') + 1]:
        return run(command)
    if '-c' not in args or '-o' not in args:
        raise ValueError('expected CMake Clang command with -c <source> and -o <object>')
    source_index = args.index('-c') + 1
    output_index = args.index('-o') + 1
    output = Path(args[output_index])
    output.unlink(missing_ok=True)
    raw_ir = str(output) + '.kxs.ll'
    cleaned_ir = str(output) + '.kxs.cleaned.ll'

    # Keep frontend flags and dependencies. Run LLVM passes only in the second
    # stage, so optimization/sanitizer instrumentation is not applied twice.
    emit_args = list(args)
    emit_args[output_index] = raw_ir
    emit_args.remove('-c')
    emit_args += ['-S', '-emit-llvm', '-Xclang', '-disable-llvm-passes']
    result = run(prefix + emit_args)
    if result:
        return result
    transform_file(raw_ir, cleaned_ir)
    if '-MF' in args:
        # Include the launcher itself in CMake's normal header dependency graph,
        # including sources added after the launcher was enabled.
        def dep_path(path):
            backslash = chr(92)
            return (str(path).replace(backslash, backslash * 2).replace('$', '$$')
                    .replace('#', backslash + '#').replace(' ', backslash + ' ')
                    .replace(':', backslash + ':'))
        dependency_file = Path(args[args.index('-MF') + 1])
        helpers = [Path(__file__).resolve(), Path(kxs_transform_ir.__file__).resolve()]
        dependencies = dependency_file.read_text()
        index = 0
        while index < len(dependencies):
            if dependencies[index] == chr(92):
                index += 2
                continue
            if (dependencies[index] == ':' and index + 1 < len(dependencies)
                    and dependencies[index + 1].isspace()):
                break
            index += 1
        if index == len(dependencies):
            raise ValueError('could not find the object rule in the Clang depfile')
        # CMake consumes the first object rule, so add prerequisites to that
        # rule rather than appending a second rule for the same output.
        dependency_file.write_text(dependencies[:index + 1] + ' ' +
                                   ' '.join(dep_path(path) for path in helpers) +
                                   dependencies[index + 1:])

    # The first stage wrote the dependency file for the original source. Keep
    # codegen/toolchain options, but do not overwrite it with an IR dependency.
    object_args = []
    index = 0
    while index < len(args):
        arg = args[index]
        if arg in ('-MF', '-MT', '-MQ', '-MJ', '-x'):
            index += 2
            continue
        if arg in ('-MD', '-MMD', '-MP', '-MG'):
            index += 1
            continue
        if index == source_index:
            object_args += ['-x', 'ir', cleaned_ir]
        else:
            object_args.append(arg)
        index += 1
    result = run(prefix + object_args + ['-Wno-unused-command-line-argument'])
    if result:
        # Leave IR for diagnostics, but never leave an older successful object.
        output.unlink(missing_ok=True)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', required=True)
    parser.add_argument('command', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    command = args.command
    if command and command[0] == '--':
        command = command[1:]
    try:
        return compile_object(command, args.compiler)
    except (OSError, ValueError) as error:
        print(f'[KXS] {error}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
