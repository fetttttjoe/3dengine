#include "gtest/gtest.h"
#include "Core/ResourceManager.h"
#include "Shader.h"
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <cstdio>

// A simple test shader source for in-memory compilation tests
const char* TEST_VERT_SHADER_SRC = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
void main() { gl_Position = vec4(aPos, 1.0); }
)glsl";

const char* TEST_FRAG_SHADER_SRC = R"glsl(
#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(1.0, 0.0, 0.0, 1.0); }
)glsl";

class ResourceManagerTest : public ::testing::Test {
protected:
    // This SetUp method ensures that the ResourceManager's static data
    // is cleared before each test, providing true test isolation.
    void SetUp() override {
        ResourceManager::Shutdown(); // Clear any state from previous tests
        ResourceManager::Initialize();
    }
};

// --- Shader Loading Tests ---

TEST_F(ResourceManagerTest, LoadShader_FromFile_Success) {
    // Action: Load a shader from a file.
    auto shader = ResourceManager::LoadShader("default", "shaders/default.vert", "shaders/default.frag");
    
    // Assert: The shader was loaded successfully.
    ASSERT_NE(shader, nullptr);
    
    // Assert: The shader is cached correctly under its logical name.
    auto cachedShader = ResourceManager::GetShader("default");
    ASSERT_EQ(shader, cachedShader);
}

TEST_F(ResourceManagerTest, LoadShader_FromFile_NonExistent) {
    // Action & Assert: Loading a non-existent file should throw a specific exception.
    // This is a more robust way to test for expected exceptions.
    try {
        ResourceManager::LoadShader("non_existent", "shaders/no.vert", "shaders/no.frag");
        FAIL() << "Expected std::runtime_error";
    }
    catch (std::runtime_error const & err) {
        EXPECT_EQ(err.what(), std::string("Could not open shader file: shaders/no.vert"));
    }
    catch (...) {
        FAIL() << "Expected std::runtime_error";
    }

    // Assert: The failed shader is not cached.
    auto cachedShader = ResourceManager::GetShader("non_existent");
    EXPECT_EQ(cachedShader, nullptr);
}

TEST_F(ResourceManagerTest, LoadShader_FromMemory_Success) {
    auto shader = ResourceManager::LoadShaderFromMemory("memory_shader", TEST_VERT_SHADER_SRC, TEST_FRAG_SHADER_SRC);
    ASSERT_NE(shader, nullptr);

    auto cachedShader = ResourceManager::GetShader("memory_shader");
    ASSERT_EQ(shader, cachedShader);
}

TEST_F(ResourceManagerTest, GetShader_Existing) {
    ResourceManager::LoadShader("existing_shader", "shaders/default.vert", "shaders/default.frag");
    auto shader = ResourceManager::GetShader("existing_shader");
    ASSERT_NE(shader, nullptr);
}

TEST_F(ResourceManagerTest, GetShader_NonExisting) {
    auto shader = ResourceManager::GetShader("definitely_not_existing");
    EXPECT_EQ(shader, nullptr);
}

// --- Mesh Loading Tests ---

TEST_F(ResourceManagerTest, LoadMesh_FromObj_Success) {
    // Setup: Create a standard cube OBJ file with normals for the test to load.
    std::ofstream ofs("test_cube.obj");
    ofs << "v -0.5 -0.5 0.5\nv 0.5 -0.5 0.5\nv -0.5 0.5 0.5\nv 0.5 0.5 0.5\n";
    ofs << "v -0.5 0.5 -0.5\nv 0.5 0.5 -0.5\nv -0.5 -0.5 -0.5\nv 0.5 -0.5 -0.5\n";
    ofs << "vn 0 0 1\nvn 0 0 -1\nvn 0 1 0\nvn 0 -1 0\nvn 1 0 0\nvn -1 0 0\n";
    ofs << "f 1//1 2//1 3//1\nf 3//1 2//1 4//1\n"; // Front face
    ofs << "f 5//2 6//2 7//2\nf 7//2 6//2 8//2\n"; // Back face
    ofs << "f 3//3 4//3 5//3\nf 5//3 4//3 6//3\n"; // Top face
    ofs << "f 7//4 8//4 1//4\nf 1//4 8//4 2//4\n"; // Bottom face
    ofs << "f 2//5 8//5 4//5\nf 4//5 8//5 6//5\n"; // Right face
    ofs << "f 7//6 1//6 5//6\nf 5//6 1//6 3//6\n"; // Left face
    ofs.close();

    // Action: Load the mesh.
    auto result = ResourceManager::LoadMesh("test_cube.obj");
    const auto& vertices = result.first;
    const auto& indices = result.second;

    // Assert: Check for the CORRECT number of vertices and indices for a lit cube.
    // A cube has 6 faces * 4 vertices/face = 24 unique vertices (pos+normal).
    // 6 faces * 2 triangles/face * 3 indices/triangle = 36 indices.
    ASSERT_EQ(vertices.size(), 24 * 3);
    ASSERT_EQ(indices.size(), 36);

    // Clean up the temporary file.
    std::remove("test_cube.obj");
}

TEST_F(ResourceManagerTest, LoadMesh_NonExistent) {
    auto result = ResourceManager::LoadMesh("non_existent_mesh.obj");
    ASSERT_TRUE(result.first.empty());
    ASSERT_TRUE(result.second.empty());
}

TEST_F(ResourceManagerTest, LoadMesh_CorruptedFile) {
    std::ofstream ofs("corrupted_mesh.obj");
    ofs << "this is not valid obj data";
    ofs.close();

    auto result = ResourceManager::LoadMesh("corrupted_mesh.obj");
    ASSERT_TRUE(result.first.empty());
    ASSERT_TRUE(result.second.empty());

    std::remove("corrupted_mesh.obj");
}
