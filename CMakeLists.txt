# Use a standard, widely available CMake version
cmake_minimum_required(VERSION 3.16)
project(IntuitiveModeler VERSION 1.0)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE INTERNAL "")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(GoogleTest)

# --- Dependencies ---
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(glad GIT_REPOSITORY https://github.com/Dav1dde/glad.git GIT_TAG v0.1.36)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.90.4)
FetchContent_Declare(nlohmann_json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3)
FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.14.0)
FetchContent_Declare(implot GIT_REPOSITORY https://github.com/epezent/implot.git GIT_TAG v0.16)
FetchContent_Declare(nfd GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git GIT_TAG v1.2.1)
FetchContent_Declare(tinyobjloader GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git GIT_TAG v2.0.0rc10)

FetchContent_MakeAvailable(glfw glad glm imgui nlohmann_json googletest implot nfd tinyobjloader)

find_package(OpenGL REQUIRED)

if(MSVC)
  add_compile_options(/wd5287)
endif()

# --- Core Engine Library ---
file(GLOB_RECURSE ENGINE_SOURCES 
  CONFIGURE_DEPENDS 
  "src/*.cpp"
  "src/Core/*.cpp"
  "src/Core/UI/*.cpp"
  "src/Factories/*.cpp"
  "src/Renderer/*.cpp"
  "src/Scene/*.cpp"
  "src/Scene/Objects/*.cpp"
  "src/Sculpting/*.cpp"
  "src/Sculpting/Tools/*.cpp"
)
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/main.cpp$")

add_library(IntuitiveModeler STATIC 

  ${ENGINE_SOURCES})

target_sources(IntuitiveModeler PRIVATE
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
  ${implot_SOURCE_DIR}/implot.cpp
  ${implot_SOURCE_DIR}/implot_items.cpp
)

target_include_directories(IntuitiveModeler PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${imgui_SOURCE_DIR}/misc/cpp
  ${nlohmann_json_SOURCE_DIR}/include
  ${implot_SOURCE_DIR}
  ${tinyobjloader_SOURCE_DIR}
  ${nfd_SOURCE_DIR}/src/include
  # Add this if src/ files need it you would add this line:
  # ${CMAKE_CURRENT_SOURCE_DIR}/tests 
)

target_compile_definitions(IntuitiveModeler PUBLIC GLM_ENABLE_EXPERIMENTAL)

target_link_libraries(IntuitiveModeler PUBLIC
  glfw glad glm OpenGL::GL nfd tinyobjloader
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:stdc++fs>
)

# --- Main Application Executable ---
add_executable(IntuitiveModeler_App src/main.cpp)
target_link_libraries(IntuitiveModeler_App PRIVATE IntuitiveModeler)
# Set the output directory for the final executable
set_target_properties(IntuitiveModeler_App PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

# Copy the shaders folder after the main app is built
add_custom_command(TARGET IntuitiveModeler_App POST_BUILD
 
 COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
  "$<TARGET_FILE_DIR:IntuitiveModeler_App>/shaders"
)

# --- Testing Setup ---
enable_testing()
# Explicitly list test source files for clarity and reliability
add_executable(runTests
    tests/main.cpp
    tests/SculptingTests.cpp
    tests/RaycasterTests.cpp
    tests/CameraTests.cpp
    tests/SceneLoadingTests.cpp
    tests/SelectionTests.cpp
    tests/SceneTests.cpp
    tests/SettingsManagerTests.cpp
    tests/TransformGizmoTests.cpp
    tests/ResourceManagerTests.cpp
    tests/TestMock.cpp
    tests/RendererTests.cpp
    tests/ApplicationTests.cpp
    tests/UITests.cpp
)
target_link_libraries(runTests PRIVATE IntuitiveModeler gtest)
target_compile_definitions(runTests PRIVATE INTUITIVE_MODELER_TESTING)

target_include_directories(runTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/tests # This makes tests/*.h findable
)

# Set the output directory for the test executable
set_target_properties(runTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY
"${CMAKE_BINARY_DIR}/$<CONFIG>")

# Use PRE_TEST mode to delay test discovery until ctest execution
# This avoids running the executable during CMake configuration on any platform.
gtest_discover_tests(runTests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
    DISCOVERY_MODE PRE_TEST
)
# Copy the resources after the test executable is built
add_custom_command(TARGET runTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_scene.json"
  "$<TARGET_FILE_DIR:runTests>/test_scene.json"
)

# Copy the resources after the test executable is built
add_custom_command(TARGET runTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_cube.obj"
  "$<TARGET_FILE_DIR:runTests>/test_cube.obj"
)

add_custom_command(TARGET runTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_scene_empty.json"
  "$<TARGET_FILE_DIR:runTests>/test_scene_empty.json"
)

add_custom_command(TARGET runTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
  "$<TARGET_FILE_DIR:runTests>/shaders"
)