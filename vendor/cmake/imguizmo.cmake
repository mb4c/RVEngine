if(TARGET imguizmo::imguizmo)
    return()
endif()

set(IMGUIZMO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imguizmo")

if(NOT EXISTS "${IMGUIZMO_DIR}")
    message(FATAL_ERROR "ImGuizmo directory not found: ${IMGUIZMO_DIR}")
endif()

set(IMGUIZMO_SOURCES
        ${IMGUIZMO_DIR}/ImGuizmo.cpp
        ${IMGUIZMO_DIR}/ImGuizmo.h
)

set(IMGUIZMO_OPTIONAL_SOURCES
        ${IMGUIZMO_DIR}/ImSequencer.cpp
        ${IMGUIZMO_DIR}/ImSequencer.h
        ${IMGUIZMO_DIR}/ImCurveEdit.cpp
        ${IMGUIZMO_DIR}/ImCurveEdit.h
        ${IMGUIZMO_DIR}/ImGradient.cpp
        ${IMGUIZMO_DIR}/ImGradient.h
        ${IMGUIZMO_DIR}/GraphEditor.cpp
        ${IMGUIZMO_DIR}/GraphEditor.h
        ${IMGUIZMO_DIR}/ImZoomSlider.h
)

add_library(imguizmo_lib STATIC
        ${IMGUIZMO_SOURCES}
        ${IMGUIZMO_OPTIONAL_SOURCES}
)

add_library(imguizmo::imguizmo ALIAS imguizmo_lib)

target_include_directories(imguizmo_lib PUBLIC
        ${IMGUIZMO_DIR}
)

if(NOT TARGET imgui::imgui)
    message(FATAL_ERROR "ImGuizmo requires ImGui. Please include imgui.cmake before imguizmo.cmake")
endif()

target_link_libraries(imguizmo_lib PUBLIC
        imgui::imgui
)

target_compile_features(imguizmo_lib PUBLIC cxx_std_11)

if(WIN32)
    target_compile_definitions(imguizmo_lib PRIVATE
            _CRT_SECURE_NO_WARNINGS
    )
endif()

set_target_properties(imguizmo_lib PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
)

message(STATUS "ImGuizmo configured from: ${IMGUIZMO_DIR}")