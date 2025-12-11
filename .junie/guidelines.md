### kotlinx.coroutines-cpp – Development Guidelines (project-specific)

#### Build and configuration

- Toolchain
  - CMake >= 3.16, a C++20 compiler (clang++ on macOS is fine), and pthreads.
  - Out-of-source builds are expected; all artifacts land under `build/`.

- Standard build
  - From repo root:
    ```bash
    mkdir -p build && cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . -- -j4
    ```
  - Output directories (set in root `CMakeLists.txt`):
    - Binaries: `build/bin/`
    - Libraries/archives: `build/lib/`

- Notable CMake options
  - `-DKOTLIN_NATIVE_RUNTIME_AVAILABLE=ON|OFF` (default OFF)
    - Weak-links support for Kotlin/Native GC coordination. Leave OFF unless the Kotlin/Native runtime is present and the GC bridge is required.
  - `-DKOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=ON|OFF` (default OFF)
    - Builds the Apple-focused clang suspend DSL plugin (`tools/clang_suspend_plugin`). Only enable if you’re iterating on the plugin.

- Core library target
  - `kotlinx-coroutines-core` is assembled from `kotlinx-coroutines-core/{common,concurrent,native,nativeDarwin}/src/...` and installs public headers from `include/`.
  - Threads are linked via `Threads::Threads`.

#### Tests: configuring, running, adding

- Test harness
  - Tests are plain C++ executables registered via CTest. See `tests/CMakeLists.txt`.
  - Helper function:
    ```cmake
    function(add_coroutine_test TEST_NAME)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.cpp")
            add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
            target_include_directories(${TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
            target_link_libraries(${TEST_NAME} PRIVATE pthread kotlinx-coroutines-core)
            add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        endif()
    endfunction()
    ```

- Running tests
  - After a successful configure/build, from `build/`:
    ```bash
    # List tests known to CTest
    ctest -N
    # Run all configured tests (will only execute tests whose executables were built)
    ctest --output-on-failure
    # Run a subset by name/regex (example below)
    ctest -R test_cancellation --output-on-failure
    ```
  - Verified example (executed during preparation of this document):
    ```bash
    ctest -R test_cancellation --output-on-failure
    # 1/1 Test #2: test_cancellation ... Passed 0.25 sec
    ```
  - Note: If one test target fails to compile, you can still run other tests via `-R` without fixing the failing target immediately (CTest will select only the matching, built executables).

- Adding a new test
  1. Create a source file `tests/your_test.cpp`.
  2. Register it in `tests/CMakeLists.txt` via `add_coroutine_test(your_test)`.
  3. Reconfigure/rebuild; CTest will pick it up automatically.

  Minimal example (this was created and run transiently to validate the flow):
  ```cpp
  // tests/test_guidelines_demo.cpp
  #include "kotlinx/coroutines/Result.hpp"
  #include <cassert>
  int main() {
      auto ok = kotlinx::coroutines::Result<int>::success(7);
      assert(ok.is_success());
      return 0;
  }
  ```
  Register in `tests/CMakeLists.txt`:
  ```cmake
  add_coroutine_test(test_guidelines_demo)
  ```
  Build and run just this test:
  ```bash
  cmake --build build --target test_guidelines_demo -- -j
  (cd build && ctest -R test_guidelines_demo --output-on-failure)
  ```
  For this repository, the demo file and its CMake entry were only used temporarily to validate the process and were removed as requested; keep this snippet as a template.

#### Kotlin/Native GC bridge tests

- The optional `tests/gc_bridge` suite integrates Kotlin/Native. See `tests/gc_bridge/README.md` for prerequisites and commands.
  - Requires Kotlin/Native tooling (`cinterop`, `kotlinc-native`) and, if linking the runtime, `-DKOTLIN_NATIVE_RUNTIME_AVAILABLE=ON` and proper environment for the runtime/libs.

#### Development notes and code style

- Language standard: C++20, with `-Wall -Wextra -Wpedantic` enabled globally.
- Public headers live under `include/kotlinx/coroutines/...`; implementation sources are split by platform concerns under `kotlinx-coroutines-core/`.
- Namespaces: `kotlinx::coroutines` (and sub-namespaces like `kotlinx::coroutines::internal`).
- Prefer explicit `override` on overridden virtuals (some legacy headers predate strict override usage; compilers warn accordingly).
- Threading: core links `Threads::Threads`; test executables additionally link `pthread` explicitly through the helper.
- Build layout is fixed by top-level `CMakeLists.txt`; multi-config generators are supported via repeated output dir settings.

#### macOS-specific observations

- During local builds, harmless `ld` warnings about `curl` search paths may appear if Homebrew paths are absent. They did not impact compiling or running the verified tests.
- If you only need to iterate specific areas, build individual targets to reduce turnaround, e.g.:
  ```bash
  cmake --build build --target kotlinx-coroutines-core -- -j
  cmake --build build --target test_cancellation -- -j
  ```

#### Quick checklist

- Configure once per build dir; prefer out-of-source.
- Use CTest filters (`-R name`) to run subsets when the full suite is not green yet.
- New tests: drop a `tests/<name>.cpp` and add `add_coroutine_test(<name>)`.
- Don’t enable Kotlin/Native options unless you have the toolchain locally.
