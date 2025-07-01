# CMake version and project name
cmake_minimum_required(VERSION 3.15)
project(IntuitiveModeler VERSION 1.0)

# Set the C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependency Management with FetchContent ---
include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.3.8)
FetchContent_MakeAvailable(glfw)

set(GLAD_API "gl=4.1" CACHE STRING "OpenGL API version")
set(GLAD_PROFILE "core" CACHE STRING "OpenGL profile")
set(GLAD_GENERATOR "c" CACHE STRING "Generator language")
FetchContent_Declare(glad GIT_REPOSITORY https://github.com/Dav1dde/glad.git GIT_TAG v0.1.36)
FetchContent_MakeAvailable(glad)

FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.90.8)
FetchContent_MakeAvailable(imgui)

# Find the OpenGL library on the system (for Linux linking)
find_package(OpenGL REQUIRED)

# --- Application Executable ---
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

list(APPEND APP_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
add_executable(IntuitiveModeler ${APP_SOURCES})

# --- Include Directories ---
target_include_directories(IntuitiveModeler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# --- Compile Definitions ---
target_compile_definitions(IntuitiveModeler PRIVATE GLM_ENABLE_EXPERIMENTAL)

# --- Linking ---
target_link_libraries(IntuitiveModeler PRIVATE glfw glad glm OpenGL::GL stdc++fs)
# --- Output and Platform Settings ---
set_target_properties(IntuitiveModeler PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
if(WIN32)
    set_target_properties(IntuitiveModeler PROPERTIES WIN32_EXECUTABLE $<$<CONFIG:Release>:TRUE>)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
endif()