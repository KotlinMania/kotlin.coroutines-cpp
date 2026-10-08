# Standalone LLVM coroutine injection; no text-rewrite implementation.
# cmake -DINPUT_FILE=in.ll -DOUTPUT_FILE=out.ll -P kxs_transform_ir.cmake
cmake_minimum_required(VERSION 3.18)

if(NOT DEFINED INPUT_FILE OR NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "Usage: cmake -DINPUT_FILE=<in.ll> -DOUTPUT_FILE=<out.ll> -P kxs_transform_ir.cmake")
endif()
if(NOT EXISTS "${INPUT_FILE}")
    message(FATAL_ERROR "Input file does not exist: ${INPUT_FILE}")
endif()
if(NOT KXS_INJECT_EXECUTABLE)
    find_program(KXS_INJECT_EXECUTABLE NAMES kxs-inject REQUIRED)
endif()
execute_process(
    COMMAND "${KXS_INJECT_EXECUTABLE}" "${INPUT_FILE}" -o "${OUTPUT_FILE}"
    RESULT_VARIABLE _KXS_RESULT
)
if(NOT _KXS_RESULT EQUAL 0)
    message(FATAL_ERROR "KXS IR injection failed for ${INPUT_FILE}")
endif()
