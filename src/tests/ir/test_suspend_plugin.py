"""Compile and execute direct/tail continuation entries emitted by the real plugin."""
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
    parser.add_argument('--plugin-compiler', required=True)
    parser.add_argument('--compiler', required=True)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--library-link-options', default='')
    parser.add_argument('--work-dir', type=Path, required=True)
    args = parser.parse_args()
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    (work / 'api.hpp').write_text('''#pragma once
#include <kotlinx/coroutines/ContinuationImpl.hpp>
#include <kotlinx/coroutines/dsl/Suspend.hpp>
using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::dsl;
[[clang::annotate("suspend")]] void* external_call(int value, std::shared_ptr<Continuation<void*>> completion);
void* direct_value(int value, std::shared_ptr<Continuation<void*>> completion);
void* tail_value(int value, std::shared_ptr<Continuation<void*>> completion);
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
    assert emitted.count('Direct continuation ABI entry') == 3, emitted
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
int main() {
    auto direct_done = std::make_shared<Done>();
    auto direct = std::unique_ptr<int>(static_cast<int*>(direct_value(41, direct_done)));
    assert(*direct == 42 && direct_done->calls == 0 && external_calls == 0);
    std::cout << "direct:42\\n";
    const int buffer[2] = {41, 1};
    auto buffered = std::unique_ptr<int>(static_cast<int*>(buffer_value(buffer, direct_done)));
    assert(*buffered == 42 && buffer[0] == 41 && buffer[1] == 1 && direct_done->calls == 0);
    for (mode = 0; mode != 4; ++mode) {
        auto done = std::make_shared<Done>();
        try {
            auto result = tail_value(41, done);
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
    assert(external_calls == 4);
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
    expected = 'direct:42\ntail:0:42\ntail:1:42\ntail:2:failure\ntail:3:failure\n'
    assert trace == expected, trace
    (work / 'trace.txt').write_text(trace)
    print('Real plugin extraction, generated compilation and five direct/tail handoff cases passed')


if __name__ == '__main__':
    main()
