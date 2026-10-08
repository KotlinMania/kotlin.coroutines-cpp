#!/usr/bin/env python3
"""Compare translated IntArray algorithms with Native stdlib and its real ABI."""
import argparse
import json
from pathlib import Path
import subprocess
import time


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cpp', required=True)
    parser.add_argument('--native', required=True)
    parser.add_argument('--work-dir', required=True)
    args = parser.parse_args()
    work = Path(args.work_dir)
    work.mkdir(parents=True, exist_ok=True)
    receipts = {}
    lines = {}
    for name, executable in (('cpp', args.cpp), ('native', args.native)):
        start = time.monotonic()
        result = subprocess.run([executable], capture_output=True, timeout=60)
        (work / f'{name}.trace').write_bytes(result.stdout)
        (work / f'{name}.stderr').write_bytes(result.stderr)
        receipts[name] = {'command': [executable], 'exit_code': result.returncode,
                          'seconds': time.monotonic() - start}
        lines[name] = result.stdout.decode().splitlines()
    handoff = 'native-int-array-handoff=get,set,length,copy,overlap,fill'
    native_algorithm = lines['native'][:-1]
    receipts['comparison'] = {
        'algorithm_observations': len(lines['cpp']),
        'identical': lines['cpp'] == native_algorithm,
        'native_handoff': lines['native'][-1:], 'expected_native_handoff': handoff,
        'limits': ['C++ fixed-length compiler storage is not Native ArrayHeader layout',
                   'reference toolchain is installed Native 2.4.10, not the pinned compiler build',
                   'runtime bounds failures are compared by class where messages are unspecified',
                   'capacity arguments satisfy the source non-negative precondition',
                   'macOS ARM64 execution does not establish bare-metal target acceptance']}
    (work / 'execution-receipt.json').write_text(json.dumps(receipts, indent=2) + '\n')
    if any(receipts[name]['exit_code'] != 0 for name in ('cpp', 'native')):
        raise RuntimeError('integer-array execution failed; inspect receipts and stderr')
    if lines['cpp'] != native_algorithm:
        differences = [(i + 1, a, b) for i, (a, b) in enumerate(zip(lines['cpp'], native_algorithm)) if a != b]
        raise AssertionError(f'integer-array observations differ: {differences[:5]}; lengths {len(lines["cpp"])}/{len(native_algorithm)}')
    if len(lines['cpp']) != 5367 or lines['native'][-1:] != [handoff]:
        raise AssertionError('integer-array contract output is incomplete')
    print('int-array-algorithm-observations=5367')
    print(handoff)


if __name__ == '__main__':
    main()
