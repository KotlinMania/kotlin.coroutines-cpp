"""Regressions for preserving Kotlin/Native state/result handoffs in IR cleanup."""

import argparse
import importlib.util
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time
import unittest


OPTIONS = None
TRANSFORM = None


class IrCleanupTests(unittest.TestCase):
    def test_unmarked_input_is_identical(self):
        original = b'; call void @__kxs_suspend_point(i32 1)\r\n!0 = !{!"text; text"}\r\n'
        cleaned, count = TRANSFORM.strip_markers(original)
        self.assertEqual((cleaned, count), (original, 0))

    def test_marker_text_inside_multiline_string_is_preserved(self):
        original = b'@text = constant [64 x i8] c"line; data\n  call void @__kxs_suspend_point(i32 1)\nend"\n'
        self.assertEqual(TRANSFORM.strip_markers(original), (original, 0))

    def test_multiple_functions_preserve_dispatch_and_results(self):
        before = b'''declare void @__kxs_suspend_point(i32)
define ptr @"first coroutine"(ptr %frame) {
  call void @__kxs_suspend_point(i32 noundef 17) #0, !dbg !1 ; marker
  %label = load ptr, ptr %frame
  indirectbr ptr %label, [label %resume]
resume:
  %value = call ptr @get_or_throw(ptr %frame)
  ret ptr %value
}
define ptr @second(ptr %frame) {
  store ptr blockaddress(@second, %resume), ptr %frame
  call void @"__kxs_suspend_point"(i32 17)
  call void @side_effect()
  br label %resume
resume:
  ret ptr %frame
}
'''
        expected = b'\n'.join(line for line in before.split(b'\n')
                              if line.lstrip().startswith(b'call void @__kxs_suspend_point') is False
                              and line.lstrip().startswith(b'call void @"__kxs_suspend_point"') is False)
        cleaned, count = TRANSFORM.strip_markers(before)
        self.assertEqual(count, 2)
        self.assertEqual(cleaned, expected)
        self.assertEqual(TRANSFORM.strip_markers(cleaned), (cleaned, 0))

    def test_unrecognized_calls_and_symbol_uses_survive(self):
        original = b'''@callback = global ptr @__kxs_suspend_point
declare void @__kxs_suspend_point(i32)
; call void @__kxs_suspend_point(i32 1)
  invoke void @__kxs_suspend_point(i32 1) to label %ok unwind label %error
  call void @__kxs_suspend_point(i32 %dynamic)
  call void @__kxs_suspend_point_extra(i32 1)
  call void @__kxs_suspend_point(i32 1) [ "funclet"(token %pad) ]
  call void @other(i32 1)
'''
        self.assertEqual(TRANSFORM.strip_markers(original), (original, 0))

    def test_standalone_cmake_paths_with_spaces(self):
        with tempfile.TemporaryDirectory(dir=OPTIONS.work_dir, prefix='script space ') as directory:
            directory = Path(directory)
            input_file = directory / 'input file.ll'
            output_file = directory / 'output file.ll'
            input_file.write_bytes(b'  call void @__kxs_suspend_point(i32 1)\r\n; kept; comment\r\n')
            self.run_command([OPTIONS.cmake, f'-DINPUT_FILE={input_file}',
                              f'-DOUTPUT_FILE={output_file}', '-P',
                              str(Path(OPTIONS.modules) / 'kxs_transform_ir.cmake')])
            self.assertEqual(output_file.read_bytes(), b'; kept; comment\r\n')
            wrapper = directory / 'wrapper.cmake'
            wrapper.write_text(f'include("{OPTIONS.modules}/KotlinxCoroutineTransform.cmake")\n'
                               f'kxs_transform_ir("{input_file}" "{output_file}")\n')
            self.run_command([OPTIONS.cmake, '-P', str(wrapper)])
            self.assertEqual(output_file.read_bytes(), b'; kept; comment\r\n')

    def run_command(self, command):
        result = subprocess.run(command, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, text=True, check=False)
        self.assertEqual(result.returncode, 0, result.stdout)
        return result.stdout

    def build_fixture(self, generator, config, sanitize=False):
        # Same-basename sources, a generated source, source options and transitive
        # settings exercise the compile graph, rather than mirroring its code.
        directory = Path(tempfile.mkdtemp(dir=OPTIONS.work_dir, prefix='compile space '))
        modules = directory / 'modules'
        modules.mkdir()
        for name in ('KotlinxCoroutineTransform.cmake', 'kxs_compile.py', 'kxs_transform_ir.py'):
            shutil.copy2(Path(OPTIONS.modules) / name, modules / name)
        proxy = directory / 'previous_launcher.py'
        trace = directory / 'launcher.log'
        proxy.write_text('import json, subprocess, sys\n'
                         'with open(sys.argv[1], "a") as trace:\n'
                         '    trace.write(json.dumps(sys.argv[2:]) + chr(10))\n'
                         'sys.exit(subprocess.run(sys.argv[2:]).returncode)\n')
        includes = directory / 'headers with spaces'
        includes.mkdir()
        header = includes / 'settings.hpp'
        header.write_text('inline constexpr int pipeline_value = 7;\n')
        for subdir in ('left', 'right'):
            (directory / subdir).mkdir()
        (directory / 'left/shared.cpp').write_text(
            '#include "settings.hpp"\n#ifndef KXS_SOURCE_OPTION\n#error lost source option\n#endif\n'
            'int left_value() { return pipeline_value + KXS_SOURCE_OPTION; }\n')
        (directory / 'right/shared.cpp').write_text(
            '#include "settings.hpp"\nint right_value() { return pipeline_value; }\n')
        main = Path(OPTIONS.core_test).read_text()
        main = '#include "settings.hpp"\n#include <span>\n' + main
        # Keep the fixture independent of sanitizer flags on a prebuilt library.
        main = main.replace('int main() {',
                            'extern "C" void __kxs_suspend_point(int) noexcept {}\nint main() {')
        main = main.replace('int main() {', '''
int left_value();
int right_value();
int generated_value();
int main() {
    static_assert(KXS_INHERITED == 41);
    static_assert(__cplusplus >= 202002L);
    int values[] = {left_value(), right_value(), generated_value()};
    auto view = std::span(values);
    assert(view[0] == pipeline_value + 5);
    assert(view[1] == pipeline_value);
    assert(view[2] == 9);
''')
        (directory / 'main.cpp').write_text(main)
        sanitizer = 'target_compile_options(settings INTERFACE -fsanitize=address -fno-omit-frame-pointer)\ntarget_link_options(settings INTERFACE -fsanitize=address)\n' if sanitize else ''
        (directory / 'CMakeLists.txt').write_text(f'''
cmake_minimum_required(VERSION 3.18)
project(ir_fixture LANGUAGES CXX)
include("{modules}/KotlinxCoroutineTransform.cmake")
add_library(settings INTERFACE)
target_include_directories(settings INTERFACE "{OPTIONS.headers}" "{OPTIONS.root}/src"
    "{OPTIONS.root}/src/kotlinx/coroutines" "{includes}")
target_compile_definitions(settings INTERFACE KXS_INHERITED=41)
target_compile_features(settings INTERFACE cxx_std_20)
target_compile_options(settings INTERFACE -Werror -Wno-gnu-label-as-value -UNDEBUG)
{sanitizer}
set_source_files_properties(left/shared.cpp PROPERTIES COMPILE_DEFINITIONS KXS_SOURCE_OPTION=5)
file(GENERATE OUTPUT "${{CMAKE_CURRENT_BINARY_DIR}}/generated.cpp" CONTENT "int generated_value() {{ return 9; }}")
foreach(name baseline transformed)
  add_executable(${{name}} main.cpp left/shared.cpp right/shared.cpp "${{CMAKE_CURRENT_BINARY_DIR}}/generated.cpp")
  target_link_libraries(${{name}} PRIVATE settings "{OPTIONS.library}")
endforeach()
set_property(TARGET transformed PROPERTY CXX_COMPILER_LAUNCHER "{sys.executable};{proxy};{trace}")
kxs_enable_coroutine_transform(transformed)
kxs_enable_coroutine_transform(transformed)
''')
        build = directory / 'build'
        self.run_command([OPTIONS.cmake, '-S', str(directory), '-B', str(build), '-G', generator,
                          f'-DCMAKE_CXX_COMPILER={OPTIONS.compiler}', f'-DCMAKE_BUILD_TYPE={config}'])
        self.run_command([OPTIONS.cmake, '--build', str(build), '--config', config, '-j', '2'])
        binary_dir = build / config if 'Multi-Config' in generator else build
        baseline = self.run_command([str(binary_dir / 'baseline')])
        transformed = self.run_command([str(binary_dir / 'transformed')])
        self.assertEqual(transformed, baseline)
        self.assertEqual(trace.read_text().count('-emit-llvm'), 4)
        self.assertIn('test_resumed_exception_stops_continuation... PASSED', transformed)
        self.assertIn('test_independent_frames... PASSED', transformed)
        ir_files = list(build.rglob('*.kxs.cleaned.ll'))
        self.assertEqual(len(ir_files), 4)
        main_ir = next(path for path in ir_files if 'main.cpp.o.' in path.name)
        ir = main_ir.read_bytes()
        self.assertIn(b'indirectbr', ir)
        self.assertIn(b'blockaddress', ir)
        self.assertNotIn(b'call void @__kxs_suspend_point', ir)
        self.assertNotIn(b'invoke void @__kxs_suspend_point', ir)
        object_file = main_ir.with_name(main_ir.name[:-len('.kxs.cleaned.ll')])
        before = object_file.stat().st_mtime_ns
        # macOS's bundled Make compares whole-second modification times.
        time.sleep(max(0, before // 1_000_000_000 + 1.1 - time.time()))
        header.write_text('inline constexpr int pipeline_value = 8;\n')
        self.run_command([OPTIONS.cmake, '--build', str(build), '--config', config, '-j', '2'])
        self.assertGreater(object_file.stat().st_mtime_ns, before, 'header dependency was lost')
        self.assertEqual(self.run_command([str(binary_dir / 'transformed')]), baseline)
        before = object_file.stat().st_mtime_ns
        time.sleep(max(0, before // 1_000_000_000 + 1.1 - time.time()))
        with (modules / 'kxs_transform_ir.py').open('a') as helper:
            helper.write('\n# Exercise helper dependency tracking.\n')
        self.run_command([OPTIONS.cmake, '--build', str(build), '--config', config, '-j', '2'])
        self.assertGreater(object_file.stat().st_mtime_ns, before, 'helper dependency was lost')

    def test_make_debug_pipeline(self):
        self.build_fixture('Unix Makefiles', 'Debug')

    def test_make_optimized_address_sanitizer_pipeline(self):
        self.build_fixture('Unix Makefiles', 'RelWithDebInfo', sanitize=True)

    @unittest.skipUnless(shutil.which('ninja'), 'Ninja is not installed')
    def test_ninja_multiconfig_pipeline(self):
        self.build_fixture('Ninja Multi-Config', 'Debug')


def main():
    global OPTIONS, TRANSFORM
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', required=True)
    parser.add_argument('--modules', required=True)
    parser.add_argument('--headers', required=True)
    parser.add_argument('--core-test', required=True)
    parser.add_argument('--library', required=True)
    parser.add_argument('--work-dir', required=True)
    parser.add_argument('--compiler', required=True)
    parser.add_argument('--cmake', required=True)
    OPTIONS, remaining = parser.parse_known_args()
    Path(OPTIONS.work_dir).mkdir(parents=True, exist_ok=True)
    spec = importlib.util.spec_from_file_location('kxs_transform_ir',
                                                Path(OPTIONS.modules) / 'kxs_transform_ir.py')
    TRANSFORM = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(TRANSFORM)
    unittest.main(argv=[sys.argv[0]] + remaining)


if __name__ == '__main__':
    main()
