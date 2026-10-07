if(NOT DEFINED AST_DISTANCE OR NOT DEFINED TEST_DIR)
    message(FATAL_ERROR "AST_DISTANCE and TEST_DIR are required")
endif()
file(REMOVE_RECURSE "${TEST_DIR}")
file(MAKE_DIRECTORY "${TEST_DIR}/source" "${TEST_DIR}/target" "${TEST_DIR}/rust" "${TEST_DIR}/kotlin")
file(WRITE "${TEST_DIR}/source/Example.kt" "class Example {\n val count: Int = 1\n fun implemented(x: Int): Int = x\n fun declared(x: Int): Int = x\n fun missing(x: Int): Int = x\n}\nenum class Mode { START, STOP }\ntypealias Alias = Int\nconst val LIMIT: Int = 5\n")
file(WRITE "${TEST_DIR}/source/Absent.kt" "class Absent\n")
file(WRITE "${TEST_DIR}/source/Lexical.kt" "context() fun broken() {}\n")
file(WRITE "${TEST_DIR}/target/Lexical.hpp" "// Transliterated from: Lexical.kt\n")
file(WRITE "${TEST_DIR}/target/Example.hpp" "// Transliterated from: Example.kt\nclass Example { public: int count = 1; int implemented(int x) { return x; } int declared(int x); };\nenum class Mode { START };\nusing Alias = int;\nconstexpr int LIMIT = 5;\n")
file(WRITE "${TEST_DIR}/target/Example.cpp" "// Implementation is inline in the companion header.\n#include \"Example.hpp\"\n")
file(WRITE "${TEST_DIR}/source/Flow.kt" "abstract class Flow<T> {\n abstract fun collect(value: T)\n}\nabstract class AbstractFlow<T> : Flow<T>() {\n override fun collect(value: T) { emit(value) }\n abstract fun collectSafely(value: T)\n}\n")
file(WRITE "${TEST_DIR}/target/Flow.hpp" [=[// Transliterated from: Flow.kt
template<class T> class Flow { public: virtual void collect(T value) = 0; };
template<class T> class AbstractFlow : public Flow<T> { public:
 void collect(T value) override;
 virtual void collect_safely(T value) = 0;
};
template<class T> void AbstractFlow<T>::collect(T value) {
  coroutine_begin(this)
  emit(value);
  coroutine_end(this)
  return;
}
]=])
execute_process(COMMAND "${AST_DISTANCE}" --deep "${TEST_DIR}/source" kotlin "${TEST_DIR}/target" cpp
    WORKING_DIRECTORY "${TEST_DIR}" OUTPUT_FILE "${TEST_DIR}/deep.txt" ERROR_FILE "${TEST_DIR}/deep.stderr" RESULT_VARIABLE status)
file(READ "${TEST_DIR}/deep.txt" report)
file(READ "${TEST_DIR}/deep.stderr" errors)
if(NOT status EQUAL 0)
    message(FATAL_ERROR "Deep comparison failed: ${status}: ${report}: ${errors}")
endif()
foreach(expected "MISSING_FILE Absent.kt" "MISSING_SYMBOL type Absent" "MISSING_SYMBOL function Example::missing" "DECLARATION_ONLY function Example::declared" "PRESENT property Example::count" "PRESENT property LIMIT" "PRESENT type_alias Alias" "MISSING_SYMBOL enum_variant Mode::STOP" "PRESENT function AbstractFlow::collect" "DECLARATION_ONLY function AbstractFlow::collectSafely" "DECLARATION_ONLY function Flow::collect")
    string(FIND "${report}" "${expected}" found)
    if(found LESS 0)
        message(FATAL_ERROR "Missing inventory evidence ${expected}: ${report}")
    endif()
endforeach()
if(report MATCHES "missing types: Alias")
    message(FATAL_ERROR "Type alias falsely reported missing: ${report}")
endif()
if(report MATCHES "Example.*\\[STUB\\]")
    message(FATAL_ERROR "Inline companion implementation misclassified as stub: ${report}")
endif()
file(READ "${TEST_DIR}/deep_symbol_inventory.txt" saved)
if(NOT saved MATCHES "grammar treated hard keyword as identifier: fun")
    message(FATAL_ERROR "Malformed declaration lost its lexical diagnostic: ${saved}")
endif()
file(READ "${TEST_DIR}/deep_transliteration_evidence.txt" lexical_receipt)
if(NOT lexical_receipt MATCHES "grammar treated hard keyword as identifier: fun" OR
   NOT lexical_receipt MATCHES "Source grammar errors; emitted source is provisional")
    message(FATAL_ERROR "Emission lost the malformed declaration diagnostic: ${lexical_receipt}")
endif()
if(NOT saved MATCHES "MISSING_SYMBOL function Example::missing")
    message(FATAL_ERROR "Deep inventory was not persisted")
