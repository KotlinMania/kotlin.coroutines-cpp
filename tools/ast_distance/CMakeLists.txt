cmake_minimum_required(VERSION 3.20)
project(ASTDistance VERSION 0.2.0 LANGUAGES C CXX)

set(CANONICAL_BIN "/Volumes/stuff/Projects/kotlinmania/bin/ast_distance")
set(LOCAL_BIN "${CMAKE_CURRENT_SOURCE_DIR}/ast_distance")

if(EXISTS "${CANONICAL_BIN}" AND NOT EXISTS "${LOCAL_BIN}")
    file(COPY "${CANONICAL_BIN}" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}" FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

if(EXISTS "${LOCAL_BIN}" OR EXISTS "${CANONICAL_BIN}")
    if(EXISTS "${LOCAL_BIN}")
        set(ACTIVE_BIN "${LOCAL_BIN}")
    else()
        set(ACTIVE_BIN "${CANONICAL_BIN}")
    endif()

    add_executable(ast_distance IMPORTED GLOBAL)
    set_target_properties(ast_distance PROPERTIES
        IMPORTED_LOCATION "${ACTIVE_BIN}"
    )

    if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        add_custom_target(refresh_ast_distance ALL
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ACTIVE_BIN}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ast_distance"
            COMMENT "Providing ast_distance binary in output directory"
        )
    endif()

    install(PROGRAMS "${ACTIVE_BIN}" DESTINATION bin RENAME ast_distance)
endif()
