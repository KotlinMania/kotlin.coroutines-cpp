"""LLVM injection, persistent-frame and runtime regressions for kxs-inject."""

import argparse
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


OPTIONS = None
MODULE = '''
declare void @__kxs_coroutine_begin(ptr)
declare void @__kxs_suspend_point(i32, ptr, ptr)
define ptr @"frame with spaces"(ptr %frame, ptr %result) {
entry:
  %label_field = getelementptr { ptr, ptr, i32 }, ptr %frame, i32 0, i32 1
  call void @__kxs_coroutine_begin(ptr %label_field)
  %counter = getelementptr { ptr, ptr, i32 }, ptr %frame, i32 0, i32 2
  %old = load i32, ptr %counter
  %next = add i32 %old, 1
  store i32 %next, ptr %counter
  call void @__kxs_suspend_point(i32 9, ptr %label_field, ptr blockaddress(@"frame with spaces", %resume))
  ret ptr inttoptr (i64 1 to ptr)
resume:
  ret ptr %result
}
'''


class NativeInjectionTests(unittest.TestCase):
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

    def test_injects_dispatch_and_persistent_frame_store(self):
        injected = self.transform(MODULE)
        self.assertNotIn('call void @__kxs_', injected)
        self.assertNotIn('declare void @__kxs_', injected)
        self.assertIn('getelementptr { ptr, ptr, i32 }, ptr %frame, i32 0, i32 1', injected)
        self.assertIn('store ptr blockaddress(@"frame with spaces", %resume), ptr %label_field', injected)
        self.assertIn('indirectbr ptr %kxs_saved_label, [label %resume]', injected)
        self.assertNotIn('alloca', injected)
        self.assertNotIn('store ptr null', injected)

    def test_translated_codegen_merges_normal_and_resumed_results(self):
        generated = self.directory / 'generated.ll'
        self.run_command([OPTIONS.codegen_tool, str(generated)])
        ir = generated.read_text()
        self.assertIn('phi i32', ir)
        self.assertEqual(ir.count('phi i32'), 5)  # Unit does not receive a value phi.
        self.assertEqual(ir.count('indirectbr ptr'), 4)
        harness = self.directory / 'generated.cpp'
        harness.write_text('''
#include <cassert>
#include <climits>
extern "C" int comparison_eq(int, int);
extern "C" int comparison_gt(int, int);
extern "C" int comparison_ge(int, int);
extern "C" int comparison_lt(int, int);
extern "C" int comparison_le(int, int);
extern "C" int comparison_ne(int, int);
extern "C" int comparison_u_lt(int, int);
extern "C" int comparison_u_le(int, int);
extern "C" int comparison_u_gt(int, int);
extern "C" int comparison_u_ge(int, int);
extern "C" int value_frame(void**, int, int);
extern "C" int two_point_frame(void**, int, int, int);
extern "C" int unit_frame(void**, int, int);
extern "C" int position_state();
extern "C" int initializer_switch_dispatch(int);
extern "C" int branching_value(int, int, int, int*);
extern "C" int branching_effect(int, int*);
extern "C" int branching_terminated_effect(int, int*);
extern "C" int branching_returned_effect(int, int*);
extern "C" int terminated_normal_frame(void**, int, int);
int main() {
    const int values[] = {INT_MIN, -1, 0, 1, INT_MAX};
    for (int left : values) {
        for (int right : values) {
            assert(comparison_eq(left, right) == (left == right));
            assert(comparison_gt(left, right) == (left > right));
            assert(comparison_ge(left, right) == (left >= right));
            assert(comparison_lt(left, right) == (left < right));
            assert(comparison_le(left, right) == (left <= right));
            assert(comparison_ne(left, right) == (left != right));
            assert(comparison_u_lt(left, right) == (static_cast<unsigned>(left) < static_cast<unsigned>(right)));
            assert(comparison_u_le(left, right) == (static_cast<unsigned>(left) <= static_cast<unsigned>(right)));
            assert(comparison_u_gt(left, right) == (static_cast<unsigned>(left) > static_cast<unsigned>(right)));
            assert(comparison_u_ge(left, right) == (static_cast<unsigned>(left) >= static_cast<unsigned>(right)));
        }
    }
    assert(position_state() == 22);
    assert(initializer_switch_dispatch(0) == 13);
    assert(initializer_switch_dispatch(1) == 17);
    assert(initializer_switch_dispatch(2) == 19);
    int effects = 0;
    assert(branching_value(1, 31, 79, &effects) == 31 && effects == 0);
    assert(branching_value(0, 31, 79, &effects) == 79 && effects == 1);
    assert(branching_effect(0, &effects) == 9 && effects == 1);
    assert(branching_effect(1, &effects) == 9 && effects == 17);
    effects = 0;
    assert(branching_terminated_effect(0, &effects) == 9 && effects == 0);
    assert(branching_terminated_effect(1, &effects) == 29 && effects == 17);
    effects = 0;
    assert(branching_returned_effect(0, &effects) == 9 && effects == 0);
    assert(branching_returned_effect(1, &effects) == 29 && effects == 17);
    void* terminated_label = nullptr;
    assert(terminated_normal_frame(&terminated_label, 31, 79) == 13 && terminated_label);
    assert(terminated_normal_frame(&terminated_label, 31, 79) == 79);
    void* two_point_label = nullptr;
    assert(two_point_frame(&two_point_label, 11, 22, 33) == 11);
    assert(two_point_label);
    void* first_resume = two_point_label;
    assert(two_point_frame(&two_point_label, 11, 22, 33) == 22);
    assert(two_point_label && two_point_label != first_resume);
    void* second_resume = two_point_label;
    assert(two_point_frame(&two_point_label, 11, 22, 33) == 33);
    assert(two_point_label == second_resume);
    void* value_label = nullptr;
    assert(value_frame(&value_label, 31, 79) == 31 && value_label);
    assert(value_frame(&value_label, 31, 79) == 79);
    void* unit_label = nullptr;
    assert(unit_frame(&unit_label, 31, 79) == 0 && unit_label);
    assert(unit_frame(&unit_label, 31, 79) == 0);
}
''')
        executable = self.directory / 'generated'
        self.run_command([OPTIONS.compiler, '-std=c++20', '-UNDEBUG', '-fsanitize=address',
                          str(harness), str(generated), '-o', str(executable)])
        self.run_command([str(executable)])

    def test_multiple_functions_use_function_local_addresses(self):
        second = MODULE[MODULE.index('define ptr'):].replace('frame with spaces', 'second frame')
        injected = self.transform(MODULE + second)
        self.assertEqual(injected.count('indirectbr ptr'), 2)
        self.assertIn('blockaddress(@"frame with spaces", %resume)', injected)
        self.assertIn('blockaddress(@"second frame", %resume)', injected)

    def test_empty_resume_list_retains_kotlin_indirect_branch(self):
        source = MODULE.replace('  call void @__kxs_suspend_point(i32 9, ptr %label_field, ptr blockaddress(@"frame with spaces", %resume))\n', '')
        injected = self.transform(source)
        self.assertIn('indirectbr ptr %kxs_saved_label, []', injected)
        self.assertNotIn('unreachable', injected)

    def test_resume_points_follow_block_addresses_not_integer_marker_order(self):
        source = MODULE.replace('@"frame with spaces"', '@run_frame').replace(
            'resume:\n  ret ptr %result',
            'resume:\n  call void @__kxs_suspend_point(i32 3, ptr %label_field, ptr blockaddress(@run_frame, %second_resume))\n'
            '  ret ptr inttoptr (i64 2 to ptr)\nsecond_resume:\n  ret ptr %result')
        injected = self.transform(source)
        self.assertIn('indirectbr ptr %kxs_saved_label, [label %resume, label %second_resume]', injected)
        harness = self.directory / 'two-points.cpp'
        harness.write_text('''
#include <cassert>
struct Frame { void* guard; void* label; int before; };
extern "C" void* run_frame(Frame*, void*);
int main() {
    int result = 42;
    Frame frame{&result, nullptr, 0};
    assert(run_frame(&frame, nullptr) == reinterpret_cast<void*>(1));
    void* first_label = frame.label;
    assert(first_label && frame.before == 1);
    assert(run_frame(&frame, nullptr) == reinterpret_cast<void*>(2));
    assert(frame.label && frame.label != first_label && frame.before == 1);
    assert(run_frame(&frame, &result) == &result);
    assert(frame.before == 1 && frame.guard == &result);
}
''')
        executable = self.directory / 'two-points'
        self.run_command([OPTIONS.compiler, '-std=c++20', '-UNDEBUG', '-fsanitize=address',
                          str(harness), str(self.directory / 'output.ll'), '-o', str(executable)])
        self.run_command([str(executable)])

    def test_stack_label_is_rejected(self):
        source = MODULE.replace('%label_field = getelementptr { ptr, ptr, i32 }, ptr %frame, i32 0, i32 1',
                                '%label_field = alloca ptr')
        self.assertIn('persistent frame storage', self.transform(source, False))

    def test_missing_frame_marker_is_rejected(self):
        self.assertIn('Missing __kxs_coroutine_begin', self.transform(
            MODULE.replace('  call void @__kxs_coroutine_begin(ptr %label_field)\n', ''), False))

    def test_different_frame_field_is_rejected(self):
        source = MODULE.replace('i32 9, ptr %label_field,', 'i32 9, ptr %frame,')
        self.assertIn('different frame label field', self.transform(source, False))

    def test_foreign_resume_address_is_rejected(self):
        source = MODULE.replace('ptr blockaddress(@"frame with spaces", %resume)', 'ptr null')
        self.assertIn('function-local blockaddress', self.transform(source, False))

    def test_indirect_marker_use_is_rejected(self):
        source = '@callback = global ptr @__kxs_suspend_point\n' + MODULE
        self.assertIn('Unsupported use', self.transform(source, False))

    def test_persistent_resume_across_calls_and_independent_frames(self):
        injected = self.transform(MODULE.replace('@"frame with spaces"', '@run_frame'))
        harness = self.directory / 'handoff.cpp'
        harness.write_text('''
#include <cassert>
#include <cstdint>
struct Frame { void* guard; void* label; int before; };
extern "C" void* run_frame(Frame*, void*);
int main() {
    int first = 17, second = 29;
    Frame a{&first, nullptr, 0}, b{&second, nullptr, 0};
    assert(run_frame(&a, nullptr) == reinterpret_cast<void*>(1));
    assert(a.label != nullptr && a.before == 1 && a.guard == &first);
    assert(run_frame(&b, nullptr) == reinterpret_cast<void*>(1));
    assert(b.label != nullptr && b.before == 1 && b.guard == &second);
    volatile int stack_noise[4096] = {};
    stack_noise[1024] = 41;
    assert(run_frame(&a, &first) == &first);
    assert(a.before == 1 && a.guard == &first);
    assert(run_frame(&b, &second) == &second);
    assert(b.before == 1 && b.guard == &second);
    return stack_noise[1024] == 41 ? 0 : 1;
}
''')
        executable = self.directory / 'handoff'
        self.run_command([OPTIONS.compiler, '-std=c++20', '-UNDEBUG', '-fsanitize=address',
                          str(harness), str(self.directory / 'output.ll'), '-o', str(executable)])
        self.run_command([str(executable)])

    def test_native_compiler_reads_lifetime_intrinsics(self):
        source = '''
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
define i32 @lifetime_value() {
  %slot = alloca i32
  call void @llvm.lifetime.start.p0(i64 4, ptr %slot)
  store i32 37, ptr %slot
  %value = load i32, ptr %slot
  call void @llvm.lifetime.end.p0(i64 4, ptr %slot)
  ret i32 %value
}
'''
        injected = self.transform(source)
        harness = self.directory / 'lifetime.cpp'
        harness.write_text('extern "C" int lifetime_value();\n'
                           'int main() { return lifetime_value() == 37 ? 0 : 1; }\n')
        executable = self.directory / 'lifetime'
        self.run_command([OPTIONS.compiler, '-O2', '-fsanitize=address', str(harness),
                          str(self.directory / 'output.ll'), '-o', str(executable)])
        self.run_command([str(executable)])

    def test_native_compiler_reads_exact_floating_point_constants(self):
        source = '''
@boundary_value = constant double 0x42012E0BE8200000
@negative_zero = constant double 0x8000000000000000
@infinity_value = constant double 0x7FF0000000000000
@nan_value = constant double 0x7FF8000000000042
@signaling_value = constant double 0x7FF0000000000042
@single_value = constant float 0x3FF0000020000000
@single_signaling = constant float 0x7FF0000020000000
@half_value = constant half 0xH0001
@bfloat_value = constant bfloat 0xR8000
@aggregate = constant { double, float } { double 0x42012E0BE8200000, float 0x3FF0000020000000 }
@vector_value = constant <2 x double> <double 0x42012E0BE8200000, double 0x7FF0000000000000>
@literal_text = constant [21 x i8] c"f0x42012E0BE8200000\\00x"
'''
        injected = self.transform(source)
        self.assertIn('c"f0x42012E0BE8200000', injected)
        harness = self.directory / 'float-bits.cpp'
        harness.write_text('''
#include <cassert>
#include <cstdint>
#include <cstring>
extern "C" {
extern const double boundary_value, negative_zero, infinity_value, nan_value, signaling_value;
extern const float single_value, single_signaling;
}
template<class T> std::uint64_t bits(const T& value) {
    std::uint64_t result = 0;
    std::memcpy(&result, &value, sizeof(value));
    return result;
}
int main() {
    assert(bits(boundary_value) == 0x42012E0BE8200000ULL);
    assert(bits(negative_zero) == 0x8000000000000000ULL);
    assert(bits(infinity_value) == 0x7FF0000000000000ULL);
    assert(bits(nan_value) == 0x7FF8000000000042ULL);
    assert(bits(signaling_value) == 0x7FF0000000000042ULL);
    assert(bits(single_value) == 0x3F800001);
    assert(bits(single_signaling) == 0x7F800001);
}
''')
        executable = self.directory / 'float-bits'
        self.run_command([OPTIONS.compiler, '-std=c++20', '-UNDEBUG', str(harness),
                          str(self.directory / 'output.ll'), '-o', str(executable)])
        self.run_command([str(executable)])

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
        after = self.directory / 'after'
        # Missing injection must not be rescued by a runtime no-op marker.
        self.run_command([OPTIONS.compiler, *flags, *includes, str(source_file), *libraries,
                          '-o', str(self.directory / 'untransformed')], success=False)
        self.run_command([OPTIONS.compiler, str(cleaned_file), *libraries, '-o', str(after)])
        output = self.run_command([str(after)])
        self.assertIn('test_independent_frames...', output)
        self.assertIn('test_resumed_exception_stops_continuation...', output)



def main():
    global OPTIONS
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tool', required=True)
    parser.add_argument('--codegen-tool', required=True)
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
