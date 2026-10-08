// NOTE(port): Ordinary C++ executable entry for the source job tests.
namespace kotlinx::coroutines {
void run_job_impl_initialization_tests();
}

int main() {
    kotlinx::coroutines::run_job_impl_initialization_tests();
}
