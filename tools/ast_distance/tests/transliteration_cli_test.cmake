file(MAKE_DIRECTORY "${TEST_DIR}")
file(WRITE "${TEST_DIR}/source.kt" [=[/** Returns inputValue plus 50.
 * @param inputValue the value passed to [nextValue].
 */
fun nextValue(inputValue: Int): Int = inputValue + 50
]=])
execute_process(COMMAND "${AST_DISTANCE}" --transliterate "${TEST_DIR}/source.kt" kotlin cpp
    WORKING_DIRECTORY "${TEST_DIR}" OUTPUT_FILE "${TEST_DIR}/emitted.cpp" ERROR_VARIABLE diagnostics RESULT_VARIABLE result)
if(NOT result EQUAL 0 OR NOT diagnostics MATCHES "misses: 0" OR NOT diagnostics MATCHES "function_declaration")
    message(FATAL_ERROR "Emission failed: ${result}: ${diagnostics}")
endif()
execute_process(COMMAND "${AST_DISTANCE}" --translit-distance "${TEST_DIR}/source.kt" kotlin "${TEST_DIR}/emitted.cpp" cpp
    WORKING_DIRECTORY "${TEST_DIR}" OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
file(WRITE "${TEST_DIR}/faithful.report.txt" "${output}\n${error}")
if(NOT result EQUAL 0 OR NOT output MATCHES "score: 1.000000" OR NOT output MATCHES "normalized_logic: 1.000000" OR NOT output MATCHES "documentation_correspondence: 1.000000" OR NOT output MATCHES "Generated parse errors: no")
    message(FATAL_ERROR "Faithful pipeline failed: ${result}: ${output}: ${error}")
endif()
file(READ "${TEST_DIR}/emitted.cpp" emitted)
if(NOT emitted MATCHES "@param input_value" OR NOT emitted MATCHES "ref next_value")
    message(FATAL_ERROR "KDoc identifier/reference lowering missing: ${emitted}")
endif()
string(REPLACE "50" "500" drift "${emitted}")
file(WRITE "${TEST_DIR}/drift.cpp" "${drift}")
execute_process(COMMAND "${AST_DISTANCE}" --translit-distance "${TEST_DIR}/source.kt" kotlin "${TEST_DIR}/drift.cpp" cpp
    WORKING_DIRECTORY "${TEST_DIR}" OUTPUT_VARIABLE drift_report ERROR_VARIABLE error RESULT_VARIABLE result)
if(NOT result EQUAL 0 OR drift_report MATCHES "score: 1.000000" OR drift_report MATCHES "normalized_logic: 1.000000")
    message(FATAL_ERROR "Literal drift invisible: ${drift_report}: ${error}")
endif()
execute_process(COMMAND "${AST_DISTANCE}" --transliterate "${TEST_DIR}/emitted.cpp" cpp kotlin
    WORKING_DIRECTORY "${TEST_DIR}" OUTPUT_VARIABLE output ERROR_VARIABLE error RESULT_VARIABLE result)
if(result EQUAL 0 OR NOT error MATCHES "unsupported language pair")
    message(FATAL_ERROR "Unsupported rule pack did not fail explicitly: ${result}: ${error}")
endif()
execute_process(COMMAND "${AST_DISTANCE}" --transliterate "${TEST_DIR}/missing.kt" kotlin cpp
    WORKING_DIRECTORY "${TEST_DIR}" ERROR_VARIABLE error RESULT_VARIABLE result)
if(result EQUAL 0 OR NOT error MATCHES "Cannot read transliteration input")
    message(FATAL_ERROR "Missing source did not fail explicitly: ${result}: ${error}")
endif()
message(STATUS "Captured emitted target buffer and full metric report passed")
