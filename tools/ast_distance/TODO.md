# AST Distance - TODO

## Features to Add

### --generate-stubs Command

Add a built-in command to generate target language stub files from source structure.

**Usage:**
```bash
./ast_distance --generate-stubs <src_dir> <src_lang> <tgt_dir> <tgt_lang> <base_package>
```

**Example:**
```bash
./ast_distance --generate-stubs tmp/starlark rust \
  src/commonMain/kotlin/io/github/kotlinmania/starlark_kotlin kotlin \
  io.github.kotlinmania.starlark_kotlin
```

**Requirements:**
1. Scan source directory for all source files
2. Create target directory structure (mkdir -p)
3. Generate stub files with:
   - Port-lint header: `// port-lint: source <rel-path>`
   - Package declaration
   - Proper naming conventions:
     - Rust → Kotlin: directories lowercase, filenames CamelCase
     - C++ → Kotlin: similar conventions
4. Skip existing files (don't overwrite)
5. Report created/skipped counts

**Reference Implementation:**
See `starlark-kotlin/tools/generate_stubs.py` for the logic.

**Benefits:**
- Eliminates need for external Python scripts
- Ensures consistent stub generation across projects
- Integrates with existing codebase scanning
- Part of complete porting workflow

**Priority:** Medium - Nice to have but workaround exists
