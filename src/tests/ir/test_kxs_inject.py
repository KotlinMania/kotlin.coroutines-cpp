"""LLVM parser/verifier and runtime regressions for kxs-inject cleanup."""

import argparse
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


OPTIONS = None
MODULE = '''
declare void @__kxs_suspend_point(i32)
declare ptr @get_or_throw(ptr)
define void @initialization() {
  call void @__kxs_suspend_point(i32 9)
  ret void
}
define ptr @"frame with spaces"(ptr %frame, ptr %result, i1 %suspends) {
entry:
  %label_field = getelementptr { ptr, ptr, i32 }, ptr %frame, i32 0, i32 1
  %saved = load ptr, ptr %label_field
  %fresh = icmp eq ptr %saved, null
  br i1 %fresh, label %start, label %dispatch
dispatch:
  indirectbr ptr %saved, [label %resume]
start:
  store ptr blockaddress(@"frame with spaces", %resume), ptr %label_field
  call void @__kxs_suspend_point(i32 9)
  br i1 %suspends, label %suspended, label %immediate
suspended:
  ret ptr inttoptr (i64 1 to ptr)
immediate:
  ret ptr %result
resume:
  %value = call ptr @get_or_throw(ptr %result)
  ret ptr %value
}
'''


class NativeCleanupTests(unittest.TestCase):
    def setUp(self):
        self.directory = Path(tempfile.mkdtemp(dir=OPTIONS.work_dir, prefix=self._testMethodName + ' '))

    def run_command(self, command, success=True):
        result = subprocess.run(command, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, text=True, check=False)
        if success:
            self.assertEqual(result.returncode, 0, result.stdout)
        else:
            self.assertNotEqual(result.returncode, 0, result.stdout)
        return result.stdout

    def transform(self, source, success=True):
        input_file = self.directory / 'input.ll'
        output_file = self.directory / 'output.ll'
        input_file.write_text(source)
        output = self.run_command([OPTIONS.tool, str(input_file), '-o', str(output_file)], success)
        if success:
            return output_file.read_text()
        self.assertFalse(output_file.exists())
        return output

    def test_multiple_functions_keep_frame_and_result_paths(self):
        cleaned = self.transform(MODULE)
        self.assertNotIn('call void @__kxs_suspend_point', cleaned)
        self.assertNotIn('declare void @__kxs_suspend_point', cleaned)
        self.assertIn('getelementptr { ptr, ptr, i32 }, ptr %frame, i32 0, i32 1', cleaned)
        self.assertIn('store ptr blockaddress(@"frame with spaces", %resume), ptr %label_field', cleaned)
        self.assertIn('indirectbr ptr %saved, [label %resume]', cleaned)
        self.assertIn('%value = call ptr @get_or_throw(ptr %result)', cleaned)
        self.assertNotIn('kxs_dispatch', cleaned)

    def test_referenced_marker_declaration_survives(self):
        cleaned = self.transform('@callback = global ptr @__kxs_suspend_point\n' + MODULE)
        self.assertIn('declare void @__kxs_suspend_point(i32)', cleaned)
        self.assertIn('@callback = global ptr @__kxs_suspend_point', cleaned)

    def test_invoke_keeps_its_unwind_edge(self):
        source = '''
declare void @__kxs_suspend_point(i32)
declare i32 @__gxx_personality_v0(...)
define void @f() personality ptr @__gxx_personality_v0 {
  invoke void @__kxs_suspend_point(i32 1) to label %ok unwind label %error
ok:
  ret void
error:
  %exception = landingpad { ptr, i32 } cleanup
  resume { ptr, i32 } %exception
}
'''
        cleaned = self.transform(source)
        self.assertIn('invoke void @__kxs_suspend_point', cleaned)
        self.assertIn('unwind label %error', cleaned)
        self.assertIn('declare void @__kxs_suspend_point', cleaned)

    def test_invalid_ssa_is_rejected_before_writing(self):
        source = '''
define i32 @f(i1 %which) {
  br i1 %which, label %a, label %b
a:
  %value = add i32 1, 2
  br label %join
b:
  br label %join
join:
  ret i32 %value
}
'''
        self.assertIn('does not dominate', self.transform(source, False))

    def test_bad_marker_signature_is_rejected(self):
        source = 'declare void @__kxs_suspend_point(i64)\ndefine void @f() { ret void }\n'
        self.assertIn('Invalid __kxs_suspend_point signature', self.transform(source, False))

    def test_bitcode_roundtrip(self):
        self.transform(MODULE)
        bitcode = self.directory / 'output.bc'
        self.run_command([OPTIONS.tool, str(self.directory / 'input.ll'), '-bc', '-o', str(bitcode)])
        self.assertEqual(bitcode.read_bytes()[:4], b'BC\xc0\xde')
        output_file = self.directory / 'roundtrip.ll'
        self.run_command([OPTIONS.tool, str(bitcode), '-o', str(output_file)])
        self.assertIn('indirectbr', output_file.read_text())
        self.assertNotIn('call void @__kxs_suspend_point', output_file.read_text())

    def test_actual_coroutine_runtime_roundtrip(self):
        core_test = Path(OPTIONS.core_test) if OPTIONS.core_test else Path(OPTIONS.root) / 'src/tests/src/test_suspension_core.cpp'
        source = core_test.read_text()
        source = source.replace('int main() {',
                                'extern "C" void __kxs_suspend_point(int) noexcept {}\nint main() {')
        source_file = self.directory / 'core.cpp'
        source_file.write_text(source)
        input_file = self.directory / 'core.ll'
        cleaned_file = self.directory / 'core.cleaned.ll'
        includes = ['-I', OPTIONS.headers, '-I', str(Path(OPTIONS.root) / 'src'),
                    '-I', str(Path(OPTIONS.root) / 'src/kotlinx/coroutines')]
        flags = ['-std=c++20', '-Wno-gnu-label-as-value', '-UNDEBUG']
        self.run_command([OPTIONS.compiler, *flags, *includes, '-S', '-emit-llvm',
                          str(source_file), '-o', str(input_file)])
        self.run_command([OPTIONS.tool, str(input_file), '-o', str(cleaned_file)])
        libraries = [OPTIONS.library] if OPTIONS.library else []
        before = self.directory / 'before'
        after = self.directory / 'after'
        self.run_command([OPTIONS.compiler, *flags, *includes, str(source_file), *libraries,
                          '-o', str(before)])
        self.run_command([OPTIONS.compiler, str(cleaned_file), *libraries, '-o', str(after)])
        self.assertEqual(self.run_command([str(after)]), self.run_command([str(before)]))


def main():
    global OPTIONS
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tool', required=True)
    parser.add_argument('--root', required=True)
    parser.add_argument('--headers', required=True)
    parser.add_argument('--compiler', required=True)
    parser.add_argument('--work-dir', required=True)
    parser.add_argument('--library')
    parser.add_argument('--core-test')
    OPTIONS, remaining = parser.parse_known_args()
    Path(OPTIONS.work_dir).mkdir(parents=True, exist_ok=True)
    unittest.main(argv=[sys.argv[0]] + remaining)


if __name__ == '__main__':
    main()
