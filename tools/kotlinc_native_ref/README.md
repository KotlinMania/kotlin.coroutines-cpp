# Kotlin/Native IR Reference (for C++ DSL plugin)

This directory holds **reference material** from the vendored Kotlin/Native
compiler snapshot under `tmp/kotlin/`.

Files:
- `IrToBitcode_coroutines.hpp` — hand‑transliterated seed of the coroutine‑specific
  lowering patterns used by kotlinc/Kotlin‑Native (`evaluateSuspendableExpression`,
  `evaluateSuspensionPoint`, etc.).
- `translate_ir_to_cpp.py` — naive whole‑file transliterator for
  `IrToBitcode.kt`. This emits a C++‑ish corpus for manual cleanup.

Generate a raw transliteration:

```bash
python3 tools/kotlinc_native_ref/translate_ir_to_cpp.py \
  tmp/kotlin/kotlin-native/backend.native/compiler/ir/backend.native/src/org/jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt \
  tools/kotlinc_native_ref/IrToBitcode_full.cpp
```

The generated file is *not* committed by default.

