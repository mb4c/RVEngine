cmake_minimum_required(VERSION 3.26)
project(RVEngine)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    add_compile_definitions(JPH_PROFILE_ENABLED)
    add_compile_definitions(JPH_DEBUG_RENDERER)
endif ()

#find_package(glfw3 CONFIG REQUIRED) #
#find_package(glad CONFIG REQUIRED) #
#find_package(imgui CONFIG REQUIRED) #
#find_package(glm CONFIG REQUIRED) #
#find_package(fmt CONFIG REQUIRED)
#find_package(assimp CONFIG REQUIRED)#
#find_package(Tracy CONFIG REQUIRED) #
#find_package(EnTT CONFIG REQUIRED) #
#find_package(imguizmo CONFIG REQUIRED) #
#find_package(yaml-cpp CONFIG REQUIRED)
#find_package(unofficial-joltphysics CONFIG REQUIRED)
#find_package(OpenImageIO REQUIRED)
#TODO: Add option to compile without tracy

file(GLOB_RECURSE engine_src src/*.cpp include/*.hpp)
add_library(RVEngine STATIC ${engine_src})

target_include_directories(RVEngine PUBLIC include)
target_link_libraries(RVEngine PUBLIC glad::glad glfw imgui::imgui glm::glm assimp::assimp Tracy::TracyClient EnTT::EnTT imguizmo::imguizmo yaml-cpp Jolt::Jolt OpenImageIO::OpenImageIO glad::glad yaml-cpp::yaml-cpp)

# if we dont do this microshits garbage linker will have aneurysm and fail
set_target_properties(RVEngine PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)