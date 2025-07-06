# ==============================================================================
# Project Setup
# ==============================================================================
cmake_minimum_required(VERSION 3.11)
project(IntuitiveModeler VERSION 2.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# ==============================================================================
# Dependencies (via FetchContent)
# ==============================================================================
include(FetchContent)

# --- GLFW (Windowing and Input) ---
FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# --- GLAD (OpenGL Loader-Generator) ---
FetchContent_Declare(glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.36
)
FetchContent_MakeAvailable(glad)

# --- GLM (Mathematics Library) ---
FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# --- ImGui (UI Library) ---
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90.8
)
FetchContent_MakeAvailable(imgui)

# --- GoogleTest (Testing Framework) ---
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)


# ==============================================================================
# Platform-Specific Setup
# ==============================================================================
find_package(OpenGL REQUIRED)

if(MSVC)
  add_compile_options(/wd5287)
endif()

# ==============================================================================
# Core Library (ModelerCore)
# ==============================================================================
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/main.cpp$")

# Add the necessary ImGui implementation files to our library sources.
list(APPEND CORE_SOURCES
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  # This line compiles the std::string helpers for ImGui, which is essential.
  ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

add_library(ModelerCore STATIC ${CORE_SOURCES})

target_link_libraries(ModelerCore PUBLIC
  glfw
  glad
  glm
  OpenGL::GL
)

# This block adds the required include paths so the compiler can find the headers.
target_include_directories(ModelerCore PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  # Core ImGui headers
  ${imgui_SOURCE_DIR}
  # Backend headers (glfw, opengl3)
  ${imgui_SOURCE_DIR}/backends
  # Stdlib headers (for std::string support) - This is the crucial missing path.
  ${imgui_SOURCE_DIR}/misc/cpp
)

target_compile_definitions(ModelerCore PUBLIC GLM_ENABLE_EXPERIMENTAL)


# ==============================================================================
# Main Application Executable (IntuitiveModeler)
# ==============================================================================
add_executable(IntuitiveModeler src/main.cpp)

target_link_libraries(IntuitiveModeler PRIVATE
  ModelerCore
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:stdc++fs>
)

set_target_properties(IntuitiveModeler PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

if(WIN32)
  set_target_properties(IntuitiveModeler PROPERTIES
    WIN32_EXECUTABLE $<$<CONFIG:Release>:TRUE>
  )
endif()

add_custom_command(TARGET IntuitiveModeler POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:IntuitiveModeler>/shaders"
  COMMENT "Copying shaders directory to build output..."
)


# ==============================================================================
# Test Executable (ModelerTests)
# ==============================================================================
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")
add_executable(ModelerTests ${TEST_SOURCES})

target_link_libraries(ModelerTests PRIVATE
  ModelerCore
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(ModelerTests)