
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was KotlinxCoroutinesConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# KotlinxCoroutinesConfig.cmake
#
# Imported targets:
#   kotlinx::coroutines        - Full coroutine support with IR transformation
#   kotlinx::coroutines_headers - Headers only (for callers, not definers)
#
# Usage:
#   find_package(KotlinxCoroutines REQUIRED)
#   target_link_libraries(my_app PRIVATE kotlinx::coroutines)
#   kxs_enable_suspend(my_app)

include("${CMAKE_CURRENT_LIST_DIR}/KotlinxCoroutinesTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/KotlinxCoroutineTransform.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/kxs_transform_ir.cmake")

# Re-export the convenience functions
include("${CMAKE_CURRENT_LIST_DIR}/KotlinxCoroutines.cmake")

check_required_components(KotlinxCoroutines)
