cmake_minimum_required(VERSION 3.21)
project(hermes)

set(CMAKE_CXX_STANDARD 20)
set(IMGUI_DIR /home/ben/Documents/repositories/imgui-docking/)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    include_directories(/usr/include/SDL2
            ${IMGUI_DIR}
            ${IMGUI_DIR}/backends)
endif()
include_directories(../common/include)
add_executable(hermes
        main.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp)
target_link_libraries(hermes
        OpenGL
        SDL2)