endif()
execute_process(COMMAND "${AST_DISTANCE}" --deep "${TEST_DIR}/source/Example.kt" kotlin "${TEST_DIR}/target/Example.cpp" cpp
    WORKING_DIRECTORY "${TEST_DIR}" OUTPUT_VARIABLE single ERROR_VARIABLE errors RESULT_VARIABLE status)
if(NOT status EQUAL 0 OR NOT single MATCHES "PRESENT function Example::implemented" OR NOT single MATCHES "PRESENT property LIMIT")
    message(FATAL_ERROR "Single C++ file lost its companion header: ${status}: ${single}: ${errors}")
endif()
# Keep primary and supplementary totals wired in Rust -> Kotlin reporting.
file(WRITE "${TEST_DIR}/rust/example.rs" "pub struct Example {}\npub const LIMIT: i32 = 5;\npub type Alias = i32;\n")
file(WRITE "${TEST_DIR}/kotlin/Example.kt" "class Example\nconst val LIMIT: Int = 5\ntypealias Alias = Int\n")
execute_process(COMMAND "${AST_DISTANCE}" --symbol-parity "${TEST_DIR}/rust" "${TEST_DIR}/kotlin"
    WORKING_DIRECTORY "${TEST_DIR}" OUTPUT_VARIABLE parity ERROR_VARIABLE errors RESULT_VARIABLE status)
if(NOT status EQUAL 0 OR NOT parity MATCHES "Production symbols [(]all[)]: 3/3" OR NOT parity MATCHES "Test symbols [(]all[)]: +0/0")
    message(FATAL_ERROR "Rust/Kotlin combined totals regression: ${status}: ${parity}: ${errors}")
endif()
execute_process(COMMAND "${AST_DISTANCE}" --deep "${TEST_DIR}/does-not-exist" kotlin "${TEST_DIR}/target" cpp
    WORKING_DIRECTORY "${TEST_DIR}" OUTPUT_VARIABLE output ERROR_VARIABLE errors RESULT_VARIABLE status)
if(status EQUAL 0 OR NOT errors MATCHES "Cannot open codebase root")
    message(FATAL_ERROR "Missing root silently reported complete: ${status}: ${output}: ${errors}")
endif()
message(STATUS "Deep missing files/API/functions/properties/enum/alias inventory and Rust/Kotlin totals verified")
# Suspension review is scoped to each callable, including owned frame bodies.
file(MAKE_DIRECTORY "${TEST_DIR}/review-source" "${TEST_DIR}/review-target")
file(WRITE "${TEST_DIR}/review-source/Review.kt" [=[suspend fun plainValue(): Int = awaitValue()
suspend fun markerValue(): Int { val value = awaitValue(); return value + 1 }
suspend fun tailValue(): Int = awaitValue()
suspend fun macroValue(): Int { awaitValue(); return 1 }
suspend fun gotoValue(): Int { awaitValue(); return 1 }
fun callbackPath(block: suspend (Int) -> Int): Flow<Int> = flow { emit(block(1)) }
fun ordinaryValue(): String = "suspend coroutine_yield"
]=])
file(WRITE "${TEST_DIR}/review-target/Review.hpp" [=[// Transliterated from: Review.kt
void* plain_value(Continuation<void*>* completion) {
  // coroutine_yield(this, fake()); goto *label;
  const char* words = "coroutine_yield coroutine_begin suspend __kxs_suspend_point";
  return nullptr;
}
void* marker_value(Continuation<void*>* completion) { dsl::suspend(await_value(completion)); return nullptr; }
void* tail_value(Continuation<void*>* completion) { return await_value(completion); }
void* macro_value(Continuation<void*>* completion) {
  class Frame { public:
    void* invoke_suspend(Result<void*> result) {
      coroutine_begin(this)
      coroutine_yield(this, await_value(this));
      coroutine_end(this)
    }
    void* _label = nullptr;
  };
  return run_frame(completion);
}
void* goto_value(Continuation<void*>* completion) { void* label = &&done; goto *label; done: return nullptr; }
void* callback_path(void* block) { return callback_helper(block); }
const char* ordinary_value() { return "suspend coroutine_yield"; }
]=])
file(WRITE "${TEST_DIR}/review-source/Supported.kt" "fun addValue(inputValue: Int): Int = inputValue + 1\n")
file(WRITE "${TEST_DIR}/review-target/Supported.hpp" "// Transliterated from: Supported.kt\nint add_value(int input_value) { return input_value + 1; }\n")
execute_process(COMMAND "${AST_DISTANCE}" --deep "${TEST_DIR}/review-source" kotlin "${TEST_DIR}/review-target" cpp
    WORKING_DIRECTORY "${TEST_DIR}" OUTPUT_VARIABLE review ERROR_VARIABLE errors RESULT_VARIABLE status)
