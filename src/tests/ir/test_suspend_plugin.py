"""Execute generated entries and verify values and lifetimes across suspension."""
import argparse
from pathlib import Path
import shlex
import subprocess


def run(command, directory, name):
    result = subprocess.run(command, cwd=directory, capture_output=True, text=True)
    (directory / (name + '.log')).write_text(result.stdout + result.stderr)
    if result.returncode:
        raise RuntimeError(f'{name} exited {result.returncode}: {result.stderr}')
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--plugin', type=Path, required=True)
    parser.add_argument('--ir-plugin', type=Path, required=True)
    parser.add_argument('--plugin-compiler', required=True)
    parser.add_argument('--compiler', required=True)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--library-link-options', default='')
    parser.add_argument('--work-dir', type=Path, required=True)
    args = parser.parse_args()
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    # Prefix frame parsing must retain the full compiler's warning policy.
    # These internal functions are used after the authored suspend definition;
    # the negative variant adds a genuinely unused function before that point.
    warning_source = work / 'prefix_warnings.cpp'
    warning_text = '''#include <kotlinx/coroutines/ContinuationImpl.hpp>
#include <kotlinx/coroutines/dsl/Suspend.hpp>
#include <kotlinx/coroutines/Job.hpp>
using namespace kotlinx::coroutines;
namespace {
void require(bool value) { if (!value) throw 1; }
[[clang::annotate("suspend")]]
void* authored(Job& job, std::shared_ptr<Continuation<void*>> completion) {
    job.join();
    require(true);
    return nullptr;
}
}
void call(Job& job, std::shared_ptr<Continuation<void*>> completion) {
    require(true);
    authored(job, std::move(completion));
}
'''
    warning_command = [args.compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
        '-Wno-unused-parameter', '-I' + str(args.root / 'src'),
        '-Xclang', '-load', '-Xclang', str(args.plugin), '-Xclang', '-add-plugin',
        '-Xclang', 'kotlinx-suspend', '-fsyntax-only', str(warning_source)]
    warning_source.write_text(warning_text)
    run(warning_command, work, 'prefix-warnings-used')
    warning_source.write_text(warning_text.replace('void require(bool value)',
        'void genuinely_unused() {}\nvoid require(bool value)'))
    rejected_warning = subprocess.run(warning_command, capture_output=True, text=True)
    (work / 'prefix-warnings-unused.log').write_text(
        rejected_warning.stdout + rejected_warning.stderr)
    assert rejected_warning.returncode, 'Unused function must fail with -Werror'
    assert "unused function 'genuinely_unused'" in rejected_warning.stderr, rejected_warning.stderr
    assert '-Wunused-function' in rejected_warning.stderr, rejected_warning.stderr
    assert '.kxs.frontend.cpp:' not in rejected_warning.stderr, rejected_warning.stderr
    # Lowering must not import definitions or includes ahead of the host parser.
    late_include = work / 'late_include.cpp'
    late_include.write_text('''#include <kotlinx/coroutines/ContinuationImpl.hpp>
#include <kotlinx/coroutines/dsl/Suspend.hpp>
using namespace kotlinx::coroutines;
[[clang::annotate("suspend")]] void* external(std::shared_ptr<Continuation<void*>>);
namespace authoring::nested {
[[suspend]] void* lowered(std::shared_ptr<Continuation<void*>> completion) {
    auto result = external(completion);
    return result;
}
}
#include <kotlinx/coroutines/internal/DispatchedContinuation.hpp>
void assign(Result<void*>& a, const Result<void*>& b) { a = b; }
''')
    run([args.compiler, '-std=c++20', '-I' + str(args.root / 'src'),
         '-Xclang', '-load', '-Xclang', str(args.plugin), '-Xclang', '-add-plugin',
         '-Xclang', 'kotlinx-suspend', '-fsyntax-only', str(late_include)],
        work, 'late-include-scope')
    rejected = subprocess.run([args.compiler, '-std=c++23',
        '-Xclang', '-load', '-Xclang', str(args.plugin), '-Xclang', '-add-plugin',
        '-Xclang', 'kotlinx-suspend', '-fsyntax-only',
        str(args.root / 'src/tests/ir/fixtures/suspend_default_rejected.cpp')],
        capture_output=True, text=True)
    (work / 'suspend-default-rejection.log').write_text(rejected.stdout + rejected.stderr)
    assert rejected.returncode and rejected.stderr.count(
        'error: suspend function call in default parameter value is unsupported') == 2, rejected.stderr
    assert rejected.stderr.count(
        'error: suspend function can only be called from a coroutine or another suspend function') == 2, rejected.stderr
    nonlocal_rejected = subprocess.run([args.compiler, '-std=c++23',
        '-Xclang', '-load', '-Xclang', str(args.plugin), '-Xclang', '-add-plugin',
        '-Xclang', 'kotlinx-suspend', '-fsyntax-only',
        str(args.root / 'src/tests/ir/fixtures/suspend_nonlocal_rejected.cpp')],
        capture_output=True, text=True)
    (work / 'suspend-nonlocal-rejection.log').write_text(nonlocal_rejected.stdout + nonlocal_rejected.stderr)
    assert nonlocal_rejected.returncode and nonlocal_rejected.stderr.count(
        'error: suspension functions can only be called within coroutine body') == 3, nonlocal_rejected.stderr
    run([args.compiler, '-std=c++23', '-I' + str(args.root / 'src'),
         '-Xclang', '-load', '-Xclang', str(args.plugin), '-Xclang', '-add-plugin',
         '-Xclang', 'kotlinx-suspend', '-fsyntax-only',
         str(args.root / 'src/tests/ir/fixtures/suspend_default_callable.cpp')],
        work, 'suspend-default-callable')
    (work / 'api.hpp').write_text('''#pragma once
#include <kotlinx/coroutines/ContinuationImpl.hpp>
#include <kotlinx/coroutines/dsl/Suspend.hpp>
using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::dsl;
[[clang::annotate("suspend")]] void* external_call(int value, std::shared_ptr<Continuation<void*>> completion);
void* direct_value(int value, std::shared_ptr<Continuation<void*>> completion);
void* tail_value(int value, std::shared_ptr<Continuation<void*>> completion);
void* branch_tail_value(int value, std::shared_ptr<Continuation<void*>> completion);
void* conditional_tail_value(int value, std::shared_ptr<Continuation<void*>> completion);
[[clang::annotate("suspend"), clang::annotate("kxs_implicit_continuation")]]
__attribute__((error("implicit_call requires suspend lowering"))) void* implicit_call(int value = 41);
void* implicit_call(int value, std::shared_ptr<Continuation<void*>> completion);
void* implicit_tail_value(int value, std::shared_ptr<Continuation<void*>> caller);
void* implicit_branch_value(int value, std::shared_ptr<Continuation<void*>> caller);
void* default_tail_value(int value, std::shared_ptr<Continuation<void*>> caller);
void* buffer_value(const int (&values)[2], std::shared_ptr<Continuation<void*>> completion) noexcept;
''')
    (work / 'input.cpp').write_text('''#include "api.hpp"
[[suspend]] void* direct_value(int value, std::shared_ptr<Continuation<void*>> completion) {
    (void)completion;
    return new int(value + 1);
}
[[suspend]] void* tail_value(int value, std::shared_ptr<Continuation<void*>> completion) {
    return suspend(external_call(value, completion));
}
[[suspend]] void* branch_tail_value(int value, std::shared_ptr<Continuation<void*>> completion) {
    if (value) return suspend(external_call(41, completion));
    return suspend(external_call(41, completion));
}
[[suspend]] void* conditional_tail_value(int value, std::shared_ptr<Continuation<void*>> completion) {
    return value ? suspend(external_call(41, completion)) : suspend(external_call(41, completion));
}
[[suspend]] void* implicit_tail_value(int value, std::shared_ptr<Continuation<void*>> caller) {
    return implicit_call(value);
}
[[suspend]] void* default_tail_value(int value, std::shared_ptr<Continuation<void*>> caller) {
    return implicit_call();
}
[[suspend]] void* implicit_branch_value(int value, std::shared_ptr<Continuation<void*>> caller) {
    if (value) return implicit_call(41);
    return implicit_call(41);
}
[[suspend]] void* buffer_value(const int (&values)[2], std::shared_ptr<Continuation<void*>> completion) noexcept {
    (void)completion;
    return new int(values[0] + values[1]);
}
''')
    extraction = [args.plugin_compiler, '-std=c++20', '-fsyntax-only', '-I' + str(args.root / 'src'),
                  '-Xclang', '-load', '-Xclang', str(args.plugin),
                  '-Xclang', '-add-plugin', '-Xclang', 'kotlinx-suspend',
                  '-Xclang', '-plugin-arg-kotlinx-suspend', '-Xclang', 'out-dir=' + str(work),
                  str(work / 'input.cpp')]
    run(extraction, work, 'extraction')
    generated = work / 'input.kx.cpp'
    emitted = generated.read_text()
    assert emitted.count('Direct continuation ABI entry') == 8, emitted
    assert '__kxs_frame_' not in emitted, 'Tail-only branches must not allocate a retained frame'
    assert '__kxs_coroutine_' not in emitted, 'Tail/direct functions must not allocate a frame'
    (work / 'main.cpp').write_text('''#include "api.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
struct Done : Continuation<void*> {
    int calls = 0;
    int value = 0;
    bool failed = false;
    std::shared_ptr<CoroutineContext> get_context() const override { return EmptyCoroutineContext::instance(); }
    void resume_with(Result<void*> result) override {
        ++calls;
        try { auto box = std::unique_ptr<int>(static_cast<int*>(result.get_or_throw())); value = *box; }
        catch (const std::runtime_error&) { failed = true; }
    }
};
int mode = 0;
int external_calls = 0;
std::shared_ptr<Continuation<void*>> pending;
std::weak_ptr<Continuation<void*>> handed_off;
void* external_call(int value, std::shared_ptr<Continuation<void*>> completion) {
    ++external_calls;
    handed_off = completion;
    if (mode == 1 || mode == 2) {
        pending = std::move(completion);
        return intrinsics::get_COROUTINE_SUSPENDED();
    }
    if (mode == 3) throw std::runtime_error("external failure");
    return new int(value + 1);
}
void* implicit_call(int value, std::shared_ptr<Continuation<void*>> completion) {
    return external_call(value, std::move(completion));
}
int main() {
    auto direct_done = std::make_shared<Done>();
    auto direct = std::unique_ptr<int>(static_cast<int*>(direct_value(41, direct_done)));
    assert(*direct == 42 && direct_done->calls == 0 && external_calls == 0);
    std::cout << "direct:42\\n";
    const int buffer[2] = {41, 1};
    auto buffered = std::unique_ptr<int>(static_cast<int*>(buffer_value(buffer, direct_done)));
    assert(*buffered == 42 && buffer[0] == 41 && buffer[1] == 1 && direct_done->calls == 0);
    for (auto entry : {&tail_value, &branch_tail_value, &conditional_tail_value, &implicit_tail_value, &implicit_branch_value, &default_tail_value}) {
    for (mode = 0; mode != 4; ++mode) {
        auto done = std::make_shared<Done>();
        try {
            auto result = entry(entry == &tail_value || entry == &implicit_tail_value ? 41 : mode % 2, done);
            assert(handed_off.lock() == done); // The caller continuation is forwarded exactly.
            if (intrinsics::is_coroutine_suspended(result)) {
                assert(done->calls == 0 && pending == done);
                auto held = std::move(pending);
                if (mode == 1) held->resume_with(Result<void*>::success(new int(42)));
                else held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("external failure"))));
            } else done->resume_with(Result<void*>::success(result));
        } catch (const std::runtime_error&) {
            done->resume_with(Result<void*>::failure(std::current_exception()));
        }
        assert(done->calls == 1 && done->failed == (mode >= 2));
        assert(done->failed || done->value == 42);
        std::cout << "tail:" << mode << ":" << (done->failed ? "failure" : "42") << "\\n";
        done.reset();
        assert(handed_off.expired() && !pending);
    }
    }
    assert(external_calls == 24);
}
''')
    executable = work / 'handoff'
    compile_command = [args.compiler, '-std=c++20', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                       '-Wno-unused-parameter', # Existing coroutine header build policy.
                       '-I' + str(args.root / 'src'), '-include', str(work / 'api.hpp'),
                       str(generated), str(work / 'main.cpp'), str(args.library), '-pthread',
                       *shlex.split(args.library_link_options), '-o', str(executable)]
    run(compile_command, work, 'generated-compile')
    trace = run([str(executable)], work, 'handoff-run')
    expected = 'direct:42\n' + 'tail:0:42\ntail:1:42\ntail:2:failure\ntail:3:failure\n' * 6
    assert trace == expected, trace
    (work / 'trace.txt').write_text(trace)
    ordinary = work / 'direct-in-process'
    ordinary_command = compile_command.copy()
    ordinary_command[ordinary_command.index(str(generated))] = str(work / 'input.cpp')
    ordinary_command[-1] = str(ordinary)
    ordinary_command[1:1] = ['-Xclang', '-load', '-Xclang', str(args.plugin),
        '-Xclang', '-add-plugin', '-Xclang', 'kotlinx-suspend',
        '-fpass-plugin=' + str(args.ir_plugin)]
    run(ordinary_command, work, 'direct-in-process-compile')
    assert run([str(ordinary)], work, 'direct-in-process-run') == expected
    unit_executable = work / 'unit-tail'
    run([args.compiler, '-std=c++20', '-I' + str(args.root / 'src'),
         '-Xclang', '-load', '-Xclang', str(args.plugin), '-Xclang', '-add-plugin',
         '-Xclang', 'kotlinx-suspend', '-fpass-plugin=' + str(args.ir_plugin),
         '-O1', '-UNDEBUG', '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
         str(args.root / 'src/tests/ir/fixtures/unit_tail.cpp'), str(args.library),
         '-pthread', *shlex.split(args.library_link_options), '-o', str(unit_executable)],
        work, 'unit-tail-compile')
    assert run([str(unit_executable)], work, 'unit-tail-run') == ''
    for name in ('factory', 'owned', 'borrowed'):
        source = args.root / 'src/tests/ir/fixtures/default_arguments' / (name + '.cpp')
        executable = work / ('default-' + name)
        command = [args.compiler, '-std=c++20', '-I' + str(args.root / 'src'),
            '-Xclang', '-load', '-Xclang', str(args.plugin), '-Xclang', '-add-plugin',
            '-Xclang', 'kotlinx-suspend', '-fpass-plugin=' + str(args.ir_plugin),
            '-O1', '-UNDEBUG', '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
            str(source), str(args.library), '-pthread', *shlex.split(args.library_link_options),
            '-o', str(executable)]
        run(command, work, 'default-' + name + '-compile')
        assert run([str(executable)], work, 'default-' + name + '-run') == 'direct implicit continuation:42\n'
    slicing_executable = work / 'expression-slicing'
    run([args.compiler, '-std=c++20', '-I' + str(args.root / 'src'),
         '-Xclang', '-load', '-Xclang', str(args.plugin), '-Xclang', '-add-plugin',
         '-Xclang', 'kotlinx-suspend', '-fpass-plugin=' + str(args.ir_plugin),
         '-O1', '-UNDEBUG', '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
         str(args.root / 'src/tests/ir/fixtures/expression_slicing.cpp'), str(args.library),
         '-pthread', *shlex.split(args.library_link_options), '-o', str(slicing_executable)],
        work, 'expression-slicing-compile')
    assert run([str(slicing_executable)], work, 'expression-slicing-run') == 'slicing:94; immutable reads; mutable snapshot; volatile load; effects once; resumed failure; trailing effects:44 in order\n'
    restricted_executable = work / 'restricted-receiver'
    run([args.compiler, '-std=c++20', '-I' + str(args.root / 'src'),
         '-Xclang', '-load', '-Xclang', str(args.plugin), '-Xclang', '-add-plugin',
         '-Xclang', 'kotlinx-suspend', '-fpass-plugin=' + str(args.ir_plugin),
         '-O1', '-UNDEBUG', '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
         str(args.root / 'src/tests/ir/fixtures/restricted_receiver.cpp'), str(args.library),
         '-pthread', *shlex.split(args.library_link_options), '-o', str(restricted_executable)],
        work, 'restricted-receiver-compile')
    assert run([str(restricted_executable)], work, 'restricted-receiver-run') == 'restricted receiver:42; ordinary argument:43\n'
    lambda_directory = work / 'lambda-reference'
    lambda_directory.mkdir(exist_ok=True)
    (lambda_directory / 'input.cpp').write_text((args.root / 'src/tests/ir/fixtures/suspend_lambda.cpp').read_text())
    (lambda_directory / 'nested.cpp').write_text((args.root / 'src/tests/ir/fixtures/nested_suspend_lambda.cpp').read_text())
    (lambda_directory / 'CMakeLists.txt').write_text(f'''cmake_minimum_required(VERSION 3.19)
project(suspend_lambda LANGUAGES CXX)
set(KXS_LLVM_PASS_PLUGIN "{args.ir_plugin}")
set(KXS_CLANG_SUSPEND_PLUGIN "{args.plugin}")
include("{args.root}/cmake/Modules/KotlinxCoroutines.cmake")
kxs_add_executable(handoff SOURCES input.cpp)
kxs_add_executable(nested_handoff SOURCES nested.cpp)
foreach(target handoff nested_handoff)
  target_compile_features(${{target}} PRIVATE cxx_std_23)
  target_link_libraries(${{target}} PRIVATE "{args.library}" pthread)
  target_compile_options(${{target}} PRIVATE -O1 -UNDEBUG -fsanitize=address,undefined -fno-sanitize-recover=all)
  target_link_options(${{target}} PRIVATE -fsanitize=address,undefined)
endforeach()
''')
    lambda_build = lambda_directory / 'build'
    run(['cmake', '-S', str(lambda_directory), '-B', str(lambda_build),
         '-DCMAKE_CXX_COMPILER=' + args.compiler, '-DCMAKE_BUILD_TYPE=Release',
         '-DCMAKE_CXX_FLAGS=' + args.library_link_options], lambda_directory, 'cmake-configure')
    run(['cmake', '--build', str(lambda_build), '-j2'], lambda_directory, 'ordinary-compile')
    assert run([str(lambda_build / 'handoff')], lambda_directory, 'handoff-run') == 'suspend lambda:42; ordinary:43; captured this:82\n'
    assert run([str(lambda_build / 'nested_handoff')], lambda_directory, 'nested-handoff-run') == 'nested lambda:43; outer failure; inner failure; captures released:4; captured this:82; shadowed captures:170; copied this:43; copied const this:42; arrays:63; object arrays:44; array cleanup\n'
    retained = work / 'retained-locals'
    retained.mkdir(exist_ok=True)
    fixtures = args.root / 'src/tests/ir/fixtures/retained_locals'
    for filename in ('api.hpp', 'input.cpp', 'main.cpp'):
        (retained / filename).write_text((fixtures / filename).read_text())
    retained_extraction = extraction.copy()
    retained_extraction[-2] = 'out-dir=' + str(retained)
    retained_extraction[-1] = str(retained / 'input.cpp')
    run(retained_extraction, retained, 'extraction')
    (retained / 'CMakeLists.txt').write_text(f'''cmake_minimum_required(VERSION 3.19)
project(retained_locals LANGUAGES CXX)
set(KXS_LLVM_PASS_PLUGIN "{args.ir_plugin}")
set(KXS_CLANG_SUSPEND_PLUGIN "{args.plugin}")
include("{args.root}/cmake/Modules/KotlinxCoroutines.cmake")
kxs_add_library(retained_frames SOURCES input.cpp)
kxs_add_executable(handoff SOURCES main.cpp)
option(KXS_TEST_FORCED_INCLUDE "Exercise command-line header inclusion" OFF)
if(KXS_TEST_FORCED_INCLUDE)
  target_compile_options(retained_frames PRIVATE -include "${{CMAKE_CURRENT_SOURCE_DIR}}/api.hpp")
  target_compile_options(handoff PRIVATE -include "${{CMAKE_CURRENT_SOURCE_DIR}}/api.hpp")
endif()
target_link_libraries(handoff PRIVATE retained_frames "{args.library}" pthread)
target_compile_options(retained_frames PRIVATE -O2 -UNDEBUG -Wno-gnu-label-as-value -fsanitize=address,undefined -fno-sanitize-recover=all)
target_compile_options(handoff PRIVATE -O2 -UNDEBUG -Wno-gnu-label-as-value -fsanitize=address,undefined -fno-sanitize-recover=all)
target_link_options(handoff PRIVATE -fsanitize=address,undefined)
''')
    cmake_build = retained / 'ordinary compile'
    run(['cmake', '-S', str(retained), '-B', str(cmake_build),
         '-DCMAKE_CXX_COMPILER=' + args.compiler, '-DCMAKE_BUILD_TYPE=Release',
         '-DCMAKE_CXX_FLAGS=' + args.library_link_options], retained, 'cmake-configure')
    run(['cmake', '--build', str(cmake_build), '-j2'], retained, 'ordinary-compile')
    retained_trace = run([str(cmake_build / 'handoff')], retained, 'handoff-run')
    assert retained_trace == ('retained:0:530\nretained:1:530\n'
                              'retained:2:failure\nretained:3:failure\n'
                              'retained:4:failure\n'), retained_trace
    forced_build = retained / 'forced include compile'
    run(['cmake', '-S', str(retained), '-B', str(forced_build),
         '-DCMAKE_CXX_COMPILER=' + args.compiler, '-DCMAKE_BUILD_TYPE=Release',
         '-DCMAKE_CXX_FLAGS=' + args.library_link_options, '-DKXS_TEST_FORCED_INCLUDE=ON'],
        retained, 'forced-cmake-configure')
    run(['cmake', '--build', str(forced_build), '-j2'], retained, 'forced-compile')
    forced_trace = run([str(forced_build / 'handoff')], retained, 'forced-handoff-run')
    assert forced_trace == retained_trace, forced_trace
    for variant, directory in [('ordinary', cmake_build), ('forced', forced_build)]:
        termination = subprocess.run([str(directory / 'handoff'), 'noexcept-terminate'],
                                     capture_output=True, text=True)
        (retained / (variant + '-noexcept-termination.log')).write_text(
            f'exit={termination.returncode}\n' + termination.stdout + termination.stderr)
        assert termination.returncode == 86, termination.stderr
    outside = retained / 'yield_outside_suspend.cpp'
    outside.write_text('#include <kotlinx/coroutines/Yield.hpp>\nvoid ordinary() { kotlinx::coroutines::yield(); }\n')
    for frontend in (False, True):
        object_file = retained / ('yield-outside-' + str(frontend) + '.o')
        object_file.unlink(missing_ok=True)
        command = [args.compiler, '-std=c++20', '-I' + str(args.root / 'src')]
        if frontend:
            command += ['-Xclang', '-load', '-Xclang', str(args.plugin), '-Xclang', '-add-plugin',
                        '-Xclang', 'kotlinx-suspend']
        rejected = subprocess.run(command + ['-c', str(outside), '-o', str(object_file)],
                                  capture_output=True, text=True)
        (retained / ('yield-outside-' + str(frontend) + '.log')).write_text(rejected.stdout + rejected.stderr)
        assert rejected.returncode and 'yield() requires suspend lowering' in rejected.stderr, rejected.stderr
        assert not object_file.exists(), 'An unlowered yield intrinsic must not produce an object'
    unsupported = {
        'variable_array': ('', '', 'int local[value];',
                          'retained arrays require a constant complete array type'),
    }
    for name, (prefix, suffix, local, diagnostic) in unsupported.items():
        invalid = retained / (name + '.cpp')
        invalid.write_text('#include "api.hpp"\n' + prefix + '''
[[suspend]] void* unsupported_value(int value, std::shared_ptr<Continuation<void*>> completion) {
''' + local + '''
    void* result = suspend(external_call(value, CallToken(value), completion));
    return result;
}
''' + suffix)
        object_file = retained / (name + '.o')
        object_file.unlink(missing_ok=True)
        rejected = subprocess.run([args.compiler, '-std=c++20', '-I' + str(args.root / 'src'),
            '-Xclang', '-load', '-Xclang', str(args.plugin), '-Xclang', '-add-plugin',
            '-Xclang', 'kotlinx-suspend', '-c', str(invalid), '-o', str(object_file)],
            capture_output=True, text=True)
        (retained / (name + '-rejection.log')).write_text(rejected.stdout + rejected.stderr)
        assert rejected.returncode and diagnostic in rejected.stderr, rejected.stderr
        assert not object_file.exists(), 'Unsupported suspend code must not produce an object'
    print('Executed direct/tail and retained-local regressions through kxs_add_library and kxs_add_executable in ordinary and forced-include builds, including noexcept termination, reference and temporary ownership, callback construction, plain yield calls, repeated suspension and cleanup')


if __name__ == '__main__':
    main()
