"""Regressions for mandatory Kotlin/Native LLVM dispatch injection."""

import argparse
import importlib.util
from pathlib import Path
import shutil
import shlex
import subprocess
import sys
import tempfile
import time
import unittest


OPTIONS = None
TRANSFORM = None


MODULE = '''
declare void @__kxs_coroutine_begin(ptr)
declare void @__kxs_suspend_point(i32, ptr, ptr)
define ptr @f(ptr %label_field, ptr %result) {
  call void @__kxs_coroutine_begin(ptr %label_field)
  call void @__kxs_suspend_point(i32 1, ptr %label_field, ptr blockaddress(@f, %resume))
  ret ptr null
resume:
  ret ptr %result
}
'''


class IrInjectionTests(unittest.TestCase):
    @unittest.skipUnless(sys.platform == 'darwin', 'Apple Clang version regression requires macOS')
    def test_mismatched_plugin_package_is_rejected(self):
        with tempfile.TemporaryDirectory(dir=OPTIONS.work_dir, prefix='mismatch ') as directory:
            directory = Path(directory)
            (directory / 'main.cpp').write_text('int main() { return 0; }\n')
            (directory / 'CMakeLists.txt').write_text(f'''
cmake_minimum_required(VERSION 3.18)
project(mismatch LANGUAGES CXX)
include("{OPTIONS.modules}/KotlinxCoroutineTransform.cmake")
add_executable(example main.cpp)
kxs_enable_coroutine_transform(example)
''')
            version = subprocess.run([OPTIONS.compiler, '-dumpversion'],
                                     capture_output=True, text=True, check=True).stdout.strip()
            selected = subprocess.run(['/usr/bin/clang++', '-dumpversion'],
                                      capture_output=True, text=True, check=True).stdout.strip()
            if version == selected:
                self.skipTest('compiler versions match')
            result = subprocess.run([OPTIONS.cmake, '-S', str(directory),
                '-B', str(directory / 'build'), '-DCMAKE_CXX_COMPILER=/usr/bin/clang++',
                '-DKXS_LLVM_PASS_PLUGIN=' + OPTIONS.plugin,
                '-DKXS_LLVM_PASS_PLUGIN_VERSION=' + version], capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('KotlinxCoroutinePass requires Clang', result.stdout + result.stderr)

    def test_standalone_cmake_paths_with_spaces(self):
        with tempfile.TemporaryDirectory(dir=OPTIONS.work_dir, prefix='script space ') as directory:
            directory = Path(directory)
            input_file = directory / 'input file.ll'
            output_file = directory / 'output file.ll'
            input_file.write_text(MODULE)
            self.run_command([OPTIONS.cmake, f'-DINPUT_FILE={input_file}',
                              f'-DOUTPUT_FILE={output_file}',
                              f'-DKXS_INJECT_EXECUTABLE={OPTIONS.injector}', '-P',
                              str(Path(OPTIONS.modules) / 'kxs_transform_ir.cmake')])
            self.assertIn('indirectbr', output_file.read_text())
            self.assertNotIn('call void @__kxs_', output_file.read_text())
            wrapper = directory / 'wrapper.cmake'
            wrapper.write_text(f'set(KXS_INJECT_EXECUTABLE "{OPTIONS.injector}")\n'
                               f'include("{OPTIONS.modules}/KotlinxCoroutineTransform.cmake")\n'
                               f'kxs_transform_ir("{input_file}" "{output_file}")\n')
            self.run_command([OPTIONS.cmake, '-P', str(wrapper)])
            self.assertIn('indirectbr', output_file.read_text())

    def test_python_wrapper_requires_native_injector(self):
        with tempfile.TemporaryDirectory(dir=OPTIONS.work_dir) as directory:
            source = Path(directory) / 'input.ll'
            output = Path(directory) / 'output.ll'
            source.write_text(MODULE)
            TRANSFORM.transform_file(source, output, OPTIONS.injector)
            self.assertIn('store ptr blockaddress', output.read_text())
            with self.assertRaises(ValueError):
                TRANSFORM.transform_file(source, output, str(Path(directory) / 'missing-tool'))

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
        for name in ('KotlinxCoroutineTransform.cmake',):
            shutil.copy2(Path(OPTIONS.modules) / name, modules / name)
        plugin = directory / Path(OPTIONS.plugin).name
        shutil.copy2(OPTIONS.plugin, plugin)
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
        library_link_options = ' '.join(
            '"' + option.replace('\\', '\\\\').replace('"', '\\"') + '"'
            for option in shlex.split(OPTIONS.library_link_options))
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
target_link_options(settings INTERFACE {library_link_options})
set_source_files_properties(left/shared.cpp PROPERTIES COMPILE_DEFINITIONS KXS_SOURCE_OPTION=5)
file(GENERATE OUTPUT "${{CMAKE_CURRENT_BINARY_DIR}}/generated.cpp" CONTENT "int generated_value() {{ return 9; }}")
foreach(name transformed)
  add_executable(${{name}} main.cpp left/shared.cpp right/shared.cpp "${{CMAKE_CURRENT_BINARY_DIR}}/generated.cpp")
  target_link_libraries(${{name}} PRIVATE settings "{OPTIONS.library}")
endforeach()
set_property(TARGET transformed PROPERTY CXX_COMPILER_LAUNCHER "{sys.executable};{proxy};{trace}")
kxs_enable_coroutine_transform(transformed)
kxs_enable_coroutine_transform(transformed)
''')
        build = directory / 'build'
        self.run_command([OPTIONS.cmake, '-S', str(directory), '-B', str(build), '-G', generator,
                          f'-DCMAKE_CXX_COMPILER={OPTIONS.compiler}', f'-DKXS_LLVM_PASS_PLUGIN={plugin}', f'-DCMAKE_BUILD_TYPE={config}'])
        self.run_command([OPTIONS.cmake, '--build', str(build), '--config', config, '-j', '2'])
        binary_dir = build / config if 'Multi-Config' in generator else build
        transformed = self.run_command([str(binary_dir / 'transformed')])
        self.assertNotIn('-emit-llvm', trace.read_text())
        self.assertEqual(trace.read_text().count('-fpass-plugin='), 4)
        self.assertIn('test_resumed_exception_stops_continuation... PASSED', transformed)
        self.assertIn('test_independent_frames... PASSED', transformed)
        self.assertFalse(list(build.rglob('*.kxs*.ll')), 'production serialized IR unexpectedly')
        object_file = next(path for path in build.rglob('main.cpp.o') if 'transformed.dir' in str(path))
        all_objects = list(build.rglob('*.o'))
        max_mtime = max((p.stat().st_mtime_ns for p in all_objects), default=object_file.stat().st_mtime_ns)
        # macOS's bundled Make compares whole-second modification times; ensure sleep crosses into next whole second.
        time.sleep(max(1.1, (max_mtime // 1_000_000_000) + 1.1 - time.time()))
        header.write_text('inline constexpr int pipeline_value = 8;\n')
        self.run_command([OPTIONS.cmake, '--build', str(build), '--config', config, '-j', '2'])
        self.assertGreater(object_file.stat().st_mtime_ns, max_mtime, 'header dependency was lost')
        self.assertEqual(self.run_command([str(binary_dir / 'transformed')]), transformed)
        all_objects = list(build.rglob('*.o'))
        max_mtime = max((p.stat().st_mtime_ns for p in all_objects), default=object_file.stat().st_mtime_ns)
        time.sleep(max(1.1, (max_mtime // 1_000_000_000) + 1.1 - time.time()))
        plugin.touch()
        self.run_command([OPTIONS.cmake, '--build', str(build), '--config', config, '-j', '2'])
        self.assertGreater(object_file.stat().st_mtime_ns, max_mtime, 'plugin dependency was lost')

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
    parser.add_argument('--library-link-options', default='')
    parser.add_argument('--work-dir', required=True)
    parser.add_argument('--compiler', required=True)
    parser.add_argument('--injector', required=True)
    parser.add_argument('--plugin', required=True)
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
