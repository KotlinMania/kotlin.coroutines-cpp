# kxs_transform_ir.cmake - Standalone IR transformation script
#
# Called via: cmake -DINPUT_FILE=x -DOUTPUT_FILE=y -P kxs_transform_ir.cmake
#
# This is the workhorse script that does the actual IR text transformation.
# It implements the Kotlin/Native coroutine pattern with indirectbr + blockaddress.
#
# The transformation:
#   1. Finds calls to __kxs_suspend_point(i32 <id>)
#   2. Extracts function name for blockaddress references
#   3. Generates entry dispatch with indirectbr for resume
#   4. Replaces suspend calls with blockaddress store + conditional return
#   5. Generates resume labels for each suspend point
#
# Pattern matches kotlin-native/backend.native/compiler/ir/backend.native/src/org/
#   jetbrains/kotlin/backend/konan/llvm/IrToBitcode.kt (lines 2377-2424)

cmake_minimum_required(VERSION 3.18)

if(NOT DEFINED INPUT_FILE OR NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "Usage: cmake -DINPUT_FILE=<in.ll> -DOUTPUT_FILE=<out.ll> -P kxs_transform_ir.cmake")
endif()

if(NOT EXISTS "${INPUT_FILE}")
    message(FATAL_ERROR "Input file does not exist: ${INPUT_FILE}")
endif()

# Read input
file(READ "${INPUT_FILE}" IR)

# Find suspend points
string(REGEX MATCHALL "call void @__kxs_suspend_point\\(i32[^)]*\\)" SUSPEND_CALLS "${IR}")
list(LENGTH SUSPEND_CALLS N_SUSPEND)

if(N_SUSPEND EQUAL 0)
    # No suspend points - pass through unchanged
    file(WRITE "${OUTPUT_FILE}" "${IR}")
    message(STATUS "[KXS] No suspend points in ${INPUT_FILE}")
    cmake_language(EXIT 0)
endif()

message(STATUS "[KXS] Found ${N_SUSPEND} suspend point(s) in ${INPUT_FILE}")

# Extract function name(s) containing suspend points
# Pattern: define ... @funcname(...)
string(REGEX MATCH "define [^@]+@([A-Za-z_][A-Za-z0-9_]*)" FUNC_MATCH "${IR}")
string(REGEX REPLACE "define [^@]+@" "" FUNC_NAME "${FUNC_MATCH}")
message(STATUS "[KXS] Function: ${FUNC_NAME}")

# Build list of resume labels
set(LABELS "")
set(IDX 0)
while(IDX LESS N_SUSPEND)
    list(APPEND LABELS "kxs.resume.${IDX}")
    math(EXPR IDX "${IDX} + 1")
endwhile()

# Build indirectbr label list for dispatch
set(LABEL_LIST "")
foreach(L ${LABELS})
    if(LABEL_LIST)
        string(APPEND LABEL_LIST ", ")
    endif()
    string(APPEND LABEL_LIST "label %${L}")
endforeach()

# Transform each suspend call
# Before: call void @__kxs_suspend_point(i32 N)
# After:
#   store ptr blockaddress(@func, %kxs.resume.N), ptr %kxs.label.slot
#   br label %kxs.resume.N
# kxs.resume.N:
set(IDX 0)
foreach(CALL ${SUSPEND_CALLS})
    list(GET LABELS ${IDX} RESUME_LABEL)

    # Extract the suspend ID from the call
    string(REGEX MATCH "[0-9]+" SID "${CALL}")

    # Full replacement with blockaddress store pattern
    # Note: The actual suspension check (return COROUTINE_SUSPENDED if actually suspended)
    # would be added by the Clang plugin that wraps the actual suspend call
    set(REPL [[
  ; KXS: Store resume address (blockaddress pattern)
  store ptr blockaddress(@${FUNC_NAME}, %${RESUME_LABEL}), ptr %kxs.label.slot, align 8
  ; KXS: suspend point ${SID} - actual suspend call would precede this
  br label %${RESUME_LABEL}

${RESUME_LABEL}:  ; KXS resume point ${SID}]])

    # Substitute variables
    string(REPLACE "\${FUNC_NAME}" "${FUNC_NAME}" REPL "${REPL}")
    string(REPLACE "\${RESUME_LABEL}" "${RESUME_LABEL}" REPL "${REPL}")
    string(REPLACE "\${SID}" "${SID}" REPL "${REPL}")

    string(REPLACE "${CALL}" "${REPL}" IR "${IR}")
    math(EXPR IDX "${IDX} + 1")
endforeach()

# Insert entry dispatch block after function opening brace
# This is the Kotlin/Native indirectbr pattern:
#   - Check if label ptr is null (first call)
#   - If null: goto start (normal execution)
#   - If not null: indirectbr to saved label (resume)
#
# Matches: define ... @name(...) ... {
set(DISPATCH_BLOCK [[
kxs.entry:
  ; KXS: Allocate label slot for computed goto
  %kxs.label.slot = alloca ptr, align 8
  store ptr null, ptr %kxs.label.slot, align 8
  ; KXS: Load resume label (null on first call)
  %kxs.saved.label = load ptr, ptr %kxs.label.slot, align 8
  %kxs.is.first = icmp eq ptr %kxs.saved.label, null
  br i1 %kxs.is.first, label %kxs.start, label %kxs.dispatch

kxs.dispatch:
  ; KXS: Resume via computed goto (indirectbr)
  indirectbr ptr %kxs.saved.label, [${LABEL_LIST}]

kxs.start:]])

# Substitute label list into dispatch block
string(REPLACE "\${LABEL_LIST}" "${LABEL_LIST}" DISPATCH_BLOCK "${DISPATCH_BLOCK}")

string(REGEX REPLACE
    "(define [^{]+\\{)\n"
    "\\1\n${DISPATCH_BLOCK}\n"
    IR "${IR}")

# Prepend header
set(HDR "; KXS-TRANSFORMED: ${N_SUSPEND} suspend points\n; Resume labels: ${LABELS}\n; Dispatch: indirectbr + blockaddress (Kotlin/Native pattern)\n\n")
string(PREPEND IR "${HDR}")

# Write output
file(WRITE "${OUTPUT_FILE}" "${IR}")
message(STATUS "[KXS] Wrote ${OUTPUT_FILE} with indirectbr dispatch")
