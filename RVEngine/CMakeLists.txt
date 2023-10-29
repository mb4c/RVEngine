cmake_minimum_required(VERSION 3.26)
project(RVEngine)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

file(GLOB_RECURSE engine_src src/*.cpp include/*.hpp)
add_library(RVEngine SHARED ${engine_src})

target_include_directories(RVEngine PUBLIC include)
target_link_libraries(RVEngine PRIVATE glfw glad::glad imgui::imgui glm::glm fmt::fmt assimp::assimp Tracy::TracyClient EnTT::EnTT imguizmo::imguizmo yaml-cpp)


