# Standalone marker cleanup; this does not synthesize coroutine lowering.
# cmake -DINPUT_FILE=in.ll -DOUTPUT_FILE=out.ll -P kxs_transform_ir.cmake
cmake_minimum_required(VERSION 3.18)

if(NOT DEFINED INPUT_FILE OR NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "Usage: cmake -DINPUT_FILE=<in.ll> -DOUTPUT_FILE=<out.ll> -P kxs_transform_ir.cmake")
endif()
if(NOT EXISTS "${INPUT_FILE}")
    message(FATAL_ERROR "Input file does not exist: ${INPUT_FILE}")
endif()
find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/kxs_transform_ir.py"
        "${INPUT_FILE}" "${OUTPUT_FILE}"
    RESULT_VARIABLE _KXS_RESULT
)
if(NOT _KXS_RESULT EQUAL 0)
    message(FATAL_ERROR "KXS marker cleanup failed for ${INPUT_FILE}")
endif()
