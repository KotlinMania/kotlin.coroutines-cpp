"""Remove canonical no-op marker instructions without parsing coroutine frames.

Only complete, single-line direct calls with a constant i32 ID are recognized.
Unrecognized instructions and symbol declarations are retained byte for byte.
In particular, an invoke's unwind edge or a referenced function declaration
cannot be removed by treating it as a call. Use kxs-inject for LLVM parsing.
"""

import argparse
from pathlib import Path
import re


MARKER_CALL = re.compile(
    rb'[ \t]*(?:tail[ \t]+)?call[ \t]+void[ \t]+'
    rb'@(?:__kxs_suspend_point|"__kxs_suspend_point")'
    rb'\([ \t]*i32[ \t]+(?:noundef[ \t]+)?-?[0-9]+[ \t]*\)'
    rb'[ \t]*(?:#[0-9]+[ \t]*)?'
    rb'(?:,[ \t]*![A-Za-z0-9_.]+[ \t]+![0-9]+[ \t]*)*'
    rb'(?:;[^\r\n]*)?'
)


def strip_markers(ir):
    """Preserve all non-marker bytes, including CRLF, metadata and semicolons."""
    output = []
    removed = 0
    quoted = False
    for line in ir.splitlines(keepends=True):
        if not quoted and MARKER_CALL.fullmatch(line.rstrip(b'\r\n')):
            removed += 1
            continue
        output.append(line)
        # LLVM quoted identifiers/strings can cross lines. A marker-like line
        # inside one is data, even when the line itself resembles a call.
        index = 0
        while index < len(line):
            byte = line[index]
            if quoted and byte == ord('\\'):
                index += 2
                continue
            if byte == ord('"'):
                quoted = not quoted
            elif not quoted and byte == ord(';'):
                break
            index += 1
    return b''.join(output), removed


def transform_file(input_file, output_file):
    original = Path(input_file).read_bytes()
    transformed, removed = strip_markers(original)
    Path(output_file).write_bytes(transformed)
    return removed


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input_file')
    parser.add_argument('output_file')
    args = parser.parse_args()
    removed = transform_file(args.input_file, args.output_file)
    print(f'[KXS] Removed {removed} no-op marker call(s); preserved existing IR')


if __name__ == '__main__':
    main()