file(WRITE "${TEST_DIR}/suspension-review.txt" "${review}\n${errors}")
if(NOT status EQUAL 0)
    message(FATAL_ERROR "Suspension review failed: ${review}: ${errors}")
endif()
foreach(expected "NO_LOCAL_LOWERING_REVIEW" "MARKER_ONLY_REVIEW" "TAIL_OR_HELPER_REVIEW" "LOWERING_SYNTAX_PRESENT" "suspend callback contract" "Explicit suspension contracts reviewed: 6" "Supported.kt\tSupported.hpp\t1.000000")
    string(FIND "${review}" "${expected}" found)
    if(found LESS 0)
        message(FATAL_ERROR "Missing suspension/literal deep evidence ${expected}: ${review}")
    endif()
endforeach()
file(READ "${TEST_DIR}/deep_transliteration_evidence.txt" receipt)
if(NOT receipt MATCHES "score_method: positional exact-token cosine" OR NOT receipt MATCHES "Suspension lowering review leads")
    message(FATAL_ERROR "Deep literal/suspension receipt missing: ${receipt}")
endif()
if(NOT review MATCHES "NO_LOCAL_LOWERING_REVIEW[^\n]*plain_value" OR
   NOT review MATCHES "MARKER_ONLY_REVIEW[^\n]*marker_value" OR
   NOT review MATCHES "TAIL_OR_HELPER_REVIEW[^\n]*tail_value" OR
   NOT review MATCHES "LOWERING_SYNTAX_PRESENT[^\n]*macro_value" OR
   NOT review MATCHES "LOWERING_SYNTAX_PRESENT[^\n]*goto_value")
    message(FATAL_ERROR "Wrong callable-scoped review classification: ${review}")
endif()
message(STATUS "Deep ordered literal score and callable-scoped suspension review passed")

# Kotlin enum instance APIs lower to C++ free functions carrying the actual enum.
file(MAKE_DIRECTORY "${TEST_DIR}/enum-source" "${TEST_DIR}/enum-target")
file(WRITE "${TEST_DIR}/enum-source/Mode.kt" [=[package demo
enum class Mode { DEFAULT, LAZY;
 fun invoke(value: Int): Int = value
 val isLazy: Boolean get() = this == LAZY
}
]=])
file(WRITE "${TEST_DIR}/enum-target/Mode.hpp" [=[// port-lint: source Mode.kt
namespace demo {
enum class Mode { DEFAULT, LAZY };
int invoke(Mode mode, int value) { return value; }
bool is_lazy(Mode mode) { return mode == Mode::LAZY; }
}
]=])
file(WRITE "${TEST_DIR}/enum-source/Wrong.kt" [=[package demo
enum class Wrong { DEFAULT; fun invoke(value: Int): Int = value; val isLazy: Boolean get() = true }
]=])
file(WRITE "${TEST_DIR}/enum-target/Wrong.hpp" [=[// port-lint: source Wrong.kt
namespace demo {
enum class Wrong { DEFAULT };
enum class Other { DEFAULT };
int invoke(Other mode, int value) { return value; }
bool is_lazy(Other mode) { return true; }
}
]=])
file(WRITE "${TEST_DIR}/enum-source/Plain.kt" "package demo
class Plain { fun invoke(value: Int): Int = value }
")
file(WRITE "${TEST_DIR}/enum-target/Plain.hpp" "// port-lint: source Plain.kt
namespace demo { class Plain {}; int invoke(Plain value, int input) { return input; } }
")
execute_process(COMMAND "${AST_DISTANCE}" --deep "${TEST_DIR}/enum-source" kotlin "${TEST_DIR}/enum-target" cpp
    WORKING_DIRECTORY "${TEST_DIR}" OUTPUT_VARIABLE enum_report ERROR_VARIABLE errors RESULT_VARIABLE status)
if(NOT status EQUAL 0 OR NOT enum_report MATCHES "PRESENT function Mode::invoke" OR
   NOT enum_report MATCHES "PRESENT property Mode::isLazy" OR
   NOT enum_report MATCHES "MISSING_SYMBOL function Wrong::invoke" OR
   NOT enum_report MATCHES "MISSING_SYMBOL property Wrong::isLazy" OR
   NOT enum_report MATCHES "MISSING_SYMBOL function Plain::invoke")
    message(FATAL_ERROR "Enum receiver projection lost presence or accepted an unrelated owner: ${enum_report}: ${errors}")
endif()
file(READ "${TEST_DIR}/port_status_report.md" enum_status)
if(NOT enum_status MATCHES "Function parity [|] 1/3 matched")
    message(FATAL_ERROR "Enum function name coverage is false: ${enum_status}")
endif()
