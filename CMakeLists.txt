cmake_minimum_required(VERSION 3.26)
project(RealVirtuality)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vendor/cmake")
include(glad)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)
set(IMGUI_SOURCES
        # Core ImGui files
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp

        # SDL3 backends
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp

        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp

)

set(IMGUI_HEADERS
        ${IMGUI_DIR}/imgui.h
        ${IMGUI_DIR}/imgui_internal.h
        ${IMGUI_DIR}/imconfig.h
        ${IMGUI_DIR}/backends/imgui_impl_glfw.h
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.h

        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.h

)

add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})

target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${IMGUI_DIR}/misc/cpp

)
#target_link_libraries(imgui
#        PUBLIC
#        SDL3::SDL3
#)


add_library(imgui::imgui ALIAS imgui)

add_subdirectory(vendor/glm)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)
set(ASSIMP_INSTALL OFF CACHE BOOL "")
set(ASSIMP_NO_EXPORT ON CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
add_subdirectory(vendor/assimp)
add_subdirectory(vendor/tracy)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/yaml-cpp)
add_subdirectory(vendor/jolt/Build)

#set(OpenImageIO_BUILD_MISSING_DEPS all CACHE STRING "" FORCE)
#set(OIIO_BUILD_UHDR OFF CACHE BOOL "" FORCE)
#set(USE_UHDR OFF CACHE BOOL "" FORCE)
#add_subdirectory(vendor/oiio)
find_package(OpenImageIO)
include(imguizmo)

add_subdirectory(RVEngine)
add_subdirectory(RVEditor)


add_subdirectory(citadel)