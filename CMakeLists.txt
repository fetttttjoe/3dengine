# Use a standard, widely available CMake version
cmake_minimum_required(VERSION 3.16)
project(IntuitiveModeler VERSION 1.0)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE INTERNAL "")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- Dependencies ---
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(glad GIT_REPOSITORY https://github.com/Dav1dde/glad.git GIT_TAG v0.1.36)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.92.0)
FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3)
FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.14.0)

FetchContent_MakeAvailable(glfw glad glm imgui nlohmann_json googletest)

find_package(OpenGL REQUIRED)

if(MSVC)
  add_compile_options(/wd5287)
endif()

# --- Core Engine Library ---
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/main.cpp$")

add_library(IntuitiveModeler STATIC ${ENGINE_SOURCES})

target_sources(IntuitiveModeler PRIVATE
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(IntuitiveModeler PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${imgui_SOURCE_DIR}/misc/cpp
  ${nlohmann_json_SOURCE_DIR}/include
)

target_compile_definitions(IntuitiveModeler PUBLIC GLM_ENABLE_EXPERIMENTAL)

target_link_libraries(IntuitiveModeler PUBLIC
  glfw glad glm OpenGL::GL
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:stdc++fs>
)

# --- Main Application Executable ---
add_executable(IntuitiveModeler_App src/main.cpp)
target_link_libraries(IntuitiveModeler_App PRIVATE IntuitiveModeler)

set_target_properties(IntuitiveModeler_App PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
if(WIN32)
  set_target_properties(IntuitiveModeler_App PROPERTIES WIN32_EXECUTABLE $<$<CONFIG:Release>:TRUE>)
endif()

add_custom_command(TARGET IntuitiveModeler_App POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
  "$<TARGET_FILE_DIR:IntuitiveModeler_App>/shaders"
)

# --- Testing Setup ---
enable_testing()
include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")
add_executable(runTests ${TEST_SOURCES})

target_link_libraries(runTests PRIVATE IntuitiveModeler gtest_main)

gtest_discover_tests(runTests)