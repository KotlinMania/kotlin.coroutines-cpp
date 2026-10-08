#!/usr/bin/env python3
"""Compare source algorithms and verify real Native array handoffs separately."""
import argparse
import json
from pathlib import Path
import subprocess
import time


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cpp", required=True)
    parser.add_argument("--native", required=True)
    parser.add_argument("--work-dir", required=True)
    args = parser.parse_args()
    work = Path(args.work_dir)
    work.mkdir(parents=True, exist_ok=True)
    receipts = {}
    lines = {}
    for name, executable in (("cpp", args.cpp), ("native", args.native)):
        start = time.monotonic()
        result = subprocess.run([executable], capture_output=True, timeout=60)
        (work / f"{name}.trace").write_bytes(result.stdout)
        (work / f"{name}.stderr").write_bytes(result.stderr)
        receipts[name] = {"command": [executable], "exit_code": result.returncode,
                          "seconds": time.monotonic() - start}
        lines[name] = result.stdout.decode().splitlines()
    native_handoff = ["native-array-handoff=copy,overlap,reset,fill,rooted-get",
                      "native-cpp-reset-releases-reference=true"]
    observed_handoff = lines["native"][-len(native_handoff):]
    native_algorithm = lines["native"][:-len(native_handoff)]
    identical = lines["cpp"] == native_algorithm
    receipts["comparison"] = {
        "algorithm_observations": len(lines["cpp"]), "identical": identical,
        "native_handoff": observed_handoff, "native_handoff_expected": native_handoff,
        "limits": ["compiler-owned C++ storage is not Native array ABI",
                   "null and implementation-dependent uninitialized reads are grouped",
                   "runtime bounds exceptions are compared by class where messages are unspecified",
                   "C++ reference lifetime uses shared Name; Native uses GC-managed Array<Int>",
                   "array-only execution does not verify shared coroutine state machines or MLX GPU execution"]}
    (work / "execution-receipt.json").write_text(json.dumps(receipts, indent=2) + "\n")
    if any(row["exit_code"] != 0 for row in (receipts["cpp"], receipts["native"])):
        raise RuntimeError("array execution failed; inspect stderr and receipts")
    if not identical:
        differences = [(i + 1, a, b) for i, (a, b) in enumerate(zip(lines["cpp"], native_algorithm)) if a != b]
        raise AssertionError(f"array observations differ: {differences[:5]}")
    assert observed_handoff == native_handoff, observed_handoff
    assert len(lines["cpp"]) == 930, len(lines["cpp"])
    print(f"array-algorithm-observations={len(lines['cpp'])}")
    print("native-array-handoff-groups=2")


if __name__ == "__main__":
    main()
