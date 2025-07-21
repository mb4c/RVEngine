# Prevent duplicate definition
if(TARGET glad::glad)
    return()
endif()

# Locate glad.h
find_path(GLAD_INCLUDE_DIR
        NAMES glad/gl.h
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include
        NO_DEFAULT_PATH
)

if(NOT GLAD_INCLUDE_DIR)
    message(FATAL_ERROR "GLAD header not found")
endif()

# Add glad.c as a compiled library
add_library(glad STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/src/gl.c
)

target_include_directories(glad PUBLIC
        ${GLAD_INCLUDE_DIR}
)

# Link OpenGL (required)
find_package(OpenGL REQUIRED)
target_link_libraries(glad PUBLIC OpenGL::GL)

add_library(glad::glad ALIAS glad)

message(STATUS "GLAD compiled and configured: ${GLAD_INCLUDE_DIR}")
