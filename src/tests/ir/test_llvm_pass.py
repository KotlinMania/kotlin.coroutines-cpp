"""Execute Clang's in-process Kotlinx lowering, including optimized handoffs."""
import argparse
from pathlib import Path
import subprocess
import tempfile
import unittest

OPTIONS = None


class CompilerPassTests(unittest.TestCase):
    def setUp(self):
        self.work = Path(tempfile.mkdtemp(dir=OPTIONS.work_dir, prefix='compiler pass '))

    def command(self, arguments, success=True):
        result = subprocess.run(arguments, capture_output=True, text=True)
        output = result.stdout + result.stderr
        if success:
            self.assertEqual(result.returncode, 0, output)
        else:
            self.assertNotEqual(result.returncode, 0, output)
        return output

    def flags(self):
        return [OPTIONS.compiler, '-std=c++20', '-UNDEBUG', '-I' + str(Path(OPTIONS.root) / 'src'),
                '-fpass-plugin=' + OPTIONS.plugin]

    def execute_core(self, optimization, sanitizer=False):
        executable = self.work / 'core'
        flags = ['-fsanitize=address', '-fno-omit-frame-pointer'] if sanitizer else []
        output = self.command([*self.flags(), optimization, '-g', *flags,
            str(Path(OPTIONS.root) / 'src/tests/src/test_suspension_core.cpp'),
            OPTIONS.library, '-pthread', '-o', str(executable)])
        self.assertNotIn('ignoring invalid debug info', output)
        trace = self.command([str(executable)])
        for name in ('test_loop_suspend', 'test_yield_value_resume_result',
                     'test_yield_value_immediate_result', 'test_independent_frames',
                     'test_resumed_exception_stops_continuation'):
            self.assertIn(name + '...', trace)
        self.assertFalse(list(self.work.glob('*.ll')))

    def test_debug_handoffs(self):
        self.execute_core('-O0')

    def test_optimized_address_sanitizer_handoffs(self):
        self.execute_core('-O2', sanitizer=True)

    def test_ir_values_and_debug_metadata_remain_in_compiler(self):
        output = self.work / 'diagnostic.ll'
        diagnostics = self.command([*self.flags(), '-O0', '-g', '-S', '-emit-llvm',
            str(Path(OPTIONS.root) / 'src/tests/src/test_suspension_core.cpp'), '-o', str(output)])
        self.assertNotIn('ignoring invalid debug info', diagnostics)
        ir = output.read_text()
        self.assertIn('indirectbr', ir)
        self.assertIn('blockaddress', ir)
        self.assertIn('!DICompileUnit', ir)
        self.assertIn('!DILocalVariable', ir)
        self.assertNotIn('call void @__kxs_', ir)
        self.assertNotIn('declare void @__kxs_', ir)

    def test_standard_cpp_marker_branches_execute_with_strict_warnings(self):
        source = self.work / 'standard.cpp'
        source.write_text('''#include <cassert>
extern "C" void __kxs_coroutine_begin(void**) noexcept;
extern "C" void __kxs_suspend_site(int, void**) noexcept;
extern "C" bool __kxs_resume_point(int) noexcept;
struct Frame { void* label = nullptr; int before = 0; };
void* run(Frame* frame, void* value) {
    __kxs_coroutine_begin(&frame->label);
    ++frame->before;
    __kxs_suspend_site(29, &frame->label);
    if (__kxs_resume_point(29)) goto resume;
    return reinterpret_cast<void*>(1);
resume:
    return value;
}
int main() {
    int value = 42;
    Frame* frame = new Frame;
    assert(run(frame, nullptr) == reinterpret_cast<void*>(1));
    assert(frame->label && frame->before == 1);
    assert(run(frame, &value) == &value && frame->before == 1);
    delete frame;
}
''')
        executable = self.work / 'standard'
        strict = ['-Wall', '-Wextra', '-Wpedantic', '-Werror']
        for optimization in ('-O0', '-O2'):
            self.command([*self.flags(), *strict, optimization, '-fsanitize=address,undefined',
                          str(source), '-o', str(executable)])
            self.command([str(executable)])
        ir_file = self.work / 'standard.ll'
        self.command([*self.flags(), *strict, '-O0', '-S', '-emit-llvm',
                      str(source), '-o', str(ir_file)])
        ir = ir_file.read_text()
        self.assertIn('blockaddress', ir)
        self.assertIn('indirectbr', ir)
        self.assertNotIn('__kxs_', ir)

    def test_stack_frame_is_a_compiler_error(self):
        source = self.work / 'invalid.cpp'
        source.write_text('''
extern "C" void __kxs_coroutine_begin(void**) noexcept;
extern "C" void __kxs_suspend_point(int, void**, void*) noexcept;
void* invalid() {
    void* label = nullptr;
    __kxs_coroutine_begin(&label);
    __kxs_suspend_point(1, &label, &&resume);
    return nullptr;
resume:
    return nullptr;
}
''')
        output = self.command([*self.flags(), '-O0', '-c', str(source),
                               '-o', str(self.work / 'invalid.o')], success=False)
        self.assertIn('persistent frame storage', output)
        self.assertFalse((self.work / 'invalid.o').exists())

    def test_no_plugin_cannot_link_markers(self):
        flags = [flag for flag in self.flags() if not flag.startswith('-fpass-plugin=')]
        output = self.command([*flags,
            str(Path(OPTIONS.root) / 'src/tests/src/test_suspension_core.cpp'),
            OPTIONS.library, '-pthread', '-o', str(self.work / 'missing')], success=False)
        self.assertIn('__kxs_', output)


def main():
    global OPTIONS
    parser = argparse.ArgumentParser(description=__doc__)
    for argument in ('compiler', 'plugin', 'root', 'library', 'work-dir'):
        parser.add_argument('--' + argument, required=True)
    OPTIONS, remaining = parser.parse_known_args()
    Path(OPTIONS.work_dir).mkdir(parents=True, exist_ok=True)
    unittest.main(argv=['test_llvm_pass'] + remaining)


if __name__ == '__main__':
    main()
