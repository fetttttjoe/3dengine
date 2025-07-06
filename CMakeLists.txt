cmake_minimum_required(VERSION 4.1)
project(IntuitiveModeler VERSION 1.0)

# allow GLAD’s old cmake_minimum_required(VERSION 3.0) to pass under CMake 4.x
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE INTERNAL "")

# — C++ standard ————————————————————————————————————————————————————————
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# — FetchContent support —————————————————————————————————————————————————————
include(FetchContent)

# — GLFW —————————————————————————————————————————————————————————————————
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# — GLAD v0.1.36 —————————————————————————————————————————————————————————————
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        v0.1.36
)
FetchContent_MakeAvailable(glad)

# — GLM ——————————————————————————————————————————————————————————————————
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# — ImGui —————————————————————————————————————————————————————————————————
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90.8
)
FetchContent_MakeAvailable(imgui)

# — OpenGL —————————————————————————————————————————————————————————————————
find_package(OpenGL REQUIRED)

# — Silence MSVC warning C5287 (enum-mismatch in Nuklear) —————————————————————————
if(MSVC)
  add_compile_options(/wd5287)
endif()

# — Application sources ————————————————————————————————————————————————————
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(APPEND APP_SOURCES
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

add_executable(IntuitiveModeler ${APP_SOURCES})

# — Include paths & defs —————————————————————————————————————————————————————
target_include_directories(IntuitiveModeler PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  # Core ImGui headers
  ${imgui_SOURCE_DIR}
  # Backend headers (glfw, opengl3)
  ${imgui_SOURCE_DIR}/backends
  # Stdlib headers (for std::string support)
  ${imgui_SOURCE_DIR}/misc/cpp
)
target_compile_definitions(IntuitiveModeler PRIVATE GLM_ENABLE_EXPERIMENTAL)

# — Link libraries ———————————————————————————————————————————————————————————
target_link_libraries(IntuitiveModeler PRIVATE
  glfw
  glad
  glm
  OpenGL::GL
  # only non-MSVC needs stdc++fs
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:stdc++fs>
)

# — Output directory & executable settings ——————————————————————————————————————
set_target_properties(IntuitiveModeler PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
if(WIN32)
  set_target_properties(IntuitiveModeler PROPERTIES
    WIN32_EXECUTABLE $<$<CONFIG:Release>:TRUE>
  )
endif()

# — Copy shaders folder next to the exe on every build —————————————————————————
add_custom_command(TARGET IntuitiveModeler POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:IntuitiveModeler>/shaders"
  COMMENT "Copying shaders/ → shaders/"
)