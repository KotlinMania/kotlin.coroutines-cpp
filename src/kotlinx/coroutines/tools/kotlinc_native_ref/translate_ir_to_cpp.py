#!/usr/bin/env python3
"""
Naive Kotlin/Native IR-to-C++ transliteration helper.

Purpose:
  Provide a quick starting corpus by "slurping" IrToBitcode.kt and emitting a
  mechanically rewritten C++-ish version. This is intentionally lossy and does
  NOT produce compiling C++; it's a scaffolding step for manual cleanup.

Usage:
  tools/kotlinc_native_ref/translate_ir_to_cpp.py <path-to-IrToBitcode.kt> [out.cpp]
"""

from __future__ import annotations

import pathlib
import re
import sys


def transliterate_line(line: str) -> str:
    l = line

    # Drop Kotlin visibility / modifiers.
    for kw in ("private ", "internal ", "protected ", "override ", "inline ", "data ", "sealed "):
        l = l.replace(kw, "")

    # Basic keyword swaps.
    l = l.replace("val ", "auto ")
    l = l.replace("var ", "auto ")
    l = l.replace("when ", "switch ")
    l = l.replace("mutableListOf<", "std::vector<")
    l = l.replace("MutableList<", "std::vector<")
    l = l.replace("List<", "std::vector<")

    # fun -> auto
    l = re.sub(r"\bfun\s+([A-Za-z0-9_<>]+)\s*\(", r"auto \1(", l)

    # Kotlin nullability -> strip '?'
    l = l.replace("?", "")

    # Kotlin 'Unit' -> nullptr placeholder
    l = l.replace("unitType", "/*Unit*/")

    # Lambda braces to C++ style (best-effort)
    l = l.replace("-> Unit", "/*-> void*/")

    return l


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    src_path = pathlib.Path(sys.argv[1])
    if not src_path.exists():
        print(f"error: {src_path} not found", file=sys.stderr)
        return 1

    out_path = pathlib.Path(sys.argv[2]) if len(sys.argv) >= 3 else None

    src = src_path.read_text(encoding="utf-8", errors="ignore").splitlines()

    out_lines: list[str] = []
    out_lines.append("// AUTO-GENERATED (naive) from IrToBitcode.kt")
    out_lines.append(f"// Source: {src_path}")
    out_lines.append("")

    for line in src:
        out_lines.append(transliterate_line(line))

    out_text = "\n".join(out_lines) + "\n"

    if out_path:
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(out_text, encoding="utf-8")
    else:
        sys.stdout.write(out_text)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

