cmake_minimum_required(VERSION 3.26)
project(RVEditor)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Tracy CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)

file(GLOB_RECURSE editor_src src/*.cpp src/*.hpp)
add_executable(RVEditor ${editor_src})

target_link_libraries(RVEditor RVEngine glfw glad::glad imgui::imgui glm::glm fmt::fmt Tracy::TracyClient EnTT::EnTT)

get_target_property(ENGINE_SOURCE_DIR RVEngine SOURCE_DIR)

set(SOURCE_PATH "${ENGINE_SOURCE_DIR}/res/")
set(DESTINATION_PATH "${CMAKE_CURRENT_BINARY_DIR}/res/")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SOURCE_PATH} ${DESTINATION_PATH})