"""Run the LLVM-backed coroutine injector; no text-rewrite substitute."""

import argparse
from pathlib import Path
import os
import shutil
import subprocess


def transform_file(input_file, output_file, injector=None):
    tool = injector or os.environ.get('KXS_INJECT_EXECUTABLE') or shutil.which('kxs-inject')
    if not tool or not Path(tool).is_file():
        raise ValueError('kxs-inject is required for coroutine IR injection')
    Path(output_file).unlink(missing_ok=True)
    subprocess.run([str(tool), str(input_file), '-o', str(output_file)], check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input_file')
    parser.add_argument('output_file')
    parser.add_argument('--injector')
    args = parser.parse_args()
    transform_file(args.input_file, args.output_file, args.injector)


if __name__ == '__main__':
    main()
